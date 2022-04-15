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
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
//#include "defines_p.h"
#include "utils.h"
#include "csplash.h"
//#include "clogger.h"
#include "clangater.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>


#ifdef _UPDMODULE
  #include "3dparty/WinSparkle/include/winsparkle.h"
  #include "../version.h"
#endif

#define CAPTURED_WINDOW_CURSOR_OFFSET_X     180
#define CAPTURED_WINDOW_CURSOR_OFFSET_Y     15

using namespace std::placeholders;

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

auto refresh_window_scaling_factor(CMainWindow * window) -> void {
    QString css{AscAppManager::getWindowStylesheets(window->m_dpiRatio)};

    if ( !css.isEmpty() ) {
        window->mainPanel()->setStyleSheet(css);
        window->mainPanel()->setScreenScalingFactor(window->m_dpiRatio);
    }
}

CMainWindow::CMainWindow(const QRect &rect) :
    CMainWindow(rect, WindowType::MAIN, QString(), nullptr)
{}

CMainWindow::CMainWindow(const QRect &rect, const QString &title, QWidget *panel) :
    CMainWindow(rect, WindowType::SINGLE, title, panel)
{}

CMainWindow::CMainWindow(const QRect &rect, const QString &title, QCefView *view) :
    CMainWindow(rect, WindowType::REPORTER, title, static_cast<QWidget*>(view))
{}

CMainWindow::CMainWindow(const QRect &rect, const WindowType winType, const QString &title, QWidget *widget) :
    CMainWindowBase(const_cast<QRect&>(rect)),
    QMainWindow(nullptr),
    m_winType(winType),
    m_previousState(Qt::WindowNoState),
    m_margins(QMargins()),
    m_frame(QMargins()),
    m_hWnd(nullptr),
    m_modalHwnd(nullptr),
    m_borderWidth(MAIN_WINDOW_BORDER_WIDTH),
    m_borderless(true),
    m_visible(false),
    m_closed(false),
    m_skipSizing(false),
    m_isMaximized(false),
    m_isResizeable(true),
    m_taskBarClicked(false),
    m_windowActivated(false)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setWindowIcon(Utils::appIcon());
    m_hWnd = (HWND)winId();
    setResizeable(m_isResizeable);

    QWidget *m_pCentralWidget = new QWidget(this);
    m_pCentralWidget->setProperty("handleTopWindow", (LONG_PTR)m_pCentralWidget->winId());
    setCentralWidget(m_pCentralWidget);
    m_pCentralWidget->setStyleSheet("background-color: transparent");
    QGridLayout *m_pCentralLayout = new QGridLayout(m_pCentralWidget);
    m_pCentralLayout->setSpacing(0);
    m_pCentralLayout->setContentsMargins(0,0,0,0);
    m_pCentralWidget->setLayout(m_pCentralLayout);

    m_window_rect = rect;   
    if (m_winType == WindowType::MAIN) {
        m_dpiRatio = CSplash::startupDpiRatio();
        if (m_window_rect.isEmpty())
            m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
        QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
        if (_screen_size.intersects(m_window_rect)) {
            if (_screen_size.width() < m_window_rect.width() || _screen_size.height() < m_window_rect.height()) {
                m_window_rect.setLeft(_screen_size.left()),
                m_window_rect.setTop(_screen_size.top());

                if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
                if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
            }
        } else {
            m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
        }
        m_moveNormalRect = m_window_rect;

        _m_pMainPanel = new CMainPanelImpl(m_pCentralWidget, true, m_dpiRatio);
        m_pCentralLayout->addWidget(_m_pMainPanel, 0, 0);
        _m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
        _m_pMainPanel->updateScaling(m_dpiRatio);
        _m_pMainPanel->goStart();

        CMainPanel * mainpanel = static_cast<CMainPanel*>(_m_pMainPanel);
        QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, bind(&CMainWindow::slot_windowChangeState, this, _1));
        QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, std::bind(&CMainWindow::slot_windowClose, this));
        QObject::connect(mainpanel, &CMainPanel::mainPageReady, std::bind(&CMainWindow::slot_mainPageReady, this));
        QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, bind(&CMainWindow::slot_modalDialog, this, _1, _2));

    } else
    if (m_winType == WindowType::SINGLE) {
        m_dpiRatio = CSplash::startupDpiRatio();
        m_modalSlotConnection = QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog,
                                         bind(&CMainWindow::slot_modalDialog, this, std::placeholders::_1, std::placeholders::_2));
    } else
    if (m_winType == WindowType::REPORTER) {
        m_borderless = false;
        m_dpiRatio = Utils::getScreenDpiRatio(m_window_rect.topLeft());

        if (m_window_rect.isEmpty())
            m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
        QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
        if (_screen_size.width() < m_window_rect.width() + 120 || _screen_size.height() < m_window_rect.height() + 120) {
            m_window_rect.setLeft(_screen_size.left()),
            m_window_rect.setTop(_screen_size.top());

            if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
            if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
        }

        setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
        m_pMainPanel = createMainPanel(m_pCentralWidget, title, true, widget);
    }
}

