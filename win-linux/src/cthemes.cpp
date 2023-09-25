
#include "cthemes.h"
#ifdef Q_OS_LINUX
# include <gio/gio.h>
# include <glib.h>
#endif
#include "defines.h"
#include "utils.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QProcess>
#include <QPalette>
#include <QApplication>
#include <QJsonArray>

#define QSTRING_FROM_WSTR(s) QString::fromStdWString(s)
#define REGISTRY_THEME_KEY "UITheme"
#define REGISTRY_THEME_KEY_7_2 "UITheme2"
#define THEME_DEFAULT_DARK_ID "theme-dark"
#define THEME_DEFAULT_LIGHT_ID "theme-classic-light"
#define THEME_ID_SYSTEM "theme-system"

namespace NSTheme {
    static const QString theme_type_dark = "dark";
    static const QString theme_type_light = "light";

    enum class ThemeType {
        ttDark,
        ttLight
    };

    static const std::map<CTheme::ColorRole, QString> map_names = {
            {CTheme::ColorRole::ecrTabWordActive, "brand-word"},
            {CTheme::ColorRole::ecrTabSlideActive, "brand-slide"},
            {CTheme::ColorRole::ecrTabCellActive, "brand-cell"},
            {CTheme::ColorRole::ecrTabViewerActive, "brand-pdf"},

            {CTheme::ColorRole::ecrWindowBackground, "window-background"},
            {CTheme::ColorRole::ecrWindowBorder, "window-border"},

            {CTheme::ColorRole::ecrTextNormal, "text-normal"},
            {CTheme::ColorRole::ecrTextPressed, "text-normal-pressed"},

    //      {  "tab-active-background": "#fff",
            {CTheme::ColorRole::ecrTabSimpleActiveBackground, "tab-simple-active-background"},
            {CTheme::ColorRole::ecrTabSimpleActiveText, "tab-simple-active-text"},
            {CTheme::ColorRole::ecrTabDefaultActiveBackground, "tab-default-active-background"},
            {CTheme::ColorRole::ecrTabDefaultActiveText, "tab-default-active-text"},
    //      {  "tab-divider": "#a5a5a5",

            {CTheme::ColorRole::ecrButtonNormalOpacity, "button-normal-opacity"},
            {CTheme::ColorRole::ecrLogoColor, "logo"}
        };

    static const std::map<QString, QString> map_alias_names = {
            {"brand-word", "toolbar-header-document"},
            {"brand-slide", "toolbar-header-presentation"},
            {"brand-cell", "toolbar-header-spreadsheet"},

            {"window-background", "background-toolbar"},
//            {CTheme::ColorRole::ecrWindowBorder, "window-border"},

//            {CTheme::ColorRole::ecrTextNormal, "text-normal"},
//            {CTheme::ColorRole::ecrTextPressed, "text-normal-pressed"},

//            {CTheme::ColorRole::ecrTabSimpleActiveBackground, "tab-simple-active-background"},
//            {CTheme::ColorRole::ecrTabSimpleActiveText, "tab-simple-active-text"},
//            {CTheme::ColorRole::ecrTabDefaultActiveBackground, "tab-default-active-background"},
//            {CTheme::ColorRole::ecrTabDefaultActiveText, "tab-default-active-text"},

//            {CTheme::ColorRole::ecrButtonNormalOpacity, "button-normal-opacity"},
//            {CTheme::ColorRole::ecrLogoColor, "logo"}
        };
}

/*
 * CThemePrivate
*/
class CTheme::CThemePrivate {
public:
    CThemePrivate() {}

    auto fromJsonObject(const QJsonObject& obj) -> void {
        id = obj.value("id").toString().toStdWString();
        type = obj.value("type").toString() == NSTheme::theme_type_dark ?
                            NSTheme::ThemeType::ttDark : NSTheme::ThemeType::ttLight;
        if ( obj.contains("colors") ) {
            QJsonObject colorValues = obj.value("colors").toObject();

            const CTheme * default_theme = type == NSTheme::ThemeType::ttDark ? defdark : deflight;
            jsonValues = QJsonObject(default_theme->m_priv->jsonValues);

            for ( const auto &color_name : NSTheme::map_names ) {
                if ( colorValues.contains(color_name.second) ) {
                    jsonValues[color_name.second] = colorValues.value(color_name.second);
                } else {
                    const auto it = NSTheme::map_alias_names.find(color_name.second);
                    if ( it != NSTheme::map_alias_names.end() &&
                            colorValues.contains(it->second) )
                    {
                        jsonValues[color_name.second] = colorValues.value(it->second).toString();
                    }
                }
            }

        } else {
            jsonValues = obj.value("values").toObject();
        }

//        is_system = false;
    }

