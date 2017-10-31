#ifndef CLANGATER_H
#define CLANGATER_H

#include <QString>

class CLangater
{
public:
    ~CLangater();

    static CLangater * getInstance();
    static void init();
    static QString getLanguageName();
    static void addTranslation(const QString& dir, const QString& name);
    static void addTranslation(const QString& dir);

private:
    CLangater();
    QString m_lang;

    class CLangaterIntf;
    CLangaterIntf * m_intf;
};

#endif // CLANGATER_H
