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

#include "MainWindow.h"

#include <dwmapi.h>
#include <windowsx.h>
#include <windows.h>
#include <stdexcept>
#include <functional>

#include <QtWidgets/QPushButton>
#include <QFile>
#include <QPixmap>
#include <QDialog>
#include <QScreen>
#include <QDesktopWidget>

#include "../cascapplicationmanagerwrapper.h"
#include "../defines.h"
#include "defines_p.h"
#include "../utils.h"
#include "../csplash.h"
#include "../clogger.h"
#include "../clangater.h"
#include "../ctabbar.h"

#include <QTimer>
#include <QSettings>
#include <QDebug>

#ifdef _UPDMODULE
  #include "3dparty/WinSparkle/include/winsparkle.h"
  #include "../version.h"
#endif

using namespace std::placeholders;

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

auto refresh_window_scaling_factor(CMainWindow * window) -> void {
    QString css{AscAppManager::getWindowStylesheets(window->m_dpiRatio)};

    if ( !css.isEmpty() ) {
        window->mainPanel()->setStyleSheet(css);
        window->mainPanel()->setScreenScalingFactor(window->m_dpiRatio);
    }
}

CMainWindow::CMainWindow(QRect& rect) :
    hWnd(nullptr),
    hInstance(GetModuleHandle(nullptr)),
    borderless( true ),
    borderlessResizeable( true ),
    closed( false ),
    visible( false ),
    m_pWinPanel(NULL)
{
    // adjust window size
    QRect _window_rect = rect;
    m_dpiRatio = CSplash::startupDpiRatio();

    if ( _window_rect.isEmpty() )
        _window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);

    // TODO: skip window min size for usability test
//    QSize _window_min_size(int(MAIN_WINDOW_MIN_WIDTH * m_dpiRatio), int(MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio));
//    if ( _window_rect.width() < _window_min_size.width() )
//        _window_rect.setWidth(_window_min_size.width());

//    if ( _window_rect.height() < _window_min_size.height() )
//        _window_rect.setHeight(_window_min_size.height());

    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if ( _screen_size.intersects(_window_rect) ) {
        if ( _screen_size.width() < _window_rect.width() ||
                _screen_size.height() < _window_rect.height() )
        {
            _window_rect.setLeft(_screen_size.left()),
            _window_rect.setTop(_screen_size.top());

            if ( _screen_size.width() < _window_rect.width() ) _window_rect.setWidth(_screen_size.width());
            if ( _screen_size.height() < _window_rect.height() ) _window_rect.setHeight(_screen_size.height());
        }
    } else {
        _window_rect = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
    }

    WNDCLASSEXW wcx{ sizeof(WNDCLASSEX) };
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = WINDOW_CLASS_NAME;
//    wcx.hbrBackground = CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
    wcx.hbrBackground = CreateSolidBrush(AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBackground));
    wcx.hCursor = LoadCursor( hInstance, IDC_ARROW );

    QIcon icon = Utils::appIcon();
    wcx.hIcon = qt_pixmapToWinHICON(QSysInfo::windowsVersion() == QSysInfo::WV_XP ?
                                        icon.pixmap(icon.availableSizes().first()) : icon.pixmap(QSize(32,32)) );

    if ( FAILED( RegisterClassExW( &wcx ) ) )
        throw std::runtime_error( "Couldn't register window class" );

    m_moveNormalRect = _window_rect;
    hWnd = CreateWindow( WINDOW_CLASS_NAME, QString(WINDOW_NAME).toStdWString().c_str(), static_cast<DWORD>(WindowBase::Style::windowed),
                            _window_rect.x(), _window_rect.y(), _window_rect.width(), _window_rect.height(), 0, 0, hInstance, nullptr );
    if ( !hWnd )
        throw std::runtime_error( "couldn't create window because of reasons" );

    SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );

    m_pWinPanel = new CWinPanel(this);

    m_pMainPanel = new CMainPanelImpl(m_pWinPanel, true, m_dpiRatio);
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);
    m_pMainPanel->goStart();