    auto setDefaultThemes(const CTheme * const l, const CTheme * const d) -> void {
        defdark = d;
        deflight = l;
    }

    std::wstring id;
    std::wstring wstype;
    NSTheme::ThemeType type;
    bool is_system{false};

    QJsonObject jsonValues;
    const CTheme * defdark = nullptr,
            * deflight = nullptr;
    QString source_file;
};

/*
 * CThemes private class
*/

class CThemes::CThemesPrivate {
public:
    CThemesPrivate(CThemes * p)
        : parent(*p)
    {
        rc_themes = {
            {"theme-light", ":/themes/theme-light.json"},
            {"theme-classic-light", ":/themes/theme-classic-light.json"},
            {"theme-dark", ":/themes/theme-dark.json"},
            {"theme-contrast-dark", ":/themes/theme-contrast-dark.json"},
        };

        GET_REGISTRY_USER(_reg_user);

        QString user_theme = _reg_user.value(REGISTRY_THEME_KEY, THEME_ID_SYSTEM).toString();

        /* TODO: remove for ver 7.3. for compatibility with ver 7.1 only */
        if ( _reg_user.contains(REGISTRY_THEME_KEY_7_2) )
            user_theme = _reg_user.value(REGISTRY_THEME_KEY_7_2, THEME_ID_SYSTEM).toString();

#ifdef Q_OS_WIN
        QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        is_system_theme_dark = _reg.value("AppsUseLightTheme", 1).toInt() == 0;
#else
        if ( WindowHelper::getEnvInfo() == "KDE" ) {
            QColor color = QPalette().base().color();
            int r, g, b;
            color.getRgb(&r, &g, &b);
            int lum = int(0.299*r + 0.587*g + 0.114*b);
            is_system_theme_dark = !(lum > 127);
        } else {
            GSettings * sett = g_settings_new("org.gnome.desktop.interface");
            GVariant * val = g_settings_get_value(sett, "gtk-theme");
            char * env = nullptr;
            g_variant_get(val, "s", &env);
            if ( env ) {
                is_system_theme_dark = !(QString::fromUtf8(env).toLower().indexOf("dark") == -1);

                free(env);
            }

            g_object_unref(sett);
        }

        // TODO: "system" theme blocked because bug 59804
        if ( user_theme == THEME_ID_SYSTEM )
            user_theme = THEME_DEFAULT_LIGHT_ID;
#endif

        current = new CTheme;
        current->m_priv->setDefaultThemes(getDefault(NSTheme::ThemeType::ttDark), getDefault(NSTheme::ThemeType::ttLight));
        if ( user_theme.endsWith(".json") ) {
            QDir directory(qApp->applicationDirPath() + "/uithemes");
            QString filepath{directory.absoluteFilePath(user_theme)};

            if ( !QFile::exists(filepath) || !current->fromFile(filepath) ) {
                user_theme = THEME_ID_SYSTEM;
            }
        } else
        if ( rc_themes.find(user_theme) == rc_themes.end() || !current->fromFile(rc_themes.at(user_theme)) ) {
            user_theme = THEME_ID_SYSTEM;
        }

        if ( user_theme == THEME_ID_SYSTEM ) {
            current->fromFile(rc_themes.at(is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID));
            current->m_priv->is_system = true;
        }
    }

    ~CThemesPrivate()
    {
        if ( current ) {
            delete current;
            current = nullptr;
        }
    }

    auto setCurrent(const QString& id) -> bool
    {
        if ( current->id() != id.toStdWString() ) {
            if ( id != THEME_ID_SYSTEM ) {
                delete current;

                current = new CTheme;
                current->m_priv->setDefaultThemes(getDefault(NSTheme::ThemeType::ttDark), getDefault(NSTheme::ThemeType::ttLight));
                if ( rc_themes.find(id) != rc_themes.end() ) {
                    return current->fromFile(rc_themes.at(id));
                } else
                if ( local_themes.find(id) != local_themes.end() ) {
                    if ( current->fromJson(local_themes.at(id).second) ) {
                        current->m_priv->source_file = local_themes.at(id).first;
                        return true;
                    }
//                    return current->fromJson(local_themes.at(id));
                }
            } else {
                QString visual_theme_id = is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
                if ( current->id() != visual_theme_id.toStdWString() ) {
                    delete current;

                    current = new CTheme;
                    if ( !current->fromFile(rc_themes.at(visual_theme_id)) )
                        return false;
                }

                current->m_priv->is_system = true;
                return true;
            }

            return false;
        }

        return true;
    }

