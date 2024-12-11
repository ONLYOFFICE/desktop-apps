#ifndef CAPTION_H
#define CAPTION_H

#include "label.h"
#include <Windows.h>


class Caption : public Label
{
public:
    Caption(Widget *parent = nullptr);
    ~Caption();

    void setResizingAvailable(bool);

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    bool isResizingAvailable();
    bool isPointInResizeArea(int posY);
    bool postMsg(DWORD cmd);

    HWND m_hwndRoot;
    bool m_isResizingAvailable;
};

#endif // CAPTION_H
