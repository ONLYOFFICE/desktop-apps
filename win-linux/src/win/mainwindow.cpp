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
extern QStringList g_cmdArgs;

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);


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
        _window_rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);

    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if ( _screen_size.width() < _window_rect.width() + 120 ||
            _screen_size.height() < _window_rect.height() + 120 )
    {
        _window_rect.setLeft(_screen_size.left()),
        _window_rect.setTop(_screen_size.top());

        if ( _screen_size.width() < _window_rect.width() ) _window_rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < _window_rect.height() ) _window_rect.setHeight(_screen_size.height());
    }

    WNDCLASSEXW wcx{ sizeof(WNDCLASSEX) };
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = WINDOW_CLASS_NAME;
    wcx.hbrBackground = CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
    wcx.hCursor = LoadCursor( hInstance, IDC_ARROW );

    QIcon icon = Utils::appIcon();
    wcx.hIcon = qt_pixmapToWinHICON(QSysInfo::windowsVersion() == QSysInfo::WV_XP ?
                                        icon.pixmap(icon.availableSizes().first()) : icon.pixmap(QSize(32,32)) );

    if ( FAILED( RegisterClassExW( &wcx ) ) )
        throw std::runtime_error( "Couldn't register window class" );

    hWnd = CreateWindow( WINDOW_CLASS_NAME, QString(WINDOW_NAME).toStdWString().c_str(), static_cast<DWORD>(WindowBase::Style::windowed),
                            _window_rect.x(), _window_rect.y(), _window_rect.width(), _window_rect.height(), 0, 0, hInstance, nullptr );
    if ( !hWnd )
        throw std::runtime_error( "couldn't create window because of reasons" );

    SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );

    m_pWinPanel = new CWinPanel(this);

    m_pMainPanel = new CMainPanelImpl(m_pWinPanel, true, m_dpiRatio);
    m_pMainPanel->setInputFiles(Utils::getInputFiles(g_cmdArgs));
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);
    m_pMainPanel->goStart();

//    SetWindowPos(HWND(m_pWinPanel->winId()), NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);
    setMinimumSize( MAIN_WINDOW_MIN_WIDTH*m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT*m_dpiRatio );

    CMainPanel * mainpanel = m_pMainPanel;
    QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, bind(&CMainWindow::slot_windowChangeState, this, _1));
    QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, bind(&CMainWindow::slot_windowClose, this));
    QObject::connect(mainpanel, &CMainPanel::mainPageReady, bind(&CMainWindow::slot_mainPageReady, this));

    m_pWinPanel->show();
}

CMainWindow::~CMainWindow()
{
    closed = true;

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

    hide();
    DestroyWindow( hWnd );
}

LRESULT CALLBACK CMainWindow::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    CMainWindow *window = reinterpret_cast<CMainWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
    if ( !window )
        return DefWindowProc( hWnd, message, wParam, lParam );
//static uint count=0;
//qDebug() << "main window message: " << ++count << QString(" 0x%1").arg(QString::number(message,16));

    switch ( message )
    {
    case WM_DPICHANGED:
        qDebug() << "WM_DPICHANGED: " << LOWORD(wParam);
        break;

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
        if (GET_SC_WPARAM(wParam) == SC_MAXIMIZE) {
            qDebug() << "wm syscommand";
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
        window->doClose();
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
        if ( !window->closed && window->m_pWinPanel) {
            if (wParam == SIZE_MINIMIZED) {
                window->m_pMainPanel->applyMainWindowState(Qt::WindowMinimized);
            } else {
                if ( IsWindowVisible(hWnd) ) {
                    if ( wParam == SIZE_MAXIMIZED )
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowMaximized);  else
                        window->m_pMainPanel->applyMainWindowState(Qt::WindowNoState);
                }

                window->adjustGeometry();
            }
        }
        break;

    case WM_MOVING: {
#if defined(__APP_MULTI_WINDOW)
        if ( window->movedByTab() ) {
            POINT pt{0};

            if ( GetCursorPos(&pt) ) {
                AscAppManager::processMainWindowMoving(size_t(window), QPoint(pt.x, pt.y));
            }
        }

#endif
        break;
    }

    case WM_EXITSIZEMOVE: {
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
#else
        uchar dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));
