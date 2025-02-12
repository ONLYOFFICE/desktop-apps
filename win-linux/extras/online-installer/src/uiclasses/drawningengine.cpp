#include "drawningengine.h"
#include "drawingsurface.h"
#include "palette.h"
#include "metrics.h"
#include <gdiplusheaders.h>


static Gdiplus::Color ColorFromColorRef(COLORREF rgb)
{
    Gdiplus::Color color;
    color.SetFromCOLORREF(rgb);
    return color;
}

static void RoundedPath(Gdiplus::GraphicsPath &ph, int x, int y, int width, int height, int rad)
{
    ph.AddArc(x, y, rad * 2, rad * 2, 180, 90);
    ph.AddLine(x + rad, y, x + width - rad, y);
    ph.AddArc(x + width - rad * 2, y, rad * 2, rad * 2, 270, 90);
    ph.AddLine(x + width, y + rad, x + width, y + height - rad);
    ph.AddArc(x + width - rad * 2, y + height - rad * 2, rad * 2, rad * 2, 0, 90);
    ph.AddLine(x + width - rad, y + height, x + rad, y + height);
    ph.AddArc(x, y + height - rad * 2, rad * 2, rad * 2, 90, 90);
    ph.AddLine(x, y + height - rad, x, y + rad);
    ph.CloseFigure();
}

DrawingEngine::DrawingEngine() :
    m_ds(nullptr),
    m_ps(nullptr),
    m_hwnd(nullptr),
    m_hdc(nullptr),
    m_memDC(nullptr),
    m_memBmp(nullptr),
    m_oldBmp(nullptr),
    m_graphics(nullptr)
{

}

DrawingEngine* DrawingEngine::instance()
{
    static DrawingEngine inst;
    return &inst;
}

DrawingEngine::~DrawingEngine()
{

}

DrawningSurface *DrawingEngine::surface()
{
    return m_ds;
}

void DrawingEngine::Begin(DrawningSurface *ds, HWND hwnd, RECT *rc)
{
    if (m_ds) {
        printf("Engine is buisy...\n");
        fflush(stdout);
        return;
    }
    m_ds = ds;
    m_rc = rc;
    m_hwnd = hwnd;
    m_ps = new PAINTSTRUCT;
    m_hdc = BeginPaint(hwnd, m_ps);
}

void DrawingEngine::FillBackground() const
{
    HBRUSH bkgBrush = CreateSolidBrush(m_ds->palette()->color(Palette::Background));
    HBRUSH oldBkgBrush = (HBRUSH)SelectObject(m_hdc, bkgBrush);
    FillRect(m_hdc, m_rc, bkgBrush);
    SelectObject(m_hdc, oldBkgBrush);
    DeleteObject(bkgBrush);
}

// void DrawingEngine::DrawRoundedRect()
// {
//     int x = m_rc->left + m_ds->metrics()->value(Metrics::BorderWidth) - 1;
//     int y = m_rc->top + m_ds->metrics()->value(Metrics::BorderWidth) - 1;
//     int width = m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::BorderWidth) * 2 + 1;
//     int height = m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::BorderWidth) * 2 + 1;
//     int rad = m_ds->metrics()->value(Metrics::BorderRadius);

//     m_memDC = CreateCompatibleDC(m_hdc);
//     m_memBmp = CreateCompatibleBitmap(m_hdc, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top);
//     m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

//     m_graphics = new Gdiplus::Graphics(m_memDC);
//     m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
//     m_graphics->Clear(ColorFromColorRef(m_ds->palette()->color(Palette::Background)));

//     Gdiplus::GraphicsPath ph;
//     RoundedPath(ph, x, y, width, height, rad);

//     Gdiplus::SolidBrush brush(ColorFromColorRef(m_ds->palette()->color(Palette::Base)));
//     m_graphics->FillPath(&brush, &ph);

//     if (m_ds->metrics()->value(Metrics::BorderWidth) != 0) {
//         Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Border)), m_ds->metrics()->value(Metrics::BorderWidth));
//         m_graphics->DrawPath(&pen, &ph);
//     }

//     BitBlt(m_hdc, m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, m_memDC, 0, 0, SRCCOPY);

