#include "abstractbutton.h"
#include "palette.h"
#include "metrics.h"
#include <gdiplus.h>


AbstractButton::AbstractButton(Widget *parent, const std::wstring &text) :
    Widget(parent, ObjectType::WidgetType),
    m_text(text)
{

}

AbstractButton::~AbstractButton()
{

}

void AbstractButton::setText(const std::wstring &text)
{
    m_text = text;
    update();
}

void AbstractButton::adjustSizeBasedOnContent()
{
    HDC hdc = GetDC(nativeWindowHandle());
    Gdiplus::Graphics gr(hdc);
    LOGFONTW logFont = {0};
    GetObject(m_hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(hdc, &logFont);
    Gdiplus::RectF lutRc, boxRc;
    gr.MeasureString(m_text.c_str(), m_text.length(), &font, lutRc, &boxRc);
    ReleaseDC(nativeWindowHandle(), hdc);
    int w = boxRc.Width + 2*metrics()->value(Metrics::IconWidth) + metrics()->value(Metrics::TextMarginLeft) + metrics()->value(Metrics::TextMarginRight);
    int h = max(boxRc.Height + metrics()->value(Metrics::TextMarginTop) + metrics()->value(Metrics::TextMarginBottom), metrics()->value(Metrics::IconHeight));
    resize(w, h);
}

int AbstractButton::onClick(const FnVoidVoid &callback)
{
    m_click_callbacks[++m_connectionId] = callback;
    return m_connectionId;
}

void AbstractButton::disconnect(int connectionId)
{
    auto it = m_click_callbacks.find(connectionId);
    if (it != m_click_callbacks.end())
        m_click_callbacks.erase(it);
}

bool AbstractButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        return false;
    }

    case WM_LBUTTONUP: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
            click();
        }
        break;
    }

    case WM_MOUSEENTER: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Hover);
            repaint();
        }
        break;
    }

    case WM_MOUSELEAVE: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Normal);
            repaint();
        }
        break;
    }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}

void AbstractButton::click()
{
    if (underMouse()) {
        for (auto it = m_click_callbacks.begin(); it != m_click_callbacks.end(); it++) {
            if (it->second)
                (it->second)();
        }
    }
}
