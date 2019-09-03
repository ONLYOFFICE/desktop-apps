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

#include <functional>


Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

CSingleWindowPlatform::CSingleWindowPlatform(const QRect& rect, const QString& title, QWidget * panel)
    : CSingleWindowBase(const_cast<QRect&>(rect))
    , m_bgColor(WINDOW_BACKGROUND_COLOR)
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

    m_hWnd = CreateWindow(L"SingleWindowClass", title.toStdWString().c_str(), static_cast<DWORD>(WindowBase::Style::windowed),
                                rect.x(), rect.y(), rect.width(), rect.height(), 0, 0, hInstance, nullptr);

    if ( !m_hWnd )
        throw std::runtime_error("couldn't create window because of reasons");

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    setMinimumSize(MAIN_WINDOW_MIN_WIDTH * m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio);

    m_pWinPanel = new CWinPanel(m_hWnd);
    m_winRect = rect;

    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CSingleWindowPlatform::slot_modalDialog, this, std::placeholders::_1, std::placeholders::_2));
}

CSingleWindowPlatform::~CSingleWindowPlatform()
{
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
    case WM_KEYDOWN: {
        if ( wParam != VK_TAB )
            return DefWindowProc(hWnd, message, wParam, lParam);

        SetFocus( HWND(window->m_pWinPanel->winId()) );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND: {
        if ( wParam == SC_KEYMENU ) {
            return 0;
        }

        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    case WM_ACTIVATE: {
        if ( !IsWindowEnabled(hWnd) && window->m_modalHwnd > 0 && window->m_modalHwnd != hWnd )
        {
            if ( LOWORD(wParam) != WA_INACTIVE ) {
                SetWindowPos(hWnd, window->m_modalHwnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                return 0;
            }
        }

        break;
    }

    case WM_SETFOCUS: {
//        window->focusMainPanel();
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
        if ( !window->m_closed ) {
            window->onSizeEvent(wParam);
        }

        break;

    case WM_EXITSIZEMOVE: {
        uchar dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

        if ( dpi_ratio != window->m_dpiRatio )
            window->setScreenScalingFactor(dpi_ratio);

        RECT lpWindowRect;
        GetWindowRect(hWnd, &lpWindowRect);
        window->m_winRect.setCoords(lpWindowRect.left, lpWindowRect.top, lpWindowRect.right, lpWindowRect.bottom);

        break;
    }

    case WM_NCACTIVATE:
        return TRUE;

    case WM_PAINT: {
        RECT rect;
        GetClientRect(hWnd, &rect);

        PAINTSTRUCT ps;
        HDC hDC = ::BeginPaint(hWnd, &ps);
        HPEN hpenOld = static_cast<HPEN>(::SelectObject(hDC, ::GetStockObject(DC_PEN)));
        ::SetDCPenColor(hDC, RGB(136, 136, 136));

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

    if ( !maximized && GetCapture() != NULL )
        captureMouse();
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

        RECT wrect{0,0,lTestW,lTestH};
        AdjustWindowRectEx(&wrect, (GetWindowStyle(m_hWnd) & ~WS_DLGFRAME), FALSE, 0);

        if (0 > wrect.left) nMaxOffsetX = -wrect.left;
        if (0 > wrect.top)  nMaxOffsetY = -wrect.top;

        if (wrect.right > lTestW)   nMaxOffsetR = (wrect.right - lTestW);
        if (wrect.bottom > lTestH)  nMaxOffsetB = (wrect.bottom - lTestH);

        // TODO: вот тут бордер!!!
        m_pWinPanel->setGeometry( nMaxOffsetX + border_size, nMaxOffsetY + border_size,
                                                    clientRect.right - (nMaxOffsetX + nMaxOffsetR + 2 * border_size),
                                                    clientRect.bottom - (nMaxOffsetY + nMaxOffsetB + 2 * border_size));
    } else {
        border_size = MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio;

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

void CSingleWindowPlatform::onScreenScalingFactor(uint f)
{
    setMinimumSize(MAIN_WINDOW_MIN_WIDTH * f, MAIN_WINDOW_MIN_HEIGHT * f);

    RECT lpWindowRect;
    GetWindowRect(m_hWnd, &lpWindowRect);

    unsigned _new_width = lpWindowRect.right - lpWindowRect.left,
            _new_height = lpWindowRect.bottom - lpWindowRect.top;

    if ( f > m_dpiRatio )
        _new_width *= 2, _new_height *= 2;
    else _new_width /= 2, _new_height /= 2;

    SetWindowPos(m_hWnd, NULL, 0, 0, _new_width, _new_height, SWP_NOMOVE | SWP_NOZORDER);
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

const QRect& CSingleWindowPlatform::geometry() const
{
    return m_winRect;
}

void CSingleWindowPlatform::activateWindow()
{
    SetActiveWindow(m_hWnd);
}

void CSingleWindowPlatform::slot_modalDialog(bool status, size_t h)
{
    EnableWindow(m_hWnd, status ? FALSE : TRUE);
    m_modalHwnd = (HWND)h;
}

void CSingleWindowPlatform::captureMouse()
{
    POINT cursor{0};
    if ( GetCursorPos(&cursor) ) {
        SetWindowPos(m_hWnd, 0, cursor.x - 300, cursor.y - 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        ReleaseCapture();
        PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cursor.x, cursor.y));
    }
}
