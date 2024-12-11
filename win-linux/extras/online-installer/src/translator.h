#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <unordered_map>
#include <string>
#include <tchar.h>

using std::wstring;
using std::unordered_map;

typedef unordered_map<wstring, wstring> LocaleMap;
typedef unordered_map<wstring, LocaleMap> TranslationsMap;


class Translator
{
public:
    Translator(unsigned long langId, int resourceId);
    ~Translator();

    static wstring tr(const char*);

private:
    void parseTranslations();

    static TranslationsMap translMap;
    wstring        translations,
                   error_substr;
    static wstring langName;
    static bool    is_translations_valid;

    enum TokenType : unsigned char {
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