//     delete m_graphics;
//     m_graphics = nullptr;
//     SelectObject(m_memDC, m_oldBmp);
//     m_oldBmp = nullptr;
//     DeleteObject(m_memBmp);
//     m_memBmp = nullptr;
//     DeleteDC(m_memDC);
//     m_memDC = nullptr;
// }

void DrawingEngine::DrawBorder() const
{
    RECT rc;
    SetRect(&rc, m_rc->left, m_rc->top, m_rc->right, m_rc->bottom);
    DWORD dwOldLayout = GetLayout(m_hdc);
    if (dwOldLayout & LAYOUT_RTL)
        rc.right -= 1;
    HBRUSH brdBrush = CreateSolidBrush(m_ds->palette()->color(Palette::Border));
    HBRUSH oldBrdBrush = (HBRUSH)SelectObject(m_hdc, brdBrush);
    for (int i = 0; i < m_ds->metrics()->value(Metrics::BorderWidth); i++) {
        FrameRect(m_hdc, &rc, brdBrush);
        rc.left += 1;
        rc.top += 1;
        rc.right -= 1;
        rc.bottom -= 1;
    }
    SelectObject(m_hdc, oldBrdBrush);
    DeleteObject(brdBrush);
}

void DrawingEngine::DrawTopBorder(int brdWidth, COLORREF brdColor) const
{
    HPEN pen = CreatePen(PS_SOLID, brdWidth, brdColor);
    HPEN oldPen = (HPEN)SelectObject(m_hdc, pen);
    MoveToEx(m_hdc, m_rc->left - 1, m_rc->top + brdWidth - 1, NULL);
    LineTo(m_hdc, m_rc->right, m_rc->top + brdWidth - 1);
    SelectObject(m_hdc, oldPen);
    DeleteObject(pen);
}

void DrawingEngine::DrawIcon(HICON hIcon) const
{
    int x = m_rc->left + (m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::IconWidth)) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;
    DrawIconEx(m_hdc, x, y, hIcon, m_ds->metrics()->value(Metrics::IconWidth), m_ds->metrics()->value(Metrics::IconHeight), 0, NULL, DI_NORMAL);
}

void DrawingEngine::DrawEmfIcon(Gdiplus::Bitmap *hEmfBmp) const
{
    int w = m_ds->metrics()->value(Metrics::IconWidth);
    int h = m_ds->metrics()->value(Metrics::IconHeight);
    int x = m_rc->left + (m_rc->right - m_rc->left - w) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - h) / 2;
    Gdiplus::Graphics gr(m_hdc);
    // gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    // gr.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    // gr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    gr.DrawImage(hEmfBmp, x, y, w, h);
}

void DrawingEngine::DrawImage(Gdiplus::Bitmap *hBmp) const
{
    int x = m_rc->left + (m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::IconWidth)) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;
    Gdiplus::Graphics gr(m_hdc);
    gr.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
    gr.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    gr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    gr.DrawImage(hBmp, x, y, m_ds->metrics()->value(Metrics::IconWidth), m_ds->metrics()->value(Metrics::IconHeight));
}

void DrawingEngine::DrawStockCloseIcon()
{
    HPEN hPen = CreatePen(PS_SOLID, m_ds->metrics()->value(Metrics::PrimitiveWidth), m_ds->palette()->color(Palette::Primitive));
    HPEN oldPen = (HPEN)SelectObject(m_hdc, hPen);
    int x = m_rc->left + (m_rc->right - m_rc->left)/2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top)/2;
    MoveToEx(m_hdc, x, y, NULL);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth)/2, y + m_ds->metrics()->value(Metrics::IconHeight)/2);
    MoveToEx(m_hdc, x, y, NULL);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth)/2, y - m_ds->metrics()->value(Metrics::IconHeight)/2);
    MoveToEx(m_hdc, x, y, NULL);
    LineTo(m_hdc, x - m_ds->metrics()->value(Metrics::IconWidth)/2, y + m_ds->metrics()->value(Metrics::IconHeight)/2);
    MoveToEx(m_hdc, x, y, NULL);
    LineTo(m_hdc, x - m_ds->metrics()->value(Metrics::IconWidth)/2, y - m_ds->metrics()->value(Metrics::IconHeight)/2);
    SelectObject(m_hdc, oldPen);
    DeleteObject(hPen);
}