    auto getDefault(NSTheme::ThemeType type) -> const CTheme * const
    {
        if ( type == NSTheme::ThemeType::ttDark ) {
            if ( !dark ) {
                dark = new CTheme;
                dark->fromFile(rc_themes[THEME_DEFAULT_DARK_ID]);
            }

            return dark;
        } else {
            if ( !light ) {
                light = new CTheme;
                light->fromFile(rc_themes[THEME_DEFAULT_LIGHT_ID]);
            }

            return light;
        }
    }

    auto searchLocalThemes() -> void {
        QDir directory(qApp->applicationDirPath() + "/uicolorthemes");
        QStringList themes = directory.entryList(QStringList() << "*.json", QDir::Files);

        QFile file;
        QJsonParseError je;
        QJsonArray json_themes_array;
        foreach(QString filename, themes) {
            file.setFileName(directory.absoluteFilePath(filename));
            if ( file.open(QIODevice::ReadOnly) ) {
                QByteArray data{file.readAll()};
                file.close();

                QJsonDocument doc = QJsonDocument::fromJson(data, &je);
                if ( je.error == QJsonParseError::NoError ) {
                    QJsonObject objRoot = doc.object();

                    if ( validateTheme(objRoot) ) {
                        json_themes_array.append(objRoot);

                        local_themes[objRoot.value("id").toString()] = std::make_pair(filename,data);
//                        parseLocalTheme(doc.object());
                    }
                }
            }
        }

        if ( json_themes_array.size() ) {
            EditorJSVariables::setVariable("localthemes", json_themes_array);
        }
    }

    auto parseLocalTheme(const QJsonObject& obj) -> void {

    }

    auto validateTheme(const QJsonObject& root) -> bool {
        if ( root.contains("id") ) {
            return true;
        }
        return false;
    }

    CThemes & parent;
    std::map<QString, QString> rc_themes;
    std::map<QString, std::pair<QString, QByteArray>> local_themes;
    bool is_system_theme_dark = false;

    CTheme * current = nullptr;
    CTheme * dark = nullptr;
    CTheme * light = nullptr;
};

/*
 * CTheme
*/

CTheme::CTheme(const QString& path)
    : m_priv(new CTheme::CThemePrivate)
{
    if ( !path.isEmpty() )
        fromFile(path);
}

CTheme::~CTheme()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

auto CTheme::fromFile(const QString& path) -> bool
{
    QFile _file(path);
    if ( _file.open(QIODevice::ReadOnly) ) {
        QString data(_file.readAll());
        _file.close();
        return fromJson(data);
    }

    return false;
}

auto CTheme::fromJson(const QString& json) -> bool
{
    QJsonParseError jerror;

    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8(), &jerror);
    if ( jerror.error == QJsonParseError::NoError ) {
        m_priv->fromJsonObject(jdoc.object());
        return true;
    }

    return false;
}

auto CTheme::id() const -> std::wstring
{
    return m_priv->is_system ? WSTR(THEME_ID_SYSTEM) : m_priv->id;
}

auto CTheme::originalId() const -> std::wstring
{
    return m_priv->id;
}

auto CTheme::stype() const -> QString
{
    switch (m_priv->type) {
    default:
    case NSTheme::ThemeType::ttLight: return NSTheme::theme_type_light;
    case NSTheme::ThemeType::ttDark: return NSTheme::theme_type_dark;
    }
}

auto CTheme::value(ColorRole r) const -> std::wstring
{
    if ( NSTheme::map_names.find(r) != NSTheme::map_names.end() ) {
        if ( m_priv->jsonValues.contains(NSTheme::map_names.at(r)) ) {
            return m_priv->jsonValues.value(NSTheme::map_names.at(r)).toString().toStdWString();
        }
    }

    return L"";
}

