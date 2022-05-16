#include "clangater.h"
#include "defines.h"
#include "defines_p.h"
#include "utils.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QRegularExpression>
#include <QTranslator>

#include <list>
#include <algorithm>

#include <QDebug>


class CLangater::CLangaterIntf
{
    friend class CLangater;
public:
    QTranslator * createTranslator()
    {
        m_list.push_back(new QTranslator);
        return m_list.back();
    }

    QTranslator * createTranslator(const QString& file)
    {
        QTranslator * t = nullptr;
        for (auto d: m_dirs) {
            t = createTranslator(file, d);
            if ( t ) break;
        }

        return t;
    }

    QTranslator * createTranslator(const QString& file, const QString& path)
    {
        auto * t = new QTranslator;
        if ( t->load(file, path) ) {
            m_list.push_back(t);

            auto d = std::find(m_dirs.begin(), m_dirs.end(), path);
            if ( d == m_dirs.end() )
                m_dirs.push_back(QString(path));
        } else {
            delete t,
            t = nullptr;
        }

        return t;
    }

    ~CLangaterIntf()
    {
        if ( !m_list.empty() ) {
            for (auto p: m_list) {
                QTranslator * t = p;
                if ( !t->parent() )
                    delete t;
            }
        }
    }

    QString langName(const QString& code) {
        return m_langs.value(code);
    }

    bool reload(const QString& lang) {
        QTranslator * t;
        for (auto p: m_list) {
            t = p;
            if ( !t->parent() )
                delete t;
        }

        m_list.clear();

        bool _res = false;
        for (auto d: m_dirs) {
            t = createTranslator(lang, d);
            if ( !_res && t ) _res = true;
        }

        return _res;
    }

    void addSearchPath(const QString& path) {
        auto d = std::find(m_dirs.begin(), m_dirs.end(), path);
        if ( d == m_dirs.end() )
            m_dirs.push_back(QString(path));
    }

    void addSearchPath(const std::list<QString>& l) {
        m_dirs.insert(std::end(m_dirs), std::begin(l), std::end(l));
    }

    QString findCloseLang(const QString& n) {
        QMap<QString, QString>::iterator i = m_langs.begin();
        while ( i != m_langs.end() ) {
            if ( i.key().startsWith(n) )
                return i.key();

            ++i;
        }

        return "";
    }

private:
    std::list<QTranslator *> m_list;
    std::list<QString> m_dirs;

    QMap<QString, QString> m_langs{
        {"en-EN", "English"},
        {"ru-RU", "Русский"},
        {"de-DE", "Deutsch"},
        {"fr-FR", "Français"},
        {"es-ES", "Español"},
        {"sk-SK", "Slovenčina"},
        {"cs-CZ", "Čeština"},
        {"it-IT", "Italiano"},
        {"pt-BR", "Português Brasileiro"}
//        ,{"pt-PT", "Portuguese (Portugal)"}
        ,{"pl-PL", "Polski"}
        ,{"zh-CN", "中文"}
//        ,{"zh-HK", "Chinese (Traditional)"}
        ,{"ca-ES", "Catalan"}
        ,{"da-DK", "Dansk"}
        ,{"el-GR", "Ελληνικά"}
//        ,{"et-EE", "Eesti"}
        ,{"fi-FI", "Suomi"}
//        ,{"ga-IE", "Gaeilge"}
//        ,{"hi-IN", "हिन्दी"}
//        ,{"hr-HR", "Hrvatska"}
        ,{"hu-HU", "Magyar nyelv"}
//        ,{"hy-AM", "Հայաստան"}
        ,{"id-ID", "Indonesian"}
        ,{"no-NO", "Norsk"}
        ,{"ro-RO", "Romanian"}
        ,{"sl-SL", "Slovene"}
        ,{"sv-SE", "Svenska"}
        ,{"tr-TR", "Türkçe"}
        ,{"ja-JP", "日本語"}
        ,{"ko-KR", "한국어"}
        ,{"bg-BG", "Български"}
        ,{"nl-NL", "Nederlands"}
        ,{"vi-VN", "Tiếng Việt"}
        ,{"lv-LV", "Latviešu valoda"}
//        ,{"lt-LT", "Lietuvių kalba"}
        ,{"be-BY", "Беларуская мова"}
        ,{"uk-UA", "Украї́нська мо́ва"}
        ,{"lo-LA", "ພາສາລາວ"}
        ,{"gl-ES", "Galego"}
    };
};

CLangater::CLangater()
    : m_intf(new CLangater::CLangaterIntf)
{
}

