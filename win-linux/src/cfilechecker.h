#ifndef CFILECHECKER_H
#define CFILECHECKER_H

#include <QThread>

class CFileChecker : public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

    QString m_inJson;

public:
    explicit CFileChecker(const QString& json);

    void abort();

signals:
    void resultReady(const QString&);
};

#endif // CFILECHECKER_H
