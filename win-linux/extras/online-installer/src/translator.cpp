
#define WIN32_LEAN_AND_MEAN
#include "translator.h"
#include "resource.h"
#include "utils.h"
#include <Windows.h>
#include <cwctype>


bool isSeparator(wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

bool isValidStringIdCharacter(wchar_t c)
{
    return std::iswalnum(c) || std::iswalpha(c) || c == L'_';
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
wstring Translator::langName = _T("en");
bool Translator::is_translations_valid = false;

Translator::Translator(unsigned short langId, int resourceId)
{
    TCHAR _langName[LOCALE_NAME_MAX_LENGTH] = {0};
    int res = GetLocaleInfo(PRIMARYLANGID(langId), LOCALE_SISO639LANGNAME, _langName, LOCALE_NAME_MAX_LENGTH);
    if (res > 0)
        langName = _langName;
    else
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
            if (!isSeparator(ch)) {
                if (isValidStringIdCharacter(ch)) {
                    token = TOKEN_BEGIN_STRING_ID;
                    continue;
                } else
                if (ch == L'/' && (pos + 1) < len && translations.at(pos + 1) == L'/') {
                    // string is comment
                    size_t end = translations.find_first_of(L'\n', pos);
                    incr = (end == wstring::npos) ? len - pos : end - pos + 1;
                } else {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, pos + 1);
                    return;
                }
            }
            break;

        case TOKEN_BEGIN_STRING_ID: {
            size_t end;
            for (end = pos; end < len; end++) {
                wchar_t c = translations.at(end);
                if (!isValidStringIdCharacter(c))
                    break;
            }
            if (end < len && !isSeparator(translations.at(end))) {
                // TOKEN_ERROR
                error_substr = translations.substr(0, end + 1);
                return;
            }
            stringId = translations.substr(pos, end - pos);
            translMap[stringId] = LocaleMap();
            token = TOKEN_END_STRING_ID;
            incr = end - pos;
            break;
        }

        case TOKEN_END_STRING_ID:
        case TOKEN_END_VALUE: {
            if (!isSeparator(ch)) {
                size_t end;
                for (end = pos; end < len; end++) {
                    wchar_t c = translations.at(end);
                    if (!std::iswalpha(c))
                        break;
                }
                if (end - pos == 2) {
                    token = TOKEN_BEGIN_LOCALE;
                    continue;
                } else {
                    if (isValidStringIdCharacter(ch)) {
                        token = TOKEN_BEGIN_STRING_ID;
                        continue;
                    } else {
                        // TOKEN_ERROR
                        error_substr = translations.substr(0, end + 1);
                        return;
                    }
                }
            }
            break;
        }

        case TOKEN_BEGIN_LOCALE: {
            currentLocale = translations.substr(pos, 2);
            if (pos + 2 == len) {
                error_substr = translations.substr(0, pos + 2);
                return;
            }
            token = TOKEN_END_LOCALE;
            incr = 2;
            break;
        }

        case TOKEN_END_LOCALE:
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
