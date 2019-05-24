#ifndef CSINGLEWINDOWPLATFORM_H
#define CSINGLEWINDOWPLATFORM_H

#include "csinglewindowbase.h"
#include "windows.h"
#include "cwinpanel.h"
#include "cwindowbase.h"
#include <QWidget>

#define TOP_NATIVE_WINDOW_HANDLE (HWND)m_pMainPanel->winId()

class CSingleWindowPlatform : public CSingleWindowBase
{
public:
    CSingleWindowPlatform(const QRect&, const QString&, QWidget *);
    virtual ~CSingleWindowPlatform();

    virtual void show(bool);
    virtual void hide();
    virtual bool visible();

    virtual Qt::WindowState windowState();
    virtual void setWindowState(Qt::WindowState);
    virtual void setWindowTitle(const QString&) override;
    virtual const QRect& geometry() const;

    void toggleBorderless(bool showmax);

protected:
    HWND m_hWnd;
    COLORREF m_bgColor;
    bool m_borderless = false;
    bool m_visible = false;
    bool m_closed = false;
    CWinPanel * m_pWinPanel;
    WindowBase::CWindowGeometry m_minSize;
    WindowBase::CWindowGeometry m_maxSize;

    void setMinimumSize(int width, int height);
    WindowBase::CWindowGeometry const& minimumSize() const;
    WindowBase::CWindowGeometry const& maximumSize() const;

    virtual void onSizeEvent(int);
    virtual void applyWindowState(Qt::WindowState);
    virtual void adjustGeometry();

//    virtual void focusMainPanel();

    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual void onScreenScalingFactor(uint f);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // CSINGLEWINDOWPLATFORM_H
