#include "cappeventfilter.h"
#include "cascapplicationmanagerwrapper_private.h"
#include <QKeyEvent>

CAppEventFilter::CAppEventFilter(QObject *parent)
    : QObject(parent)
{
}

CAppEventFilter::CAppEventFilter(CAscApplicationManagerWrapper_Private * p)
    : QObject (nullptr)
    , app_private(p)
{

}

bool CAppEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);

        if ( app_private->handleAppKeyPress(keyEvent) )
            return true;
    }

    return QObject::eventFilter(obj, event);
}
