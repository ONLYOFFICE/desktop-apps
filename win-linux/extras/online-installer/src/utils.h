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

#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <string>

using std::string;
using std::wstring;
using std::to_wstring;

#define ERROR_LAUNCH 0x20000000
#define DEFAULT_ERROR_MESSAGE _T("An error occurred: ") + \
    wstring(_T(__FUNCTION__)) + _T(" Line: ") + to_wstring(__LINE__)
#define ADVANCED_ERROR_MESSAGE DEFAULT_ERROR_MESSAGE + \
    _T(" ") + NS_Utils::GetLastErrorAsString()


namespace NS_Utils
{
void parseCmdArgs(int argc, wchar_t *argv[]);
bool cmdArgContains(const wstring &param);
wstring cmdArgValue(const wstring &param);
wstring GetLastErrorAsString(DWORD errID = 0);
void ShowMessage(wstring str, bool showError = false);
int  ShowTaskDialog(HWND parent, const wstring &msg, PCWSTR icon);
bool IsRtlLanguage(unsigned long lcid);
bool IsWin64();
bool IsAppInstalled(wstring &path, wstring *arch = nullptr);
bool checkAndWaitForAppClosure(HWND parent = nullptr);
void InstalledVerInfo(LPCWSTR value, wstring &name, wstring &arch);
void Replace(wstring &str, const wstring &from, const wstring &to);
wstring MsiGetProperty(LPCWSTR prodCode, LPCWSTR propName);
wstring MsiProductCode(const wstring &prodName);
}

namespace NS_File
{
DWORD runProcess(const wstring &fileName, const wstring &args, bool runAsAdmin = false, bool wait = true);
// bool isProcessRunning(const wstring &fileName);
bool fileExists(const wstring &filePath);
bool removeFile(const wstring &filePath);
bool removeDirRecursively(const wstring &dir);
wstring fromNativeSeparators(const wstring &path);
wstring toNativeSeparators(const wstring &path);
wstring parentPath(const wstring &path);
wstring tempPath();
wstring appPath();
wstring appDataPath();
wstring generateTmpFileName(const wstring &ext);
bool verifyEmbeddedSignature(const wstring &fileName);
}

namespace NS_Logger
{
void AllowWriteLog();
void WriteLog(const wstring &log, bool showMessage = false);
}

#endif // UTILS_H
