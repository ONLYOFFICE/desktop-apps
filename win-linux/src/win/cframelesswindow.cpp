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

#include "cframelesswindow.h"


CFramelessWindow::CFramelessWindow(QWidget *parent) :
    QMainWindow(parent),
    m_borderWidth(5),
    m_margins(QMargins()),
    m_frames(QMargins()),
    m_bJustMaximized(false),
    m_bResizeable(true),
    m_taskBarClicked(false),
    m_previousState(Qt::WindowNoState)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setResizeable(m_bResizeable);
}

CFramelessWindow::~CFramelessWindow()
{

}

void CFramelessWindow::setResizeable(bool resizeable)
{
    bool visible = isVisible();
    m_bResizeable = resizeable;
    HWND hwnd = (HWND)winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    if (m_bResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
    setVisible(visible);
}

void CFramelessWindow::setResizeableAreaWidth(int width)
{
    m_borderWidth = (width < 0) ? 0 : width;
}

bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch (msg->message)
    {
    case WM_SYSKEYDOWN: {
        if (msg->wParam == VK_SPACE) {
            //RECT winrect;
            //GetWindowRect(msg->hwnd, &winrect);
            //TrackPopupMenu(GetSystemMenu(msg->hwnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, msg->hwnd, NULL);
        }
        break;
    }

    case WM_KEYDOWN: {
        if (msg->wParam == VK_F5 || msg->wParam == VK_F6 || msg->wParam == VK_F7) {
            //SendMessage(msg->hwnd, WM_KEYDOWN, msg->wParam, msg->lParam);
        }
        break;
    }

    case WM_SYSCOMMAND: {
        if (GET_SC_WPARAM(msg->wParam) == 61728 || GET_SC_WPARAM(msg->wParam) == 61472) {
            m_taskBarClicked = true;
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

        if (m_bResizeable) {
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
            m_bJustMaximized = true;
        } else {
            if (m_bJustMaximized) {
                QMainWindow::setContentsMargins(m_margins);
                m_frames = QMargins();
                m_bJustMaximized = false;
            }
        }
        return false;
    }

    case WM_MOVING:
        onMoveEvent(QRect());
        break;

    default:
        break;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void CFramelessWindow::setContentsMargins(int left, int top, int right, int bottom)
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

