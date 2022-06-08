#include "csnap.h"
#include <windowsx.h>
#include <QEvent>

#define WINDOW_CLASS_NAME L"CSnap"
#define DELAY 500
#define ALPHA 0x01


CSnap::CSnap(QPushButton *btn) :
    m_pBtn(btn),
    m_pTopLevelWidget(nullptr),
    m_allowedChangeSize(false)
{
    m_hInstance = GetModuleHandle(nullptr);
    WNDCLASSEX wcx{sizeof(WNDCLASSEX)};
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = m_hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = WINDOW_CLASS_NAME;
    wcx.hbrBackground = CreateSolidBrush(0x00ffffff);
    wcx.hCursor = LoadCursor(m_hInstance, IDC_ARROW);
    RegisterClassEx(&wcx);

    QPoint pos = btn->mapToGlobal(QPoint(0,0));
    QSize size = btn->size();
    m_hWnd = CreateWindowEx(
                WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                WINDOW_CLASS_NAME,
                L"",
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
    connect(m_pBtn, &QPushButton::destroyed, this, [=](){
        DestroyWindow(m_hWnd);
        this->deleteLater();
    });
    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    m_pTimer->setInterval(DELAY);
    connect(m_pTimer, &QTimer::timeout, this, &CSnap::show);
}

CSnap::~CSnap()
{

}

void CSnap::show()
{
    QPoint pos = m_pBtn->mapToGlobal(QPoint(0,0));
    QSize size = m_pBtn->size();
    SetWindowPos(m_hWnd, NULL, pos.x(), pos.y(), size.width(), size.height(),
                 SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
    ShowWindow(m_hWnd, SW_SHOW);
    SetLayeredWindowAttributes(m_hWnd, 0, ALPHA, LWA_ALPHA);
}

bool CSnap::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_pBtn) {
        if (e->type() == QEvent::HoverEnter) {
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

LRESULT CSnap::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CSnap *window = reinterpret_cast<CSnap*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
        }
        window->m_allowedChangeSize = false;
        break;

    case WM_NCMOUSELEAVE: {
        SetLayeredWindowAttributes(hWnd, 0, 0x00, LWA_ALPHA);
        window->m_allowedChangeSize = true;
        break;
    }

    case WM_NCLBUTTONDOWN: {
        emit window->m_pBtn->click();
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
