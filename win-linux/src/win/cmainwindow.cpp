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

#include "win/cmainwindow.h"
#include <stdexcept>
#include <functional>

#include <QtWidgets/QPushButton>
#include <QFile>
#include <QPixmap>
#include <QScreen>

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

CMainWindow::CMainWindow(const QRect &rect, bool singleMode) :
    QMainWindow(nullptr),
    hWnd(nullptr),
    m_singleMode(singleMode),
    m_borderWidth(5),
    m_margins(QMargins()),
    m_frames(QMargins()),
    m_isJustMaximized(false),
    m_isResizeable(true),
    m_taskBarClicked(false),
    m_previousState(Qt::WindowNoState),
    closed( false ),
    visible( false ),
    borderless( true ),
    borderlessResizeable( true ),
    m_windowActivated(false)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setResizeable(m_isResizeable);

    _window_rect = rect;
    m_dpiRatio = CSplash::startupDpiRatio();

    if ( _window_rect.isEmpty() )
        _window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);

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

    QIcon icon = Utils::appIcon();
    setWindowIcon(icon);
    m_moveNormalRect = _window_rect;

    QWidget *_pCentralWidget = new QWidget(this);
    _pCentralWidget->setProperty("handleTopWindow", (LONG_PTR)_pCentralWidget->winId());
    setCentralWidget(_pCentralWidget);
    _pCentralWidget->setStyleSheet("background-color: transparent");
    QGridLayout *_pMainLayout = new QGridLayout(_pCentralWidget);
    _pMainLayout->setSpacing(0);
    _pMainLayout->setContentsMargins(0,0,0,0);
    _pCentralWidget->setLayout(_pMainLayout);

    m_pMainPanel = new CMainPanelImpl(_pCentralWidget, true, m_dpiRatio);
    _pMainLayout->addWidget(m_pMainPanel, 0, 0);
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);
    m_pMainPanel->goStart();

    CMainPanel * mainpanel = static_cast<CMainPanel*>(m_pMainPanel);
    QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, bind(&CMainWindow::slot_windowChangeState, this, _1));
    QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, std::bind(&CMainWindow::slot_windowClose, this));
    QObject::connect(mainpanel, &CMainPanel::mainPageReady, std::bind(&CMainWindow::slot_mainPageReady, this));
    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    hWnd = (HWND)winId();
}

CMainWindow::~CMainWindow()
{
    closed = true;

    /*if ( isVisible() ) {
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
    DestroyWindow( hWnd );*/
}

void CMainWindow::setResizeable(bool isResizeable)
{
    bool visible = isVisible();
    m_isResizeable = isResizeable;
    hWnd = (HWND)winId();
    DWORD style = ::GetWindowLong(hWnd, GWL_STYLE);
    if (m_isResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hWnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hWnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hWnd, &shadow);
    setVisible(visible);
}

void CMainWindow::setResizeableAreaWidth(int width)
{
    m_borderWidth = (width < 0) ? 0 : width;
}

void CMainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    QMainWindow::setContentsMargins(left + m_frames.left(),
                                    top + m_frames.top(),
                                    right + m_frames.right(),
                                    bottom + m_frames.bottom());
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}

