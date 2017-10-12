#ifndef CSINGLEWINDOW_H
#define CSINGLEWINDOW_H

#include <windows.h>
#include <QRect>
#include <QPushButton>

#include "cwindowbase.h"
#include "cwinpanel.h"

class CSingleWindow
{

private:
    bool m_borderless = false;
    bool m_visible = false;
    bool m_borderlessResizeable = true;
    bool m_closed = false;
    HWND m_hWnd = 0;
    uchar m_dpiRatio = 1;

    CWinPanel * m_pWinPanel;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_pMainView = nullptr;
    QWidget * m_boxTitleBtns = nullptr;
    WindowBase::CWindowGeometry minimumSize;
    WindowBase::CWindowGeometry maximumSize;

    QPushButton * m_pButtonMinimize;
    QPushButton * m_pButtonMaximize;
    QPushButton * m_pButtonClose;

    QWidget * createMainPanel(QWidget *, bool, QWidget *);
    void recalculatePlaces();
    void pushButtonCloseClicked();
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void focusMainPanel();
    void applyWindowState(Qt::WindowState);

public:
    CSingleWindow(const QRect&);
    CSingleWindow(const QRect&, const QString&, QWidget *);
    ~CSingleWindow();

    static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    void show(bool maximized = false);
    void hide();
    bool isVisible();

    void toggleBorderless(bool);
    void toggleResizeable();
//    bool isResizeable();

    void setMinimumSize( const int width, const int height );
    void removeMinimumSize();
    int getMinimumHeight() const;
    int getMinimumWidth() const;

    void setMaximumSize( const int width, const int height );
    int getMaximumHeight();
    int getMaximumWidth();
    void removeMaximumSize();
    void adjustGeometry();

    void setScreenScalingFactor(uchar);
    void doClose();

    bool holdView(int id) const;
};

#endif // CSINGLEWINDOW_H
