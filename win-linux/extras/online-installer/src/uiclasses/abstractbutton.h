#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "widget.h"
#include <unordered_map>


class AbstractButton : public Widget
{
public:
    AbstractButton(Widget *parent = nullptr, const std::wstring &text = L"");
    virtual ~AbstractButton();

    void setText(const std::wstring &text);

    /* callback */
    int onClick(const FnVoidVoid &callback);
    virtual void disconnect(int) override;

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void click();

    std::wstring m_text;

private:
    std::unordered_map<int, FnVoidVoid> m_click_callbacks;
};

#endif // ABSTRACTBUTTON_H
