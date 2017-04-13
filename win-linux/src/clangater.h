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

private:
    CLangater();
    QString m_lang;

    class CLangaterIntf;
    CLangaterIntf * m_intf;
};

#endif // CLANGATER_H