bool CMainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_ACTIVATE: {
        if (!m_singleMode) {
            if (LOWORD(msg->wParam) != WA_INACTIVE) {
                WindowHelper::correctModalOrder(hWnd, m_modalHwnd);
                return false;
            }
        } else {
            static bool is_mainwindow_prev;
            is_mainwindow_prev = false;
            if (!IsWindowEnabled(hWnd) && m_modalHwnd && m_modalHwnd != hWnd) {
                if (LOWORD(msg->wParam) != WA_INACTIVE ) {
                    SetActiveWindow(m_modalHwnd);
                    SetWindowPos(hWnd, m_modalHwnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                    return 0;
                }
            } else {
                if ( LOWORD(msg->wParam) != WA_INACTIVE ) {
                    static HWND top_window;
                    top_window = NULL;
                    EnumWindows([](HWND hw, LPARAM lp) {
                        if (!IsWindowVisible(hw) || GetWindowTextLength(hw) == 0) {
                            return TRUE;
                        }
                        if (hw == (HWND)lp) {
                            top_window = hw;
                        } else
                        if (top_window) {
                            top_window = NULL;
                            if (AscAppManager::mainWindow() && hw == AscAppManager::mainWindow()->handle())
                                is_mainwindow_prev = true;
                        }
                        return TRUE;
                    }, (LPARAM)hWnd);
                }
            }
        }
        break;
    }

    case WM_DPICHANGED: {
        if (!WindowHelper::isLeftButtonPressed()) {
            double dpi_ratio = Utils::getScreenDpiRatioByHWND(LONG_PTR(hWnd));
            if (dpi_ratio != m_dpiRatio) {
                if (!m_singleMode) {
                    m_dpiRatio = dpi_ratio;
                    refresh_window_scaling_factor(this);
                    adjustGeometry();
                } else {
                    //onDpiChanged(dpi_ratio, m_dpiRatio);
                }
            }
        }
        qDebug() << "WM_DPICHANGED: " << LOWORD(msg->wParam);
        break;
    }

    case WM_SYSKEYDOWN: {
        if (msg->wParam == VK_SPACE) {
            RECT winrect;
            GetWindowRect(msg->hwnd, &winrect);
            TrackPopupMenu(GetSystemMenu(msg->hwnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, msg->hwnd, NULL);
        }
        break;
    }

    case WM_KEYDOWN: {
        if (msg->wParam == VK_F5 || msg->wParam == VK_F6 || msg->wParam == VK_F7) {
            SendMessage(msg->hwnd, WM_KEYDOWN, msg->wParam, msg->lParam);
        } else
        if (msg->wParam == VK_TAB) {
            SetFocus(HWND(winId()));
        }
        break;
    }

    case WM_SYSCOMMAND: {
        if (GET_SC_WPARAM(msg->wParam) == SC_KEYMENU) {
            return false;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_RESTORE) {
            m_taskBarClicked = true;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MINIMIZE) {
            m_taskBarClicked = true;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_SIZE) {
            break;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MOVE) {
            break;
        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MAXIMIZE) {
            break;
        }
        break;
    }

    case WM_NCCALCSIZE: {
        NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
        if (params.rgrc[0].top != 0) params.rgrc[0].top -= 1;
        Qt::WindowStates _currentState = windowState();
        if ((m_previousState == Qt::WindowNoState && _currentState == Qt::WindowNoState)
                && !m_taskBarClicked) {
            *result = WVR_REDRAW;
        } else m_taskBarClicked = false;
        m_previousState = _currentState;
        return true;
    }

    case WM_NCHITTEST: {
        *result = 0;
        const LONG border_width = (LONG)m_borderWidth;
        RECT winrect;
        GetWindowRect(HWND(winId()), &winrect);
        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        if (m_isResizeable) {
            bool resizeWidth = minimumWidth() != maximumWidth();
            bool resizeHeight = minimumHeight() != maximumHeight();

            if (resizeWidth) {
                if (x >= winrect.left && x < winrect.left + border_width) {
                    *result = HTLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width) {
                    *result = HTRIGHT;
                }
            }
            if (resizeHeight) {
                if (y < winrect.bottom && y >= winrect.bottom - border_width) {
                    *result = HTBOTTOM;
                }
                if (y >= winrect.top && y < winrect.top + border_width) {
                    *result = HTTOP;
                }
            }
            if (resizeWidth && resizeHeight) {
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width) {
                    *result = HTBOTTOMLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y < winrect.bottom && y >= winrect.bottom - border_width) {
                    *result = HTBOTTOMRIGHT;
                }
                if (x >= winrect.left && x < winrect.left + border_width &&
                        y >= winrect.top && y < winrect.top + border_width) {
                    *result = HTTOPLEFT;
                }
                if (x < winrect.right && x >= winrect.right - border_width &&
                        y >= winrect.top && y < winrect.top + border_width) {
                    *result = HTTOPRIGHT;
                }
            }
        }
        if (*result != 0) return true;
        return false;
    }

    case WM_GETMINMAXINFO: {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            double dpr = this->devicePixelRatioF();
            m_frames.setLeft(int(double(abs(frame.left))/dpr + 0.5));
            m_frames.setTop(int(double(abs(frame.bottom))/dpr + 0.5));
            m_frames.setRight(int(double(abs(frame.right))/dpr + 0.5));
            m_frames.setBottom(int(double(abs(frame.bottom))/dpr + 0.5));
            QMainWindow::setContentsMargins(m_frames.left() + m_margins.left(),
                                            m_frames.top() + m_margins.top(),
                                            m_frames.right() + m_margins.right(),
                                            m_frames.bottom() + m_margins.bottom());
            m_isJustMaximized = true;
        } else {
            if (m_isJustMaximized) {
                QMainWindow::setContentsMargins(m_margins);
                m_frames = QMargins();
                m_isJustMaximized = false;
            }
        }
        return false;
    }

    case WM_MOVING: {
        if (m_singleMode)
            onMoveEvent(QRect());
        break;
    }

    case WM_SIZE: {
        if (m_singleMode)
            onSizeEvent(msg->wParam);
        break;
    }

    /*case WM_ERASEBKGND:
        return true;

    case WM_NCACTIVATE:
        return true;

    case WM_NCPAINT:
        return false;*/

    case WM_SETFOCUS: {
        if (!closed) {
            if (m_singleMode) {
                //focus();
            } else {
                if (IsWindowEnabled(hWnd))
                    m_pMainPanel->focus();
            }
        }
        break;
    }

    case WM_CLOSE: {
        if (!m_singleMode) {
            AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(this)});
        } else {
            if (m_pMainPanel) {
                QTimer::singleShot(0, m_pMainPanel, [=]() {
                    AscAppManager::getInstance().closeQueue().enter(sWinTag{2, size_t(this)});
                });
            } else return true;
        }
        return false;
    }

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    default:
        break;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void CMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(_window_rect);
        int border_size = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border_size, border_size + 1, border_size, border_size);
        COLORREF color = AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBackground);
        setStyleSheet(QString("background-color: rgb(%1,%2,%3)").arg(QString::number(GetRValue(color)),
                                                                     QString::number(GetGValue(color)),
                                                                     QString::number(GetBValue(color))));
    }
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    qDebug() << "Close MainWindow...";
    if (isVisible()) {
        GET_REGISTRY_USER(reg_user)
        if (isMaximized()) {
            reg_user.setValue("maximized", true);
        } else {
            reg_user.remove("maximized");
            reg_user.setValue("position", geometry());
        }
    }
    hide();
    event->accept();
}

void CMainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            m_pMainPanel->applyMainWindowState(Qt::WindowMinimized);
        } else {
            if (isVisible()) {
                if (WindowHelper::isLeftButtonPressed()) {
                    double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));
                    if (dpi_ratio != m_dpiRatio)
                        setScreenScalingFactor(dpi_ratio);
                }
                if (isMaximized()) {
                    m_pMainPanel->applyMainWindowState(Qt::WindowMaximized);
                } else {
                    m_pMainPanel->applyMainWindowState(Qt::WindowNoState);
                }
            }
            adjustGeometry();
        }
    }
}

/*LRESULT CALLBACK CMainWindow::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    CMainWindow *window = reinterpret_cast<CMainWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
    switch ( message )
    {
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

    case WM_ENTERSIZEMOVE: {
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
        window->updateScaling();
#endif

        break;
    }

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
*/

void CMainWindow::toggleBorderless(bool showmax)
{
    if ( visible )
    {
        borderless = !borderless;
        show(showmax);
    }
}

void CMainWindow::toggleResizeable()
{
    borderlessResizeable = borderlessResizeable ? false : true;
}

void CMainWindow::show(bool maximized)
{
    maximized ? QMainWindow::showMaximized() : QMainWindow::show();
    visible = true;
}

void CMainWindow::hide()
{
    QMainWindow::hide();
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
    /*RECT lpWindowRect, clientRect;
    GetWindowRect(hWnd, &lpWindowRect);
    GetClientRect(hWnd, &clientRect);*/

    int border_size = 0;
        /*nMaxOffsetX = 0,
        nMaxOffsetY = 0,
        nMaxOffsetR = 0,
        nMaxOffsetB = 0;*/

    //if ( IsZoomed(hWnd) != 0 ) {      // is window maximized
        /*LONG lTestW = 640,
             lTestH = 480;

        QScreen * _screen = Utils::screenAt(QRect(QPoint(lpWindowRect.left, lpWindowRect.top),QPoint(lpWindowRect.right,lpWindowRect.bottom)).center());
        double _screen_dpi_ratio = _screen->logicalDotsPerInch() / 96;

        RECT wrect{0,0,lTestW,lTestH};
        WindowHelper::adjustWindowRect(hWnd, _screen_dpi_ratio, &wrect);

        if (0 > wrect.left) nMaxOffsetX = -wrect.left;
        if (0 > wrect.top)  nMaxOffsetY = -wrect.top;

        if (wrect.right > lTestW)   nMaxOffsetR = (wrect.right - lTestW);
        if (wrect.bottom > lTestH)  nMaxOffsetB = (wrect.bottom - lTestH);*/

    //} else {
        border_size = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border_size,border_size + 1,border_size,border_size);
    //}
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
    switch (s) {
    case Qt::WindowMaximized:  showMaximized(); break;
    case Qt::WindowMinimized:  showMinimized(); break;
    case Qt::WindowFullScreen: hide(); break;
    case Qt::WindowNoState:
    default: showNormal(); break;
    }
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

/*bool CMainWindow::isMaximized() const
{
    bool _is_maximized = false;

    WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
    if ( GetWindowPlacement(hWnd, &wp) ) {
        _is_maximized = wp.showCmd == SW_MAXIMIZE;
    }

    return _is_maximized;
}*/

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
    COLORREF color = AscAppManager::themes().current().colorRef(CTheme::ColorRole::ecrWindowBackground);
    setStyleSheet(QString("background-color: rgb(%1,%2,%3)").arg(QString::number(GetRValue(color)),
                                                                 QString::number(GetGValue(color)),
                                                                 QString::number(GetBValue(color))));
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
