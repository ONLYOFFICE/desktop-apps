#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <unordered_map>
#include <string>
#ifdef _WIN32
# include <tchar.h>
# define tchar wchar_t
# define tstring std::wstring
#else
# define _T(str) str
# define tchar char
# define tstring std::string
#endif

#define _TR(str) Translator::tr(#str)

using std::unordered_map;

typedef unordered_map<tstring, tstring> LocaleMap;
typedef unordered_map<tstring, LocaleMap> TranslationsMap;


class Translator
{
public:
#ifdef _WIN32
    Translator(const tstring &lang, int resourceId);
#else
    Translator(const tstring &lang, const char *resourcePath);
#endif
    ~Translator();

    static tstring tr(const char*);
    static void setLanguage(const tstring &lang);

private:
    void parseTranslations();

    static TranslationsMap translMap;
    tstring        translations,
                   error_substr;
    static tstring langName;
    static bool    is_translations_valid;

    enum TokenType {
        TOKEN_BEGIN_DOCUMENT = 0,
        TOKEN_END_DOCUMENT,
        TOKEN_BEGIN_STRING_ID,
        TOKEN_END_STRING_ID,
        TOKEN_BEGIN_LOCALE,
        TOKEN_END_LOCALE,
        TOKEN_BEGIN_VALUE,
        TOKEN_END_VALUE
    };
};

#endif // TRANSLATOR_H
