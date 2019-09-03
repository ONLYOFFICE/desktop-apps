#ifndef CTABUNDOCKEVENT_H
#define CTABUNDOCKEVENT_H

#include <QEvent>
#include <QWidget>

//const QEvent::Type TAB_UNDOCK_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);

class CTabUndockEvent : public QEvent
{
public:
    CTabUndockEvent(QWidget * panel)
        : QEvent(CTabUndockEvent::type())
        , m_widget(panel)
    {
        ignore();
    }

    static QEvent::Type type()
    {
        return static_cast<QEvent::Type>(QEvent::User + 1);
    }

    QWidget * panel()
    {
        return m_widget;
    }

private:
    QWidget * m_widget = nullptr;
};

#endif // CTABUNDOCKEVENT_H