CMainWindow::~CMainWindow()
{
    if (m_winType == WindowType::SINGLE) {
        QObject::disconnect(m_modalSlotConnection);
    }
    m_closed = true;

    //DestroyWindow( m_hWnd );
}

void CMainWindow::setResizeable(bool isResizeable)
{
    bool visible = isVisible();
    m_isResizeable = isResizeable;
    DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    if (m_isResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(m_hWnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(m_hWnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hWnd, &shadow);
    setVisible(visible);
}

void CMainWindow::setResizeableAreaWidth(int width)
{
    m_borderWidth = (width < 0) ? 0 : width;
}

void CMainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    QMainWindow::setContentsMargins(left + m_frame.left(),
                                    top + m_frame.top(),
                                    right + m_frame.right(),
                                    bottom + m_frame.bottom());
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
        if (m_winType == WindowType::MAIN) {
            if (LOWORD(msg->wParam) != WA_INACTIVE) {
                WindowHelper::correctModalOrder(m_hWnd, m_modalHwnd);
                return false;
            }
        } else
        if (m_winType == WindowType::SINGLE) {
            //static bool is_mainwindow_prev;
            //is_mainwindow_prev = false;
            if (!IsWindowEnabled(m_hWnd) && m_modalHwnd && m_modalHwnd != m_hWnd) {
                if (LOWORD(msg->wParam) != WA_INACTIVE ) {
                    SetActiveWindow(m_modalHwnd);
                    SetWindowPos(m_hWnd, m_modalHwnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
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
                            //if (AscAppManager::mainWindow() && hw == AscAppManager::mainWindow()->handle())
                                //is_mainwindow_prev = true;
                        }
                        return TRUE;
                    }, (LPARAM)m_hWnd);
                }
            }
        }
        break;
    }

    case WM_DPICHANGED: {
        if (!WindowHelper::isLeftButtonPressed()) {
            double dpi_ratio = Utils::getScreenDpiRatioByHWND(LONG_PTR(m_hWnd));
            if (dpi_ratio != m_dpiRatio) {
                if (m_winType == WindowType::MAIN) {
                    m_dpiRatio = dpi_ratio;
                    refresh_window_scaling_factor(this);
                    adjustGeometry();
                } else
                if (m_winType == WindowType::SINGLE) {
                    onDpiChanged(dpi_ratio, m_dpiRatio);
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
            if (m_winType != WindowType::REPORTER) {
                SetFocus(HWND(winId()));
            } else {
                SetFocus(HWND(winId()));
            }
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
        if (m_borderless) {
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
        break;
    }

    case WM_GETMINMAXINFO: {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = {0, 0, 0, 0};
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            double dpr = devicePixelRatioF();
            m_frame.setLeft(int(double(abs(frame.left))/dpr + 0.5));
            m_frame.setTop(int(double(abs(frame.bottom))/dpr + 0.5));
            m_frame.setRight(int(double(abs(frame.right))/dpr + 0.5));
            m_frame.setBottom(int(double(abs(frame.bottom))/dpr + 0.5));
            QMainWindow::setContentsMargins(m_frame.left() + m_margins.left(),
                                            m_frame.top() + m_margins.top(),
                                            m_frame.right() + m_margins.right(),
                                            m_frame.bottom() + m_margins.bottom());
            m_isMaximized = true;
        } else {
            if (m_isMaximized) {
                QMainWindow::setContentsMargins(m_margins);
                m_frame = QMargins();
                m_isMaximized = false;
            }
        }
        MINMAXINFO* minMaxInfo = ( MINMAXINFO* )msg->lParam;
        if (m_minSize.required) {
            minMaxInfo->ptMinTrackSize.x = getMinimumWidth();
            minMaxInfo->ptMinTrackSize.y = getMinimumHeight();
        }
        if (m_maxSize.required) {
            minMaxInfo->ptMaxTrackSize.x = getMaximumWidth();
            minMaxInfo->ptMaxTrackSize.y = getMaximumHeight();
        }
        return true;
    }

    case WM_MOVING: {
        if (m_winType == WindowType::SINGLE) {
            onMoveEvent(geometry());
            return true;
        }
        break;
    }

    case WM_SIZE: {
        if (m_winType == WindowType::SINGLE)
            if (!m_skipSizing && !m_closed) {
                onSizeEvent(msg->wParam);
            }
        break;
    }

    /*case WM_ERASEBKGND:
        return true;

    case WM_NCACTIVATE:
        return true;

    case WM_NCPAINT:
        return false;*/

    case WM_SETFOCUS: {
        if (!m_closed) {
            if (m_winType == WindowType::MAIN) {
                if (IsWindowEnabled(m_hWnd))
                    _m_pMainPanel->focus();
            } else
            if (m_winType == WindowType::SINGLE) {
                focus();
            } else
            if (m_winType == WindowType::REPORTER) {
                focusMainPanel();
            }
        }
        break;
    }

    case WM_CLOSE: {
        if (m_winType == WindowType::MAIN) {
            AscAppManager::getInstance().closeQueue().enter(sWinTag{1, size_t(this)});
        } else
        if (m_winType == WindowType::SINGLE) {
            if (m_pMainPanel) {
                QTimer::singleShot(0, m_pMainPanel, [=]() {
                    AscAppManager::getInstance().closeQueue().enter(sWinTag{2, size_t(this)});
                });
            } else return true;
        } else
        if (m_winType == WindowType::REPORTER) {
            QTimer::singleShot(0, m_pMainPanel, [=] {
                onCloseEvent();
            });
        }
        return false;
    }

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    case WM_ENDSESSION:
        if (m_winType == WindowType::MAIN)
            CAscApplicationManagerWrapper::getInstance().CloseApplication();
        break;

    case UM_INSTALL_UPDATE:
        if (m_winType == WindowType::MAIN)
            doClose();
        break;

    case WM_ENTERSIZEMOVE: {
        if (m_winType != WindowType::REPORTER) {
            WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
            if (GetWindowPlacement(m_hWnd, &wp)) {
                MONITORINFO info{sizeof(MONITORINFO)};
                GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &info);
                m_moveNormalRect = QRect{QPoint{wp.rcNormalPosition.left - info.rcMonitor.left, wp.rcNormalPosition.top - info.rcMonitor.top},
                                         QSize{wp.rcNormalPosition.right - wp.rcNormalPosition.left, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top}};
            }
        }
        break;
    }

    case WM_EXITSIZEMOVE: {
        if (m_winType == WindowType::MAIN) {
            setMinimumSize(0, 0);
#if defined(DEBUG_SCALING) && defined(_DEBUG)
            QRect windowRect;
            WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
            if (GetWindowPlacement(m_hWnd, &wp)) {
                GET_REGISTRY_USER(reg_user)
                wp.showCmd == SW_MAXIMIZE ?
                            reg_user.setValue("maximized", true) : reg_user.remove("maximized");

                windowRect.setTopLeft(QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));
                windowRect.setBottomRight(QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
                windowRect.adjust(0,0,-1,-1);
            }

            int _scr_num = QApplication::desktop()->screenNumber(windowRect.topLeft()) + 1;
            uchar dpi_ratio = _scr_num;

            if ( dpi_ratio != m_dpiRatio ) {
                if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
                    setScreenScalingFactor(dpi_ratio);
                } else {
                    m_dpiRatio = dpi_ratio;
                    refresh_window_scaling_factor(this);
                }
                adjustGeometry();
            }
#else
            updateScaling();
#endif
        } else
        if (m_winType == WindowType::SINGLE) {
            onExitSizeMove();
            return false;
        } else
        if (m_winType == WindowType::REPORTER) {
            double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(m_hWnd));
            if (dpi_ratio != m_dpiRatio)
                setScreenScalingFactor(dpi_ratio);
        }
        break;
    }

    case WM_COPYDATA: {
        if (m_winType != WindowType::REPORTER) {
            COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)msg->lParam;
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
                LocalFree(szArglist);
                if (m_winType == WindowType::MAIN) {
                    ::SetFocus(m_hWnd);
                    bringToTop();
                }
            }
        }
        break;
    }

