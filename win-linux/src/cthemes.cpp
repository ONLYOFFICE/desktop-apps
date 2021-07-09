
#include "cthemes.h"
#include "defines.h"

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QDebug>

#define QSTRING_FROM_WSTR(s) QString::fromStdWString(s)
#define REGISTRY_THEME_KEY "UITheme"

class CThemes::CThemesPrivate {
public:
    CThemesPrivate(CThemes * p)
        : parent(*p)
    {
        support_ids = { NSThemeLight::theme_id,
                        NSThemeClassicLight::theme_id,
                        NSThemeDark::theme_id };
    }

    CThemes & parent;
    std::wstring current = L"";
    std::vector<std::wstring> support_ids;
};

CThemes::~CThemes()
{
    if ( m_priv ) {
        delete m_priv;
        m_priv = nullptr;
    }
}

CThemes::CThemes()
    : m_priv(new CThemes::CThemesPrivate(this))
{
    GET_REGISTRY_USER(_reg_user);
#ifdef Q_OS_WIN
    if ( _reg_user.contains(REGISTRY_THEME_KEY) )
        m_priv->current = _reg_user.value(REGISTRY_THEME_KEY, QString::fromStdWString(NSThemeClassicLight::theme_id)).toString().toStdWString();
    else {
        QSettings _reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        m_priv->current = _reg.value("AppsUseLightTheme", 1).toInt() == 0 ? NSThemeDark::theme_id : NSThemeClassicLight::theme_id;
    }
#else
    m_priv->current = _reg_user.value(REGISTRY_THEME_KEY, QString::fromStdWString(NSThemeClassicLight::theme_id)).toString().toStdWString();
#endif
}

auto CThemes::current() -> std::wstring
{
    return m_priv->current;
}

auto CThemes::setCurrent(const std::wstring& name) -> void
{
    if ( !isCurrent(name) ) {
        GET_REGISTRY_USER(_reg_user);
        _reg_user.setValue(REGISTRY_THEME_KEY, QString::fromStdWString(name));

        m_priv->current = name;
    }
}

auto CThemes::isCurrent(const std::wstring& name) -> bool
{
    return m_priv->current == name;
}

auto CThemes::isCurrentDark() -> bool
{
    return m_priv->current == NSThemeDark::theme_id;
}

auto CThemes::isThemeDark(const std::wstring& name) -> bool
{
    return name == NSThemeDark::theme_id;
}

auto CThemes::color(ColorRole r) -> QColor
{
    return color(current(), r);
}

auto CThemes::color(const std::wstring& theme, ColorRole r) -> QColor
{
    return QColor(QSTRING_FROM_WSTR(value(theme, r)));
}

#ifdef Q_OS_WIN
auto CThemes::colorRef(ColorRole role) -> COLORREF
{
    int r, g, b;
    color(role).getRgb(&r, &g, &b);

    return RGB(r,g,b);
}
#endif

auto CThemes::value(ColorRole r) -> std::wstring
{
    return value(m_priv->current, r);
}

auto CThemes::value(const std::wstring& theme, ColorRole r) -> std::wstring
{
    if ( theme == NSThemeDark::theme_id ) {
        switch (r) {
        case ColorRole::ecrWindowBackground: return NSThemeDark::color_window_background;
        case ColorRole::ecrWindowBorder: return NSThemeDark::color_window_border;
        case ColorRole::ecrTextNormal: return NSThemeDark::color_text_normal;
        case ColorRole::ecrTextPressed: return NSThemeDark::color_text_normal_pressed;
        case ColorRole::ecrLogoColor: return NSThemeDark::color_logo;
        case ColorRole::ecrTabWordActive: return NSThemeDark::color_brand_word;
        case ColorRole::ecrTabCellActive: return NSThemeDark::color_brand_cell;
        case ColorRole::ecrTabSlideActive: return NSThemeDark::color_brand_slide;
        case ColorRole::ecrTabSimpleActiveBackground: return NSThemeDark::color_tab_simple_active_background;
        case ColorRole::ecrTabSimpleActiveText: return NSThemeDark::color_tab_simple_active_text;
        case ColorRole::ecrTabDefaultActiveBackground: return NSThemeDark::color_tab_default_active_background;
        case ColorRole::ecrTabDefaultActiveText: return NSThemeDark::color_tab_default_active_text;
        }
    } else {
        switch (r) {
        case ColorRole::ecrWindowBackground: return NSThemeLight::color_window_background;
        case ColorRole::ecrWindowBorder: return NSThemeLight::color_window_border;
        case ColorRole::ecrTextNormal: return NSThemeLight::color_text_normal;
        case ColorRole::ecrTextPressed: return NSThemeLight::color_text_normal_pressed;
        case ColorRole::ecrLogoColor: return NSThemeLight::color_logo;
        case ColorRole::ecrTabWordActive: return NSThemeLight::color_brand_word;
        case ColorRole::ecrTabCellActive: return NSThemeLight::color_brand_cell;
        case ColorRole::ecrTabSlideActive: return NSThemeLight::color_brand_slide;
        case ColorRole::ecrTabSimpleActiveBackground: return NSThemeLight::color_tab_simple_active_background;
        case ColorRole::ecrTabSimpleActiveText: return NSThemeLight::color_tab_simple_active_text;
        case ColorRole::ecrTabDefaultActiveBackground: return NSThemeLight::color_tab_default_active_background;
        case ColorRole::ecrTabDefaultActiveText: return NSThemeLight::color_tab_default_active_text;
        }
    }

    return L"";
}

auto CThemes::isColorDark(ColorRole role) -> bool
{
    return isColorDark(value(role));
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

            return obj.contains("name") ? obj["name"].toString().toStdWString() : NSThemeClassicLight::theme_id;
        }
    }

    return wjson;
}
