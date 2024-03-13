
#define WIN32_LEAN_AND_MEAN
#include "translator.h"
#include "resource.h"
#include "utils.h"
#include <Windows.h>
#include <cwctype>
#include <sstream>


bool isSeparator(wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

bool isValidStringIdCharacter(wchar_t c)
{
    return std::iswalnum(c) || std::iswalpha(c) || c == L'_';
}

bool isValidLocaleCharacter(wchar_t c)
{
    return iswalpha(c) || c == L'_';
}

wstring getPrimaryLang(const wstring &lang, bool withScript = false)
{
    if (lang.empty()) {
        NS_Logger::WriteLog(_T("An error occurred: ") + wstring(_T(__FUNCTION__)));
        return L"en";
    }
    std::wistringstream iss(lang);
    wstring primlang, script;
    std::getline(iss, primlang, L'_');
    if (primlang.length() == 2 || primlang.length() == 3) {
        if (!withScript)
            return primlang;
        std::getline(iss, script, L'_');
        return (script.length() == 4) ? primlang + L"_" + script : primlang;
    }
    NS_Logger::WriteLog(_T("An error occurred: ") + wstring(_T(__FUNCTION__)));
    return L"en";
}

wstring StrToWStr(const char* str)
{
    wstring wstr;
    {
        size_t len = strlen(str), outSize = 0;
        wchar_t *pDestBuf = new wchar_t[len + 1];
        mbstowcs_s(&outSize, pDestBuf, len + 1, str, len);
        if (outSize > 0)
            wstr = pDestBuf;
        else
            NS_Logger::WriteLog(_T("An error occurred: ") + wstring(_T(__FUNCTION__)));
        delete[] pDestBuf;
    }
    return wstr;
}

TranslationsMap Translator::translMap = TranslationsMap();
wstring Translator::langName = _T("en_EN");
bool Translator::is_translations_valid = false;

Translator::Translator(unsigned long langId, int resourceId)
{
    TCHAR _langName[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetLocaleInfo(langId, LOCALE_SNAME, _langName, LOCALE_NAME_MAX_LENGTH) > 0) {
        langName = _langName;
        wstring::size_type pos = 0;
        while ((pos = langName.find(L'-', pos)) != wstring::npos) {
            langName.replace(pos, 1, L"_");
            pos++;
        }
    } else
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);

    NS_Logger::WriteLog(_T("Current locale: ") + langName);

    HMODULE hInst = GetModuleHandle(NULL);
    if (HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(resourceId), RT_RCDATA)) {
        if (HGLOBAL hResData = LoadResource(hInst, hRes)) {
            if (LPVOID pData = LockResource(hResData)) {
                DWORD dataSize = SizeofResource(hInst, hRes);
                if (dataSize > 0) {
                    string text((const char*)pData, dataSize);
                    translations = StrToWStr(text.c_str());
                } else
                    NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            } else
                NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            FreeResource(hResData);
        } else
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
    } else
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);

    if (!translations.empty()) {
        parseTranslations();
        if (!is_translations_valid)
            NS_Logger::WriteLog(_T("Cannot parse translations, error in string: ") + error_substr + L" <---");
    } else
        NS_Logger::WriteLog(_T("Error: translations is empty."));
}

Translator::~Translator()
{

}

wstring Translator::tr(const char *str)
{
    wstring translatedStr = StrToWStr(str);
    if (is_translations_valid) {
        for (auto &strIdPair : translMap) {
            //LocaleMap locMap = strIdPair.second;
            for (LocaleMap::const_iterator it = strIdPair.second.begin(); it != strIdPair.second.end(); ++it) {
                //wcout << L"\n\n" << translatedStr << L"\n" << it->second;
                if (it->second == translatedStr) {
                    if (strIdPair.second.find(langName) != strIdPair.second.end())
                        translatedStr = strIdPair.second[langName];
                    else {
                        wstring primaryLangAndScript = getPrimaryLang(langName, true);
                        if (strIdPair.second.find(primaryLangAndScript) != strIdPair.second.end())
                            translatedStr = strIdPair.second[primaryLangAndScript];
                        else {
                            wstring primaryLang = getPrimaryLang(langName);
                            if (strIdPair.second.find(primaryLang) != strIdPair.second.end())
                                translatedStr = strIdPair.second[primaryLang];
                        }
                    }
                    break;
                }
            }
        }
    }
    return translatedStr;
}

