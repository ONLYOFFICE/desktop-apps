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
#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
#include <QTimer>
#include <QDesktopWidget>
#include <QWindow>
#include <QScreen>
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
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint
                   | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    m_hWnd = (HWND)winId();
    LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
    style |= (WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    style |= (Utils::getWinVersion() > Utils::WinVer::Win7) ?
                WS_OVERLAPPEDWINDOW : WS_POPUP;
    ::SetWindowLong(m_hWnd, GWL_STYLE, style);
#ifndef __OS_WIN_XP
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hWnd, &shadow);
#endif
    connect(this->window()->windowHandle(), &QWindow::screenChanged, this, [=]() {
        SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
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

void CWindowPlatform::adjustGeometry()
{
    if (windowState().testFlag(Qt::WindowMinimized) || windowState().testFlag(Qt::WindowNoState)) {
        const int border = int(MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio);
        setContentsMargins(border, border, border, border+1);
        setResizeableAreaWidth(border);
    } else
    if (windowState().testFlag(Qt::WindowMaximized)) {
        QTimer::singleShot(0, this, [=]() {
            setContentsMargins(0,0,0,0);
            auto dsk = QApplication::desktop();
            const QSize offset(0, !isTaskbarAutoHideOn() ? -1 : 1);
            resize(dsk->availableGeometry(this).size() - offset);
            move(dsk->availableGeometry(this).topLeft());
            setWindowState(Qt::WindowMaximized);
        });
    }
}

bool CWindowPlatform::isTaskbarAutoHideOn()
{
    APPBARDATA ABData;
    ABData.cbSize = sizeof(ABData);
    return (SHAppBarMessage(ABM_GETSTATE, &ABData) & ABS_AUTOHIDE) != 0;
}

void CWindowPlatform::bringToTop()
{
    if (IsIconic(m_hWnd)) {
        ShowWindow(m_hWnd, SW_SHOWNORMAL);
    }
    HWND hWndFrg = ::GetForegroundWindow();
    DWORD appID = ::GetCurrentThreadId();
    DWORD frgID = ::GetWindowThreadProcessId(hWndFrg, NULL);
    ::AttachThreadInput(frgID, appID, TRUE);
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    ::SetForegroundWindow(m_hWnd);
    ::SetFocus(m_hWnd);
    ::SetActiveWindow(m_hWnd);
    ::AttachThreadInput(frgID, appID, FALSE);
}

void CWindowPlatform::show(bool maximized)
{
    maximized ? CWindowBase::showMaximized() : CWindowBase::show();
}

/** Private **/

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
    case WM_DPICHANGED: {
        if (!WindowHelper::isLeftButtonPressed() || AscAppManager::IsUseSystemScaling()) {
            updateScaling();
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
            //SendMessage(msg->hwnd, WM_KEYDOWN, msg->wParam, msg->lParam);
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

        } else
        if (GET_SC_WPARAM(msg->wParam) == SC_MINIMIZE) {

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
        if (params.rgrc[0].bottom != 0)
            params.rgrc[0].bottom += 1;
        *result = WVR_REDRAW;
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
            focus();
        }
        break;
    }

    case WM_WININICHANGE: {
        adjustGeometry();
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

    default:
        break;
    }
    return CWindowBase::nativeEvent(eventType, message, result);
}