#if 0
    case WM_INPUTLANGCHANGE:
    case WM_INPUTLANGCHANGEREQUEST: {
        if (m_winType == WindowType::MAIN) {
            int _lang = LOWORD(msg->lParam);
            m_oLanguage.Check(_lang);
        }
        break;
    }
#endif

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
        setGeometry(m_window_rect);
        int border_size = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border_size, border_size + 1, border_size, border_size);
        if (m_winType == WindowType::MAIN || m_winType == WindowType::REPORTER) {
            QColor color = AscAppManager::themes().current().
                    color(CTheme::ColorRole::ecrWindowBackground);
            setWindowBackgroundColor(color);
        }
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
        if (m_winType == WindowType::MAIN) {
            if (isMinimized()) {
                _m_pMainPanel->applyMainWindowState(Qt::WindowMinimized);
            } else {
                if (isVisible()) {
                    if (WindowHelper::isLeftButtonPressed()) {
                        double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(m_hWnd));
                        if (dpi_ratio != m_dpiRatio)
                            setScreenScalingFactor(dpi_ratio);
                    }
                    if (isMaximized()) {
                        _m_pMainPanel->applyMainWindowState(Qt::WindowMaximized);
                    } else {
                        _m_pMainPanel->applyMainWindowState(Qt::WindowNoState);
                    }
                }
                adjustGeometry();
            }
        } else
        if (m_winType == WindowType::SINGLE || m_winType == WindowType::REPORTER) {
            if (isMinimized()) {
                applyWindowState(Qt::WindowMinimized);
            } else {
                if (isMaximized())
                    applyWindowState(Qt::WindowMaximized);
                else applyWindowState(Qt::WindowNoState);

                adjustGeometry();
            }
        }
    }
}