CLangater::~CLangater()
{
    delete m_intf, m_intf = nullptr;
}

CLangater * CLangater::getInstance()
{
    static CLangater _instance;
    return &_instance;
}

void CLangater::init()
{
    GET_REGISTRY_USER(reg_user)
    GET_REGISTRY_SYSTEM(reg_system)

    QString _lang = QString::fromStdWString(InputArgs::argument_value(L"--keeplang"));
    if ( _lang.isEmpty() ) {
        _lang = QString::fromStdWString(InputArgs::argument_value(L"--lang"));
    } else reg_user.setValue("locale", _lang);

    if ( _lang.isEmpty() )
        _lang = reg_user.value("locale").value<QString>();

#ifdef __linux
//    if ( _lang.isEmpty() ) {
//        _lang = QLocale::system().name();
//    }

    if ( _lang.isEmpty() ) {
        if ( APP_DEFAULT_SYSTEM_LOCALE ) {
            QRegularExpression _re;
            QString _env_name = qgetenv("LANG");
            _re.setPattern("^(\\w{2,5})\\.?");
            QRegularExpressionMatch _re_match = _re.match(_env_name);

            if ( _re_match.hasMatch() ) {
                _lang = _re_match.captured(1);
            }
        } else _lang = APP_DEFAULT_LOCALE;
    }
#else
    // read setup language and set application locale
    _lang.isEmpty() &&
        !((_lang = reg_system.value("locale").value<QString>()).size()) && (_lang = "en").size();
#endif

    getInstance()->m_intf->addSearchPath({"./langs", ":/i18n/langs", ":/i18n"});

    auto _check_lang = [=](const std::list<QString>& dirs, const QString& l) {
        for ( auto& d : dirs ) {
            if ( QFile(d + "/" + l + ".qm").exists() ) return true;
        }

        return false;
    };

    bool _exist = _check_lang(getInstance()->m_intf->m_dirs, _lang);
    if ( !_exist && _lang.length() == 2 ) {
        QString _close_lang = getInstance()->m_intf->findCloseLang(_lang);
        if ( !_close_lang.isEmpty() )
            _lang = _close_lang,
            _exist = _check_lang(getInstance()->m_intf->m_dirs, _lang);
    }

    if ( !_exist ) {
        if ( _lang.at(2) == '-' ) _lang.replace(2, 1, '_'); else
        if ( _lang.at(2) == '_' ) _lang.replace(2, 1, '-');

        if ( !_check_lang(getInstance()->m_intf->m_dirs, _lang) ) {
            _lang = APP_DEFAULT_LOCALE;
        }
    }

    QTranslator * tr = getInstance()->m_intf->createTranslator("qtbase_" + _lang);
    if ( tr ) QCoreApplication::installTranslator(tr);

    tr = getInstance()->m_intf->createTranslator(_lang);
    if ( tr ) getInstance()->m_lang = _lang;

    QCoreApplication::installTranslator(tr);
}

void CLangater::reloadTranslations(const QString& lang)
{
    for ( auto p : getInstance()->m_intf->m_list ) {
        QCoreApplication::removeTranslator(p);
    }

    getInstance()->m_intf->createTranslator("qtbase_" + lang);
    if ( getInstance()->m_intf->reload(lang) ) {
        getInstance()->m_lang = lang;

        for ( auto t : getInstance()->m_intf->m_list ) {
            QCoreApplication::installTranslator(t);
        }

        emit getInstance()->onLangChanged(lang);
    }
}

void CLangater::refreshLangs(const QMap<QString,QString>& map)
{
    getInstance()->m_intf->m_langs = {map};
}

QString CLangater::getCurrentLangCode()
{
    return getInstance()->m_lang;
}

QString CLangater::getLangName(const QString& code)
{
    if ( code.isEmpty() ) {
        return getInstance()->m_lang;
    }

    return getInstance()->m_intf->langName(code);
}

QJsonObject CLangater::availableLangsToJson()
{
    QJsonObject _out_obj;

    QMap<QString, QString>::const_iterator i = getInstance()->m_intf->m_langs.constBegin();
    while ( i != getInstance()->m_intf->m_langs.constEnd() ) {
        _out_obj.insert(i.key(), i.value());
        ++i;
    }

    return _out_obj;
}

void CLangater::addTranslation(const QString& dir, const QString& lang)
{
    QTranslator * tr = getInstance()->m_intf->createTranslator(lang, dir);
    if ( tr ) {
        QCoreApplication::installTranslator(tr);
    }
}

void CLangater::addTranslation(const QString& dir)
{
    addTranslation(dir, getInstance()->m_lang);
}