auto CTheme::color(ColorRole role) const -> QColor
{
    QString v = QSTRING_FROM_WSTR(value(role));
    if ( !v.isEmpty() ) {
        if ( v.startsWith("rgba") ) {
            QRegularExpression re("\\((\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})");
            QRegularExpressionMatch match = re.match(v);
            if ( match.hasMatch() ) {
                const uint r = match.captured(1).toUInt(),
                            g = match.captured(2).toUInt(),
                            b = match.captured(3).toUInt(),
                            a = match.captured(4).toUInt();
                v = QString("#%1%2%3%4").arg(a, 2, 16, QLatin1Char('0'))
                                            .arg(r, 2, 16, QLatin1Char('0'))
                                            .arg(g, 2, 16, QLatin1Char('0'))
                                            .arg(b, 2, 16, QLatin1Char('0'));

                // keep normalized color
                m_priv->jsonValues[NSTheme::map_names.at(role)] = v;
            }
        }

        return QColor(v);
    }

    return QColor();
}

#ifdef Q_OS_WIN
auto CTheme::colorRef(ColorRole role) const -> COLORREF
{
    int r, g, b;
    color(role).getRgb(&r, &g, &b);

    return RGB(r,g,b);
}
#endif

auto CTheme::isDark() const -> bool
{
    return m_priv->type == NSTheme::ThemeType::ttDark;
}

auto CTheme::isSystem() const -> bool
{
    return m_priv->is_system;
}

/**/

CThemes::CThemes()
    : m_priv(new CThemes::CThemesPrivate(this))
{
    m_priv->searchLocalThemes();
}

CThemes::~CThemes()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

auto CThemes::current() -> const CTheme &
{
    return *(m_priv->current);
}

auto CThemes::defaultDark() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttDark);
}

auto CThemes::defaultLight() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttLight);
}

auto CThemes::setCurrentTheme(const std::wstring& name) -> void
{
    if ( !isThemeCurrent(name) && m_priv->setCurrent(QString::fromStdWString(name)) ) {
        GET_REGISTRY_USER(_reg_user);

        if ( !m_priv->current->m_priv->source_file.isEmpty() )
            _reg_user.setValue(REGISTRY_THEME_KEY, m_priv->current->m_priv->source_file);
        else _reg_user.setValue(REGISTRY_THEME_KEY, QString::fromStdWString(name));

        // TODO: remove after ver 7.5. back to keep theme id in REGISTRY_THEME_KEY
        if ( _reg_user.contains(REGISTRY_THEME_KEY_7_2) )
            _reg_user.remove(REGISTRY_THEME_KEY_7_2);

    }
}

auto CThemes::isThemeCurrent(const std::wstring& id) -> bool
{
    if ( m_priv->current->id() != id ) return false;
    else {
        if ( m_priv->current->isSystem() ) {
            return m_priv->current->isDark() == m_priv->is_system_theme_dark;
        }

        return true;
    }
}

auto CThemes::themeActualId(const std::wstring& id) const -> std::wstring
{
    return WSTR(THEME_ID_SYSTEM) != id ? id :
        m_priv->is_system_theme_dark ? WSTR(THEME_DEFAULT_DARK_ID) : WSTR(THEME_DEFAULT_LIGHT_ID);
}

auto CThemes::isColorDark(const std::wstring& color) -> bool
{
    return isColorDark(QString::fromStdWString(color));
}

auto CThemes::isColorDark(const QString& color) -> bool
{
    int r, g, b;
    QColor(color).getRgb(&r, &g, &b);

    int luma = int(0.2126f * r) + int(0.7152f * g) + int(0.0722f * b);

    return luma < 128;
}

auto CThemes::parseThemeName(const std::wstring& wjson) -> std::wstring
{
    size_t pos = wjson.find(L"name\":");                // check if json in params
    if ( pos != std::wstring::npos ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject obj = jdoc.object();

            return obj.contains("name") ? obj["name"].toString().toStdWString() : QString(THEME_DEFAULT_LIGHT_ID).toStdWString();
        }
    }

    return wjson;
}

auto CThemes::onSystemDarkColorScheme(bool isdark) -> void
{
    if ( isdark != m_priv->is_system_theme_dark ) {
        m_priv->is_system_theme_dark = isdark;
    }
}

auto CThemes::isSystemSchemeDark() -> const bool
{
    return m_priv->is_system_theme_dark;
}
