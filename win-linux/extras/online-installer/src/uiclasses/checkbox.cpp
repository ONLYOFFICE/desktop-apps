#include "checkbox.h"
#include "drawningengine.h"
#include "metrics.h"
#include <windowsx.h>


CheckBox::CheckBox(Widget *parent, const std::wstring &text) :
    AbstractButton(parent, text),
    m_checked(false)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

CheckBox::~CheckBox()
{

}

void CheckBox::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

bool CheckBox::isChecked()
{
    return m_checked;
}

bool CheckBox::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->DrawCheckBox(m_text, m_hFont, m_checked);
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();

        engine()->End();

        *result = FALSE;
        return true;
    }

    default:
        break;
    }
    return AbstractButton::event(msg, wParam, lParam, result);
}

void CheckBox::click()
{
    m_checked = !m_checked;
    update();
    AbstractButton::click();
}
