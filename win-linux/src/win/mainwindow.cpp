/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include <QtWidgets/QPushButton>
#include <QFile>
#include <QPixmap>
#include <QDialog>
#include <QScreen>

#include "../cascapplicationmanagerwrapper.h"
#include "../defines.h"

#include <QSettings>
#include <QDebug>

HWND gWinId = 0;
extern byte g_dpi_ratio;

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);


CMainWindow::CMainWindow(CAscApplicationManager* pManager, HBRUSH windowBackground) :
    hWnd(0),
    hInstance( GetModuleHandle(NULL) ),
    borderless( false ),
    borderlessResizeable( true ),
    aeroShadow( true ),
    closed( false ),
    visible( false ),
    m_pWinPanel(NULL)
{
    GET_REGISTRY_USER(reg_user)

    // adjust window size
    QRect _window_rect = reg_user.value("position", QRect(100, 100, 1324 * g_dpi_ratio, 800 * g_dpi_ratio)).toRect();
    QRect _screen_size = qApp->primaryScreen()->availableGeometry();
    if (_screen_size.width() < _window_rect.width())
        _window_rect.setWidth(_screen_size.width()), _window_rect.setLeft(0);

    if (_screen_size.height() < _window_rect.height())
        _window_rect.setHeight(_screen_size.width()), _window_rect.setTop(0);

    m_pManager = pManager;
    m_pManager->StartSpellChecker();
    m_pManager->StartKeyboardChecker();

    WNDCLASSEXW wcx = { 0 };
    wcx.cbSize = sizeof( WNDCLASSEX );
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = L"WindowClass";
    wcx.hbrBackground = windowBackground;
    wcx.hCursor = LoadCursor( hInstance, IDC_ARROW );

    QIcon icon(":/res/icons/desktopeditors.ico");
    wcx.hIcon = qt_pixmapToWinHICON(QSysInfo::windowsVersion() == QSysInfo::WV_XP ?
                                        icon.pixmap(icon.availableSizes().first()) : icon.pixmap(QSize(32,32)) );

    if ( FAILED( RegisterClassExW( &wcx ) ) )
        throw std::runtime_error( "Couldn't register window class" );

    hWnd = CreateWindowW( L"WindowClass", L"ONLYOFFICE Desktop Editors", static_cast<DWORD>(Style::windowed),
                          _window_rect.x(), _window_rect.y(), _window_rect.width(), _window_rect.height(), 0, 0, hInstance, nullptr );
    if ( !hWnd )
        throw std::runtime_error( "couldn't create window because of reasons" );

    SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );

    m_pWinPanel = new CWinPanel(hWnd, m_pManager);
    ((CAscApplicationManagerWrapper *)pManager)->setMainPanel(m_pWinPanel->getMainPanel());

    gWinId = ( HWND )m_pWinPanel->winId();

    SetWindowPos(gWinId, NULL, 0, 0, _window_rect.width(), _window_rect.height(), SWP_FRAMECHANGED);

    bool _is_maximized = reg_user.value("maximized", false).toBool();
    show(_is_maximized);
//    visible = true;
    toggleBorderless(_is_maximized);
    m_pWinPanel->goStartPage();

    if (_is_maximized) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(hWnd, &wp)) {
            wp.rcNormalPosition = {_window_rect.x(), _window_rect.y(), _window_rect.right(), _window_rect.left()};
            SetWindowPlacement(hWnd, &wp);
        }
    }

    m_nTimerLanguageId = 5000;

    SetTimer(hWnd, m_nTimerLanguageId, 100, NULL);
}

CMainWindow::~CMainWindow()
{
    ::KillTimer(hWnd, m_nTimerLanguageId);

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

    //m_pManager->StopSpellChecker();
    //RELEASEOBJECT(m_pManager);
}