void DrawingEngine::DrawStockMinimizeIcon()
{
    HPEN hPen = CreatePen(PS_SOLID, m_ds->metrics()->value(Metrics::PrimitiveWidth), m_ds->palette()->color(Palette::Primitive));
    HPEN oldPen = (HPEN)SelectObject(m_hdc, hPen);
    int x = m_rc->left + (m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::IconWidth)) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;
    MoveToEx(m_hdc, x, y + m_ds->metrics()->value(Metrics::IconHeight)/2, NULL);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth), y + m_ds->metrics()->value(Metrics::IconHeight)/2);
    SelectObject(m_hdc, oldPen);
    DeleteObject(hPen);
}

void DrawingEngine::DrawStockMaximizeIcon()
{
    HPEN hPen = CreatePen(PS_SOLID, m_ds->metrics()->value(Metrics::PrimitiveWidth), m_ds->palette()->color(Palette::Primitive));
    HPEN oldPen = (HPEN)SelectObject(m_hdc, hPen);
    int x = m_rc->left + (m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::IconWidth)) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;
    int quarterw = m_ds->metrics()->value(Metrics::IconWidth)/4;
    int restw = m_ds->metrics()->value(Metrics::IconWidth) - quarterw;
    int quarterh = m_ds->metrics()->value(Metrics::IconHeight)/4;
    int resth = m_ds->metrics()->value(Metrics::IconHeight) - quarterh;
    MoveToEx(m_hdc, x, y + quarterh, NULL);
    LineTo(m_hdc, x + restw - 1, y + quarterh);
    LineTo(m_hdc, x + restw - 1, y + m_ds->metrics()->value(Metrics::IconHeight) - 1);
    LineTo(m_hdc, x, y + m_ds->metrics()->value(Metrics::IconHeight) - 1);
    LineTo(m_hdc, x, y + quarterh + m_ds->metrics()->value(Metrics::PrimitiveWidth) - 1);
    MoveToEx(m_hdc, x + quarterw, y + quarterh, NULL);
    LineTo(m_hdc, x + quarterw, y);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth) - 1, y);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth) - 1, y + resth - 1);
    LineTo(m_hdc, x + restw - 1, y + resth - 1);
    SelectObject(m_hdc, oldPen);
    DeleteObject(hPen);
}

void DrawingEngine::DrawStockRestoreIcon()
{
    HPEN hPen = CreatePen(PS_SOLID, m_ds->metrics()->value(Metrics::PrimitiveWidth), m_ds->palette()->color(Palette::Primitive));
    HPEN oldPen = (HPEN)SelectObject(m_hdc, hPen);
    int x = m_rc->left + (m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::IconWidth)) / 2;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;
    MoveToEx(m_hdc, x, y, NULL);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth) - 1, y);
    LineTo(m_hdc, x + m_ds->metrics()->value(Metrics::IconWidth) - 1, y + m_ds->metrics()->value(Metrics::IconHeight) - 1);
    LineTo(m_hdc, x, y + m_ds->metrics()->value(Metrics::IconHeight) - 1);
    LineTo(m_hdc, x, y + m_ds->metrics()->value(Metrics::PrimitiveWidth) - 1);
    SelectObject(m_hdc, oldPen);
    DeleteObject(hPen);
}

