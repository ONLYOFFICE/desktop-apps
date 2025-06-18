#include "window.h"
#include "baseutils.h"
#include "palette.h"
#include "metrics.h"
#include "drawningengine.h"
#include <windowsx.h>

#define DCX_USESTYLE 0x00010000
#define NC_AREA_WIDTH 3
#define MAIN_WINDOW_BORDER_WIDTH 1

using WinVer = Utils::WinVer;


static BOOL CALLBACK EnumChildProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    return TRUE;
}

static double GetLogicalDpi(HWND hWnd)
{
    if (HMODULE module = GetModuleHandleA("user32")) {
        UINT(WINAPI *_GetDpiForWindow)(HWND) = NULL;
        *(FARPROC*)&_GetDpiForWindow = GetProcAddress(module, "GetDpiForWindow");
        if (_GetDpiForWindow)
            return (double)_GetDpiForWindow(hWnd)/96;
    }
    HDC hdc = GetDC(NULL);
    double dpi = (double)GetDeviceCaps(hdc, LOGPIXELSX)/96;
    ReleaseDC(NULL, hdc);
    return dpi;
}

static Rect availableGeometry(HWND hwnd)
{
    Rect rc;
    if (HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)) {
        MONITORINFOEX monInfo;
        ZeroMemory(&monInfo, sizeof(monInfo));
        monInfo.cbSize = sizeof(MONITORINFOEX);
        if (GetMonitorInfo(monitor, &monInfo))
            rc = Rect(monInfo.rcWork.left, monInfo.rcWork.top, monInfo.rcWork.right - monInfo.rcWork.left, monInfo.rcWork.bottom - monInfo.rcWork.top);
    }
    return rc;
}

