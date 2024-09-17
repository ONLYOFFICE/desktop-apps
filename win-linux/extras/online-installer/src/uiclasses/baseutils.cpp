#include "baseutils.h"
#include <string>
#include <shlwapi.h>
#include <sddl.h>


static int getLuma(COLORREF color)
{
    return int(0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color));
}

static COLORREF LighterColor(COLORREF color, WORD factor)
{
    WORD h = 0, l = 0, s = 0;
    ColorRGBToHLS(color, &h, &l, &s);
    double k = (double)factor/100;
    l = min(240, (unsigned)round(k * l));
    return ColorHLSToRGB(h, l, s);
}

static std::wstring GetCurrentUserSID()
{
    static std::wstring user_sid;
    if (user_sid.empty()) {
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DWORD tokenLen = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenLen);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenLen)) {
                    if (GetTokenInformation(hToken, TokenUser, pTokenUser, tokenLen, &tokenLen)) {
                        LPWSTR sid = NULL;
                        if (ConvertSidToStringSid(pTokenUser->User.Sid, &sid)) {
                            user_sid = sid;
                            LocalFree(sid);
                        }
                    }
                    free(pTokenUser);
                }
            }
            CloseHandle(hToken);
        }
    }
    return user_sid;
}

static DWORD RegQueryDwordValue(HKEY rootKey, LPCWSTR subkey, LPCWSTR value)
{
    HKEY hKey;
    DWORD dwValue = 0;
    if (RegOpenKeyEx(rootKey, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, value, nullptr, &dwType, (LPBYTE)&dwValue, &dwSize);
        RegCloseKey(hKey);
    }
    return dwValue;
}

Utils::WinVer Utils::getWinVersion()
{
    static WinVer winVer = WinVer::Undef;
    if (winVer == WinVer::Undef) {
        if (HMODULE module = GetModuleHandleA("ntdll")) {
            NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
            *(FARPROC*)&RtlGetVersion = GetProcAddress(module, "RtlGetVersion");
            if (RtlGetVersion) {
                OSVERSIONINFOEXW os = {0};
                os.dwOSVersionInfoSize = sizeof(os);
                RtlGetVersion(&os);
#define MjrVer os.dwMajorVersion
#define MinVer os.dwMinorVersion
#define BldVer os.dwBuildNumber
                winVer = MjrVer == 5L && (MinVer == 1L || MinVer == 2L) ? WinVer::WinXP :
                         MjrVer == 6L && MinVer == 0L ? WinVer::WinVista :
                         MjrVer == 6L && MinVer == 1L ? WinVer::Win7 :
                         MjrVer == 6L && MinVer == 2L ? WinVer::Win8 :
                         MjrVer == 6L && MinVer == 3L ? WinVer::Win8_1 :
                         MjrVer == 10L && MinVer == 0L && BldVer < 22000 ? WinVer::Win10 :
                         MjrVer == 10L && MinVer == 0L && BldVer >= 22000 ? WinVer::Win11 :
                         MjrVer == 10L && MinVer > 0L ? WinVer::Win11 :
                         MjrVer > 10L ? WinVer::Win11 : WinVer::Undef;
            }
        }
    }
    return winVer;
}

COLORREF Utils::getColorizationColor(bool isActive, COLORREF topColor)
{
    int luma = getLuma(topColor);
    if (isActive) {
        if (RegQueryDwordValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM", L"ColorPrevalence") != 0) {
            DWORD dwcolor = 0;
            BOOL opaque = TRUE;
            HRESULT(WINAPI *DwmGetColorizationColor)(DWORD*, BOOL*) = NULL;
            if (HMODULE module = LoadLibrary(L"dwmapi")) {
                *(FARPROC*)&DwmGetColorizationColor = GetProcAddress(module, "DwmGetColorizationColor");
                if (DwmGetColorizationColor && !SUCCEEDED(DwmGetColorizationColor(&dwcolor, &opaque))) {
                    dwcolor = 0;
                }
                FreeLibrary(module);
                if (dwcolor)
                    return RGB((dwcolor & 0xff0000) >> 16, (dwcolor & 0xff00) >> 8, dwcolor & 0xff);
            }
        } else {
            if (RegQueryDwordValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"SystemUsesLightTheme") != 0) {
                std::wstring userSid = GetCurrentUserSID();
                if (!userSid.empty()) {
                    userSid.append(L"\\Control Panel\\Desktop");
                    if (RegQueryDwordValue(HKEY_USERS, userSid.c_str(), L"AutoColorization") != 0)
                        return LighterColor(topColor, 95);
                }
            }
        }
        int res = -0.002*luma*luma + 0.93*luma + 6;
        return RGB(res, res, res);
    }
    int res = -0.0007*luma*luma + 0.78*luma + 25;
    return RGB(res, res, res);
}

// bool Utils::isColorDark(COLORREF color)
// {
//     return int(0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color)) < 128;
// }