//    SetWindowPos(HWND(m_pWinPanel->winId()), NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);

    CMainPanel * mainpanel = m_pMainPanel;
    QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, bind(&CMainWindow::slot_windowChangeState, this, _1));
    QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, std::bind(&CMainWindow::slot_windowClose, this));
    QObject::connect(mainpanel, &CMainPanel::mainPageReady, std::bind(&CMainWindow::slot_mainPageReady, this));
    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    m_pWinPanel->show();
}

CMainWindow::~CMainWindow()
{
    closed = true;

    if ( isVisible() ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(hWnd, &wp)) {
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

    hide();
    DestroyWindow( hWnd );
}

LRESULT CALLBACK CMainWindow::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    CMainWindow *window = reinterpret_cast<CMainWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
    if ( !window )
        return DefWindowProc( hWnd, message, wParam, lParam );
//static uint count=0;
//qDebug() << "main window message: " << ++count << QString(" 0x%1").arg(message,4,16,QChar('0'));

    switch ( message )
    {
    case WM_DPICHANGED:
        if ( !WindowHelper::isLeftButtonPressed() ) {
            double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

            if ( dpi_ratio != window->m_dpiRatio ) {
                window->m_dpiRatio = dpi_ratio;
                refresh_window_scaling_factor(window);
                window->adjustGeometry();
            }
        } else
        if ( AscAppManager::IsUseSystemScaling() ) {
            window->updateScaling();
        }

        qDebug() << "WM_DPICHANGED: " << LOWORD(wParam);
        break;

    case WM_ACTIVATE: {
        if ( LOWORD(wParam) != WA_INACTIVE ) {
            WindowHelper::correctModalOrder(hWnd, window->m_modalHwnd);
            return 0;
        }

        break;
    }

    case WM_KEYDOWN:
    {
        switch ( wParam )
        {
            case VK_F4:
                if ( HIBYTE(GetKeyState(VK_SHIFT)) & 0x80 ) {
                    qDebug() << "shift pressed";
                } else {
                    qDebug() << "shift doesn't pressed";
                }
                break;
            case VK_F5:
            {
//                window->borderlessResizeable = !window->borderlessResizeable;
                break;
            }
            case VK_F6:
            {
//                window->toggleShadow();
//                window->toggleBorderless();
//                SetFocus( winId );
                break;
            }
            case VK_F7:
            {
//                window->toggleShadow();
                break;
            }
        }

        if ( wParam != VK_TAB )
            return DefWindowProc( hWnd, message, wParam, lParam );

        SetFocus( HWND(window->m_pWinPanel->winId()) );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND:
    {
        if ( GET_SC_WPARAM(wParam) == SC_KEYMENU )
        {
//            RECT winrect;
//            GetWindowRect( hWnd, &winrect );
//            TrackPopupMenu( GetSystemMenu( hWnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, hWnd, NULL);
//            break;
            return 0;
        } else
        if ( GET_SC_WPARAM(wParam) == SC_SIZE ) {
            // TODO: skip window min size for usability test
//            window->setMinimumSize(int(MAIN_WINDOW_MIN_WIDTH * window->m_dpiRatio), int(MAIN_WINDOW_MIN_HEIGHT * window->m_dpiRatio));
            break;
        } else
        if ( GET_SC_WPARAM(wParam) == SC_MOVE ) {
            break;
        } else
        if (GET_SC_WPARAM(wParam) == SC_MAXIMIZE) {
            qDebug() << "wm syscommand maximized";
            break;
        }
        else
        if (GET_SC_WPARAM(wParam) == SC_RESTORE) {
//            if ( !WindowHelper::isLeftButtonPressed() )
//                WindowHelper::correctWindowMinimumSize(window->handle());

            break;
        }
        else
        if (GET_SC_WPARAM(wParam) == SC_MINIMIZE) {
            break;
        }
        else
        {
            return DefWindowProc( hWnd, message, wParam, lParam );
        }
    }

    case WM_SETFOCUS:
    {
//        QString str( "Got focus" );
//        QWidget *widget = QWidget::find( ( WId )HWND( wParam ) );
//        if ( widget )
//            str += QString( " from %1 (%2)" ).arg( widget->objectName() ).arg(widget->metaObject()->className() );
//        str += "\n";
//        OutputDebugStringA( str.toLocal8Bit().data() );

        if ( IsWindowEnabled(hWnd) )
            window->m_pMainPanel->focus();
        break;
    }

    case WM_NCCALCSIZE:
    {
        //this kills the window frame and title bar we added with
        //WS_THICKFRAME and WS_CAPTION
        if (window->borderless)
        {

            return 0;
        }
        break;
    }

    case WM_KILLFOCUS:
        break;

    case WM_CLOSE:
qDebug() << "WM_CLOSE";

        AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(window)});
        return 0;

    case WM_DESTROY:
    {
//        PostQuitMessage(0);
        break;
    }

    case WM_TIMER:
    {
        CAscApplicationManagerWrapper::getInstance().CheckKeyboard();
        break;
    }

    case WM_NCPAINT:
        return 0;

    case WM_NCHITTEST:
    {
        if ( window->borderless )
        {
            const LONG borderWidth = 8; //in pixels
            RECT winrect;
            GetWindowRect( hWnd, &winrect );
            long x = GET_X_LPARAM( lParam );
            long y = GET_Y_LPARAM( lParam );
            if ( window->borderlessResizeable )
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
        if ( !window->skipsizing && !window->closed && window->m_pWinPanel) {
            if (wParam == SIZE_MINIMIZED) {
                window->m_pMainPanel->applyMainWindowState(Qt::WindowMinimized);
            } else {
                if ( IsWindowVisible(hWnd) ) {
                    if ( WindowHelper::isLeftButtonPressed() ) {
                        double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));
                        if ( dpi_ratio != window->m_dpiRatio )
                            window->setScreenScalingFactor(dpi_ratio);
                    }

                    if ( wParam == SIZE_MAXIMIZED )
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowMaximized);  else
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowNoState);
                }

                window->adjustGeometry();
            }
        }
        break;

    case WM_MOVING: {
        break;
    }

    case WM_ENTERSIZEMOVE: {
//        WindowHelper::correctWindowMinimumSize(window->handle());

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if ( GetWindowPlacement(hWnd, &wp) ) {
            MONITORINFO info{sizeof(MONITORINFO)};
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &info);

            window->m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                                QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
        }
        break;}

    case WM_EXITSIZEMOVE: {
        window->setMinimumSize(0, 0);
//#define DEBUG_SCALING
#if defined(DEBUG_SCALING) && defined(_DEBUG)
        QRect windowRect;

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(hWnd, &wp)) {
            GET_REGISTRY_USER(reg_user)
            wp.showCmd == SW_MAXIMIZE ?
                        reg_user.setValue("maximized", true) : reg_user.remove("maximized");

            windowRect.setTopLeft(QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));
            windowRect.setBottomRight(QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
            windowRect.adjust(0,0,-1,-1);
        }

        int _scr_num = QApplication::desktop()->screenNumber(windowRect.topLeft()) + 1;
        uchar dpi_ratio = _scr_num;

        if ( dpi_ratio != window->m_dpiRatio ) {
            if ( !WindowHelper::isWindowSystemDocked(hWnd) ) {
                window->setScreenScalingFactor(dpi_ratio);
            } else {
                window->m_dpiRatio = dpi_ratio;
                refresh_window_scaling_factor(window);
            }

            window->adjustGeometry();
        }
