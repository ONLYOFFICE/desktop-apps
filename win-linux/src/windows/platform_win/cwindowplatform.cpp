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

#include "windows/platform_win/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QTimer>
#include <QDesktopWidget>
#include <QWindow>
#include <QScreen>
#include <QJsonObject>
#include <shellapi.h>

#define UM_SNAPPING 0x02


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    m_hWnd(nullptr),
    m_resAreaWidth(MAIN_WINDOW_BORDER_WIDTH),
    m_borderless(true),
    m_closed(false),
    m_isResizeable(true),
    m_allowMaximize(true)
{
    if (AscAppManager::isRtlEnabled())
        setLayoutDirection(Qt::RightToLeft);
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint
                   | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint
                   |Qt::WindowMinimizeButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    m_hWnd = (HWND)winId();
    LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
    style |= (WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    style |= (Utils::getWinVersion() > Utils::WinVer::Win7) ? WS_OVERLAPPEDWINDOW : WS_POPUP;
    ::SetWindowLong(m_hWnd, GWL_STYLE, style);
    connect(this->window()->windowHandle(), &QWindow::screenChanged, this, [=]() {
        SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    });

    setProperty("stabilized", true);
    m_propertyTimer = new QTimer(this);
    m_propertyTimer->setSingleShot(true);
    m_propertyTimer->setInterval(100);
    connect(m_propertyTimer, &QTimer::timeout, this, [=]() {
        setProperty("stabilized", true);
    });
}

CWindowPlatform::~CWindowPlatform()
{
    m_closed = true;
}

/** Public **/

void CWindowPlatform::toggleBorderless(bool showmax)
{
    if (isVisible()) {
        m_borderless = !m_borderless;
        show(showmax);
    }
}

void CWindowPlatform::toggleResizeable()
{
    m_isResizeable = !m_isResizeable;
}

void CWindowPlatform::bringToTop()
{
    if (IsIconic(m_hWnd)) {
        ShowWindow(m_hWnd, SW_RESTORE);
    }
    WindowHelper::bringToTop(m_hWnd);
}

void CWindowPlatform::show(bool maximized)
{
    maximized ? CWindowBase::showMaximized() : CWindowBase::show();
}

void CWindowPlatform::adjustGeometry()
{
    if (windowState().testFlag(Qt::WindowMinimized) || windowState().testFlag(Qt::WindowNoState)) {
        const int border = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border, border, border, border+1);
        setResizeableAreaWidth(border);
    } else
    if (windowState().testFlag(Qt::WindowMaximized)) {
        QTimer::singleShot(25, this, [=]() {
            auto rc = QApplication::desktop()->availableGeometry(this);
            const QSize offset(0, !isTaskbarAutoHideOn() ? 0 : 2);
            SetWindowPos(m_hWnd, NULL, rc.x(), rc.y(), rc.width(), rc.height() - offset.height(),
                         SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);

#ifdef __OS_WIN_XP
            setContentsMargins(0, 0, 0, 0);
#else
            int border = 0;
            if (!isTaskbarAutoHideOn() && Utils::getWinVersion() > Utils::WinVer::Win7) {
                double dpi = qApp->screenAt(geometry().center())->logicalDotsPerInch()/96;
                border = (dpi <= 1.0) ? 8 :
                         (dpi == 1.25) ? 9 :
                         (dpi == 1.5) ? 11 :
                         (dpi == 1.75) ? 12 :
                         (dpi == 2.0) ? 13 :
                         (dpi == 2.25) ? 14 :
                         (dpi == 2.5) ? 16 : 6 * dpi;
            }
            setContentsMargins(border, border, border, border);
#endif
        });
    }
}

/** Protected **/

bool CWindowPlatform::isSessionInProgress()
{
    return m_isSessionInProgress;
}

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (m_pMainPanel) {
            m_pMainPanel->setProperty("rtl", AscAppManager::isRtlEnabled());
            onLayoutDirectionChanged();
        }
    }
    return CWindowBase::event(event);
}

/** Private **/

