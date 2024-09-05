#ifndef DRAWNINGENGINE_H
#define DRAWNINGENGINE_H

#include <Windows.h>
#include <gdiplus.h>
#include <string>


class DrawningSurface;
class DrawingEngine
{
public:
    DrawingEngine(const DrawingEngine&) = delete;
    DrawingEngine& operator=(const DrawingEngine&) = delete;
    static DrawingEngine *instance();


    DrawningSurface *surface();
    void Begin(DrawningSurface*, HWND, RECT *rc);
    void FillBackground() const;
    // void DrawRoundedRect();
    void DrawBorder() const;
    void DrawTopBorder(int, COLORREF) const;
    void DrawIcon(HICON hIcon) const;
    void DrawEmfIcon(HENHMETAFILE hIconc) const;
    void DrawImage(Gdiplus::Bitmap *hBmp) const;
    void DrawStockCloseIcon();
    void DrawStockMinimizeIcon();
    void DrawStockMaximizeIcon();
    void DrawStockRestoreIcon();
    void DrawCheckBox(const std::wstring &text, bool checked = false);
    void DrawRadioButton(const std::wstring &text, bool checked = false);
    void DrawProgressBar(int progress, int pulse_pos);
    void DrawText(const RECT &rc, const std::wstring &text, bool multiline = false) const;
    void End();

    // void LayeredBegin(DrawningSurface*, HWND, RECT *rc);
    // void LayeredDrawRoundedRect() const;
    void LayeredDrawText(RECT &rc, const std::wstring &text) const;
    // void LayeredDrawShadow(int shadowWidth, int rad);
    // void LayeredUpdate(BYTE alpha);
    // void LayeredEnd();

private:
    DrawingEngine();
    ~DrawingEngine();

    DrawningSurface *m_ds;
    RECT *m_rc;
    PAINTSTRUCT *m_ps;
    HWND m_hwnd;
    HDC  m_hdc;
    HDC  m_memDC;
    HBITMAP m_memBmp;
    HBITMAP m_oldBmp;
    Gdiplus::Graphics *m_graphics;
};

#endif // DRAWNINGENGINE_H