void DrawingEngine::DrawCheckBox(const std::wstring &text, HFONT hFont, bool checked)
{
    int x = m_rc->left + 1;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;

    m_memDC = CreateCompatibleDC(m_hdc);
    m_memBmp = CreateCompatibleBitmap(m_hdc, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top);
    m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

    m_graphics = new Gdiplus::Graphics(m_memDC);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->Clear(ColorFromColorRef(m_ds->palette()->color(Palette::Background)));

    DWORD dwOldLayout = GetLayout(m_memDC);
    Gdiplus::Matrix origMatrix;
    m_graphics->GetTransform(&origMatrix);
    if (dwOldLayout & LAYOUT_RTL) {
        Gdiplus::Matrix rtlMatrix(-1.0f, 0.0f, 0.0f, 1.0f, float(m_rc->right + m_rc->left - 1), 0.0f);
        m_graphics->SetTransform(&rtlMatrix);
    }

    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), m_ds->metrics()->value(Metrics::PrimitiveWidth));
    Gdiplus::Rect rc(x, y, m_ds->metrics()->value(Metrics::IconWidth) - 1, m_ds->metrics()->value(Metrics::IconHeight) - 1);
    // m_graphics->DrawRectangle(&pen, rc);
    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, rc.X, rc.Y, rc.Width, rc.Height, m_ds->metrics()->value(Metrics::PrimitiveRadius));
    m_graphics->DrawPath(&pen, &ph);
    if (checked) {
        pen.SetWidth(m_ds->metrics()->value(Metrics::AlternatePrimitiveWidth));
        pen.SetColor(ColorFromColorRef(m_ds->palette()->color(Palette::AlternatePrimitive)));
        Gdiplus::PointF pts[3] = {
            Gdiplus::PointF(float(x + 2), float(y + m_ds->metrics()->value(Metrics::IconHeight)/2 - 1)),
            Gdiplus::PointF(float(x + m_ds->metrics()->value(Metrics::IconWidth)/2 - 2), float(y + m_ds->metrics()->value(Metrics::IconHeight) - 5)),
            Gdiplus::PointF(float(x + m_ds->metrics()->value(Metrics::IconWidth) - 3), float(y + 4))
        };
        m_graphics->DrawLines(&pen, pts, 3);
    }
    if (!text.empty()) {
        RECT rc;
        int offset = (dwOldLayout & LAYOUT_RTL) ? m_ds->metrics()->value(Metrics::IconWidth) : 0;
        SetRect(&rc, m_rc->left + m_ds->metrics()->value(Metrics::IconWidth) - offset, m_rc->top, m_rc->right - offset, m_rc->bottom);
        m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        m_graphics->SetTransform(&origMatrix);
        LayeredDrawText(rc, text, hFont, dwOldLayout & LAYOUT_RTL);
    }
    StretchBlt(m_hdc, m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, m_memDC, 0, 0, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, SRCCOPY);

    delete m_graphics;
    m_graphics = nullptr;
    SelectObject(m_memDC, m_oldBmp);
    m_oldBmp = nullptr;
    DeleteObject(m_memBmp);
    m_memBmp = nullptr;
    DeleteDC(m_memDC);
    m_memDC = nullptr;
}

void DrawingEngine::DrawRadioButton(const std::wstring &text, HFONT hFont, bool checked)
{
    int x = m_rc->left + 1;
    int y = m_rc->top + (m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::IconHeight)) / 2;

    m_memDC = CreateCompatibleDC(m_hdc);
    m_memBmp = CreateCompatibleBitmap(m_hdc, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top);
    m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

    m_graphics = new Gdiplus::Graphics(m_memDC);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    m_graphics->Clear(ColorFromColorRef(m_ds->palette()->color(Palette::Background)));

    DWORD dwOldLayout = GetLayout(m_memDC);
    Gdiplus::Matrix origMatrix;
    m_graphics->GetTransform(&origMatrix);
    if (dwOldLayout & LAYOUT_RTL) {
        Gdiplus::Matrix rtlMatrix(-1.0f, 0.0f, 0.0f, 1.0f, float(m_rc->right + m_rc->left - 1), 0.0f);
        m_graphics->SetTransform(&rtlMatrix);
    }

    Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Primitive)), m_ds->metrics()->value(Metrics::PrimitiveWidth));
    m_graphics->DrawEllipse(&pen, x, y, m_ds->metrics()->value(Metrics::IconHeight) - 1, m_ds->metrics()->value(Metrics::IconHeight) - 1);
    if (checked) {
        Gdiplus::SolidBrush chunkBrush(ColorFromColorRef(m_ds->palette()->color(Palette::AlternatePrimitive)));
        m_graphics->FillEllipse(&chunkBrush, float(x) + 2.7f, float(y) + 2.7f, float(m_ds->metrics()->value(Metrics::IconHeight)) - 5.4f - 1.0f, float(m_ds->metrics()->value(Metrics::IconHeight)) - 5.4f - 1.0f);
    }    
    if (!text.empty()) {
        RECT rc;
        int offset = (dwOldLayout & LAYOUT_RTL) ? m_ds->metrics()->value(Metrics::IconWidth) : 0;
        SetRect(&rc, m_rc->left + m_ds->metrics()->value(Metrics::IconWidth) - offset, m_rc->top, m_rc->right - offset, m_rc->bottom);
        m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        m_graphics->SetTransform(&origMatrix);
        LayeredDrawText(rc, text, hFont, dwOldLayout & LAYOUT_RTL);
    }
    StretchBlt(m_hdc, m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, m_memDC, 0, 0, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, SRCCOPY);

    delete m_graphics;
    m_graphics = nullptr;
    SelectObject(m_memDC, m_oldBmp);
    m_oldBmp = nullptr;
    DeleteObject(m_memBmp);
    m_memBmp = nullptr;
    DeleteDC(m_memDC);
    m_memDC = nullptr;
}