bool CWindowPlatform::isTaskbarAutoHideOn()
{
    APPBARDATA ABData;
    ABData.cbSize = sizeof(ABData);
    return (SHAppBarMessage(ABM_GETSTATE, &ABData) & ABS_AUTOHIDE) != 0;
}

void CWindowPlatform::setResizeableAreaWidth(int width)
{
    m_resAreaWidth = (width < 0) ? 0 : width;
}

void CWindowPlatform::changeEvent(QEvent *event)
{
    CWindowBase::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        applyWindowState();
        adjustGeometry();
    }
}

bool CWindowPlatform::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    static uchar movParam = 0;
    switch (msg->message)
    {
    case WM_ACTIVATE: {
#ifndef __OS_WIN_XP
        MARGINS mrg;
        mrg.cxLeftWidth = 4;
        mrg.cxRightWidth = 4;
        mrg.cyBottomHeight = 4;
        mrg.cyTopHeight = 29;
        DwmExtendFrameIntoClientArea(m_hWnd, &mrg);
#endif
        break;
    }

    case WM_DPICHANGED: {
        setMinimumSize(0,0);
        if (AscAppManager::IsUseSystemScaling()) {
            if (WindowHelper::isLeftButtonPressed() || (m_scaleChanged && !isMaximized())) {
                RECT *prefRect = (RECT*)msg->lParam;
                setGeometry(prefRect->left, prefRect->top, prefRect->right - prefRect->left, prefRect->bottom - prefRect->top);
            }
            QTimer::singleShot(0, this, [=]() {
                updateScaling(false);
            });
        } else
        if (m_scaleChanged && !isMaximized()) {
            RECT *prefRect = (RECT*)msg->lParam;
            setGeometry(prefRect->left, prefRect->top, prefRect->right - prefRect->left, prefRect->bottom - prefRect->top);
        }
        m_scaleChanged = false;
        break;
    }

    case WM_DISPLAYCHANGE: {
        m_scaleChanged = true;
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

    case WM_NCCALCSIZE: {
        if (!msg->wParam)
            break;
        NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)msg->lParam;
        params->rgrc[0].bottom += 1;
        *result = WVR_ALIGNLEFT | WVR_ALIGNTOP | WVR_REDRAW;
        return true;
    }

    case WM_NCHITTEST: {
        if (m_borderless) {
            *result = 0;
            const LONG border = (LONG)m_resAreaWidth;
            RECT rect;
            GetWindowRect(msg->hwnd, &rect);
            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);
            if (m_isResizeable) {
                if (x <= rect.left + border) {
                    if (y <= rect.top + border)
                        *result = HTTOPLEFT;
                    else
                    if (y > rect.top + border && y < rect.bottom - border)
                        *result = HTLEFT;
                    else
                    if (y >= rect.bottom - border)
                        *result = HTBOTTOMLEFT;
                } else
                if (x > rect.left + border && x < rect.right - border) {
                    if (y <= rect.top + border)
                        *result = HTTOP;
                    else
                    if (y >= rect.bottom - border)
                        *result = HTBOTTOM;
                } else
                if (x >= rect.right - border) {
                    if (y <= rect.top + border)
                        *result = HTTOPRIGHT;
                    else
                    if (y > rect.top + border && y < rect.bottom - border)
                        *result = HTRIGHT;
                    else
                    if (y >= rect.bottom - border)
                        *result = HTBOTTOMRIGHT;
                }
            }
            if (*result != 0)
                return true;
            return false;
        }
        break;
    }

    case WM_SETFOCUS: {
        if (!m_closed && IsWindowEnabled(m_hWnd)) {
            QTimer::singleShot(0, this, [=]() {
                focus();
            });
        }
        m_propertyTimer->stop();
        if (property("stabilized").toBool())
            setProperty("stabilized", false);
        m_propertyTimer->start();
        break;
    }

    case WM_SETTINGCHANGE: {
        if (msg->wParam == SPI_SETWORKAREA) {
            static RECT oldWorkArea = {0,0,0,0};
            RECT workArea; // Taskbar show/hide detection
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
            if (!EqualRect(&oldWorkArea, &workArea)) {
                oldWorkArea = workArea;
                QTimer::singleShot(200, this, [=]() {
                    adjustGeometry();
                });
            }
        } else if (msg->wParam == 0) {
            const std::wstring param{(wchar_t*)msg->lParam};
            if (param == L"ImmersiveColorSet") {
                QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
                bool _changed_theme_dark = _reg.value("AppsUseLightTheme", 1).toInt() == 0;

                if (_changed_theme_dark != AscAppManager::themes().isSystemSchemeDark()) {
                    NSEditorApi::CAscCefMenuEvent * ns_event = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);
                    NSEditorApi::CAscExecCommand * pData = new NSEditorApi::CAscExecCommand;
                    pData->put_Command(L"system:changed");

                    QJsonObject _json_obj{{"colorscheme", _changed_theme_dark ? "dark" : "light"}};
                    pData->put_Param(Utils::stringifyJson(_json_obj).toStdWString());

                    ns_event->m_pData = pData;
                    ns_event->m_nSenderId = 0;
                    AscAppManager::getInstance().OnEvent(ns_event);
                }
            }
        }
        break;
    }

    case WM_TIMER:
        AscAppManager::getInstance().CheckKeyboard();
        break;

    case UM_INSTALL_UPDATE:
        QTimer::singleShot(500, this, [=](){
            onCloseEvent();
        });
        break;

    case WM_POWERBROADCAST: {
        if (msg->wParam == PBT_APMRESUMEAUTOMATIC) {
            HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONULL);
            if (!monitor) {
                moveToPrimaryScreen();
            } else {
                MONITORINFOEX monInfo;
                ZeroMemory(&monInfo, sizeof(monInfo));
                monInfo.cbSize = sizeof(MONITORINFOEX);
                if (GetMonitorInfo(monitor, &monInfo)) {
                    DISPLAY_DEVICE  dispDevice;
                    ZeroMemory(&dispDevice, sizeof(dispDevice));
                    dispDevice.cb = sizeof(dispDevice);
                    if (EnumDisplayDevices(monInfo.szDevice, 0, &dispDevice, EDD_GET_DEVICE_INTERFACE_NAME)) {
                        HANDLE hDevice;
                        hDevice = CreateFile(dispDevice.DeviceID,
                                             GENERIC_READ,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_EXISTING,
                                             FILE_ATTRIBUTE_READONLY,
                                             NULL);
                        if (hDevice != INVALID_HANDLE_VALUE) {
                            BOOL res;
                            if (GetDevicePowerState(hDevice, &res)) {
                                if (res == FALSE)
                                    moveToPrimaryScreen();
                            } else {
                                moveToPrimaryScreen();
                            }
                            CloseHandle(hDevice);
                        } else {
                            moveToPrimaryScreen();
                        }
                    }
                } else {
                    moveToPrimaryScreen();
                }
            }
        }
        break;
    }

    case WM_EXITSIZEMOVE:
        if (m_allowMaximize)
            QApplication::postEvent(this, new QEvent(QEvent::User));
        break;

    case WM_MOVE:
        if (movParam != 0)
            movParam = 0;
        break;

    case WM_MOVING:
        if (m_allowMaximize && ++movParam == UM_SNAPPING)
            m_allowMaximize = false;
        break;

    case WM_PAINT:
        return false;

    case WM_ERASEBKGND:
        return true;

    case WM_NCACTIVATE: {
        // Prevent the title bar from being drawn when the window is restored or maximized
        if (m_borderless) {
            if (!LOWORD(msg->wParam)) {
                *result = TRUE;
                break;
            }
            return true;
        }
        break;
    }

    case WM_QUERYENDSESSION:
        Utils::setSessionInProgress(false);
        m_isSessionInProgress = false;
        break;

    case WM_ENDSESSION:
        if (!msg->wParam) {
            Utils::setSessionInProgress(true);
            m_isSessionInProgress = true;
        }
        break;

    default:
        break;
    }
    return CWindowBase::nativeEvent(eventType, message, result);
}
