/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "csinglewindowplatform.h"
#include "defines.h"
#include "utils.h"
#include <windows.h>
#include <windowsx.h>
#include "cascapplicationmanagerwrapper.h"

#include <QScreen>
#include <functional>

#define CAPTURED_WINDOW_CURSOR_OFFSET_X     180
#define CAPTURED_WINDOW_CURSOR_OFFSET_Y     15
#define EDITOR_WINDOW_MIN_WIDTH             700

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

CSingleWindowPlatform::CSingleWindowPlatform(const QRect& rect, const QString& title, QWidget * panel)
    : CSingleWindowBase(const_cast<QRect&>(rect))
    , m_bgColor(AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBackground))
    , m_borderColor(AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBorder))
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEXW wcx{ sizeof(WNDCLASSEX) };
    wcx.style           = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance       = hInstance;
    wcx.lpfnWndProc     = CSingleWindowPlatform::WndProc;
    wcx.cbClsExtra      = 0;
    wcx.cbWndExtra      = 0;
    wcx.lpszClassName   = L"SingleWindowClass";
    wcx.hbrBackground   = CreateSolidBrush(m_bgColor);
    wcx.hCursor         = LoadCursor(hInstance, IDC_ARROW);

    QIcon icon = Utils::appIcon();
    wcx.hIcon = qt_pixmapToWinHICON(icon.pixmap(QSize(32,32)));

    if ( FAILED(RegisterClassEx(&wcx)) )
        throw std::runtime_error( "Couldn't register window class" );

    m_hWnd = CreateWindow(WINDOW_EDITOR_CLASS_NAME, title.toStdWString().c_str(), static_cast<DWORD>(WindowBase::Style::windowed),
                                rect.x(), rect.y(), rect.width(), rect.height(), 0, 0, hInstance, nullptr);

    if ( !m_hWnd )
        throw std::runtime_error("couldn't create window because of reasons");

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    m_pWinPanel = new CWinPanel(m_hWnd);

    m_modalSlotConnection = QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog,
                                                bind(&CSingleWindowPlatform::slot_modalDialog, this, std::placeholders::_1, std::placeholders::_2));
}

CSingleWindowPlatform::~CSingleWindowPlatform()
{
    QObject::disconnect(m_modalSlotConnection);

    if ( !AscAppManager::mainWindow() || !AscAppManager::mainWindow()->isVisible() ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(m_hWnd, &wp)) {
            GET_REGISTRY_USER(reg_user)
            wp.showCmd == SW_MAXIMIZE ?
                        reg_user.setValue("maximized", true) : reg_user.remove("maximized");

            QRect windowRect;
            windowRect.setTopLeft(QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));
            windowRect.setBottomRight(QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
            windowRect.adjust(0,0,-1,-1);

            reg_user.setValue("position", windowRect);
        }
    }

    m_closed = true;
    DestroyWindow(m_hWnd);

    qDebug() << "destroy platform window";
}

HWND CSingleWindowPlatform::handle() const
{
    return m_hWnd;
}

