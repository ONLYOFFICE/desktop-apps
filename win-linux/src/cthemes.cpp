
#include "cthemes.h"
#include "defines.h"

#include <QSettings>
#include <QDebug>

#define QSTRING_FROM_WSTR(s) QString::fromStdWString(s)
#define REGISTRY_THEME_KEY "UITheme"

class CThemes::CThemesPrivate {
public:
    CThemesPrivate(CThemes * p)
        : parent(*p)
    {}

    CThemes & parent;
    std::wstring current = L"";
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
    m_priv->current = _reg_user.value(REGISTRY_THEME_KEY, QString::fromStdWString(NSThemeLight::theme_id)).toString().toStdWString();
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
        }
    }

    return L"";
}
