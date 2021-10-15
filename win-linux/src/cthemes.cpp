
#include "cthemes.h"
#include "defines.h"

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

#define QSTRING_FROM_WSTR(s) QString::fromStdWString(s)
#define REGISTRY_THEME_KEY "UITheme"
#define THEME_DEFAULT_DARK_ID "theme-dark"
#define THEME_DEFAULT_LIGHT_ID "theme-classic-light"

namespace NSTheme {
    static const QString theme_type_dark = "dark";
    static const QString theme_type_light = "light";

    enum class ThemeType {
        ttDark,
        ttLight
    };

    static const std::map<CTheme::ColorRole, QString> map_names = {
            {CTheme::ColorRole::ecrTabWordActive, "brand_word"},
            {CTheme::ColorRole::ecrTabSlideActive, "brand_slide"},
            {CTheme::ColorRole::ecrTabCellActive, "brand_cell"},

            {CTheme::ColorRole::ecrWindowBackground, "window_background"},
            {CTheme::ColorRole::ecrWindowBorder, "window_border"},

            {CTheme::ColorRole::ecrTextNormal, "text_normal"},
            {CTheme::ColorRole::ecrTextPressed, "text_normal_pressed"},

    //      {  "tab_active_background": "#fff",
            {CTheme::ColorRole::ecrTabSimpleActiveBackground, "tab_simple_active_background"},
            {CTheme::ColorRole::ecrTabSimpleActiveText, "tab_simple_active_text"},
            {CTheme::ColorRole::ecrTabDefaultActiveBackground, "tab_default_active_background"},
            {CTheme::ColorRole::ecrTabDefaultActiveText, "tab_default_active_text"},
    //      {  "tab_divider": "#a5a5a5",

            {CTheme::ColorRole::ecrButtonNormalOpacity, "button_normal_opacity"},
            {CTheme::ColorRole::ecrLogoColor, "logo"}
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
        jsonValues = obj.value("values").toObject();
    }

    std::wstring id;
    std::wstring wstype;
    NSTheme::ThemeType type;

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
        rc_themes = {
            {"theme-light", ":/themes/theme-light.json"},
            {"theme-classic-light", ":/themes/theme-classic-light.json"},
            {"theme-dark", ":/themes/theme-dark.json"},
        };

        GET_REGISTRY_USER(_reg_user);
        QString user_theme = _reg_user.value(REGISTRY_THEME_KEY).toString();
#ifdef Q_OS_WIN
//        if ( _reg_user.contains(REGISTRY_THEME_KEY) )
//            user_theme = _reg_user.value(REGISTRY_THEME_KEY, THEME_DEFAULT_LIGHT_ID).toString();
//        else {
//            QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
//            user_theme = _reg.value("AppsUseLightTheme", 1).toInt() == 0 ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
//        }
        if ( user_theme.isEmpty() ) {
            QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
            user_theme = _reg.value("AppsUseLightTheme", 1).toInt() == 0 ? THEME_DEFAULT_DARK_ID : THEME_DEFAULT_LIGHT_ID;
        }
#else
        if ( user_theme.isEmpty() ) {
            user_theme = THEME_DEFAULT_LIGHT_ID;
        }
#endif

        current = new CTheme(rc_themes.at(user_theme));
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
          if ( rc_themes.find(id) != rc_themes.end() ) {
            return current->load(rc_themes.at(id));
        }

        return false;
    }

    auto getDefault(NSTheme::ThemeType type) -> const CTheme *
    {
        if ( type == NSTheme::ThemeType::ttDark ) {
            if ( !dark ) {
                dark = new CTheme;
                dark->load(rc_themes[THEME_DEFAULT_DARK_ID]);
            }

            return dark;
        } else {
            if ( !light ) {
                light = new CTheme;
                light->load(rc_themes[THEME_DEFAULT_LIGHT_ID]);
            }

            return light;
        }
    }

    CThemes & parent;
    std::map<QString, QString> rc_themes;

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
        load(path);
}

CTheme::~CTheme()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

auto CTheme::load(const QString& path) -> bool
{
    QFile _file(path);
    if ( _file.open(QIODevice::ReadOnly) ) {
        QJsonParseError jerror;

        QJsonDocument jdoc = QJsonDocument::fromJson(_file.readAll(), &jerror);
        if( jerror.error == QJsonParseError::NoError ) {
            m_priv->fromJsonObject(jdoc.object());
            return true;
        }
    }

    return false;
}

auto CTheme::id() const -> std::wstring
{
    return m_priv->id;
}

auto CTheme::stype() const -> QString
{
    return m_priv->type == NSTheme::ThemeType::ttDark ?
                NSTheme::theme_type_dark : NSTheme::theme_type_light;
}

auto CTheme::value(ColorRole r) const -> std::wstring
{
    if ( NSTheme::map_names.find(r) != NSTheme::map_names.end() ) {
        if ( m_priv->jsonValues.contains(NSTheme::map_names.at(r)) )
            return m_priv->jsonValues.value(NSTheme::map_names.at(r)).toString().toStdWString();
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

auto CThemes::dark() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttDark);
}

auto CThemes::light() -> const CTheme&
{
    return *m_priv->getDefault(NSTheme::ThemeType::ttLight);
}

auto CThemes::setCurrentTheme(const std::wstring& name) -> void
{
    if ( !isCurrent(name) && m_priv->setCurrent(QString::fromStdWString(name)) ) {
        GET_REGISTRY_USER(_reg_user);
        _reg_user.setValue(REGISTRY_THEME_KEY, QString::fromStdWString(name));
    }
}

auto CThemes::isCurrent(const std::wstring& name) -> bool
{
    return m_priv->current->id() == name;
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
