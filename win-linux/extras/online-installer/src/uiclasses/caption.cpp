#include "caption.h"
#include "baseutils.h"
#include "metrics.h"
#include "drawningengine.h"
#include <windowsx.h>

#define RESIZE_AREA_PART 0.14


Caption::Caption(Widget *parent) :
    Label(parent),
    m_isResizingAvailable(true)
{
    m_hwndRoot = GetAncestor(m_hWnd, GA_ROOT);
}

Caption::~Caption()
{

}

void Caption::setResizingAvailable(bool isResizingAvailable)
{
    m_isResizingAvailable = isResizingAvailable;
}

bool Caption::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        //    DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (!m_title.empty())
            engine()->DrawText(rc, m_title);

        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN: {
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        if (postMsg(WM_NCLBUTTONDOWN)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_LBUTTONDBLCLK: {
        if (postMsg(WM_NCLBUTTONDBLCLK)) {
            *result = TRUE;
            return true;
        }
        return false;
    }

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE: {
        if (isResizingAvailable()) {
            int y = GET_Y_LPARAM(lParam);
            if (HCURSOR hCursor = LoadCursor(NULL, isPointInResizeArea(y) ? IDC_SIZENS : IDC_ARROW))
                SetCursor(hCursor);
        }
        break;
    }

    case WM_MOUSEENTER: {
        //palette()->setCurrentState(Palette::Hover);
        repaint();
        break;
    }

    case WM_MOUSELEAVE: {
        //palette()->setCurrentState(Palette::Normal);
        repaint();
        break;
    }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}

bool Caption::isResizingAvailable()
{
    return m_isResizingAvailable && Utils::getWinVersion() >= Utils::WinVer::Win10 && !IsZoomed(m_hwndRoot);
}

bool Caption::isPointInResizeArea(int posY)
{
    int w = 0, h = 0;
    size(&w, &h);
    return posY <= RESIZE_AREA_PART * h;
}

bool Caption::postMsg(DWORD cmd) {
    POINT pt;
    ::GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    ::ReleaseCapture();
    ::PostMessage(m_hwndRoot, cmd, isResizingAvailable() && isPointInResizeArea(pt.y) ? HTTOP : HTCAPTION, POINTTOPOINTS(pt));
    return true;
}