#else
        if ( !AscAppManager::IsUseSystemScaling() )
            window->updateScaling();
#endif

        break;
    }

    case WM_NCACTIVATE: {
        return TRUE;
        break;
    }

    case WM_PAINT: {
        RECT rect;
        GetClientRect(hWnd, &rect);

        PAINTSTRUCT ps;
        HDC hDC = ::BeginPaint(hWnd, &ps);
        HPEN hpenOld = static_cast<HPEN>(::SelectObject(hDC, ::GetStockObject(DC_PEN)));
        ::SetDCPenColor(hDC, AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBorder));

        HBRUSH hBrush = ::CreateSolidBrush(AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBackground));
        HBRUSH hbrushOld = static_cast<HBRUSH>(::SelectObject(hDC, hBrush));

        ::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

        ::SelectObject(hDC, hbrushOld);
        ::DeleteObject(hBrush);

        ::SelectObject(hDC, hpenOld);
        ::EndPaint(hWnd, &ps);
        return 0; }

    case WM_ERASEBKGND: {
        return TRUE; }

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* minMaxInfo = ( MINMAXINFO* )lParam;
        if ( window->minimumSize.required )
        {
            minMaxInfo->ptMinTrackSize.x = window->getMinimumWidth();;
            minMaxInfo->ptMinTrackSize.y = window->getMinimumHeight();
        }

        if ( window->maximumSize.required )
        {
            minMaxInfo->ptMaxTrackSize.x = window->getMaximumWidth();
            minMaxInfo->ptMaxTrackSize.y = window->getMaximumHeight();
        }
        return 1;
    }
    case WM_ENDSESSION:
        CAscApplicationManagerWrapper::getInstance().CloseApplication();

        break;

    case WM_WINDOWPOSCHANGING: { break; }
    case WM_COPYDATA: {
        COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
        if (pcds->dwData == 1) {
            int nArgs;
            LPWSTR * szArglist = CommandLineToArgvW((WCHAR *)(pcds->lpData), &nArgs);

            if (szArglist != nullptr) {
                std::vector<std::wstring> _v_inargs;
                for(int i(1); i < nArgs; i++) {
                    _v_inargs.push_back(szArglist[i]);
                }

                if ( !_v_inargs.empty() ) {
                    AscAppManager::handleInputCmd(_v_inargs);
                }
            }

            ::SetFocus(hWnd);
            LocalFree(szArglist);

            window->bringToTop();
        }
        break;}
    case UM_INSTALL_UPDATE:
        window->doClose();
        break;
    default: {
        break;
    }
