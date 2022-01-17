#ifndef CMAINWINDOWBASE_H
#define CMAINWINDOWBASE_H

#include <QWidget>
#include "cmainpanel.h"

class CMainWindowBase
{
public:
    CMainWindowBase();
    virtual ~CMainWindowBase();

    virtual CMainPanel * mainPanel() const = 0;
    virtual QRect windowRect() const = 0;
    virtual bool isMaximized() const = 0;
    virtual void bringToTop() const = 0;
    virtual int attachEditor(QWidget *, int index = -1);
    virtual int attachEditor(QWidget *, const QPoint&);
    virtual bool pointInTabs(const QPoint& pt) const;
//    virtual bool movedByTab();
    virtual QWidget * editor(int index);
    virtual bool holdView(int id) const;
    virtual void selectView(int id) const;
    virtual void selectView(const QString& url) const;
    virtual int editorsCount() const;
    virtual int editorsCount(const std::wstring& portal) const;
    virtual QString documentName(int vid);
//    virtual WId handle() const = 0;
    virtual void applyTheme(const std::wstring&);
    virtual void updateScaling() = 0;

    virtual void captureMouse(int tab_index);
protected:
};

#endif // CMAINWINDOWBASE_H
