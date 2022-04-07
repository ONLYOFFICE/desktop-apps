#ifndef CMAINWINDOWBASE_H
#define CMAINWINDOWBASE_H

#define WINDOW_MIN_WIDTH    500
#define WINDOW_MIN_HEIGHT   300

#define MAIN_WINDOW_MIN_WIDTH  960
#define MAIN_WINDOW_MIN_HEIGHT 661
#define MAIN_WINDOW_DEFAULT_SIZE QSize(1324,800)

#define BUTTON_MAIN_WIDTH   112
#define MAIN_WINDOW_BORDER_WIDTH 4
#define WINDOW_TITLE_MIN_WIDTH 200
#define TOOLBTN_HEIGHT      28
#define TOOLBTN_WIDTH       40
#define TITLE_HEIGHT        28

#ifdef _WIN32
# include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "cmainpanel.h"

namespace WindowBase
{
#ifdef _WIN32
    enum class Style : DWORD
    {
        windowed        = ( WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN ),
        aero_borderless = ( WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN )
    };

    struct CWindowGeometry
    {
        CWindowGeometry() {}

        bool required = false;
        int width = 0;
        int height = 0;
    };
#endif
}


class CElipsisLabel : public QLabel
{
public:
    CElipsisLabel(const QString &text, QWidget *parent=Q_NULLPTR);
    CElipsisLabel(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());

    auto setText(const QString&) -> void;
    auto setEllipsisMode(Qt::TextElideMode) -> void;
    auto updateText() -> void;
protected:
    void resizeEvent(QResizeEvent *event) override;

    using QLabel::setText;
private:
    QString orig_text;
    Qt::TextElideMode elide_mode = Qt::ElideRight;
};

class CMainWindowBase
{
public:
    CMainWindowBase();
    CMainWindowBase(QRect& rect);

    virtual ~CMainWindowBase();

    virtual void setScreenScalingFactor(double);
    //virtual bool holdView(int uid) const = 0;
    virtual QWidget * createMainPanel(QWidget * parent, const QString& title);
    virtual const QObject * receiver() {return nullptr;}
    virtual void setWindowTitle(const QString&);
    virtual void adjustGeometry();
    virtual void bringToTop() {}
    virtual void focus() {}
    virtual bool isCustomWindowStyle();
    virtual void updateScaling();
    virtual double scaling() const;


    virtual CMainPanel * mainPanel() const {return nullptr;}
    virtual QRect windowRect() const {return QRect();}
    //virtual bool isMaximized() const {return false;};
    virtual void bringToTop() const {}
    virtual int attachEditor(QWidget *, int index = -1);
    virtual int attachEditor(QWidget *, const QPoint&);
    virtual bool pointInTabs(const QPoint& pt) const;
    virtual QWidget * editor(int index);
    virtual bool holdView(int id) const;
    virtual void selectView(int id) const;
    virtual void selectView(const QString& url) const;
    virtual int editorsCount() const;
    virtual int editorsCount(const std::wstring& portal) const;
    virtual QString documentName(int vid);
    virtual void applyTheme(const std::wstring&);
    //virtual void updateScaling() = 0;

    virtual void captureMouse(int tab_index);


protected:
    double m_dpiRatio = 1;

    QWidget * m_boxTitleBtns = nullptr;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_pMainView = nullptr;

    QPushButton * m_buttonMinimize = nullptr;
    QPushButton * m_buttonMaximize = nullptr;
    QPushButton * m_buttonClose = nullptr;
    CElipsisLabel * m_labelTitle = nullptr;

    virtual void onCloseEvent();
    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual void onSizeEvent(int);
    virtual void onMoveEvent(const QRect&) {};
    virtual QPushButton * createToolButton(QWidget * parent = nullptr);
    virtual void onExitSizeMove();
    virtual void onDpiChanged(double newfactor, double prevfactor);
    virtual int calcTitleCaptionWidth();
    virtual void updateTitleCaption();

    inline int dpiCorrectValue(int v) const
    {
        return int(v * m_dpiRatio);
    }

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

#endif // CMAINWINDOWBASE_H