#if 0
    case WM_INPUTLANGCHANGE:
    case WM_INPUTLANGCHANGEREQUEST:
    {
        int _lang = LOWORD(lParam);
        m_oLanguage.Check(_lang);
    }
#endif
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CMainWindow::toggleBorderless(bool showmax)
{
    if ( visible )
    {
        // чтобы не было мерцания. перерисовку при "неактивном окне" - перекроем
        LONG newStyle = borderless ?
                    long(WindowBase::Style::aero_borderless) : long(WindowBase::Style::windowed)/* & ~WS_CAPTION*/;

        SetWindowLongPtr( hWnd, GWL_STYLE, newStyle );

        borderless = !borderless;

        //redraw frame
        SetWindowPos( hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE /*| SWP_NOZORDER | SWP_NOOWNERZORDER */);
        show(showmax);
    }
}

void CMainWindow::toggleResizeable()
{
    borderlessResizeable = borderlessResizeable ? false : true;
}

void CMainWindow::show(bool maximized)
{
    ShowWindow( hWnd, maximized ? SW_MAXIMIZE : SW_SHOW);

    visible = true;
}

void CMainWindow::hide()
{
    ShowWindow( hWnd, SW_HIDE );
    visible = false;
}

bool CMainWindow::isVisible()
{
    return visible ? true : false;
}

// Minimum size
void CMainWindow::setMinimumSize( const int width, const int height )
{
    this->minimumSize.required = true;
    this->minimumSize.width = width;
    this->minimumSize.height = height;
}