void CMainWindow::toggleBorderless(bool showmax)
{
    if (m_visible) {
        m_borderless = !m_borderless;
        show(showmax);
    }
}

void CMainWindow::toggleResizeable()
{
    m_isResizeable = !m_isResizeable;
}

void CMainWindow::show(bool maximized)
{
    maximized ? QMainWindow::showMaximized() : QMainWindow::show();
    m_visible = true;
}

void CMainWindow::hide()
{
    QMainWindow::hide();
    m_visible = false;
}

bool CMainWindow::isVisible()
{
    return m_visible;
}

// Minimum size
void CMainWindow::setMinimumSize( const int width, const int height )
{
    m_minSize.required = true;
    m_minSize.width = width;
    m_minSize.height = height;
}

bool CMainWindow::isSetMinimumSize()
{
    return m_minSize.required;
}
void CMainWindow::removeMinimumSize()
{
    m_minSize.required = false;
    m_minSize.width = 0;
    m_minSize.height = 0;
}

int CMainWindow::getMinimumWidth() const
{
    return m_minSize.width;
}

int CMainWindow::getMinimumHeight() const
{
    return m_minSize.height;
}

// Maximum size
void CMainWindow::setMaximumSize( const int width, const int height )
{
    m_maxSize.required = true;
    m_maxSize.width = width;
    m_maxSize.height = height;
}
bool CMainWindow::isSetMaximumSize()
{
    return m_maxSize.required;
}

void CMainWindow::removeMaximumSize()
{
    m_maxSize.required = false;
    m_maxSize.width = 0;
    m_maxSize.height = 0;
}
int CMainWindow::getMaximumWidth()
{
    return m_maxSize.width;
}
int CMainWindow::getMaximumHeight()
{
    return m_maxSize.height;
}

void CMainWindow::adjustGeometry()
{
    int border_size = 0;
    if (IsZoomed(m_hWnd) != FALSE) {      // is window maximized

    } else {
        border_size = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border_size,border_size + 1,border_size,border_size);
    }
}

