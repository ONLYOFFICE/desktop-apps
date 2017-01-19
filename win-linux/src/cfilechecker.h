#ifndef CFILECHECKER_H
#define CFILECHECKER_H

#include <set>
#include <QThread>

class CFileInspector : public QThread
{
    Q_OBJECT

public:
    CFileInspector(QObject *parent, const QString&, int);

signals:
    void examined(const QString&, int, int);

private:
    const QString m_file;
    const int m_uid;

    void run() Q_DECL_OVERRIDE;
};

class CExistanceController : public QObject
{
    Q_OBJECT

public:
    ~CExistanceController();

    static CExistanceController * getInstance();
    static void check(const QString& json);

public slots:
    void handleResults(const QString&, int, int);

signals:
    void operate(const QString&);
    void checked(const QString&, int, bool);

private:
    CExistanceController();

    void parseJson(const QString&);
    void processMap();

private:
    QMutex m_mutex;
    std::map<int, CFileInspector *> m_mapStaff;
    std::set<int> m_setUncheck;
};

#endif // CFILECHECKER_H