bool CMainWindow::isSetMinimumSize()
{
    return this->minimumSize.required;
}
void CMainWindow::removeMinimumSize()
{
    this->minimumSize.required = false;
    this->minimumSize.width = 0;
    this->minimumSize.height = 0;
}

int CMainWindow::getMinimumWidth() const
{
    return minimumSize.width;
}

int CMainWindow::getMinimumHeight() const
{
    return minimumSize.height;
}

// Maximum size
void CMainWindow::setMaximumSize( const int width, const int height )
{
    this->maximumSize.required = true;
    this->maximumSize.width = width;
    this->maximumSize.height = height;
}
bool CMainWindow::isSetMaximumSize()
{
    return this->maximumSize.required;
}

void CMainWindow::removeMaximumSize()
{
    this->maximumSize.required = false;
    this->maximumSize.width = 0;
    this->maximumSize.height = 0;
}
int CMainWindow::getMaximumWidth()
{
    return maximumSize.width;
}
int CMainWindow::getMaximumHeight()
{
    return maximumSize.height;
}

void CMainWindow::adjustGeometry()
{
    RECT lpWindowRect, clientRect;
    GetWindowRect(hWnd, &lpWindowRect);
    GetClientRect(hWnd, &clientRect);

    int border_size = 0,
        nMaxOffsetX = 0,
        nMaxOffsetY = 0,
        nMaxOffsetR = 0,
        nMaxOffsetB = 0;

    if ( IsZoomed(hWnd) != 0 ) {      // is window maximized
        LONG lTestW = 640,
             lTestH = 480;

        QScreen * _screen = Utils::screenAt(QRect(QPoint(lpWindowRect.left, lpWindowRect.top),QPoint(lpWindowRect.right,lpWindowRect.bottom)).center());
        double _screen_dpi_ratio = _screen->logicalDotsPerInch() / 96;

        RECT wrect{0,0,lTestW,lTestH};
        WindowHelper::adjustWindowRect(hWnd, _screen_dpi_ratio, &wrect);

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

    SetWindowRgn(hWnd, hRgn, TRUE);
    DeleteObject(hRgn);
}

void CMainWindow::setScreenScalingFactor(double factor)
{
    skipsizing = true;

    QString css(AscAppManager::getWindowStylesheets(factor));

    if ( !css.isEmpty() ) {
        double change_factor = factor / m_dpiRatio;
        m_dpiRatio = factor;

        m_pMainPanel->setStyleSheet(css);
        m_pMainPanel->setScreenScalingFactor(factor);

        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if ( GetWindowPlacement(hWnd, &wp) ) {
            if ( wp.showCmd == SW_MAXIMIZE ) {
                MONITORINFO info{sizeof(MONITORINFO)};
                GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &info);

                int dest_width_change = int(m_moveNormalRect.width() * (1 - change_factor));
                m_moveNormalRect = QRect{m_moveNormalRect.translated(dest_width_change/2,0).topLeft(), m_moveNormalRect.size() * change_factor};

                wp.rcNormalPosition.left = info.rcMonitor.left + m_moveNormalRect.left();
                wp.rcNormalPosition.top = info.rcMonitor.top + m_moveNormalRect.top();
                wp.rcNormalPosition.right = wp.rcNormalPosition.left + m_moveNormalRect.width();
                wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + m_moveNormalRect.height();

                SetWindowPlacement(hWnd, &wp);
            } else {
                QRect source_rect = QRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top),QPoint(wp.rcNormalPosition.right,wp.rcNormalPosition.bottom)};
                int dest_width_change = int(source_rect.width() * (1 - change_factor));
                QRect dest_rect = QRect{source_rect.translated(dest_width_change/2,0).topLeft(), source_rect.size() * change_factor};

                SetWindowPos(hWnd, NULL, dest_rect.left(), dest_rect.top(), dest_rect.width(), dest_rect.height(), SWP_NOZORDER);
            }
        }
    }

    skipsizing = false;
}

