#ifndef CAPPEVENTFILTER_H
#define CAPPEVENTFILTER_H

#include <QObject>
#include <memory>

class CAscApplicationManagerWrapper_Private;
class CAppEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit CAppEventFilter(QObject *parent = nullptr);
    CAppEventFilter(CAscApplicationManagerWrapper_Private *);

protected:
    bool eventFilter(QObject *, QEvent *);

    std::unique_ptr<CAscApplicationManagerWrapper_Private> app_private;

signals:
public slots:
};

#endif // CAPPEVENTFILTER_H
