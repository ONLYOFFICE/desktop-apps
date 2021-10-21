#ifndef CLANGATER_H
#define CLANGATER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class CLangater : public QObject
{
    Q_OBJECT

public:
    ~CLangater();

    static CLangater * getInstance();
    static void init();
    static void reloadTranslations(const QString&);
    static void refreshLangs(const QMap<QString, QString>&);
    static QString getCurrentLangCode();
    static QString getLangName(const QString& code = QString());
    static void addTranslation(const QString& dir, const QString& name);
    static void addTranslation(const QString& dir);

    static QJsonObject availableLangsToJson();

signals:
    void onLangChanged(const QString&);

private:
    CLangater();
    QString m_lang;

    class CLangaterIntf;
    CLangaterIntf * m_intf;
};

#endif // CLANGATER_H
