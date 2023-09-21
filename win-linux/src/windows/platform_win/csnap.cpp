#include "csnap.h"
#include <windowsx.h>
#include <QEvent>

#define WINDOW_CLASS_NAME L"CWin11Snap"
#define SNAP_POPUP_NAME L"PopupHost"
#define DELAY 500
#define ALPHA 0x30
#define COLOR 0x00ffffff
#ifndef SPI_GETWINARRANGING
# define SPI_GETWINARRANGING 0x0082
#endif


QMap<HWINEVENTHOOK, HWND> CWin11Snap::m_hookMap = QMap<HWINEVENTHOOK, HWND>();

CWin11Snap::CWin11Snap(QPushButton *btn) :
    QObject(btn),
    m_pBtn(btn),
    m_pTopLevelWidget(nullptr),
    m_allowedChangeSize(false)
{
    HINSTANCE m_hInstance = GetModuleHandle(nullptr);
    WNDCLASSEX wcx{sizeof(WNDCLASSEX)};
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = m_hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = WINDOW_CLASS_NAME;
    wcx.hbrBackground = CreateSolidBrush(COLOR);
    wcx.hCursor = LoadCursor(m_hInstance, IDC_ARROW);
    RegisterClassEx(&wcx);

    QPoint pos = btn->mapToGlobal(QPoint(0,0));
    QSize size = btn->size();
    m_hWnd = CreateWindowEx(
                WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                WINDOW_CLASS_NAME,
                L"CWin11SnapWindow",
                WS_MAXIMIZEBOX | WS_THICKFRAME,
                pos.x(), pos.y(), size.width(), size.height(),
                nullptr,
                nullptr,
                m_hInstance,
                nullptr);
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    SetLayeredWindowAttributes(m_hWnd, 0, 0x00, LWA_ALPHA);

    m_pTopLevelWidget = m_pBtn->nativeParentWidget();
    m_pBtn->installEventFilter(this);
    m_pBtn->setAttribute(Qt::WA_Hover);
    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    m_pTimer->setInterval(DELAY);
    connect(m_pTimer, &QTimer::timeout, this, &CWin11Snap::show);
    m_snapPopupEventHook = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, NULL, snapPopupEventProc,
                                              0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    m_hookMap[m_snapPopupEventHook] = m_hWnd;
}

CWin11Snap::~CWin11Snap()
{
    if (m_hookMap.contains(m_snapPopupEventHook))
        m_hookMap.remove(m_snapPopupEventHook);
    UnhookWinEvent(m_snapPopupEventHook);
    DestroyWindow(m_hWnd);
}

void CWin11Snap::show()
{
    QPoint pos = m_pBtn->mapToGlobal(QPoint(0,0));
    QSize size = m_pBtn->size();
    SetWindowPos(m_hWnd, NULL, pos.x(), pos.y(), size.width(), size.height(),
                 SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
    ShowWindow(m_hWnd, SW_SHOW);
    SetLayeredWindowAttributes(m_hWnd, 0, ALPHA, LWA_ALPHA);
}

bool CWin11Snap::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_pBtn) {
        if (e->type() == QEvent::HoverEnter) {
            BOOL arranging;
            SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
            if (arranging == TRUE)
                m_pTimer->start();
        } else
        if (e->type() == QEvent::HoverLeave) {
            m_pTimer->stop();
        } else
        if (e->type() == QEvent::MouseButtonPress) {
            m_pTimer->stop();
        }
    }
    return QObject::eventFilter(obj, e);
}

LRESULT CWin11Snap::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWin11Snap *window = reinterpret_cast<CWin11Snap*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!window)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg) {
    case WM_SIZE:
        RECT rect;
        GetWindowRect(hWnd, &rect);
        if (window->m_allowedChangeSize) {
            if (window->m_pTopLevelWidget) {
                if (window->m_pTopLevelWidget->isMaximized())
                    window->m_pTopLevelWidget->showNormal();
                window->m_pTopLevelWidget->setGeometry(int(rect.left), int(rect.top),
                                                       int(rect.right - rect.left),
                                                       int(rect.bottom - rect.top));
            }
            ShowWindow(hWnd, SW_HIDE);
            window->m_allowedChangeSize = false;
        }
        break;

    case WM_SETFOCUS:
        window->m_allowedChangeSize = false;
        break;

    case WM_NCMOUSELEAVE: {
        SetLayeredWindowAttributes(hWnd, 0, 0x00, LWA_ALPHA);
        break;
    }

    case WM_NCLBUTTONDOWN: {
        window->m_pBtn->click();
        SetLayeredWindowAttributes(hWnd, 0, 0x00, LWA_ALPHA);
        ShowWindow(hWnd, SW_HIDE);
        return TRUE;
    }

    case WM_NCCALCSIZE: {
        if (wParam == TRUE) {
            SetWindowLong(hWnd, DWLP_MSGRESULT, 0);
            return TRUE;
        }
        return FALSE;
    }

    case WM_NCHITTEST: {
        if (!window->m_allowedChangeSize)
            window->m_allowedChangeSize = true;
        return HTMAXBUTTON;
    }

    case WM_NCACTIVATE: {
        return TRUE;
    }

    default:
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CWin11Snap::snapPopupEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG, LONG , DWORD, DWORD)
{
    if (event == EVENT_OBJECT_DESTROY) {
        const int len = sizeof(SNAP_POPUP_NAME)/sizeof(WCHAR);
        WCHAR title[len + 1] = {0};
        GetWindowText(hwnd, title, len);
        if (lstrcmpi(title, SNAP_POPUP_NAME) == 0 && m_hookMap.contains(hook))
            ShowWindow(m_hookMap[hook], SW_HIDE);
    }
}