void CMainWindow::slot_windowChangeState(Qt::WindowState s)
{
    int cmdShow = SW_RESTORE;
    switch (s) {
    case Qt::WindowMaximized:   cmdShow = SW_MAXIMIZE; break;
    case Qt::WindowMinimized:   cmdShow = SW_MINIMIZE; break;
    case Qt::WindowFullScreen:  cmdShow = SW_HIDE; break;
    default:
    case Qt::WindowNoState: break;
    }

    ShowWindow(hWnd, cmdShow);
}

void CMainWindow::slot_windowClose()
{
    AscAppManager::closeMainWindow();
}

void CMainWindow::slot_modalDialog(bool status, HWND h)
{
    if ( h != hWnd ) {
        EnableWindow(hWnd, status ? FALSE : TRUE);
        m_modalHwnd = h;
    } else {
        m_modalHwnd = nullptr;

        if ( !status && IsWindowEnabled(hWnd) )
            m_pMainPanel->focus();
    }
}

void CMainWindow::slot_mainPageReady()
{
    CSplash::hideSplash();

#ifdef _UPDMODULE
    GET_REGISTRY_SYSTEM(reg_system)

    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    // skip updates for XP
    if ( osvi.dwMajorVersion > 5 && reg_system.value("CheckForUpdates", true).toBool() ) {
        win_sparkle_set_lang(CLangater::getCurrentLangCode().toLatin1());

        const std::wstring argname{L"--updates-appcast-url"};
        QString _appcast_url = !InputArgs::contains(argname) ? URL_APPCAST_UPDATES : QString::fromStdWString(InputArgs::argument_value(argname));
        static bool _init = false;
        if ( !_init ) {
            _init = true;

            QString _prod_name = WINDOW_NAME;

            GET_REGISTRY_USER(_user)
            if (!_user.contains("CheckForUpdates")) {
                _user.setValue("CheckForUpdates", "1");
            }

            win_sparkle_set_app_details(QString(VER_COMPANYNAME_STR).toStdWString().c_str(),
                                            _prod_name.toStdWString().c_str(),
                                            QString(VER_FILEVERSION_STR).toStdWString().c_str());
            win_sparkle_set_appcast_url(_appcast_url.toStdString().c_str());
            win_sparkle_set_registry_path(QString("Software\\%1\\%2").arg(REG_GROUP_KEY).arg(REG_APP_NAME).toLatin1());

            win_sparkle_set_did_find_update_callback(&CMainWindow::updateFound);
            win_sparkle_set_did_not_find_update_callback(&CMainWindow::updateNotFound);
            win_sparkle_set_error_callback(&CMainWindow::updateError);

            win_sparkle_init();
        }

        AscAppManager::sendCommandTo(0, "updates:turn", "on");
        CLogger::log("updates is on: " + _appcast_url);

#define RATE_MS_DAY 3600*24
#define RATE_MS_WEEK RATE_MS_DAY*7

        std::wstring _wstr_rate{L"day"};
        if ( !win_sparkle_get_automatic_check_for_updates() ) {
            _wstr_rate = L"never";
        } else {
            int _rate{win_sparkle_get_update_check_interval()};
            if ( !(_rate < RATE_MS_WEEK) ) {
                if ( _rate != RATE_MS_WEEK )
                    win_sparkle_set_update_check_interval(RATE_MS_WEEK);

                _wstr_rate = L"week";
            } else {
                if ( _rate != RATE_MS_DAY )
                    win_sparkle_set_update_check_interval(RATE_MS_DAY);
            }
        }

        AscAppManager::sendCommandTo(0, L"settings:check.updates", _wstr_rate);
    }
#endif
}

#if defined(_UPDMODULE)
void CMainWindow::updateFound()
{
    CLogger::log("updates found");
}

