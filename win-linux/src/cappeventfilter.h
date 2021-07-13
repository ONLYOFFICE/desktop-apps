#ifndef CAPPEVENTFILTER_H
#define CAPPEVENTFILTER_H

#include <QObject>


class CAscApplicationManagerWrapper_Private;
class CAppEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit CAppEventFilter(QObject *parent = nullptr);
    CAppEventFilter(CAscApplicationManagerWrapper_Private *);

protected:
    bool eventFilter(QObject *, QEvent *);

    CAscApplicationManagerWrapper_Private * app_private = nullptr;

signals:
public slots:
};

#endif // CAPPEVENTFILTER_H
