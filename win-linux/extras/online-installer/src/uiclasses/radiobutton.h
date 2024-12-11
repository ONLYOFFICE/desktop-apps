#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "abstractbutton.h"


class RadioButton : public AbstractButton
{
public:
    RadioButton(Widget *parent = nullptr, const std::wstring &text = L"");
    virtual ~RadioButton();

    void setChecked(bool checked);
    bool isChecked();

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
    virtual void click() override;

private:
    bool m_checked;
};

#endif // RADIOBUTTON_H
