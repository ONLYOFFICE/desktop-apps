#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "abstractbutton.h"


class CheckBox : public AbstractButton
{
public:
    CheckBox(Widget *parent = nullptr, const std::wstring &text = L"");
    virtual ~CheckBox();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void click() override;

private:
    bool m_checked;
};

#endif // CHECKBOX_H
