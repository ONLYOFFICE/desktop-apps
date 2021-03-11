
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
    if ( theme == NSThemeDark::theme_id ) {
        switch (r) {
        case ColorRole::ecrWindowBackground: return QColor(QSTRING_FROM_WSTR(NSThemeDark::color_window_background));
        case ColorRole::ecrWindowBorder: return QColor(QSTRING_FROM_WSTR(NSThemeDark::color_window_border));
        case ColorRole::ecrTextNormal: return QColor(QSTRING_FROM_WSTR(NSThemeDark::color_text_normal));
        case ColorRole::ecrTextPressed: return QColor(QSTRING_FROM_WSTR(NSThemeDark::color_text_normal_pressed));
        }
qDebug() << "return green";
        return QColor(Qt::green);
    } else {
        switch (r) {
        case ColorRole::ecrWindowBackground: return QColor(QSTRING_FROM_WSTR(NSThemeLight::color_window_background));
        case ColorRole::ecrWindowBorder: return QColor(QSTRING_FROM_WSTR(NSThemeLight::color_window_border));
        case ColorRole::ecrTextNormal: return QColor(QSTRING_FROM_WSTR(NSThemeLight::color_text_normal));
        case ColorRole::ecrTextPressed: return QColor(QSTRING_FROM_WSTR(NSThemeLight::color_text_normal_pressed));
        }

        return QColor(Qt::yellow);
    }
}

#ifdef Q_OS_WIN
auto CThemes::colorRef(ColorRole role) -> COLORREF
{
    QColor c{color(role)};

    int r, g, b;
    c.getRgb(&r, &g, &b);

    return RGB(r,g,b);
}
#endif

auto CThemes::value(ColorRole r) -> std::wstring
{
    if ( m_priv->current == NSThemeDark::theme_id ) {
        switch (r) {
        case ColorRole::ecrLogoColor: return NSThemeDark::color_logo;
        }
    } else {
        switch (r) {
        case ColorRole::ecrLogoColor: return NSThemeLight::color_logo;
        }
    }

    return L"";
}

auto CThemes::value(const std::wstring& theme, ColorRole r) -> std::wstring
{
    if ( theme == NSThemeDark::theme_id ) {
        switch (r) {
        case ColorRole::ecrLogoColor: return NSThemeDark::color_logo;
        }
    } else {
        switch (r) {
        case ColorRole::ecrLogoColor: return NSThemeLight::color_logo;
        }
    }

    return L"";
}