void DrawingEngine::DrawProgressBar(int progress, int pulse_pos)
{
    int x = m_rc->left + m_ds->metrics()->value(Metrics::BorderWidth) + m_ds->metrics()->value(Metrics::IconMarginLeft);
    int y = m_rc->top + m_ds->metrics()->value(Metrics::BorderWidth) + m_ds->metrics()->value(Metrics::IconMarginTop);
    int width = m_rc->right - m_rc->left - m_ds->metrics()->value(Metrics::BorderWidth) * 2 -
                m_ds->metrics()->value(Metrics::IconMarginRight) - m_ds->metrics()->value(Metrics::IconMarginLeft) - 1;
    int height = m_rc->bottom - m_rc->top - m_ds->metrics()->value(Metrics::BorderWidth) * 2 -
                 m_ds->metrics()->value(Metrics::IconMarginBottom) - m_ds->metrics()->value(Metrics::IconMarginTop) - 1;
    int rad = m_ds->metrics()->value(Metrics::BorderRadius);

    m_memDC = CreateCompatibleDC(m_hdc);
    m_memBmp = CreateCompatibleBitmap(m_hdc, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top);
    m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

    m_graphics = new Gdiplus::Graphics(m_memDC);
    m_graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
    m_graphics->Clear(ColorFromColorRef(m_ds->palette()->color(Palette::Background)));

    Gdiplus::GraphicsPath ph;
    RoundedPath(ph, x, y, width, height, rad);

    Gdiplus::SolidBrush prgBrush(ColorFromColorRef(m_ds->palette()->color(Palette::Base)));
    m_graphics->FillPath(&prgBrush, &ph);
    {
        int _x = x, _width;
        if (pulse_pos != -1) {
            _width = width/5;
            _x = x + (int)round(double((width - _width) * pulse_pos)/100);
        } else {
            if (progress < 0)
                progress = 0;
            else
            if (progress > 100)
                progress = 100;
            _width = (int)round(double(width * progress)/100);
        }
        Gdiplus::GraphicsPath _ph;
        RoundedPath(_ph, _x, y, _width, height, rad);

        Gdiplus::SolidBrush chunkBrush(ColorFromColorRef(m_ds->palette()->color(Palette::AlternateBase)));
        m_graphics->FillPath(&chunkBrush, &_ph);
    }

    if (m_ds->metrics()->value(Metrics::BorderWidth) != 0) {
        Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Border)), m_ds->metrics()->value(Metrics::BorderWidth));
        m_graphics->DrawPath(&pen, &ph);
    }

    StretchBlt(m_hdc, m_rc->left, m_rc->top, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, m_memDC, 0, 0, m_rc->right - m_rc->left, m_rc->bottom - m_rc->top, SRCCOPY);

    delete m_graphics;
    m_graphics = nullptr;
    SelectObject(m_memDC, m_oldBmp);
    m_oldBmp = nullptr;
    DeleteObject(m_memBmp);
    m_memBmp = nullptr;
    DeleteDC(m_memDC);
    m_memDC = nullptr;
}