LRESULT CALLBACK CSingleWindowPlatform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSingleWindowPlatform * window = reinterpret_cast<CSingleWindowPlatform *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if ( !window )
        return DefWindowProc(hWnd, message, wParam, lParam);

    switch ( message ) {
    case WM_DPICHANGED:
        if ( !WindowHelper::isLeftButtonPressed() ) {
            double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

            if ( dpi_ratio != window->m_dpiRatio ) {
                window->onDpiChanged(dpi_ratio, window->m_dpiRatio);
            }
        }
        break;
    case WM_KEYDOWN: {
        if ( wParam != VK_TAB )
            return DefWindowProc(hWnd, message, wParam, lParam);

        SetFocus( HWND(window->m_pWinPanel->winId()) );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND: {
        if (  GET_SC_WPARAM(wParam) == SC_KEYMENU ) {
            return 0;
        } else
        if ( GET_SC_WPARAM(wParam) == SC_SIZE ) {
            // TODO: skip window min size for usability test
//            if ( WindowHelper::isWindowSystemDocked(hWnd) )
//                window->setMinimumSize(int(EDITOR_WINDOW_MIN_WIDTH * window->m_dpiRatio), int(MAIN_WINDOW_MIN_HEIGHT * window->m_dpiRatio));
//            else window->setMinimumSize(int(MAIN_WINDOW_MIN_WIDTH * window->m_dpiRatio), int(MAIN_WINDOW_MIN_HEIGHT * window->m_dpiRatio));

            break;
        } else
        if (GET_SC_WPARAM(wParam) == SC_RESTORE) {
//            if ( !WindowHelper::isLeftButtonPressed() ) {
            // TODO: skip window min size for usability test
//                WindowHelper::correctWindowMinimumSize(hWnd);

            break;
        }

        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    case WM_ACTIVATE: {
        static bool is_mainwindow_prev;
        is_mainwindow_prev = false;
        if ( !IsWindowEnabled(hWnd) && window->m_modalHwnd && window->m_modalHwnd != hWnd )
        {
            if ( LOWORD(wParam) != WA_INACTIVE ) {
                SetActiveWindow(window->m_modalHwnd);
                SetWindowPos(hWnd, window->m_modalHwnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                return 0;
            }
        } else {
            if ( LOWORD(wParam) != WA_INACTIVE ) {
                static HWND top_window;
                top_window = NULL;

                EnumWindows([](HWND hw, LPARAM lp){
                    if (!IsWindowVisible(hw) || GetWindowTextLength(hw) == 0) {
                        return TRUE;
                    }

                    if (hw == (HWND)lp) {
                        top_window = hw;
                    } else
                    if ( top_window ) {
                        top_window = NULL;
                        if ( AscAppManager::mainWindow() && hw == AscAppManager::mainWindow()->handle() )
                            is_mainwindow_prev = true;
                    }

                    return TRUE;
                }, (LPARAM)hWnd);
            }
        }
        break;
    }

    case WM_SETFOCUS: {
        if ( !window->m_closed ) {
//        window->focusMainPanel();
            window->focus();
        }
        break;
    }

    case WM_NCCALCSIZE: {
        //this kills the window frame and title bar we added with
        //WS_THICKFRAME and WS_CAPTION
        if (window->m_borderless)
            return 0;

        break;
    }

    case WM_CLOSE: {
        qDebug() << "wm_close";
        if ( window->m_pMainPanel )
            QTimer::singleShot(0, window->m_pMainPanel, [=]{
                AscAppManager::getInstance().closeQueue().enter(sWinTag{2, size_t(window)});
            });
        else return 1;
        }
        return 0;

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    case WM_NCPAINT:
        return 0;

    case WM_NCHITTEST: {
        if ( window->m_borderless ) {
            const LONG borderWidth = 8; //in pixels
            RECT winrect;
            GetWindowRect( hWnd, &winrect );
            long x = GET_X_LPARAM( lParam );
            long y = GET_Y_LPARAM( lParam );

//            if ( window->m_borderlessResizeable )
            {
                //bottom left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMLEFT;
                }
                //bottom right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMRIGHT;
                }
                //top left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPLEFT;
                }
                //top right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPRIGHT;
                }
                //left border
                if ( x >= winrect.left && x < winrect.left + borderWidth )
                {
                    return HTLEFT;
                }
                //right border
                if ( x < winrect.right && x >= winrect.right - borderWidth )
                {
                    return HTRIGHT;
                }
                //bottom border
                if ( y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOM;
                }
                //top border
                if ( y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOP;
                }
            }

            return HTCAPTION;
        }
        break;
    }

    case WM_SIZE:
        if ( !window->m_skipSizing && !window->m_closed ) {
            window->onSizeEvent(wParam);
        }

        break;

    case WM_MOVING: {
        RECT rc = *(RECT*)lParam;
        window->onMoveEvent(QRect(rc.left,rc.top,rc.right,rc.bottom));

        return TRUE;}

    case WM_ENTERSIZEMOVE: {
        WindowHelper::correctWindowMinimumSize(hWnd);
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if ( GetWindowPlacement(hWnd, &wp) ) {
            MONITORINFO info{sizeof(MONITORINFO)};
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &info);

            window->m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                                QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
        }
        break;}

    case WM_EXITSIZEMOVE: {
        window->onExitSizeMove();
        return 0;
    }

    case WM_NCACTIVATE:
        return TRUE;

    case WM_PAINT: {
        RECT rect;
        GetClientRect(hWnd, &rect);

        PAINTSTRUCT ps;
        HDC hDC = ::BeginPaint(hWnd, &ps);
        HPEN hpenOld = static_cast<HPEN>(::SelectObject(hDC, ::GetStockObject(DC_PEN)));
        ::SetDCPenColor(hDC, window->m_borderColor);

        HBRUSH hBrush = ::CreateSolidBrush(window->m_bgColor);
        HBRUSH hbrushOld = static_cast<HBRUSH>(::SelectObject(hDC, hBrush));

        ::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

        ::SelectObject(hDC, hbrushOld);
        ::DeleteObject(hBrush);

        ::SelectObject(hDC, hpenOld);
        ::EndPaint(hWnd, &ps);
        return 0;}

    case WM_ERASEBKGND: {
        return TRUE; }

    case WM_GETMINMAXINFO: {
        MINMAXINFO * minMaxInfo = (MINMAXINFO *)lParam;
        if ( window->minimumSize().required ) {
            minMaxInfo->ptMinTrackSize.x = window->minimumSize().width;
            minMaxInfo->ptMinTrackSize.y = window->minimumSize().height;
        }

        if ( window->maximumSize().required ) {
            minMaxInfo->ptMaxTrackSize.x = window->maximumSize().width;
            minMaxInfo->ptMaxTrackSize.y = window->maximumSize().height;
        }

        return 1;
    }

    case WM_ENDSESSION:
