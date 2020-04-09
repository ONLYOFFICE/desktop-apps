#ifndef CTABUNDOCKEVENT_H
#define CTABUNDOCKEVENT_H

#include <QEvent>
#include <QWidget>

//const QEvent::Type TAB_UNDOCK_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);

class CTabUndockEvent : public QEvent
{
public:
    CTabUndockEvent(int index)
        : QEvent(CTabUndockEvent::type())
        , m_index(index)
    {
        ignore();
    }

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

    QWidget * panel() const
    {
        return m_widget;
    }

    int index() const
    {
        return m_index;
    }

private:
    QWidget * m_widget = nullptr;
    int m_index = -1;
};

#endif // CTABUNDOCKEVENT_H