LRESULT CALLBACK CMainWindow::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    CMainWindow *window = reinterpret_cast<CMainWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
    if ( !window )
        return DefWindowProc( hWnd, message, wParam, lParam );

    switch ( message )
    {
    case WM_HOTKEY:
        qDebug() << "key down";
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

        SetFocus( gWinId );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND:
    {
        if ( wParam == SC_KEYMENU )
        {
//            RECT winrect;
//            GetWindowRect( hWnd, &winrect );
//            TrackPopupMenu( GetSystemMenu( hWnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, hWnd, NULL);
//            break;
            return 0;
        } else
        if (wParam == SC_MAXIMIZE) {
            qDebug() << "wm syscommand";
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

        window->m_pWinPanel->focus();
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
//        qDebug() << "WM_CLOSE action";
        window->m_pWinPanel->doClose();
        return 0;

    case WM_DESTROY:
    {
//        PostQuitMessage(0);
        break;
    }

    case WM_TIMER:
    {
        if (NULL != window->m_pManager)
            window->m_pManager->CheckKeyboard();
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
        if (window->m_pWinPanel) {
            RECT lpWindowRect, clientRect;
            GetWindowRect(hWnd, &lpWindowRect);
            GetClientRect(hWnd, &clientRect);

            int border_size = 0;

            int nMaxOffsetX = 0;
            int nMaxOffsetY = 0;
            int nMaxOffsetR = 0;
            int nMaxOffsetB = 0;

            if ( wParam == SIZE_MAXIMIZED ) {
                LONG lTestW = 640;
                LONG lTestH = 480;
                RECT wrect{0,0,lTestW,lTestH};
                AdjustWindowRectEx(&wrect, (GetWindowStyle(hWnd) & ~WS_DLGFRAME), FALSE, 0);

                if (0 > wrect.left) nMaxOffsetX = -wrect.left;
                if (0 > wrect.top)  nMaxOffsetY = -wrect.top;

                if (wrect.right > lTestW)   nMaxOffsetR = (wrect.right - lTestW);
                if (wrect.bottom > lTestH)  nMaxOffsetB = (wrect.bottom - lTestH);

                // TODO: вот тут бордер!!!
                window->m_pWinPanel->setGeometry( nMaxOffsetX + border_size, nMaxOffsetY + border_size,
                                                    clientRect.right - (nMaxOffsetX + nMaxOffsetR + 2 * border_size),
                                                    clientRect.bottom - (nMaxOffsetY + nMaxOffsetB + 2 * border_size));
                window->m_pWinPanel->applyWindowState(Qt::WindowMaximized);
            } else {
                border_size = 3 * g_dpi_ratio;

                // TODO: вот тут бордер!!!
                window->m_pWinPanel->setGeometry(border_size, border_size,
                                clientRect.right - 2 * border_size, clientRect.bottom - 2 * border_size);
                window->m_pWinPanel->applyWindowState(Qt::WindowNoState);
            }

            HRGN hRgn = CreateRectRgn(nMaxOffsetX, nMaxOffsetY,
                                lpWindowRect.right - lpWindowRect.left - nMaxOffsetX,
                                lpWindowRect.bottom - lpWindowRect.top - nMaxOffsetY);

            SetWindowRgn(hWnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
        break;

    case WM_NCACTIVATE: {
        return TRUE;
        break;
    }

    case WM_PAINT:
        break;

    case WM_ERASEBKGND: {
        RECT rect;
        GetClientRect(hWnd, &rect);

        HBRUSH hBrush = CreateSolidBrush(RGB(49, 52, 55));
        FillRect((HDC)wParam, &rect, (HBRUSH)hBrush);
        DeleteObject(hBrush);
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
        window->m_pManager->DestroyCefView(-1);
        break;

    case WM_WINDOWPOSCHANGING: {
        break; }
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
                    long(Style::aero_borderless) : long(Style::windowed)/* & ~WS_CAPTION*/;

        SetWindowLongPtr( hWnd, GWL_STYLE, newStyle );

        borderless = !borderless;
        if ( !borderless ) {
            toggleShadow();
        }

        //redraw frame
        SetWindowPos( hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE /*| SWP_NOZORDER | SWP_NOOWNERZORDER */);
        show(showmax);
    }
}

void CMainWindow::toggleShadow()
{
    if ( borderless )
    {
        aeroShadow = !aeroShadow;
//        const MARGINS shadow_on = { 1, 1, 1, 1 };
//        const MARGINS shadow_off = { 0, 0, 0, 0 };
//        DwmExtendFrameIntoClientArea( hWnd, ( aeroShadow ) ? ( &shadow_on ) : ( &shadow_off ) );
    }
}

void CMainWindow::toggleResizeable()
{
    borderlessResizeable = borderlessResizeable ? false : true;
}

bool CMainWindow::isResizeable()
{
    return borderlessResizeable ? true : false;
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

int CMainWindow::getMinimumWidth()
{
    return minimumSize.width;
}

int CMainWindow::getMinimumHeight()
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