//        CAscApplicationManagerWrapper::getInstance().CloseApplication();

        break;

    case WM_COPYDATA: {
        COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
        if (pcds->dwData == 1) {
            int nArgs;
            LPWSTR * szArglist = CommandLineToArgvW((WCHAR *)(pcds->lpData), &nArgs);

            if (szArglist != nullptr) {
                std::vector<std::wstring> v_inargs;
                for(int i(1); i < nArgs; i++) {
                    v_inargs.push_back(szArglist[i]);
                }

                if ( !v_inargs.empty() ) {
                    AscAppManager::handleInputCmd(v_inargs);
                }
            }

            LocalFree(szArglist);
        }
        break;}

    case WM_WINDOWPOSCHANGING: { break; }
    default: break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CSingleWindowPlatform::setMinimumSize( const int width, const int height )
{
    m_minSize.required = true;
    m_minSize.width = width;
    m_minSize.height = height;
}

void CSingleWindowPlatform::toggleBorderless(bool showmax)
{
    if ( m_visible ) {
        LONG newStyle = m_borderless ?
                    long(WindowBase::Style::aero_borderless) : long(WindowBase::Style::windowed)/* & ~WS_CAPTION*/;

        SetWindowLongPtr( m_hWnd, GWL_STYLE, newStyle );

        m_borderless = !m_borderless;

        //redraw frame
//        SetWindowPos( m_hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE /*| SWP_NOZORDER | SWP_NOOWNERZORDER */);
        show(showmax);
    }
}

WindowBase::CWindowGeometry const& CSingleWindowPlatform::minimumSize() const
{
    return m_minSize;
}

WindowBase::CWindowGeometry const& CSingleWindowPlatform::maximumSize() const
{
    return m_maxSize;
}

void CSingleWindowPlatform::show(bool maximized)
{
    ShowWindow(m_hWnd, maximized ? SW_MAXIMIZE : SW_SHOW);
//    UpdateWindow(m_hWnd);
    m_visible = true;
}

void CSingleWindowPlatform::hide()
{
    ShowWindow(m_hWnd, SW_HIDE);
    m_visible = false;
}

bool CSingleWindowPlatform::visible()
{
    return m_visible;
}

void CSingleWindowPlatform::bringToTop()
{
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
    SetActiveWindow(m_hWnd);
}