void CMainWindow::updateNotFound()
{
    CLogger::log("updates isn't found");
}

void CMainWindow::updateError()
{
    CLogger::log("updates error");
}

void CMainWindow::checkUpdates()
{
    win_sparkle_check_update_with_ui();
}

void CMainWindow::setAutocheckUpdatesInterval(const QString& s)
{
    if ( s == "never" )
        win_sparkle_set_automatic_check_for_updates(0);
    else {
        win_sparkle_set_automatic_check_for_updates(1);

        s == "week" ?
            win_sparkle_set_update_check_interval(RATE_MS_WEEK):
                win_sparkle_set_update_check_interval(RATE_MS_DAY);

    }
}
#endif

void CMainWindow::doClose()
{
    qDebug() << "doClose";

    QTimer::singleShot(500, m_pMainPanel, [=]{
        m_pMainPanel->pushButtonCloseClicked();
    });
}

CMainPanel * CMainWindow::mainPanel() const
{
    return m_pMainPanel;
}

QRect CMainWindow::windowRect() const
{
    QRect _win_rect;
    QPoint _top_left;

    WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
    if ( GetWindowPlacement(hWnd, &wp) ) {
        _top_left = QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
        _win_rect = QRect( _top_left, QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
    }

    return _win_rect;
}

bool CMainWindow::isMaximized() const
{
    bool _is_maximized = false;

    WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
    if ( GetWindowPlacement(hWnd, &wp) ) {
        _is_maximized = wp.showCmd == SW_MAXIMIZE;
    }

    return _is_maximized;
}

HWND CMainWindow::handle() const
{
    return hWnd;
}
void CMainWindow::captureMouse(int tabindex)
{
    CMainWindowBase::captureMouse(tabindex);

    if ( !(tabindex < 0) &&
            tabindex < mainPanel()->tabWidget()->count() )
    {
        QPoint spt = mainPanel()->tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        QPoint gpt = mainPanel()->tabWidget()->tabBar()->mapToGlobal(spt);
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
        gpt = m_pWinPanel->mapToGlobal(gpt);
#endif

        SetCursorPos(gpt.x(), gpt.y());
        //SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(gpt.x(), gpt.y()));
      
        QWidget * _widget = mainPanel()->tabWidget()->tabBar();
        QTimer::singleShot(0,[_widget,spt] {
            INPUT _input{INPUT_MOUSE};
            _input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &_input, sizeof(INPUT));

            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(_widget, &event);
            _widget->grabMouse();
        });
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
bool CMainWindow::pointInTabs(const QPoint& pt) const
{
    QRect _rc_title(mainPanel()->geometry());
    _rc_title.setHeight(mainPanel()->tabWidget()->tabBar()->height());

    return _rc_title.contains(m_pWinPanel->mapFromGlobal(pt));
}
#endif

auto SetForegroundWindowInternal(HWND hWnd)
{
    AllocConsole();
    auto hWndConsole = GetConsoleWindow();
    SetWindowPos(hWndConsole, nullptr, 0, 0, 0, 0, SWP_NOACTIVATE);
    FreeConsole();
    SetForegroundWindow(hWnd);
}

void CMainWindow::bringToTop() const
{
    if (IsIconic(hWnd)) {
        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

//    uint foreThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
//    if ( foreThread != GetCurrentThreadId() ) {
//        SetForegroundWindowInternal(handle());
//    } else {
        SetForegroundWindow(handle());
        SetFocus(handle());
        SetActiveWindow(handle());
//    }
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    CMainWindowBase::applyTheme(theme);

    RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
}

void CMainWindow::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

    if ( dpi_ratio != m_dpiRatio ) {
        if ( !WindowHelper::isWindowSystemDocked(hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        } else {
            m_dpiRatio = dpi_ratio;
            refresh_window_scaling_factor(this);
        }

        adjustGeometry();
    }
}