void DrawingEngine::DrawText(const RECT &rc, const std::wstring &text, HFONT hFont, bool multiline) const
{
    HFONT hOldFont = (HFONT) SelectObject(m_hdc, hFont);
    SetBkMode(m_hdc, TRANSPARENT);
    SetTextColor(m_hdc, m_ds->palette()->color(Palette::Text));
    RECT _rc{rc.left + m_ds->metrics()->value(Metrics::TextMarginLeft), rc.top + m_ds->metrics()->value(Metrics::TextMarginTop),
             rc.right - m_ds->metrics()->value(Metrics::TextMarginRight), rc.bottom - m_ds->metrics()->value(Metrics::TextMarginBottom)};
    UINT fmt = multiline ? 0 : DT_SINGLELINE;
    UINT algn = m_ds->metrics()->value(Metrics::TextAlignment);
    if (algn & Metrics::AlignHLeft)
        fmt |= DT_LEFT;
    if (algn & Metrics::AlignHCenter)
        fmt |= DT_CENTER;
    if (algn & Metrics::AlignHRight)
        fmt |= DT_RIGHT;
    if (algn & Metrics::AlignVTop)
        fmt |= DT_TOP;
    if (algn & Metrics::AlignVCenter)
        fmt |= DT_VCENTER;
    if (algn & Metrics::AlignVBottom)
        fmt |= DT_BOTTOM;
    ::DrawText(m_hdc, text.c_str(), text.length(), &_rc, fmt);
    SelectObject(m_hdc, hOldFont);
    SetBkMode(m_hdc, OPAQUE);
}

void DrawingEngine::End()
{
    EndPaint(m_hwnd, m_ps);
    delete m_ps;
    m_ps = nullptr;
    m_hdc = nullptr;
    m_hwnd = nullptr;
    m_rc = nullptr;
    m_ds = nullptr;
}

// void DrawingEngine::LayeredBegin(DrawningSurface *ds, HWND hwnd, RECT *rc)
// {
//     if (m_ds) {
//         printf("Engine is buisy....\n");
//         fflush(stdout);
//         return;
//     }
//     m_ds = ds;
//     m_rc = rc;
//     m_hwnd = hwnd;
//     m_hdc = GetDC(m_hwnd);
//     m_memDC = CreateCompatibleDC(m_hdc);
//     m_memBmp = CreateCompatibleBitmap(m_hdc, rc->right - rc->left, rc->bottom - rc->top);
//     m_oldBmp = (HBITMAP)SelectObject(m_memDC, m_memBmp);

//     m_graphics = new Gdiplus::Graphics(m_memDC);
//     m_graphics->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
//     // gr->SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
//     // gr->SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
// }

// void DrawingEngine::LayeredDrawRoundedRect() const
// {
//     int x = m_rc->left + m_ds->metrics()->value(Metrics::ShadowWidth) + m_ds->metrics()->value(Metrics::BorderWidth) - 1;
//     int y = m_rc->top + m_ds->metrics()->value(Metrics::ShadowWidth) + m_ds->metrics()->value(Metrics::BorderWidth) - 1;
//     int width = m_rc->right - m_rc->left - (m_ds->metrics()->value(Metrics::ShadowWidth) + m_ds->metrics()->value(Metrics::BorderWidth)) * 2 + 1;
//     int height = m_rc->bottom - m_rc->top - (m_ds->metrics()->value(Metrics::ShadowWidth) + m_ds->metrics()->value(Metrics::BorderWidth)) * 2 + 1;

//     int rad = m_ds->metrics()->value(Metrics::BorderRadius);
//     Gdiplus::GraphicsPath ph;
//     RoundedPath(ph, x, y, width, height, rad);

//     if (m_ds->metrics()->value(Metrics::BorderWidth) != 0) {
//         Gdiplus::Pen pen(ColorFromColorRef(m_ds->palette()->color(Palette::Border)), m_ds->metrics()->value(Metrics::BorderWidth));
//         m_graphics->DrawPath(&pen, &ph);
//     }
//     Gdiplus::SolidBrush brush(ColorFromColorRef(m_ds->palette()->color(Palette::Background)));
//     m_graphics->FillPath(&brush, &ph);
// }

