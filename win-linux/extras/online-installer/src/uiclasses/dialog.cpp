#include "dialog.h"
#include "metrics.h"
#include "palette.h"
#include "drawningengine.h"


Dialog::Dialog(Widget *parent, const Rect &rc) :
    Widget(parent, ObjectType::DialogType, rc)
{

}

Dialog::~Dialog()
{

}

bool Dialog::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            if (Widget *parent = parentWidget())
                EnableWindow(parent->nativeWindowHandle(), FALSE);
        }
        break;
    }

    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd);
        engine()->FillBackground(rc);
        //engine()->DrawRoundedRect(rc);
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder(rc);
        // if (!m_title.empty())
        //     engine()->DrawText(rc, m_title);


        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_MOUSEENTER: {
        palette()->setCurrentState(Palette::Hover);
        repaint();
        break;
    }

    case WM_NCMOUSELEAVE:
    case WM_MOUSELEAVE: {
        palette()->setCurrentState(Palette::Normal);
        repaint();
        break;
    }

    /*case WM_NCHITTEST: {
        *result = HTCAPTION;
        return true;
    }*/

    case WM_CLOSE: {
        if (Widget *parent = parentWidget())
            EnableWindow(parent->nativeWindowHandle(), TRUE);
        break;
    }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}