void CMainWindow::setScreenScalingFactor(double factor)
{
    if (m_winType == WindowType::MAIN) {
        m_skipSizing = true;
        QString css(AscAppManager::getWindowStylesheets(factor));
        if ( !css.isEmpty() ) {
            double change_factor = factor / m_dpiRatio;
            m_dpiRatio = factor;
            _m_pMainPanel->setStyleSheet(css);
            _m_pMainPanel->setScreenScalingFactor(factor);

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
        }

        m_skipSizing = false;
    } else
    if (m_winType == WindowType::SINGLE) {
        double change_factor = factor / m_dpiRatio;
        CMainWindowBase::setScreenScalingFactor(factor);

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
    } else
    if (m_winType == WindowType::REPORTER) {
        QString css(AscAppManager::getWindowStylesheets(factor));
        if ( !css.isEmpty() ) {
            bool increase = factor > m_dpiRatio;
            m_dpiRatio = factor;
            m_pMainPanel->setStyleSheet(css);
            QSize small_btn_size(40*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);
            m_buttonMinimize->setFixedSize(small_btn_size);
            m_buttonMaximize->setFixedSize(small_btn_size);
            m_buttonClose->setFixedSize(small_btn_size);
            m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT*m_dpiRatio);
            QLayout * layoutBtns = m_boxTitleBtns->layout();
            layoutBtns->setSpacing(1*m_dpiRatio);
            setMinimumSize(WINDOW_MIN_WIDTH * factor, WINDOW_MIN_HEIGHT * factor);
            RECT lpWindowRect;
            GetWindowRect(m_hWnd, &lpWindowRect);
            unsigned _new_width = lpWindowRect.right - lpWindowRect.left,
                     _new_height = lpWindowRect.bottom - lpWindowRect.top;
            if ( increase )
                _new_width *= 2, _new_height *= 2;  else
                _new_width /= 2, _new_height /= 2;

            SetWindowPos(m_hWnd, NULL, 0, 0, _new_width, _new_height, SWP_NOMOVE | SWP_NOZORDER);
        }
    }
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
    if (h != m_hWnd) {
        setEnabled(status ? false : true);
        m_modalHwnd = h;
    } else {
        m_modalHwnd = nullptr;
        if (!status && IsWindowEnabled(m_hWnd) && m_winType == WindowType::MAIN)
            _m_pMainPanel->focus();
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
    QTimer::singleShot(500, _m_pMainPanel, [=]{
        _m_pMainPanel->pushButtonCloseClicked();
    });
}

CMainPanel * CMainWindow::mainPanel() const
{
    return _m_pMainPanel;
}

QRect CMainWindow::windowRect() const
{
    QRect _win_rect;
    QPoint _top_left;

    WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
    if ( GetWindowPlacement(m_hWnd, &wp) ) {
        _top_left = QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
        _win_rect = QRect( _top_left, QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom));
    }

    return _win_rect;
}

HWND CMainWindow::handle() const
{
    return m_hWnd;
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
        //SendMessage(m_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(gpt.x(), gpt.y()));
      
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

void CMainWindow::bringToTop()
{
    if (IsIconic(m_hWnd) && m_winType == WindowType::MAIN) {
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
    }
    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
    SetActiveWindow(m_hWnd);
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    QColor color = AscAppManager::themes().current().
            color(CTheme::ColorRole::ecrWindowBackground);
    setWindowBackgroundColor(color);
    if (m_winType == WindowType::MAIN || m_winType == WindowType::SINGLE) {
        mainPanel()->applyTheme(theme);
    }
    if (m_winType == WindowType::REPORTER) {
        m_pMainPanel->setProperty("uitheme", QString::fromStdWString(theme));
        m_labelTitle->style()->polish(m_labelTitle);
        m_buttonMinimize->style()->polish(m_buttonMinimize);
        m_buttonMaximize->style()->polish(m_buttonMaximize);
        m_buttonClose->style()->polish(m_buttonClose);
        m_boxTitleBtns->style()->polish(m_boxTitleBtns);
        m_pMainPanel->style()->polish(m_pMainPanel);
        update();
    }
}

void CMainWindow::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(m_hWnd));
    if ( dpi_ratio != m_dpiRatio ) {
        if ( !WindowHelper::isWindowSystemDocked(m_hWnd) ) {
            setScreenScalingFactor(dpi_ratio);
        } else {
            m_dpiRatio = dpi_ratio;
            refresh_window_scaling_factor(this);
        }
        adjustGeometry();
    }
}

WindowBase::CWindowGeometry const& CMainWindow::minimumSize() const
{
    return m_minSize;
}

WindowBase::CWindowGeometry const& CMainWindow::maximumSize() const
{
    return m_maxSize;
}