#endif

        if ( dpi_ratio != window->m_dpiRatio )
            window->setScreenScalingFactor(dpi_ratio);

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
        ::SetDCPenColor(hDC, RGB(136, 136, 136));

        HBRUSH hBrush = ::CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
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
                QStringList _in_args;
                for(int i(1); i < nArgs; i++) {
                    _in_args.append(QString::fromStdWString(szArglist[i]));
                }

                if ( _in_args.size() ) {
                    QStringList * _file_list = Utils::getInputFiles(_in_args);

                    if (_file_list->size()) {
                        window->mainPanel()->doOpenLocalFiles(*_file_list);

                        if ( ::IsIconic(hWnd) ) {
                            window->slot_windowChangeState( Qt::WindowNoState );
                            ::SetForegroundWindow(hWnd);
                            ::FlashWindow(hWnd, TRUE);
                        }
                    }

                    delete _file_list;
                }
            }

            ::SetFocus(hWnd);
            LocalFree(szArglist);
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

        RECT wrect{0,0,lTestW,lTestH};
        AdjustWindowRectEx(&wrect, (GetWindowStyle(hWnd) & ~WS_DLGFRAME), FALSE, 0);

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

    SetWindowRgn(hWnd, hRgn, TRUE);
    DeleteObject(hRgn);
}

void CMainWindow::setScreenScalingFactor(uchar factor)
{
    QString css(AscAppManager::getWindowStylesheets(factor));

    if ( !css.isEmpty() ) {
        bool increase = factor > m_dpiRatio;
        m_dpiRatio = factor;

        m_pMainPanel->setStyleSheet(css);
        m_pMainPanel->setScreenScalingFactor(factor);
        setMinimumSize( MAIN_WINDOW_MIN_WIDTH*factor, MAIN_WINDOW_MIN_HEIGHT*factor );

        RECT lpWindowRect;
        GetWindowRect(hWnd, &lpWindowRect);

        unsigned _new_width = lpWindowRect.right - lpWindowRect.left,
                _new_height = lpWindowRect.bottom - lpWindowRect.top;

        if ( increase )
            _new_width *= 2, _new_height *= 2;  else
            _new_width /= 2, _new_height /= 2;

        SetWindowPos(hWnd, NULL, 0, 0, _new_width, _new_height, SWP_NOMOVE | SWP_NOZORDER);
    }
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
    AscAppManager::closeMainWindow( size_t(this) );
}

void CMainWindow::slot_mainPageReady()
{
    CSplash::hideSplash();

#ifdef _UPDMODULE
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    // skip updates for XP
    if ( osvi.dwMajorVersion > 5 ) {
        win_sparkle_set_lang(CLangater::getCurrentLangCode().toLatin1());

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
            win_sparkle_set_appcast_url(URL_APPCAST_UPDATES);
            win_sparkle_set_registry_path(QString("Software\\%1\\%2").arg(REG_GROUP_KEY).arg(REG_APP_NAME).toLatin1());

            win_sparkle_set_did_find_update_callback(&CMainWindow::updateFound);
            win_sparkle_set_did_not_find_update_callback(&CMainWindow::updateNotFound);
            win_sparkle_set_error_callback(&CMainWindow::updateError);

            win_sparkle_init();
        }

        AscAppManager::sendCommandTo(0, "updates", "on");
        CLogger::log(QString("updates is on: ") + URL_APPCAST_UPDATES);
    }
#endif
}

#if defined(_UPDMODULE)
void CMainWindow::updateFound()
{
    CLogger::log("found updates");
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