void CSingleWindowPlatform::applyWindowState(Qt::WindowState s)
{
    m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
    m_buttonMaximize->style()->polish(m_buttonMaximize);
}

void CSingleWindowPlatform::onSizeEvent(int type)
{
    if (type == SIZE_MINIMIZED) {
        applyWindowState(Qt::WindowMinimized);
    } else {
        if (type == SIZE_MAXIMIZED)
            applyWindowState(Qt::WindowMaximized);
        else applyWindowState(Qt::WindowNoState);

        adjustGeometry();
    }

    CSingleWindowBase::onSizeEvent(type);
}

void CSingleWindowPlatform::onExitSizeMove()
{
    setMinimumSize(0, 0);
    double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(m_hWnd));

    if ( dpi_ratio != m_dpiRatio ) {
        if ( WindowHelper::isWindowSystemDocked(m_hWnd) )
            onDpiChanged(dpi_ratio, m_dpiRatio);
        else setScreenScalingFactor(dpi_ratio);
    }
}

void CSingleWindowPlatform::adjustGeometry()
{
    RECT lpWindowRect, clientRect;
    GetWindowRect(m_hWnd, &lpWindowRect);
    GetClientRect(m_hWnd, &clientRect);

    int border_size = 0,
        nMaxOffsetX = 0,
        nMaxOffsetY = 0,
        nMaxOffsetR = 0,
        nMaxOffsetB = 0;

    if ( IsZoomed(m_hWnd) != 0 ) {      // is window maximized
        LONG lTestW = 640,
             lTestH = 480;

        QScreen * _screen = Utils::screenAt(QRect(QPoint(lpWindowRect.left, lpWindowRect.top),QPoint(lpWindowRect.right,lpWindowRect.bottom)).center());
        double _screen_dpi_ratio = _screen->logicalDotsPerInch() / 96;

        RECT wrect{0,0,lTestW,lTestH};
        WindowHelper::adjustWindowRect(m_hWnd, _screen_dpi_ratio, &wrect);

        if (0 > wrect.left) nMaxOffsetX = -wrect.left;
        if (0 > wrect.top)  nMaxOffsetY = -wrect.top;

        if (wrect.right > lTestW)   nMaxOffsetR = (wrect.right - lTestW);
        if (wrect.bottom > lTestH)  nMaxOffsetB = (wrect.bottom - lTestH);

        // TODO: вот тут бордер!!!
        m_pWinPanel->setGeometry( nMaxOffsetX + border_size, nMaxOffsetY + border_size,
                                                    clientRect.right - (nMaxOffsetX + nMaxOffsetR + 2 * border_size),
                                                    clientRect.bottom - (nMaxOffsetY + nMaxOffsetB + 2 * border_size));
    } else {
        border_size = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);

        // TODO: вот тут бордер!!!
        m_pWinPanel->setGeometry(border_size, border_size,
                            clientRect.right - 2 * border_size, clientRect.bottom - 2 * border_size);
    }

    HRGN hRgn = CreateRectRgn(nMaxOffsetX, nMaxOffsetY,
                                lpWindowRect.right - lpWindowRect.left - nMaxOffsetX,
                                lpWindowRect.bottom - lpWindowRect.top - nMaxOffsetY);

    SetWindowRgn(m_hWnd, hRgn, TRUE);
    DeleteObject(hRgn);
}

void CSingleWindowPlatform::onMinimizeEvent()
{
    ShowWindow(m_hWnd, SW_MINIMIZE);
}

void CSingleWindowPlatform::onMaximizeEvent()
{
    ShowWindow(m_hWnd, IsZoomed(m_hWnd) ? SW_RESTORE : SW_MAXIMIZE);
}

