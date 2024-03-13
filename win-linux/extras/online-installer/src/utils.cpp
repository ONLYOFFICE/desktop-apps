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
#include "resource.h"
#include "translator.h"
#include <Windows.h>
#include <fstream>
#include <regex>
#include <Softpub.h>
#include <TlHelp32.h>
#include <sstream>

#define _TR(str) Translator::tr(str).c_str()
#define BIT123_LAYOUTRTL 0x08000000
#ifndef LOCALE_IREADINGLAYOUT
# define LOCALE_IREADINGLAYOUT 0x70
#endif


namespace NS_Utils
{
    wstring GetLastErrorAsString()
    {
        DWORD errID = ::GetLastError();
        if (errID == 0)
            return _T("");

        LPTSTR msgBuff = NULL;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       NULL, errID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuff, 0, NULL);
        wstring msg(msgBuff, (int)size);
        LocalFree(msgBuff);
        return msg;
    }

    void ShowMessage(wstring str, bool showError)
    {
        if (showError)
            str += _T(" ") + GetLastErrorAsString();
        wstring caption(_T("    "));
        caption.append(_TR(CAPTION_TEXT));
        MessageBox(NULL, str.c_str(), caption.c_str(), MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
    }

    bool IsRtlLanguage(unsigned long lcid)
    {
        if (NS_File::getWinVersion() >= WinVer::Win7) {
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
}

namespace NS_File
{
    bool runProcess(const wstring &fileName, const wstring &args, bool runAsAdmin)
    {
        SHELLEXECUTEINFO shExInfo = {0};
        shExInfo.cbSize = sizeof(shExInfo);
        shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE | SEE_MASK_FLAG_NO_UI;
        shExInfo.hwnd = NULL;
        shExInfo.lpVerb = runAsAdmin ? _T("runas") : _T("open");
        shExInfo.lpFile = fileName.c_str();
        shExInfo.lpParameters = args.c_str();
        shExInfo.lpDirectory = NULL;
        shExInfo.nShow = SW_HIDE;
        shExInfo.hInstApp = NULL;
        if (ShellExecuteEx(&shExInfo)) {
            WaitForSingleObject(shExInfo.hProcess, INFINITE);
            CloseHandle(shExInfo.hProcess);
            return true;
        }
        return false;
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

    wstring fromNativeSeparators(const wstring &path)
    {
        return std::regex_replace(path, std::wregex(_T("\\\\")), _T("/"));
    }

    wstring toNativeSeparators(const wstring &path)
    {
        return std::regex_replace(path, std::wregex(_T("\\/")), _T("\\"));
    }

    wstring parentPath(const wstring &path)
    {
        wstring::size_type delim = path.find_last_of(_T("\\/"));
        return (delim == wstring::npos) ? _T("") : path.substr(0, delim);
    }

//    wstring tempPath()
//    {
//        TCHAR buff[MAX_PATH] = {0};
//        DWORD res = ::GetTempPath(MAX_PATH, buff);
//        return (res != 0) ? fromNativeSeparators(parentPath(buff)) : _T("");
//    }

    wstring appPath()
    {
        TCHAR buff[MAX_PATH] = {0};
        DWORD res = ::GetModuleFileName(NULL, buff, MAX_PATH);
        return (res != 0) ? fromNativeSeparators(parentPath(buff)) : _T("");
    }

    WinVer getWinVersion()
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
                    winVer = os.dwMajorVersion == 5L && (os.dwMinorVersion == 1L || os.dwMinorVersion == 2L) ? WinVer::WinXP :
                             os.dwMajorVersion == 6L && os.dwMinorVersion == 0L ? WinVer::WinVista :
                             os.dwMajorVersion == 6L && os.dwMinorVersion == 1L ? WinVer::Win7 :
                             os.dwMajorVersion == 6L && os.dwMinorVersion == 2L ? WinVer::Win8 :
                             os.dwMajorVersion == 6L && os.dwMinorVersion == 3L ? WinVer::Win8_1 :
                             os.dwMajorVersion == 10L && os.dwMinorVersion == 0L && os.dwBuildNumber < 22000 ? WinVer::Win10 :
                             os.dwMajorVersion == 10L && os.dwMinorVersion == 0L && os.dwBuildNumber >= 22000 ? WinVer::Win11 :
                             os.dwMajorVersion == 10L && os.dwMinorVersion > 0L ? WinVer::Win11 :
                             os.dwMajorVersion > 10L ? WinVer::Win11 : WinVer::Undef;
                }
            }
        }
        return winVer;
    }

//    bool verifyEmbeddedSignature(const wstring &fileName)
//    {
//        WINTRUST_FILE_INFO fileInfo;
//        ZeroMemory(&fileInfo, sizeof(fileInfo));
//        fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
//        fileInfo.pcwszFilePath = fileName.c_str();
//        fileInfo.hFile = NULL;
//        fileInfo.pgKnownSubject = NULL;

//        GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
//        WINTRUST_DATA winTrustData;
//        ZeroMemory(&winTrustData, sizeof(winTrustData));
//        winTrustData.cbStruct = sizeof(WINTRUST_DATA);
//        winTrustData.pPolicyCallbackData = NULL;
//        winTrustData.pSIPClientData = NULL;
//        winTrustData.dwUIChoice = WTD_UI_NONE;
//        winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
//        winTrustData.dwUnionChoice = WTD_CHOICE_FILE;
//        winTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
//        winTrustData.hWVTStateData = NULL;
//        winTrustData.pwszURLReference = NULL;
//        winTrustData.dwUIContext = 0;
//        winTrustData.pFile = &fileInfo;
//        return WinVerifyTrust(NULL, &guidAction, &winTrustData) == ERROR_SUCCESS;
//    }
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
