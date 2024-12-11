#include "abstractbutton.h"
#include "palette.h"


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
