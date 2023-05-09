
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
#include <QRegularExpression>
#include <QProcess>
#include <QPalette>
#include <QDebug>

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
        Dark,
        Light
    };

    static const std::map<CTheme::ColorRole, QString> map_names = {
            {CTheme::ColorRole::TabWordActive, "brand_word"},
            {CTheme::ColorRole::TabSlideActive, "brand_slide"},
            {CTheme::ColorRole::TabCellActive, "brand_cell"},

            {CTheme::ColorRole::WindowBackground, "window_background"},
            {CTheme::ColorRole::WindowBorder, "window_border"},

            {CTheme::ColorRole::TextNormal, "text_normal"},
            {CTheme::ColorRole::TextPressed, "text_normal_pressed"},

    //      {  "tab_active_background": "#fff",
            {CTheme::ColorRole::TabSimpleActiveBackground, "tab_simple_active_background"},
            {CTheme::ColorRole::TabSimpleActiveText, "tab_simple_active_text"},
            {CTheme::ColorRole::TabDefaultActiveBackground, "tab_default_active_background"},
            {CTheme::ColorRole::TabDefaultActiveText, "tab_default_active_text"},
    //      {  "tab_divider": "#a5a5a5",

            {CTheme::ColorRole::ButtonNormalOpacity, "button_normal_opacity"},
            {CTheme::ColorRole::LogoColor, "logo"}
        };

    CTheme * theme_default_dark = nullptr;
    CTheme * theme_default_light = nullptr;
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
                            NSTheme::ThemeType::Dark : NSTheme::ThemeType::Light;
        jsonValues = obj.value("values").toObject();
        is_system = false;
    }

    std::wstring id;
    std::wstring wstype;
    NSTheme::ThemeType type;
    bool is_system{false};

    QJsonObject jsonValues;
};

/*
 * CThemes private class
*/

class CThemes::CThemesPrivate {
public:
    CThemesPrivate(CThemes * p)
        : parent(*p)
    {
        map_themes = {
            {THEME_ID_SYSTEM, ""},
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
#endif
        if ( user_theme == THEME_ID_SYSTEM || map_themes.find(user_theme) == map_themes.end() ) {
            current = new CTheme(map_themes.at(is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID));
            current->m_priv->is_system = true;
        } else current = new CTheme(map_themes.at(user_theme));
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
        if ( id != THEME_ID_SYSTEM ) {
            if ( map_themes.find(id) != map_themes.end() ) {
                return current->fromFile(map_themes.at(id));
            }
        } else {
            QString visual_theme_id = is_system_theme_dark ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
            if ( current->id() != visual_theme_id.toStdWString() ) {
                if ( !current->fromFile(map_themes.at(visual_theme_id)) )
                    return false;
            }

            current->m_priv->is_system = true;
            return true;
        }

        return false;
    }

    auto getDefault(NSTheme::ThemeType type) -> const CTheme *
    {
        if ( type == NSTheme::ThemeType::Dark ) {
            if ( !default_dark ) {
                default_dark = new CTheme;
                default_dark->fromFile(map_themes[THEME_DEFAULT_DARK_ID]);
            }

            return default_dark;
        } else {
            if ( !default_light ) {
                default_light = new CTheme;
                default_light->fromFile(map_themes[THEME_DEFAULT_LIGHT_ID]);
            }

            return default_light;
        }
    }

    CThemes & parent;
    std::map<QString, QString> map_themes;
    bool is_system_theme_dark = false;

    CTheme * current = nullptr;
    CTheme * default_light = nullptr,
            * default_dark = nullptr;
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

auto CTheme::relevantId() const -> std::wstring
{
    return m_priv->id;
}

auto CTheme::typeSting() const -> QString
{
    switch (m_priv->type) {
    default:
    case NSTheme::ThemeType::Light: return NSTheme::theme_type_light;
    case NSTheme::ThemeType::Dark: return NSTheme::theme_type_dark;
    }
}

auto CTheme::colorHexValue(ColorRole r) const -> std::wstring
{
    if ( NSTheme::map_names.find(r) != NSTheme::map_names.end() ) {
        if ( m_priv->jsonValues.contains(NSTheme::map_names.at(r)) )
            return m_priv->jsonValues.value(NSTheme::map_names.at(r)).toString().toStdWString();
    }

    return L"";
}

auto CTheme::color(ColorRole role) const -> QColor
{
    QString v = QSTRING_FROM_WSTR(colorHexValue(role));
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
    return m_priv->type == NSTheme::ThemeType::Dark;
}

auto CTheme::isSystem() const -> bool
{
    return m_priv->is_system;
}

/**/

CThemes::CThemes()
    : m_priv(new CThemes::CThemesPrivate(this))
{
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

auto CThemes::defaultDarkTheme() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::Dark);
}

auto CThemes::defaultLightTheme() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::Light);
}

auto CThemes::setCurrentTheme(const std::wstring& name) -> void
{
    if ( !isThemeCurrent(name) && m_priv->setCurrent(QString::fromStdWString(name)) ) {
        GET_REGISTRY_USER(_reg_user);
        // for tests in ver 7.5 if there are no bugs with using REGISTRY_THEME_KEY key
        // if succeffuly, clear REGISTRY_THEME_KEY_7_2
        _reg_user.setValue(REGISTRY_THEME_KEY, QString::fromStdWString(name));
        _reg_user.setValue(REGISTRY_THEME_KEY_7_2, QString::fromStdWString(name));
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
