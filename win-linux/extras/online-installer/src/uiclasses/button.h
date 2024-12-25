#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"
#include <gdiplus.h>


class Button : public AbstractButton
{
public:
    Button(Widget *parent = nullptr, const std::wstring &text = L"");
    virtual ~Button();

    enum StockIcon : BYTE {
        None,
        MinimizeIcon,
        MaximizeIcon,
        RestoreIcon,
        CloseIcon
    };

    void setIcon(const std::wstring &path, int w, int h);
    void setIcon(int id, int w, int h);
    void setEMFIcon(const std::wstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setIconSize(int w, int h);
    void setSupportSnapLayouts();
    void setStockIcon(StockIcon stockIcon);

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    HICON m_hIcon;
    Gdiplus::Bitmap *m_hEmfBmp;
    int  m_stockIcon;
    bool supportSnapLayouts,
         snapLayoutAllowed;
    bool snapLayoutTimerIsSet;
};

#endif // BUTTON_H
