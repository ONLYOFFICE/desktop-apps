#ifndef DIALOG_H
#define DIALOG_H

#include "widget.h"
#include <Windows.h>

#define DEFAULT_DLG_RECT Rect(100,100,800,600)


class Dialog : public Widget
{
public:
    Dialog(Widget *parent = nullptr, const Rect &rc = DEFAULT_DLG_RECT);
    virtual ~Dialog();

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:

};

#endif // DIALOG_H
