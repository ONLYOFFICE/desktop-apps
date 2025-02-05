#ifndef LABEL_H
#define LABEL_H

#include "widget.h"
#include <Windows.h>
#include <gdiplus.h>


class Label : public Widget
{
public:
    Label(Widget *parent = nullptr);
    virtual ~Label();

    void setText(const std::wstring &text, bool multiline = false);
    void setIcon(const std::wstring &path, int w, int h);
    void setIcon(int id, int w, int h);
    void setEMFIcon(const std::wstring &path, int w, int h);
    void setEMFIcon(int id, int w, int h);
    void setImage(int id, int w, int h);
    void setIconSize(int w, int h);
    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    std::wstring m_text;
    HICON m_hIcon;
    Gdiplus::Bitmap *m_hEmfBmp;
    Gdiplus::Bitmap *m_hBmp;
    bool  m_multiline;
};

#endif // LABEL_H
