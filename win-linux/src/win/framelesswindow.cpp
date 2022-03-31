#undef NOMINMAX

#include "framelesswindow.h"
#include <QApplication>
#include <QPoint>
#include <QSize>


CFramelessWindow::CFramelessWindow(QWidget *parent) :
    QMainWindow(parent),
    m_titlebar(nullptr),
    m_whiteList(QList<QWidget*>()),
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
    if (m_bResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        HWND hwnd = (HWND)winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        HWND hwnd = (HWND)winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
    setVisible(visible);
}

void CFramelessWindow::setResizeableAreaWidth(int width)
{
    if (width < 1) width = 1;
    m_borderWidth = width;
}

void CFramelessWindow::setTitleBar(QWidget* titlebar)
{
    m_titlebar = titlebar;
    if (!titlebar) return;
    connect(titlebar, SIGNAL(destroyed(QObject*)), this, SLOT(onTitleBarDestroyed()));
}

void CFramelessWindow::onTitleBarDestroyed()
{
    if (m_titlebar == QObject::sender()) {
        m_titlebar = Q_NULLPTR;
    }
}

void CFramelessWindow::addIgnoreWidget(QWidget* widget)
{
    if (!widget) return;
    if (m_whiteList.contains(widget)) return;
    m_whiteList.append(widget);
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
    case WM_ACTIVATE : {
        //qDebug() << "Activate...";
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
        const LONG border_width = m_borderWidth;
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
        if (!m_titlebar) return false;

        //support highdpi
        double dpr = this->devicePixelRatioF();
        QPoint pos = m_titlebar->mapFromGlobal(QPoint(int(x/dpr), int(y/dpr)));
        if (!m_titlebar->rect().contains(pos)) return false;
        QWidget* child = m_titlebar->childAt(pos);
        if (!child) {
            *result = HTCAPTION;
            return true;
        } else {
            if (m_whiteList.contains(child)) {
                *result = HTCAPTION;
                return true;
            }
        }
        return false;
    }

    case WM_GETMINMAXINFO: {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

            //record frame area data
            double dpr = this->devicePixelRatioF();
            m_frames.setLeft(int(abs(frame.left)/dpr + 0.5));
            m_frames.setTop(int(abs(frame.bottom)/dpr + 0.5));
            m_frames.setRight(int(abs(frame.right)/dpr + 0.5));
            m_frames.setBottom(int(abs(frame.bottom)/dpr + 0.5));
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
    default:
        break;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void CFramelessWindow::setContentsMargins(const QMargins &margins)
{
    QMainWindow::setContentsMargins(margins+m_frames);
    m_margins = margins;
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

QMargins CFramelessWindow::contentsMargins() const
{
    QMargins margins = QMainWindow::contentsMargins();
    margins -= m_frames;
    return margins;
}

void CFramelessWindow::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    QMainWindow::getContentsMargins(left, top, right, bottom);
    if (!(left && top && right && bottom)) return;
    if (isMaximized()) {
        *left -= m_frames.left();
        *top -= m_frames.top();
        *right -= m_frames.right();
        *bottom -= m_frames.bottom();
    }
}

QRect CFramelessWindow::contentsRect() const
{
    QRect rect = QMainWindow::contentsRect();
    int width = rect.width();
    int height = rect.height();
    rect.setLeft(rect.left() - m_frames.left());
    rect.setTop(rect.top() - m_frames.top());
    rect.setWidth(width);
    rect.setHeight(height);
    return rect;
}

void CFramelessWindow::showFullScreen()
{
    if (isMaximized()) {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    QMainWindow::showFullScreen();
}

