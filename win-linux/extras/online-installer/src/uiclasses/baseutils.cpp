#include "baseutils.h"


static int getLuma(COLORREF color)
{
    return int(0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color));
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
    HKEY hKey;
    DWORD dwValue = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);
        if (RegQueryValueEx(hKey, L"ColorPrevalence", nullptr, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
        }
        RegCloseKey(hKey);
    }
    if (isActive && dwValue != 0) {
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
    }
#define BORDER_ACTIVE_DARK       RGB(0x2a, 0x2a, 0x2a) // Dark theme
#define BORDER_INACTIVE_DARK     RGB(0x3a, 0x3a, 0x3a)
#define BORDER_ACTIVE_LIGHT_V1   RGB(0x58, 0x58, 0x58) // Light theme and colored background
#define BORDER_ACTIVE_LIGHT_V2   RGB(0x77, 0x77, 0x77) // Light theme and white background
#define BORDER_INACTIVE_LIGHT_V1 RGB(0x60, 0x60, 0x60)
#define BORDER_INACTIVE_LIGHT_V2 RGB(0xaa, 0xaa, 0xaa)
    int luma = getLuma(topColor);
    COLORREF color = luma < 85 ? (isActive ? BORDER_ACTIVE_DARK : BORDER_INACTIVE_DARK) :
                     luma < 170 ? (isActive ? BORDER_ACTIVE_LIGHT_V1 : BORDER_INACTIVE_LIGHT_V1) :
                                  (isActive ? BORDER_ACTIVE_LIGHT_V2 : BORDER_INACTIVE_LIGHT_V2);
    return color;
}

bool Utils::isColorDark(COLORREF color)
{   
    return int(0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color)) < 128;
}
