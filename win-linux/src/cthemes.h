#ifndef CTHEMES_H
#define CTHEMES_H

#include <QColor>

#ifdef Q_OS_WIN
# include <windows.h>
#endif


class CThemes;
class CTheme {
public:
    enum class ColorRole {
        ecrWindowBackground
        , ecrWindowBorder
        , ecrTextNormal
        , ecrTextPressed
        , ecrLogoColor
        , ecrTabWordActive
        , ecrTabCellActive
        , ecrTabSlideActive
        , ecrTabSimpleActiveBackground
        , ecrTabSimpleActiveText
        , ecrTabDefaultActiveBackground
        , ecrTabDefaultActiveText
        , ecrButtonNormalOpacity
    };

    auto fromFile(const QString&) -> bool;
    auto fromJson(const QString&) -> bool;

    auto id() const -> std::wstring;
    auto originalId() const -> std::wstring;
    auto stype() const -> QString;
    auto color(ColorRole r) const -> QColor;
#ifdef Q_OS_WIN
    auto colorRef(ColorRole r) const -> COLORREF;
#endif
    auto value(ColorRole) const -> std::wstring;
    auto isDark() const -> bool;
    auto isSystem() const -> bool;

private:
    CTheme(const QString& id = QString());
    ~CTheme();

    class CThemePrivate;
    CThemePrivate * m_priv = nullptr;

    friend class CThemes;
};

class CThemes
{
public:
    CThemes();
    ~CThemes();

    auto current() -> const CTheme&;
    auto defaultDark() -> const CTheme&;
    auto defaultLight() -> const CTheme&;

    auto setCurrentTheme(const std::wstring&) -> void;
    auto isThemeCurrent(const std::wstring& id) -> bool;
//    auto isThemeDark(const std::wstring& id) -> bool;

    auto isColorDark(const std::wstring&) -> bool;
    auto isColorDark(const QString&) -> bool;

    auto onSystemDarkColorScheme(bool isdark) -> void;
    auto parseThemeName(const std::wstring&) -> std::wstring;
private:
    class CThemesPrivate;
    CThemesPrivate * m_priv = nullptr;
};

#endif // CTHEMES_H
