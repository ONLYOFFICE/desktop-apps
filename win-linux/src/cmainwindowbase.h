#ifndef CMAINWINDOWBASE_H
#define CMAINWINDOWBASE_H

#include <QWidget>
#include "cmainpanel.h"

class CMainWindowBase
{
public:
    CMainWindowBase();

    virtual CMainPanel * mainPanel() const = 0;
    virtual QRect windowRect() const = 0;
    virtual bool isMaximized() const = 0;
    virtual int attachEditor(QWidget *, int index = -1);
    virtual int attachEditor(QWidget *, const QPoint&);
    virtual bool pointInTabs(const QPoint& pt);
    virtual bool movedByTab();
    virtual QWidget * getEditor(int index);
    virtual bool holdView(int id) const;
    virtual int editorsCount() const;
    virtual int editorsCount(const wstring& portal) const;
    virtual QString documentName(int vid);
//    virtual WId handle() const = 0;

protected:
};

#endif // CMAINWINDOWBASE_H
