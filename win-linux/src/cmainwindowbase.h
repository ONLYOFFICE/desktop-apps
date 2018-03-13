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

protected:
};

#endif // CMAINWINDOWBASE_H