void CSingleWindowPlatform::setScreenScalingFactor(double f)
{
    double change_factor = f / m_dpiRatio;
    CSingleWindowBase::setScreenScalingFactor(f);

    if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
        m_skipSizing = true;

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if ( GetWindowPlacement(m_hWnd, &wp) ) {

            if ( wp.showCmd == SW_MAXIMIZE ) {
                MONITORINFO info{sizeof(MONITORINFO)};
                GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &info);

                int dest_width_change = int(m_moveNormalRect.width() * (1 - change_factor));
                m_moveNormalRect = QRect{m_moveNormalRect.translated(dest_width_change/2,0).topLeft(), m_moveNormalRect.size() * change_factor};

                wp.rcNormalPosition.left = info.rcMonitor.left + m_moveNormalRect.left();
                wp.rcNormalPosition.top = info.rcMonitor.top + m_moveNormalRect.top();
                wp.rcNormalPosition.right = wp.rcNormalPosition.left + m_moveNormalRect.width();
                wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + m_moveNormalRect.height();

                SetWindowPlacement(m_hWnd, &wp);
            } else {
                QRect source_rect = QRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top),QPoint(wp.rcNormalPosition.right,wp.rcNormalPosition.bottom)};
                int dest_width_change = int(source_rect.width() * (1 - change_factor));
                QRect dest_rect = QRect{source_rect.translated(dest_width_change/2,0).topLeft(), source_rect.size() * change_factor};

                SetWindowPos(m_hWnd, NULL, dest_rect.left(), dest_rect.top(), dest_rect.width(), dest_rect.height(), SWP_NOZORDER);
            }
        }
        m_skipSizing = false;
    }
}

Qt::WindowState CSingleWindowPlatform::windowState()
{
    return IsZoomed(m_hWnd) ? Qt::WindowMaximized :
                IsIconic(m_hWnd) ? Qt::WindowMinimized : Qt::WindowNoState;
}

void CSingleWindowPlatform::setWindowState(Qt::WindowState state)
{
    switch (state) {
    case Qt::WindowMaximized: ShowWindow(m_hWnd, SW_MAXIMIZE); break;
    case Qt::WindowMinimized: ShowWindow(m_hWnd, SW_MINIMIZE); break;
    case Qt::WindowNoState:
    default: ShowWindow(m_hWnd, IsZoomed(m_hWnd) || IsIconic(m_hWnd) ? SW_RESTORE : SW_NORMAL);
    }
}

void CSingleWindowPlatform::setWindowTitle(const QString& title)
{
    CSingleWindowBase::setWindowTitle(title);
    SetWindowText(m_hWnd, title.toStdWString().c_str());
}

void CSingleWindowPlatform::setWindowBackgroundColor(const QColor& color)
{
    int r, g, b;
    color.getRgb(&r, &g, &b);

    m_bgColor = RGB(r, g, b);
    RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
}

void CSingleWindowPlatform::setWindowColors(const QColor& background, const QColor& border)
{
    int r, g, b;
    border.getRgb(&r, &g, &b);
    m_borderColor = RGB(r, g, b);

    background.getRgb(&r, &g, &b);
    m_bgColor = RGB(r, g, b);

    RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
}

QRect CSingleWindowPlatform::geometry() const
{
    WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(m_hWnd, &wp);

    return QRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top), QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom)};
}

void CSingleWindowPlatform::activateWindow()
{
    SetActiveWindow(m_hWnd);
}

void CSingleWindowPlatform::slot_modalDialog(bool status, HWND h)
{
    if ( h != m_hWnd ) {
        EnableWindow(m_hWnd, status ? FALSE : TRUE);
        m_modalHwnd = h;
    } else m_modalHwnd = nullptr;

}

void CSingleWindowPlatform::captureMouse()
{
    POINT cursor{0,0};
    if ( GetCursorPos(&cursor) ) {
        QRect _g{geometry()};

        int _window_offset_x;
        if ( cursor.x - _g.x() < dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X) ) _window_offset_x = dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X);
        else if ( cursor.x > _g.right() - dpiCorrectValue(150) ) _window_offset_x = _g.right() - dpiCorrectValue(150);
        else _window_offset_x = cursor.x - _g.x();

        SetWindowPos(m_hWnd, nullptr, cursor.x - _window_offset_x, cursor.y - dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_Y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        ReleaseCapture();
        PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cursor.x, cursor.y));
    }
}
