#ifndef CLANGATER_H
#define CLANGATER_H

#include <QString>
#include <QJsonObject>

class CLangater
{
public:
    ~CLangater();

    static CLangater * getInstance();
    static void init();
    static QString getCurrentLangCode();
    static QString getLangName(const QString& code = QString());
    static void addTranslation(const QString& dir, const QString& name);
    static void addTranslation(const QString& dir);

    static QJsonObject availableLangsToJson();

private:
    CLangater();
    QString m_lang;

    class CLangaterIntf;
    CLangaterIntf * m_intf;
};

#endif // CLANGATER_H
