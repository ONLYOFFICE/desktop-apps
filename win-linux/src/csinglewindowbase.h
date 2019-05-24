#ifndef CSINGLEWINDOWBASE_H
#define CSINGLEWINDOWBASE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class CSingleWindowBase
{
public:
    CSingleWindowBase();
    CSingleWindowBase(QRect& rect);

    virtual ~CSingleWindowBase();

    virtual void setScreenScalingFactor(int);
    virtual bool holdView(int uid) const = 0;
    virtual QWidget * createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * panel);
    virtual const QObject * receiver() = 0;
    virtual Qt::WindowState windowState() = 0;
    virtual void setWindowState(Qt::WindowState) = 0;
    virtual void setWindowTitle(const QString&);

protected:
    uint m_dpiRatio;

    QWidget * m_boxTitleBtns = nullptr;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_pMainView = nullptr;

    QPushButton * m_buttonMinimize = nullptr;
    QPushButton * m_buttonMaximize = nullptr;
    QPushButton * m_buttonClose = nullptr;
    QLabel * m_labelTitle = nullptr;

protected:
    virtual void onCloseEvent();
    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual QPushButton * createToolButton(QWidget * parent = nullptr);
    virtual void onScreenScalingFactor(uint f) = 0;
};

#endif // CSINGLEWINDOWBASE_H