void CMainWindow::applyWindowState(Qt::WindowState s)
{
    m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
    m_buttonMaximize->style()->polish(m_buttonMaximize);
}

void CMainWindow::onExitSizeMove()
{
    setMinimumSize(0, 0);
    double dpi_ratio = Utils::getScreenDpiRatioByHWND(int(m_hWnd));
    if ( dpi_ratio != m_dpiRatio ) {
        if ( WindowHelper::isWindowSystemDocked(m_hWnd) )
            onDpiChanged(dpi_ratio, m_dpiRatio);
        else setScreenScalingFactor(dpi_ratio);
    }
}

void CMainWindow::onMinimizeEvent()
{
    QMainWindow::showMinimized();
}

void CMainWindow::onMaximizeEvent()
{
    if (m_winType == WindowType::REPORTER) {
        bool _is_maximized = isMaximized();
#ifdef __linux__
            layout()->setMargin(s == _is_maximized ? 0 : CX11Decoration::customWindowBorderWith());
#endif
        m_buttonMaximize->setProperty("class", _is_maximized ? "min" : "normal");
        m_buttonMaximize->style()->polish(m_buttonMaximize);
    }
    isMaximized() ? QMainWindow::showNormal() : QMainWindow::showMaximized();
}

void CMainWindow::onCloseEvent() // Reporter mode
{
    if (m_pMainView) {
        AscAppManager::getInstance().DestroyCefView(((QCefView *)m_pMainView)->GetCefView()->GetId() );
    }
}

void CMainWindow::setWindowState(Qt::WindowState state)
{
    switch (state) {
    case Qt::WindowMaximized:  showMaximized(); break;
    case Qt::WindowMinimized:  showMinimized(); break;
    case Qt::WindowFullScreen: hide(); break;
    case Qt::WindowNoState:
    default: showNormal(); break;}
}

void CMainWindow::setWindowTitle(const QString& title)
{
    CMainWindowBase::setWindowTitle(title);
    //SetWindowText(m_hWnd, title.toStdWString().c_str());
    QMainWindow::setWindowTitle(title);
}

void CMainWindow::setWindowBackgroundColor(const QColor& color)
{
    int r, g, b;
    color.getRgb(&r, &g, &b);
    setStyleSheet(QString("background-color: rgb(%1,%2,%3)")
                  .arg(QString::number(r), QString::number(g), QString::number(b)));
}

void CMainWindow::setWindowColors(const QColor& background, const QColor& border)
{
    int r, g, b;
    border.getRgb(&r, &g, &b);
    //COLORREF m_borderColor = RGB(r, g, b);
    background.getRgb(&r, &g, &b);
    setStyleSheet(QString("background-color: rgb(%1,%2,%3)")
                  .arg(QString::number(r), QString::number(g), QString::number(b)));
}

void CMainWindow::activateWindow()
{
    //SetActiveWindow(m_hWnd);
    QMainWindow::activateWindow();
}

void CMainWindow::captureMouse()
{
    POINT cursor{0,0};
    if ( GetCursorPos(&cursor) ) {
        QRect _g{geometry()};

        int _window_offset_x;
        if ( cursor.x - _g.x() < dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X) ) _window_offset_x = dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_X);
        else if ( cursor.x > _g.right() - dpiCorrectValue(150) ) _window_offset_x = _g.right() - dpiCorrectValue(150);
        else _window_offset_x = cursor.x - _g.x();

        //SetWindowPos(m_hWnd, nullptr, cursor.x - _window_offset_x, cursor.y - dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_Y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        move(cursor.x - _window_offset_x, cursor.y - dpiCorrectValue(CAPTURED_WINDOW_CURSOR_OFFSET_Y));

        ReleaseCapture();
        PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cursor.x, cursor.y));
    }
}

bool CMainWindow::holdView(int id) const
{
    if (m_winType == WindowType::REPORTER) {
        return ((QCefView *)m_pMainView)->GetCefView()->GetId() == id;
    } else {
        return mainPanel()->holdUid(id);
    }
}

void CMainWindow::focusMainPanel()
{
    if (m_pMainView)
        ((QCefView *)m_pMainView)->setFocusToCef();
}

void CMainWindow::onDpiChanged(double newfactor, double prevfactor)
{
    Q_UNUSED(prevfactor)
    setScreenScalingFactor(newfactor);
}