void Translator::parseTranslations()
{
    int token = TOKEN_BEGIN_DOCUMENT;
    wstring stringId, currentLocale;
    size_t pos = 0, len = translations.length();
    while (pos < len) {
        size_t incr = 1;
        wchar_t ch = translations.at(pos);

        switch (token) {
        case TOKEN_BEGIN_DOCUMENT:
        case TOKEN_END_VALUE:
            if (!isSeparator(ch)) {
                if (ch == L';') {
                    // string is comment
                    size_t end = translations.find_first_of(L'\n', pos);
                    incr = (end == wstring::npos) ? len - pos : end - pos + 1;
                } else {
                    size_t end;
                    for (end = pos; end < len; end++) {
                        wchar_t c = translations.at(end);
                        if (!isValidLocaleCharacter(c))
                            break;
                    }
                    size_t locale_len = end - pos;
                    if (locale_len < 12 && locale_len != 0 && locale_len != 1 && locale_len != 4 && locale_len != 9) {
                        token = TOKEN_BEGIN_LOCALE;
                        continue;
                    } else {
                        // TOKEN_ERROR
                        error_substr = translations.substr(0, pos + 1);
                        return;
                    }
                }
            }
            break;

        case TOKEN_BEGIN_STRING_ID:
            if (!isSeparator(ch)) {
                size_t end;
                wchar_t c;
                for (end = pos; end < len; end++) {
                    c = translations.at(end);
                    if (!isValidStringIdCharacter(c))
                        break;
                }
                c = translations.at(end);
                if (end < len && !isSeparator(c) && c != L'=') {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, end + 1);
                    return;
                }
                stringId = translations.substr(pos, end - pos);
                if (!stringId.empty() && translMap.find(stringId) == translMap.end())
                    translMap[stringId] = LocaleMap();

                token = TOKEN_END_STRING_ID;
                incr = end - pos;
            }
            break;

        case TOKEN_END_STRING_ID:
            if (!isSeparator(ch)) {
                if (ch == L'=') {
                    token = TOKEN_BEGIN_VALUE;
                } else {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, pos + 1);
                    return;
                }
            }
            break;

        case TOKEN_BEGIN_LOCALE: {
            size_t end;
            for (end = pos; end < len; end++) {
                wchar_t c = translations.at(end);
                if (!isValidLocaleCharacter(c))
                    break;
            }
            size_t locale_len = end - pos;
            currentLocale = translations.substr(pos, locale_len);
            if (pos + locale_len == len) {
                error_substr = translations.substr(0, pos + locale_len);
                return;
            }
            token = TOKEN_END_LOCALE;
            incr = locale_len;
            break;
        }

        case TOKEN_END_LOCALE:
            if (!isSeparator(ch)) {
                if (ch == L'.') {
                    token = TOKEN_BEGIN_STRING_ID;
                } else {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, pos + 1);
                    return;
                }
            }
            break;

        case TOKEN_BEGIN_VALUE: {
            size_t end = translations.find_first_of(L'\n', pos);
            wstring val;
            if (end == wstring::npos) {
                val = translations.substr(pos);
                incr = len - pos;
            } else {
                val = translations.substr(pos, end - pos);
                incr = end - pos;
            }

            if (!val.empty() && val.back() == L'\r')
                val.pop_back();

            size_t p = val.find(L"\\n");
            while (p != std::string::npos) {
                val.replace(p, 2, L"\\");
                val[p] = L'\n';
                p = val.find(L"\\n", p + 1);
            }
            if (!currentLocale.empty() && translMap.find(stringId) != translMap.end())
                translMap[stringId][currentLocale] = val;

            token = TOKEN_END_VALUE;
            break;
        }

        default:
            break;
        }
        pos += incr;
        if (pos == len)
            token = TOKEN_END_DOCUMENT;
    }

    if (token == TOKEN_END_DOCUMENT)
        is_translations_valid = true;
}
