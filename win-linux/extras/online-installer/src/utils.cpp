/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "utils.h"
#include "baseutils.h"
#include "resource.h"
#include "translator.h"
#include <shlwapi.h>
#include <fstream>
#include <algorithm>
#include <Softpub.h>
#include <TlHelp32.h>
#include <Msi.h>
#include <ShlObj.h>
// #include <sstream>
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#define APP_REG_PATH "\\" REG_GROUP_KEY "\\" REG_APP_NAME
#define BIT123_LAYOUTRTL 0x08000000
#ifndef LOCALE_IREADINGLAYOUT
# define LOCALE_IREADINGLAYOUT 0x70
#endif


static void RegQueryStringValue(HKEY rootKey, LPCWSTR subkey, REGSAM advFlags, LPCWSTR value, wstring &result)
{
    HKEY hKey;
    if (RegOpenKeyExW(rootKey, subkey, 0, KEY_READ | advFlags, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_SZ, cbData = 0;
        if (SHGetValue(hKey, L"", value, &type, NULL, &cbData) == ERROR_SUCCESS) {
            wchar_t *pvData = (wchar_t*)malloc(cbData);
            if (SHGetValue(hKey, L"", value, &type, (void*)pvData, &cbData) == ERROR_SUCCESS)
                result = pvData;
            free(pvData);
        }
        RegCloseKey(hKey);
    }
}

namespace NS_Utils
{
    std::vector<wstring> cmd_args;

    void parseCmdArgs(int argc, wchar_t *argv[])
    {
        for (int i = 0; i < argc; i++)
            cmd_args.push_back(argv[i]);
    }

    bool cmdArgContains(const wstring &param)
    {
        auto len = param.length();
        return std::any_of(cmd_args.cbegin(), cmd_args.cend(), [&param, len](const wstring &arg) {
            return arg.find(param) == 0 && (len == arg.length() || arg[len] == L'=' || arg[len] == L':' || arg[len] == L'|');
        });
    }

    wstring cmdArgValue(const wstring &param)
    {
        auto len = param.length();
        for (const auto &arg : cmd_args) {
            if (arg.find(param) == 0 && len < arg.length() && (arg[len] == L'=' || arg[len] == L':' || arg[len] == L'|'))
                return arg.substr(len + 1);
        }
        return L"";
    }

    wstring GetLastErrorAsString(DWORD _errID)
    {
        DWORD errID = _errID != 0 ? _errID : ::GetLastError();
        if (errID == 0)
            return _T("");

        LPTSTR msgBuff = NULL;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       NULL, errID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuff, 0, NULL);
        wstring msg = _TR(LABEL_ERR_COMMON) + wstring(L" ") + std::to_wstring(errID);
        if (size > 0) {
            msg.append(L"\n" + wstring(msgBuff, (int)size));
            LocalFree(msgBuff);
        }
        return msg;
    }

    void ShowMessage(wstring str, bool showError)
    {
        if (showError)
            str += _T(" ") + GetLastErrorAsString();
        wstring caption(_T("    "));
        caption.append(_TR(CAPTION));
        MessageBox(NULL, str.c_str(), caption.c_str(), MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
    }

    int ShowTaskDialog(HWND parent, const wstring &msg, PCWSTR icon)
    {
        HWND fakeParent = NULL;
        HMODULE hInst = GetModuleHandle(NULL);
        if (!parent) {
            WNDCLASS wc = {0};
            wc.lpfnWndProc   = DefWindowProc;
            wc.hInstance     = hInst;
            wc.lpszClassName = L"FakeWindowClass";
            RegisterClass(&wc);
            fakeParent = CreateWindowEx(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
            parent = fakeParent;
        }

        int result = IDCANCEL;
        wstring caption(_T("    "));
        caption.append(_TR(CAPTION));
        if (HMODULE lib = LoadLibrary(L"Comctl32")) {
            HRESULT (WINAPI *_TaskDialog)(HWND, HINSTANCE, PCWSTR, PCWSTR, PCWSTR, TASKDIALOG_COMMON_BUTTON_FLAGS, PCWSTR, int*);
            *(FARPROC*)&_TaskDialog = GetProcAddress(lib, "TaskDialog");
            if (_TaskDialog)
                _TaskDialog(parent, hInst, caption.c_str(), msg.c_str(), NULL, TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON, icon, &result);
            FreeLibrary(lib);
        }

        if (fakeParent)
            DestroyWindow(fakeParent);
        return result;
    }

    bool IsRtlLanguage(unsigned long lcid)
    {
        if (Utils::getWinVersion() >= Utils::WinVer::Win7) {
            DWORD layout = 0;
            if (GetLocaleInfo(lcid, LOCALE_IREADINGLAYOUT | LOCALE_RETURN_NUMBER, (LPWSTR)&layout, sizeof(layout)/sizeof(WCHAR)) > 0)
                return layout == 1;
        } else {
            LOCALESIGNATURE lsig;
            if (GetLocaleInfo(lcid, LOCALE_FONTSIGNATURE, (LPWSTR)&lsig, sizeof(lsig)/sizeof(WCHAR)) > 0)
                return (lsig.lsUsb[3] & BIT123_LAYOUTRTL) != 0;
        }
        return false;
    }

    bool IsWin64()
    {
#ifdef _WIN64
        return true;
#else
        BOOL wow64 = FALSE;
        return IsWow64Process(GetCurrentProcess(), &wow64) && wow64;
#endif
    }

    bool IsAppInstalled(wstring &path, wstring *arch)
    {
        std::vector<REGSAM> flags{0};
        if (NS_Utils::IsWin64()) {
#ifdef _WIN64
            flags.push_back(KEY_WOW64_32KEY);
#else
            flags.push_back(KEY_WOW64_64KEY);
#endif
        }
        wstring subkey(L"Software");
        subkey += _T(APP_REG_PATH);
        for (auto &flag : flags) {
            RegQueryStringValue(HKEY_LOCAL_MACHINE, subkey.c_str(), flag, L"AppPath", path);
            if (!path.empty() && (path.back() == L'\\' || path.back() == L'/'))
                path.pop_back();
            if (!path.empty() /*&& NS_File::fileExists(path + _T(APP_LAUNCH_NAME))*/) {                    
                if (arch) {
#ifdef _WIN64
                    *arch = (flag == 0) ? L"x64" : L"x86";
#else
                    *arch = (flag == 0) ? L"x86" : L"x64";
#endif
                }
                return true;
            }
        }
        return false;
    }

    bool checkAndWaitForAppClosure(HWND parent)
    {
        bool accept = true;
        if (HWND app_hwnd = FindWindow(WINDOW_CLASS_NAME, NULL)) {
            wstring msg(_TR(MSG_ERR_TRY_CLOSE_APP));
            NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
            accept = (IDOK == NS_Utils::ShowTaskDialog(parent, msg.c_str(), TD_INFORMATION_ICON));
            if (accept) {
                PostMessage(app_hwnd, UM_INSTALL_UPDATE, 0, 0);
                Sleep(1000);
                while(true) {
                    if ((app_hwnd = FindWindow(WINDOW_CLASS_NAME, NULL)) != nullptr) {
                        wstring msg(_TR(MSG_ERR_CLOSE_APP));
                        NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
                        int result = NS_Utils::ShowTaskDialog(parent, msg.c_str(), TD_WARNING_ICON);
                        if (result != IDOK) {
                            accept = false;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
        return accept;
    }

    void InstalledVerInfo(LPCWSTR value, wstring &name, wstring &arch)
    {
        if (!name.empty())
            name.clear();
        std::vector<REGSAM> flags{0};
        if (NS_Utils::IsWin64()) {
#ifdef _WIN64
            flags.push_back(KEY_WOW64_32KEY);
#else
            flags.push_back(KEY_WOW64_64KEY);
#endif
        }
        for (auto &flag : flags) {
            wstring subkey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
            subkey += _T(REG_UNINST_KEY);
            for (int i = 0; i < 2; i++) {
                RegQueryStringValue(HKEY_LOCAL_MACHINE, subkey.c_str(), flag, value, name);
                if (!name.empty()) {
                    if (arch.empty()) {
#ifdef _WIN64
                        arch = (flag == 0) ? L"x64" : L"x86";
#else
                        arch = (flag == 0) ? L"x86" : L"x64";
#endif
                    }
                    return;
                }
                subkey += L"_is1";
            }
        }
    }

    void Replace(wstring &str, const wstring &from, const wstring &to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    wstring MsiGetProperty(LPCWSTR prodCode, LPCWSTR propName)
    {
        DWORD buffSize = 0;
        UINT res = MsiGetProductInfoW(prodCode, propName, NULL, &buffSize);
        if ((res == ERROR_MORE_DATA || res == ERROR_SUCCESS) && buffSize > 0) {
            ++buffSize;
            wchar_t *value = new wchar_t[buffSize];
            if (MsiGetProductInfoW(prodCode, propName, value, &buffSize) == ERROR_SUCCESS) {
                wstring propValue = value;
                delete[] value;
                return propValue;
            }
            delete[] value;
        }
        return wstring();
    }

    wstring MsiProductCode(const wstring &prodName)
    {
        DWORD ind = 0;
        WCHAR prodCode[39];
        while (MsiEnumProductsEx(NULL, NULL, MSIINSTALLCONTEXT_MACHINE, ind++, prodCode, NULL, NULL, NULL) == ERROR_SUCCESS) {
            if (MsiGetProperty(prodCode, INSTALLPROPERTY_PRODUCTNAME) == prodName)
                return prodCode;
        }
        return wstring();
    }
}

namespace NS_File
{
    DWORD runProcess(const wstring &fileName, const wstring &args, bool runAsAdmin, bool wait)
    {
        SHELLEXECUTEINFO shExInfo = {0};
        shExInfo.cbSize = sizeof(shExInfo);
        shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE /*| SEE_MASK_FLAG_NO_UI*/;
        shExInfo.hwnd = NULL;
        shExInfo.lpVerb = runAsAdmin ? _T("runas") : _T("open");
        shExInfo.lpFile = fileName.c_str();
        shExInfo.lpParameters = args.c_str();
        shExInfo.lpDirectory = NULL;
        shExInfo.nShow = SW_HIDE;
        shExInfo.hInstApp = NULL;
        if (ShellExecuteEx(&shExInfo)) {
            DWORD exitCode = 0;
            if (wait && (WaitForSingleObject(shExInfo.hProcess, INFINITE) == WAIT_FAILED || !GetExitCodeProcess(shExInfo.hProcess, &exitCode)))
                exitCode = GetLastError();
            CloseHandle(shExInfo.hProcess);
            return exitCode;
        }
        return GetLastError() | ERROR_LAUNCH;
    }

//    bool isProcessRunning(const wstring &fileName)
//    {
//        HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//        if (snapShot == INVALID_HANDLE_VALUE)
//            return false;

//        PROCESSENTRY32 entry;
//        entry.dwSize = sizeof(PROCESSENTRY32);
//        if (!Process32First(snapShot, &entry)) {
//            CloseHandle(snapShot);
//            return false;
//        }

//        do {
//            if (lstrcmpi(entry.szExeFile, fileName.c_str()) == 0) {
//                CloseHandle(snapShot);
//                return true;
//            }
//        } while (Process32Next(snapShot, &entry));

//        CloseHandle(snapShot);
//        return false;
//    }

    bool fileExists(const wstring &filePath)
    {
        DWORD attr = ::GetFileAttributes(filePath.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool removeFile(const wstring &filePath)
    {
        return DeleteFile(filePath.c_str()) != 0;
    }

    bool removeDirRecursively(const wstring &dir)
    {
        WCHAR pFrom[_MAX_PATH + 1] = {0};
        swprintf_s(pFrom, sizeof(pFrom)/sizeof(WCHAR), L"%s%c", dir.c_str(), L'\0');
        SHFILEOPSTRUCT fop = {
            NULL,
            FO_DELETE,
            pFrom,
            NULL,
            FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
            FALSE,
            0,
            NULL
        };
        return SHFileOperation(&fop) == 0;
    }

    wstring fromNativeSeparators(const wstring &path)
    {
        wstring _path(path);
        std::replace(_path.begin(), _path.end(), L'\\', L'/');
        return _path;
    }

    wstring toNativeSeparators(const wstring &path)
    {
        wstring _path(path);
        std::replace(_path.begin(), _path.end(), L'/', L'\\');
        return _path;
    }

    wstring parentPath(const wstring &path)
    {
        auto delim = (path.size() > 2) ? path.find_last_of(_T("\\/"), path.size() - 2) : wstring::npos;
        return (delim == wstring::npos) ? _T("") : path.substr(0, delim);
    }

    wstring tempPath()
    {
        TCHAR buff[MAX_PATH + 1] = {0};
        DWORD res = ::GetTempPath(MAX_PATH + 1, buff);
        if (res != 0) {
            buff[res - 1] = '\0';
            return fromNativeSeparators(buff);
        }
        return _T("");
    }

    wstring appPath()
    {
        TCHAR buff[MAX_PATH] = {0};
        DWORD res = ::GetModuleFileName(NULL, buff, MAX_PATH);
        return (res != 0) ? fromNativeSeparators(parentPath(buff)) : _T("");
    }

    wstring generateTmpFileName(const wstring &ext)
    {
        wstring uuid_tstr;
        UUID uuid = {0};
        RPC_WSTR wszUuid = NULL;
        if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringW(&uuid, &wszUuid) == RPC_S_OK) {
            uuid_tstr = ((wchar_t*)wszUuid);
            RpcStringFreeW(&wszUuid);
        } else
            uuid_tstr = L"00000000-0000-0000-0000-000000000000";
        return NS_File::tempPath() + _T("/") + _T(FILE_PREFIX) + uuid_tstr + ext;
    }

    bool verifyEmbeddedSignature(const wstring &fileName)
    {
        WINTRUST_FILE_INFO wfi;
        ZeroMemory(&wfi, sizeof(wfi));
        wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
        wfi.pcwszFilePath = fileName.c_str();
        wfi.hFile = NULL;
        wfi.pgKnownSubject = NULL;

        GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        WINTRUST_DATA wtd;
        ZeroMemory(&wtd, sizeof(wtd));
        wtd.cbStruct = sizeof(WINTRUST_DATA);
        wtd.pPolicyCallbackData = NULL;
        wtd.pSIPClientData = NULL;
        wtd.dwUIChoice = WTD_UI_NONE;
        wtd.fdwRevocationChecks = WTD_REVOKE_NONE;
        wtd.dwUnionChoice = WTD_CHOICE_FILE;
        wtd.dwStateAction = WTD_STATEACTION_VERIFY;
        wtd.hWVTStateData = NULL;
        wtd.pwszURLReference = NULL;
        wtd.dwUIContext = 0;
        wtd.pFile = &wfi;
        return WinVerifyTrust(NULL, &guidAction, &wtd) == ERROR_SUCCESS;
    }

    wstring appDataPath()
    {
        TCHAR buff[MAX_PATH] = {0};
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buff))) {
            wstring path(buff);
            path.append(_T(APP_REG_PATH));
            path.append(_T("\\data"));
            return path;
        }
        return _T("");
    }
}

namespace NS_Logger
{
    bool allow_write_log = false;

    void AllowWriteLog()
    {
        allow_write_log = true;
    }

    void WriteLog(const wstring &log, bool showMessage)
    {
        if (allow_write_log) {
            wstring filpPath(NS_File::appPath() + _T("/installer_log.txt"));
            std::wofstream file(filpPath.c_str(), std::ios::app);
            if (!file.is_open()) {
                return;
            }
            file << log << std::endl;
            file.close();
        }
        if (showMessage)
            NS_Utils::ShowMessage(log);
    }
}
