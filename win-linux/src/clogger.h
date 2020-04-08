#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class CLogger : public QObject
{
    Q_OBJECT

public:
    explicit CLogger(QObject *parent, QString fileName);
    ~CLogger();

//    void setShowDateTime(bool value);
    static void log(const QString&);

private:
    QFile * m_file = nullptr;
    bool m_showDate = true;

signals:

public slots:
    void write(const QString &value);
};

#endif // CLOGGER_H