static void GetFrameMetricsForDpi(FRAME &frame, double dpi, bool maximized = false)
{
    WinVer ver = Utils::getWinVersion();
    int row = ver == WinVer::WinXP ? 0 :
                  ver <= WinVer::Win7 ? 1 :
                  ver <= WinVer::Win8_1 ? 2 :
                  ver <= WinVer::Win10 ? 3 : 4;

    int column = dpi <= 1.0 ? 0 :
                     dpi <= 1.25 ? 1 :
                     dpi <= 1.5 ? 2 :
                     dpi <= 1.75 ? 3 :
                     dpi <= 2.0 ? 4 :
                     dpi <= 2.25 ? 5 :
                     dpi <= 2.5 ? 6 :
                     dpi <= 3.0 ? 7 :
                     dpi <= 3.5 ? 8 :
                     dpi <= 4.0 ? 9 :
                     dpi <= 4.5 ? 10 :
                     dpi <= 5.0 ? 11 : 12;

    const int left[5][13] = { // Left margin for scales 100-500%
        {0, 0, 0,  0,  0,  1,  1,  1,  2,  2,  2,  2,  2}, // WinXp: for NC width 3px
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // WinVista - Win7
        {7, 8, 10, 11, 12, 13, 15, 17, 20, 22, 25, 27, 32}, // Win8 - Win8.1
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };
    frame.left = left[row][column];

    const int top[5][13] = { // Top margin for scales 100-500%
        {0,  0,  0,  0,  0,  1,  1,  1,  2,  2,   2,   2,   2}, // WinXp: for NC width 3px
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // WinVista - Win7
        {7,  8,  10, 11, 12, 13, 15, 17, 20, 22,  25,  27,  32}, // Win8 - Win8.1
        {31, 38, 45, 52, 58, 65, 72, 85, 99, 112, 126, 139, 167}, // Win10
        {30, 37, 43, 50, 56, 63, 69, 82, 95, 108, 121, 134, 161}  // Win11
    };
    frame.top = top[row][column];

    if (!maximized)
        return;

    const int left_ofs[5][13] = { // Left offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win10
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  // Win11
    };
    frame.left -= left_ofs[row][column];

    const int top_ofs[5][13] = { // Top offset for scales 100-500%
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinXp
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // WinVista - Win7
        {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // Win8 - Win8.1
        {8,  9,  11, 12, 13, 14, 16, 18, 21, 24, 27, 30, 36}, // Win10
        {7,  8,  9,  10, 11, 12, 13, 15, 17, 19, 21, 23, 28}  // Win11
    };
    frame.top -= top_ofs[row][column];
}

static bool isTaskbarAutoHideOn()
{
    APPBARDATA ABData;
    ABData.cbSize = sizeof(ABData);
    return (SHAppBarMessage(ABM_GETSTATE, &ABData) & ABS_AUTOHIDE) != 0;
}

static bool isThemeActive()
{
    static BOOL(WINAPI *IsThemeActive)() = NULL;
    if (!IsThemeActive) {
        if (HMODULE module = GetModuleHandleA("uxtheme"))
            *(FARPROC*)&IsThemeActive = GetProcAddress(module, "IsThemeActive");
    }
    return IsThemeActive ? (bool)IsThemeActive() : true;
}

Window::Window(Widget *parent, const Rect &rc) :
    Widget(parent, ObjectType::WindowType, nullptr, rc),
    m_centralWidget(nullptr),
    m_contentMargins(0,0,0,0),
    m_resAreaWidth(0),
    m_state(-1),
    m_borderless(true),
    m_isResizable(true),
    m_scaleChanged(false),
    m_init_size(rc.width, rc.height)
{
    //setLayout(new BoxLayout(BoxLayout::Vertical));
    m_isThemeActive = isThemeActive();
    m_isTaskbarAutoHideOn = isTaskbarAutoHideOn();
    m_borderless = true;//isCustomWindowStyle();

    if (m_borderless && Utils::getWinVersion() < WinVer::Win10) {
        LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW;
        ::SetWindowLong(m_hWnd, GWL_STYLE, style & ~WS_CAPTION);
    }
    m_isMaximized = IsZoomed(m_hWnd);
    m_dpi = GetLogicalDpi(m_hWnd);
    GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);

    if (m_borderless && Utils::getWinVersion() == WinVer::Win10) {
        HDC hdc = GetDC(NULL);
        m_brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
        ReleaseDC(NULL, hdc);
        m_brdColor = Utils::getColorizationColor(true, palette()->color(Palette::Background));
    }
    SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

Window::~Window()
{
    //if (m_layout)
    //    delete m_layout, m_layout = nullptr;
}

void Window::setCentralWidget(Widget *wgt)
{
    m_centralWidget = wgt;
}

void Window::setContentsMargins(int left, int top, int right, int bottom)
{
    m_contentMargins = Margins(left, top, right, bottom);
    if (IsWindowVisible(m_hWnd))
        SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void Window::setResizable(bool isResizable)
{
    if (m_isResizable != isResizable) {
        m_isResizable = isResizable;
        LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
        ::SetWindowLong(m_hWnd, GWL_STYLE, m_isResizable ? style | WS_MAXIMIZEBOX : style & ~WS_MAXIMIZEBOX);
    }
}

void Window::showAll()
{
    ShowWindow(m_hWnd, SW_SHOWNORMAL);
    UpdateWindow(m_hWnd);
    EnumChildWindows(m_hWnd, EnumChildProc, 0);
    SetForegroundWindow(m_hWnd);
}

void Window::showNormal()
{
    ShowWindow(m_hWnd, SW_RESTORE);
}

void Window::showMinimized()
{
    ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
}

void Window::showMaximized()
{
    ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
}

void Window::setIcon(int id)
{
    HMODULE hInstance = GetModuleHandle(NULL);
    HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(id), IMAGE_ICON, 96, 96, LR_DEFAULTCOLOR | LR_SHARED);
    SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

bool Window::isMinimized()
{
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(wpl);
    if (GetWindowPlacement(m_hWnd, &wpl))
        return wpl.showCmd == SW_SHOWMINIMIZED;
    return false;
}

bool Window::isMaximized()
{
    WINDOWPLACEMENT wpl;
    wpl.length = sizeof(wpl);
    if (GetWindowPlacement(m_hWnd, &wpl))
        return wpl.showCmd == SW_SHOWMAXIMIZED;
    return false;
}

Widget *Window::centralWidget()
{
    return m_centralWidget;
}

int Window::onStateChanged(const FnVoidInt &callback)
{
    m_state_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void Window::disconnect(int connectionId)
{
    auto it = m_state_callbacks.find(connectionId);
    if (it != m_state_callbacks.end())
        m_state_callbacks.erase(it);
}

bool Window::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_DPICHANGED: {
        m_dpi = (double)HIWORD(wParam)/96;
        RECT *prc = (RECT*)lParam;
        GetFrameMetricsForDpi(m_frame, m_dpi, m_isMaximized);
        SetWindowPos(m_hWnd, NULL, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER | SWP_NOACTIVATE);
        break;
    }

    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE: {
        //int x = GET_X_LPARAM(lParam);
        //int y = GET_Y_LPARAM(lParam);
        //printf("Window Move Event: %d x %d \t %lld\n", x, y, (long long)this);
        //fflush(stdout);
        break;
    }

    case WM_MOUSEHOVER:
    case WM_NCMOUSEHOVER: {
        //int x = GET_X_LPARAM(lParam);
        //int y = GET_Y_LPARAM(lParam);
        //printf("Window Hover Event: %d x %d \t %lld\n", x, y, (long long)this);
        //fflush(stdout);
        break;
    }

    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_brdWidth != 0)
            engine()->DrawTopBorder(m_brdWidth, m_brdColor);
        engine()->End();
        *result = FALSE;
        return true;
    }

    case WM_NCPAINT: {
        if (Utils::getWinVersion() > WinVer::Win7 || !m_borderless)
            return false;
        if (HDC hdc = ::GetDCEx(m_hWnd, 0, DCX_WINDOW | DCX_USESTYLE)) {
            RECT rcc, rcw;
            ::GetClientRect(m_hWnd, &rcc);
            ::GetWindowRect(m_hWnd, &rcw);
            POINT pt;
            pt.x = rcw.left;
            pt.y = rcw.top;
            ::MapWindowPoints(0, m_hWnd, (LPPOINT)&rcw, (sizeof(RECT)/sizeof(POINT)));
            ::OffsetRect(&rcc, -rcw.left, -rcw.top);
            ::OffsetRect(&rcw, -rcw.left, -rcw.top);
            HRGN rgntemp = NULL;
            if (wParam == NULLREGION || wParam == ERROR) {
                ::ExcludeClipRect(hdc, rcc.left, rcc.top, rcc.right, rcc.bottom);
            } else {
                rgntemp = ::CreateRectRgn(rcc.left + pt.x, rcc.top + pt.y, rcc.right + pt.x, rcc.bottom + pt.y);
                if (::CombineRgn(rgntemp, (HRGN)wParam, rgntemp, RGN_DIFF) == NULLREGION) {
                    // nothing to paint
                }
                ::OffsetRgn(rgntemp, -pt.x, -pt.y);
                ::ExtSelectClipRgn(hdc, rgntemp, RGN_AND);
            }
            HBRUSH hbrushBkg = ::CreateSolidBrush(palette()->color(Palette::Background));
            ::FillRect(hdc, &rcw, hbrushBkg);
            ::DeleteObject(hbrushBkg);

            // HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
            // GetWindowRgn(msg->hwnd, hrgn);
            HBRUSH hbrushBrd = ::CreateSolidBrush(palette()->color(Palette::Border));
            ::FrameRect(hdc, &rcw, hbrushBrd); // Drawing NC border when using ~WS_CAPTION
            // ::FrameRgn(hdc, hrgn, hbrushBrd, 1, 1); // Drawing NC border when using WS_CAPTION
            ::DeleteObject(hbrushBrd);
            // ::DeleteObject(hrgn);

            ::ReleaseDC(m_hWnd, hdc);
            if (rgntemp != 0)
                ::DeleteObject(rgntemp);
            return true;
        }
        return false;
    }

    case WM_NCCALCSIZE: {
        if (!m_borderless || !wParam)
            return false;
        NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)lParam;
        if (!m_isThemeActive) {
            *result = m_isMaximized ? 0 : DefWindowProc(m_hWnd, WM_NCCALCSIZE, wParam, lParam);
            return true;
        }
        LRESULT res = DefWindowProc(m_hWnd, WM_NCCALCSIZE, wParam, lParam);
        params->rgrc[0].left -= m_frame.left;
        params->rgrc[0].top -= m_frame.top;
        params->rgrc[0].right += m_frame.left;
        params->rgrc[0].bottom += m_frame.left;
        if (m_isMaximized && m_isTaskbarAutoHideOn && (Utils::getWinVersion() >= WinVer::Win10))
            params->rgrc[0].bottom -= 2;
        *result = res;
        return true;
    }

    case WM_NCHITTEST: {
        if (m_isResizable) {
            if (m_borderless) {
                RECT rect;
                GetWindowRect(m_hWnd, &rect);
                long x = GET_X_LPARAM(lParam);
                long y = GET_Y_LPARAM(lParam);
                if (x <= rect.left + m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOPLEFT;
                    else
                    if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                        *result = HTLEFT;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOMLEFT;
                } else
                if (x > rect.left + m_resAreaWidth && x < rect.right - m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOP;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOM;
                } else
                if (x >= rect.right - m_resAreaWidth) {
                    if (y <= rect.top + m_resAreaWidth)
                        *result = HTTOPRIGHT;
                    else
                    if (y > rect.top + m_resAreaWidth && y < rect.bottom - m_resAreaWidth)
                        *result = HTRIGHT;
                    else
                    if (y >= rect.bottom - m_resAreaWidth)
                        *result = HTBOTTOMRIGHT;
                }
                return *result != 0;
            }
        } else {
            LRESULT hit = DefWindowProc(m_hWnd, msg, wParam, lParam);
            if (hit == HTBOTTOM || hit == HTLEFT || hit == HTRIGHT || hit == HTTOP ||
                    hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT || hit == HTTOPLEFT || hit == HTTOPRIGHT) {
                *result = HTCLIENT;
                return true;
            }
        }
        return false;
    }

    case WM_NCACTIVATE: {
        if (m_borderless) {
            if (Utils::getWinVersion() > WinVer::WinXP && Utils::getWinVersion() < WinVer::Win10) {
                // Prevent drawing of inactive system frame (needs ~WS_CAPTION or temporary ~WS_VISIBLE to work)
                *result = DefWindowProc(m_hWnd, WM_NCACTIVATE, wParam, -1);
                return true;
            } else
            if (Utils::getWinVersion() == WinVer::Win10) {
                m_brdColor = Utils::getColorizationColor(LOWORD(wParam), palette()->color(Palette::Background));
                RECT rc;
                GetClientRect(m_hWnd, &rc);
                rc.bottom = m_brdWidth;
                RedrawWindow(m_hWnd, &rc, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT | RDW_UPDATENOW);
            }
        }
        return false;
    }

    case WM_GETMINMAXINFO: {
        bool isMaximized = (bool)IsZoomed(m_hWnd);
        if (m_isMaximized != isMaximized) {
            m_isMaximized = isMaximized;
            GetFrameMetricsForDpi(m_frame, m_dpi, isMaximized);
            if (m_borderless && Utils::getWinVersion() == WinVer::Win10) {
                if (isMaximized) {
                    m_brdWidth = 0;
                } else {
                    HDC hdc = GetDC(NULL);
                    m_brdWidth = GetSystemMetrics(SM_CXBORDER) * GetDeviceCaps(hdc, LOGPIXELSX)/96;
                    ReleaseDC(NULL, hdc);
                }
            }
        }
        if (!m_isResizable) {
            MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;
            minMaxInfo->ptMinTrackSize.x = m_init_size.width;
            minMaxInfo->ptMinTrackSize.y = m_init_size.height;
            minMaxInfo->ptMaxTrackSize.x = m_init_size.width;
            minMaxInfo->ptMaxTrackSize.y = m_init_size.height;
        }
        break;
    }

    case WM_THEMECHANGED: {
        bool _isThemeActive = isThemeActive();
        if (m_isThemeActive != _isThemeActive)
            m_isThemeActive = _isThemeActive;
        break;
    }

    case WM_SETTINGCHANGE: {
        // if (wParam == SPI_SETWINARRANGING) {
        //     if (Utils::getWinVersion() > Utils::WinVer::Win10)
        //         SendMessage((HWND)m_boxTitleBtns->winId(), WM_SETTINGCHANGE, 0, 0);
        // }
        //     printf(" Settings...\n");
        //     fflush(stdout);
        //     snapLayoutAllowed = isArrangingAllowed();
        break;
    }

    case WM_SIZING: {
        // if (m_borderless)
        //     RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT);
        break;
    }

    case WM_SIZE: {            
        switch (wParam) {
        case SIZE_MAXIMIZED:
        case SIZE_MINIMIZED:
        case SIZE_RESTORED: {
            if (m_state != (int)wParam) {
                m_state = (int)wParam;
                for (auto it = m_state_callbacks.begin(); it != m_state_callbacks.end(); it++)
                    if (it->second)
                        (it->second)(m_state);

                if (m_borderless) {
                    if (m_isMaximized) {
                        if (Utils::getWinVersion() < WinVer::Win10) {
                            m_resAreaWidth = 0;
                            Rect rc = availableGeometry(m_hWnd);
                            int offset = 0;
                            if (Utils::getWinVersion() == WinVer::WinXP) {
                                if (isTaskbarAutoHideOn())
                                    offset += NC_AREA_WIDTH + 1;
                                if (m_isThemeActive) {
                                    rc.x += -NC_AREA_WIDTH;
                                    rc.y += -NC_AREA_WIDTH;
                                    rc.width += 2*NC_AREA_WIDTH;
                                    rc.height += 2*NC_AREA_WIDTH;
                                }
                            } else
                            if (Utils::getWinVersion() > WinVer::WinXP && isTaskbarAutoHideOn())
                                offset += 2;
                            SetWindowPos(m_hWnd, NULL, rc.x, rc.y, rc.width, rc.height - offset, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                        }
                    } else {
                        if (Utils::getWinVersion() < WinVer::Win10) {
                            m_resAreaWidth = (int)round(MAIN_WINDOW_BORDER_WIDTH * m_dpi);
                            if (Utils::getWinVersion() == WinVer::WinXP)
                                m_resAreaWidth -= NC_AREA_WIDTH;
                        }
                    }
                }
            }
            if (m_centralWidget) {
                int top_offset = 0;
                if (m_borderless && !m_isMaximized && Utils::getWinVersion() == Utils::WinVer::Win10)
                    top_offset = m_brdWidth;
                m_centralWidget->setGeometry(m_contentMargins.left + m_resAreaWidth, m_contentMargins.top + top_offset + m_resAreaWidth,
                                             LOWORD(lParam) - m_contentMargins.right - m_contentMargins.left - 2*m_resAreaWidth,
                                             HIWORD(lParam) - m_contentMargins.bottom - m_contentMargins.top - top_offset - 2*m_resAreaWidth);
            }
            break;
        }
        default:
            break;
        }
        break;
    }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}
