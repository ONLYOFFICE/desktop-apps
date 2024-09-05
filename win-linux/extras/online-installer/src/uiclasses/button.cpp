#include "button.h"
#include "baseutils.h"
#include "drawningengine.h"
#include "metrics.h"
#include "palette.h"
#include <windowsx.h>


static bool isArrangingAllowed() {
    BOOL arranging = FALSE;
    SystemParametersInfoA(SPI_GETWINARRANGING, 0, &arranging, 0);
    return (arranging == TRUE);
}

Button::Button(Widget *parent, const std::wstring &text) :
    AbstractButton(parent, text),
    m_hIcon(nullptr),
    m_hMetaFile(nullptr),
    m_stockIcon(StockIcon::None),
    supportSnapLayouts(false),
    snapLayoutAllowed(false),
    snapLayoutTimerIsSet(false)
{

}

Button::~Button()
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
}

void Button::setIcon(const std::wstring &path, int w, int h)
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    m_hIcon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, w, h, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_SHARED);
    update();
}

void Button::setIcon(int id, int w, int h)
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    m_hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, w, h, LR_COPYFROMRESOURCE | LR_DEFAULTCOLOR | LR_SHARED);
    update();
}

void Button::setEMFIcon(const std::wstring &path, int w, int h)
{
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    m_hMetaFile = GetEnhMetaFile(path.c_str());
    //m_hMetaFile = new Metafile(path.c_str());
    update();
}

void Button::setEMFIcon(int id, int w, int h)
{
    if (m_hMetaFile) {
        //delete m_hMetaFile;
        DeleteEnhMetaFile(m_hMetaFile);
        m_hMetaFile = nullptr;
    }
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(id), RT_RCDATA)) {
        if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
            if (LPVOID pData = LockResource(hResData)) {
                DWORD dataSize = SizeofResource(hInst, hRes);
                if (dataSize > 0)
                    m_hMetaFile = SetEnhMetaFileBits(dataSize, (BYTE*)pData);
            }
            FreeResource(hResData);
        }
    }
    update();
}

void Button::setIconSize(int w, int h)
{
    metrics()->setMetrics(Metrics::IconWidth, w);
    metrics()->setMetrics(Metrics::IconHeight, h);
    update();
}

void Button::setSupportSnapLayouts()
{
    if (Utils::getWinVersion() > Utils::WinVer::Win10) {
        snapLayoutAllowed = isArrangingAllowed();
        supportSnapLayouts = true;
    }
}

void Button::setStockIcon(StockIcon stockIcon)
{
    m_stockIcon = stockIcon;
    update();
}

bool Button::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->FillBackground();
        // engine()->DrawRoundedRect();
        if (metrics()->value(Metrics::BorderWidth) != 0)
            engine()->DrawBorder();
        if (m_hIcon)
            engine()->DrawIcon(m_hIcon);
        if (m_hMetaFile)
            engine()->DrawEmfIcon(m_hMetaFile);
        if (!m_text.empty())
            engine()->DrawText(rc, m_text);

        if (m_stockIcon == StockIcon::CloseIcon)
            engine()->DrawStockCloseIcon();
        else
        if (m_stockIcon == StockIcon::RestoreIcon)
            engine()->DrawStockRestoreIcon();
        else
        if (m_stockIcon == StockIcon::MinimizeIcon)
            engine()->DrawStockMinimizeIcon();
        else
        if (m_stockIcon == StockIcon::MaximizeIcon)
            engine()->DrawStockMaximizeIcon();

        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_NCHITTEST: {
        if (supportSnapLayouts && snapLayoutAllowed) {
            if (!snapLayoutTimerIsSet) {
                snapLayoutTimerIsSet = true;
                palette()->setCurrentState(Palette::Hover);
                SetTimer(m_hWnd, SNAP_LAYOUTS_TIMER_ID, 100, NULL);
                repaint();
            }
            *result = HTMAXBUTTON;
            return true;
        }
        return false;
    }

    case WM_TIMER: {
        if (wParam == SNAP_LAYOUTS_TIMER_ID) {
            if (!underMouse()) {
                KillTimer(m_hWnd, wParam);
                snapLayoutTimerIsSet = false;
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_CAPTURECHANGED: {
        if (Utils::getWinVersion() > Utils::WinVer::Win10) {
            click();
        }
        break;
    }

    default:
        break;
    }
    return AbstractButton::event(msg, wParam, lParam, result);
}