void DrawingEngine::LayeredDrawText(RECT &rc, const std::wstring &text, HFONT hFont, bool rtl) const
{
//     Gdiplus::FontFamily fntFam(L"Segoe UI");
//     Gdiplus::Font font(&fntFam, m_ds->metrics()->value(Metrics::FontHeight), Gdiplus::FontStyleRegular, Gdiplus::Unit::UnitPixel);
    LOGFONTW logFont = {0};
    GetObject(hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(m_memDC, &logFont);
    Gdiplus::RectF rcF(rc.left + m_ds->metrics()->value(Metrics::TextMarginLeft), rc.top + m_ds->metrics()->value(Metrics::TextMarginTop),
                       rc.right - m_ds->metrics()->value(Metrics::TextMarginRight) - rc.left - m_ds->metrics()->value(Metrics::TextMarginLeft),
                       rc.bottom - m_ds->metrics()->value(Metrics::TextMarginBottom) - rc.top - m_ds->metrics()->value(Metrics::TextMarginTop));
    Gdiplus::StringAlignment h_algn, v_algn;
    UINT algn = m_ds->metrics()->value(Metrics::TextAlignment);
    if (algn & Metrics::AlignHLeft)
        h_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignHCenter)
        h_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignHRight)
        h_algn = Gdiplus::StringAlignmentFar;
    if (algn & Metrics::AlignVTop)
        v_algn = Gdiplus::StringAlignmentNear;
    if (algn & Metrics::AlignVCenter)
        v_algn = Gdiplus::StringAlignmentCenter;
    if (algn & Metrics::AlignVBottom)
        v_algn = Gdiplus::StringAlignmentFar;
    Gdiplus::StringFormat strFmt;
    strFmt.SetAlignment(h_algn);
    strFmt.SetLineAlignment(v_algn);
    if (rtl)
        strFmt.SetFormatFlags(Gdiplus::StringFormatFlagsDirectionRightToLeft);
    Gdiplus::SolidBrush brush(ColorFromColorRef(m_ds->palette()->color(Palette::Text)));
    m_graphics->DrawString(text.c_str(), -1, &font, rcF, &strFmt, &brush);
}

// void DrawingEngine::LayeredDrawShadow(int shadowWidth, int rad)
// {
// #define SHADOW_TRANSPATENCY 0x26
//     for (int i = 0; i < shadowWidth; i++) {
//         int x = m_rc->left + i;
//         int y = m_rc->top + i;
//         int width = m_rc->right - m_rc->left - i * 2 - 1;
//         int height = m_rc->bottom - m_rc->top - i * 2 - 1;

//         Gdiplus::GraphicsPath ph;
//         RoundedPath(ph, x, y, width, height, rad);

//         int alpha = shadowWidth > 1 ? SHADOW_TRANSPATENCY * (i * i) / ((shadowWidth - 1) * (shadowWidth - 1)) : SHADOW_TRANSPATENCY;
//         Gdiplus::Pen pen(Gdiplus::Color(alpha, 0, 0, 0), 1);
//         m_graphics->DrawPath(&pen, &ph);
//     }
// }

// void DrawingEngine::LayeredUpdate(BYTE alpha)
// {
//     RECT wrc;
//     GetWindowRect(m_hwnd, &wrc);
//     HDC scrDC = GetDC(NULL);
//     POINT ptSrc = {0, 0};
//     POINT ptDst = {wrc.left, wrc.top};
//     SIZE szDst = {wrc.right - wrc.left, wrc.bottom - wrc.top};
//     BLENDFUNCTION bf;
//     bf.AlphaFormat = AC_SRC_ALPHA;
//     bf.BlendFlags = 0;
//     bf.BlendOp = AC_SRC_OVER;
//     bf.SourceConstantAlpha = alpha;
//     UpdateLayeredWindow(m_hwnd, scrDC, &ptDst, &szDst, m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
//     ReleaseDC(NULL, scrDC);
// }

// void DrawingEngine::LayeredEnd()
// {
//     delete m_graphics;
//     m_graphics = nullptr;
//     SelectObject(m_memDC, m_oldBmp);
//     m_oldBmp = nullptr;
//     DeleteObject(m_memBmp);
//     m_memBmp = nullptr;
//     DeleteDC(m_memDC);
//     m_memDC = nullptr;
//     ReleaseDC(m_hwnd, m_hdc);
//     m_hdc = nullptr;
//     m_hwnd = nullptr;
//     m_rc = nullptr;
//     m_ds = nullptr;
// }
