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

#include "cupdatemanager.h"
#include <algorithm>
#include <functional>
#include <locale>
#include <codecvt>
#include <vector>
#include "utils.h"
#include "../../src/defines.h"
#include <Windows.h>
#include <WinInet.h>
#include <shlwapi.h>
#include <tchar.h>
#include <sstream>

#define UPDATE_PATH      TEXT("/" REG_APP_NAME "Updates")
#define BACKUP_PATH      TEXT("/" REG_APP_NAME "Backup")
#define APP_LAUNCH_NAME  L"/DesktopEditors.exe"
#define APP_LAUNCH_NAME2 L"/editors.exe"
#define APP_HELPER       L"/editors_helper.exe"
#define DAEMON_NAME      L"/update-daemon.exe"
#define SUCCES_UNPACKED  L"/.success_unpacked"

using std::vector;


auto currentArch()->wstring
{
#ifdef _WIN64
    return L"_x64";
#else
    return L"_x86";
#endif
}

auto generateTmpFileName(const wstring &ext)->wstring
{
    wstring uuid_wstr;
    UUID uuid = {0};
    RPC_WSTR wszUuid = NULL;
    if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringW(&uuid, &wszUuid) == RPC_S_OK) {
        uuid_wstr = ((wchar_t*)wszUuid);
        RpcStringFreeW(&wszUuid);
    } else
        uuid_wstr = L"00000000-0000-0000-0000-000000000000";
    return NS_File::tempPath() + L"/" + TEXT(FILE_PREFIX) + uuid_wstr + currentArch() + ext;
}

auto isSuccessUnpacked(const wstring &successFilePath, const wstring &version)->bool
{
    list<wstring> lines;
    if (NS_File::readFile(successFilePath, lines)) {
        if (std::find(lines.begin(), lines.end(), version) != lines.end())
            return true;
    }
    return false;
}

CUpdateManager::CUpdateManager():
    m_downloadMode(Mode::CHECK_UPDATES),
    m_socket(new CSocket(APP_PORT, SVC_PORT)),
    m_pDownloader(new CDownloader),
    m_pUnzip(new CUnzip)
{
    init();
}

CUpdateManager::~CUpdateManager()
{
    if (m_future_clear.valid())
        m_future_clear.wait();
    delete m_pUnzip, m_pUnzip = nullptr;
    delete m_pDownloader, m_pDownloader = nullptr;
    delete m_socket, m_socket = nullptr;
    if (m_quit_callback)
        m_quit_callback();
}

void CUpdateManager::aboutToQuit(FnVoidVoid callback)
{
    m_quit_callback = callback;
}

void CUpdateManager::init()
{
    m_pDownloader->onComplete([=](int error) {
        onCompleteSlot(error, m_pDownloader->GetFilePath());
    });
    m_pDownloader->onProgress([=](int percent) {
        onProgressSlot(percent);
    });
    m_pUnzip->onComplete([=](int error) {
        onCompleteUnzip(error);
    });
    m_socket->onMessageReceived([=](void *data, size_t size) {
        wstring str((const wchar_t*)data), tmp;
        vector<wstring> params;
        std::wstringstream wss(str);
        while (std::getline(wss, tmp, L'|'))
            params.push_back(std::move(tmp));

        if (params.size() == 4) {
            switch (std::stoi(params[0])) {
            case MSG_CheckUpdates: {
                DeleteUrlCacheEntry(params[1].c_str());
                m_downloadMode = Mode::CHECK_UPDATES;
                if (m_pDownloader)
                    m_pDownloader->downloadFile(params[1], generateTmpFileName(L".json"));
                NS_Logger::WriteLog(L"Received MSG_CheckUpdates, URL: " + params[1]);
                break;
            }
            case MSG_LoadUpdates: {
                m_downloadMode = Mode::DOWNLOAD_UPDATES;
                if (m_pDownloader)
                    m_pDownloader->downloadFile(params[1], generateTmpFileName(L".zip"));
                NS_Logger::WriteLog(L"Received MSG_LoadUpdates, URL: " + params[1]);
                break;
            }
            case MSG_StopDownload: {
                m_downloadMode = Mode::CHECK_UPDATES;
                if (m_pDownloader)
                    m_pDownloader->stop();
                break;
            }
            case MSG_UnzipIfNeeded:
                unzipIfNeeded(params[1], params[2]);
                break;

            case MSG_StartReplacingFiles:
                startReplacingFiles();
                break;

            case MSG_ClearTempFiles:
                clearTempFiles(params[1], params[2]);
                break;

            default:
                break;
            }
        }
    });

    m_socket->onError([](const char* error) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        NS_Logger::WriteLog(converter.from_bytes(error));
    });
}

void CUpdateManager::onCompleteUnzip(const int error)
{
    if (error == UNZIP_OK) {
        // Сreate a file about successful unpacking for use in subsequent launches
        const wstring updPath = NS_File::tempPath() + UPDATE_PATH;
        list<wstring> successList{m_newVersion};
        if (!NS_File::writeToFile(updPath + SUCCES_UNPACKED, successList)) {
            m_lock = false;
            return;
        }
        if (!sendMessage(MSG_ShowStartInstallMessage))
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

    } else
    if (error == UNZIP_ERROR) {
        wstring error(L"An error occured while unpacking the archive");
        if (!sendMessage(MSG_OtherError, error))
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

    } else
    if (error == UNZIP_ABORT) {
        // Stop unzip
    }
    m_lock = false;
}

void CUpdateManager::onCompleteSlot(const int error, const wstring &filePath)
{
    if (error == 0) {
        switch (m_downloadMode) {
        case Mode::CHECK_UPDATES:
            sendMessage(MSG_LoadCheckFinished, filePath);
            break;
        case Mode::DOWNLOAD_UPDATES:
            sendMessage(MSG_LoadUpdateFinished, filePath);
            break;
        default:
            break;
        }
    } else
    if (error == 1) {
        // Pause or Stop
    } else
    if (error == -1) {
        sendMessage(MSG_OtherError, L"Update download failed: out of memory!");
    } else
    if (error == -2) {
        sendMessage(MSG_OtherError, L"Update download failed: server connection error!");
    } else {
        sendMessage(MSG_OtherError, L"Update download failed: network error!");
    }
}

void CUpdateManager::onProgressSlot(const int percent)
{
    if (m_downloadMode == Mode::DOWNLOAD_UPDATES)
        sendMessage(MSG_Progress, to_wstring(percent));
}

void CUpdateManager::unzipIfNeeded(const wstring &filePath, const wstring &newVersion)
{
    if (m_lock)
        return;
    m_lock = true;

    m_newVersion = newVersion;
    const wstring updPath = NS_File::tempPath() + UPDATE_PATH;
    auto unzip = [=]()->void {
        if (!NS_File::dirExists(updPath) && !NS_File::makePath(updPath)) {
            NS_Logger::WriteLog(L"An error occurred while creating dir: " + updPath);
            m_lock = false;
            return;
        }
        m_pUnzip->extractArchive(filePath, updPath);
    };

    if (!NS_File::dirExists(updPath) || NS_File::dirIsEmpty(updPath)) {
        unzip();
    } else {
        if (isSuccessUnpacked(updPath + SUCCES_UNPACKED, newVersion)) {
            m_lock = false;
            if (!sendMessage(MSG_ShowStartInstallMessage))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

        } else {
            if (!NS_File::removeDirRecursively(updPath))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            unzip();
        }
    }
}

void CUpdateManager::clearTempFiles(const wstring &prefix, const wstring &except)
{
    m_future_clear = std::async(std::launch::async, [=]() {
        list<wstring> filesList;
        wstring _error;
        if (!NS_File::GetFilesList(NS_File::tempPath(), &filesList, _error)) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return;
        }
        for (auto &filePath : filesList) {
            if (PathMatchSpec(filePath.c_str(), L"*.json") || PathMatchSpec(filePath.c_str(), L"*.zip")) {
                wstring lcFilePath(filePath);
                std::transform(lcFilePath.begin(), lcFilePath.end(), lcFilePath.begin(), ::tolower);
                if (lcFilePath.find(prefix) != wstring::npos && filePath != except)
                    NS_File::removeFile(filePath);
            }
        }
    });
}

void CUpdateManager::startReplacingFiles()
{
    wstring appPath = NS_File::appPath();
    wstring updPath = NS_File::tempPath() + UPDATE_PATH;
    wstring tmpPath = NS_File::tempPath() + BACKUP_PATH;
    if (!NS_File::dirExists(updPath)) {
        NS_Logger::WriteLog(L"Update cancelled. Can't find folder: " + updPath, true);
        return;
    }

#ifndef DONT_VERIFY_SIGNATURE
    // Verify the signature of executable files
    if (!NS_File::verifyEmbeddedSignature(updPath + APP_LAUNCH_NAME)) {
        NS_Logger::WriteLog(L"Update cancelled. The file signature is missing: " + updPath + APP_LAUNCH_NAME, true);
        return;
    }
    if (!NS_File::verifyEmbeddedSignature(updPath + APP_LAUNCH_NAME2)) {
        NS_Logger::WriteLog(L"Update cancelled. The file signature is missing: " + updPath + APP_LAUNCH_NAME2, true);
        return;
    }
#endif

    // Check backup folder
    if (NS_File::dirExists(tmpPath) && !NS_File::removeDirRecursively(tmpPath)) {
        NS_Logger::WriteLog(L"Update cancelled. Can't delete folder: " + tmpPath, true);
        return;
    }

    // Wait until the main app closes
    {
        int retries = 10;
        wstring app(APP_LAUNCH_NAME2);
        app = app.substr(1);
        while (NS_File::isProcessRunning(app) && retries-- > 0)
            Sleep(500);

        if (NS_File::isProcessRunning(app)) {
            NS_Logger::WriteLog(L"Update cancelled. The main application is not closed!", true);
            return;
        }
    }

    // Wait until editors_helper.exe closes
    {
        int retries = 10;
        wstring app(APP_HELPER);
        app = app.substr(1);
        while (NS_File::isProcessRunning(app) && retries-- > 0)
            Sleep(500);

        if (NS_File::isProcessRunning(app)) {
            NS_Logger::WriteLog(L"Update cancelled. The editors_helper.exe is not closed!", true);
            return;
        }
    }

    // Replace app path to Backup
    if (!NS_File::replaceFolder(appPath, tmpPath, true)) {
        NS_Logger::WriteLog(L"Update cancelled. Can't replace files to backup: " + NS_Utils::GetLastErrorAsString(), true);
        if (NS_File::dirExists(tmpPath) && !NS_File::dirIsEmpty(tmpPath)
                && !NS_File::replaceFolder(tmpPath, appPath))
            NS_Logger::WriteLog(L"Can't restore files from backup!", true);
        return;
    }

    // Move update path to app path
    if (!NS_File::replaceFolder(updPath, appPath, true)) {
        NS_Logger::WriteLog(L"Update cancelled. Can't move updates to App path: " + NS_Utils::GetLastErrorAsString(), true);

        if (NS_File::dirExists(appPath) && !NS_File::removeDirRecursively(appPath)) {
            NS_Logger::WriteLog(L"An error occurred while remove App path: " + NS_Utils::GetLastErrorAsString(), true);
            return;
        }
        if (!NS_File::replaceFolder(tmpPath, appPath, true))
            NS_Logger::WriteLog(L"An error occurred while restore files from backup: " + NS_Utils::GetLastErrorAsString(), true);

        NS_File::removeDirRecursively(updPath);
        return;
    }

    // To support a version with unins000 files inside the working folder
    if (NS_File::fileExists(tmpPath + L"/unins000.dat"))
        NS_File::replaceFile(tmpPath + L"/unins000.dat", appPath + L"/unins000.dat");
    if (NS_File::fileExists(tmpPath + L"/unins000.exe"))
        NS_File::replaceFile(tmpPath + L"/unins000.exe", appPath + L"/unins000.exe");

    // To support a version without update-daemon.exe inside the working folder
    if (!NS_File::fileExists(appPath + DAEMON_NAME) && NS_File::fileExists(tmpPath + DAEMON_NAME))
        NS_File::replaceFile(tmpPath + DAEMON_NAME, appPath + DAEMON_NAME);

    // Update version in registry
    {
        wstring ver;
        list<wstring> lines;
        if (NS_File::readFile(appPath + SUCCES_UNPACKED, lines)) {
            if (lines.size() > 0)
                ver = lines.front();
            NS_File::removeFile(appPath + SUCCES_UNPACKED);
        }
        if (!ver.empty()) {
            HKEY hKey, hAppKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                wstring app_name(TEXT(WINDOW_NAME));
                wstring app_key = app_name + L"_is1";
                if (RegOpenKeyEx(hKey, app_key.c_str(), 0, KEY_ALL_ACCESS, &hAppKey) == ERROR_SUCCESS) {
                    wstring disp_name = app_name + L" " + ver + L" (" + currentArch().substr(1) + L")";
                    if (RegSetValueEx(hAppKey, TEXT("DisplayName"), 0, REG_SZ, (const BYTE*)disp_name.c_str(), (DWORD)(disp_name.length() + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
                        NS_Logger::WriteLog(L"Can't update DisplayName in registry!");
                    if (RegSetValueEx(hAppKey, TEXT("DisplayVersion"), 0, REG_SZ, (const BYTE*)ver.c_str(), (DWORD)(ver.length() + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
                        NS_Logger::WriteLog(L"Can't update DisplayVersion in registry!");
                    RegCloseKey(hAppKey);
                }
                RegCloseKey(hKey);
            }
        }
    }

    // Remove Backup dir
    NS_File::removeDirRecursively(tmpPath);

    // Restart program
    if (!NS_File::runProcess(appPath + APP_LAUNCH_NAME, L""))
        NS_Logger::WriteLog(L"An error occurred while restarting the program!", true);
}

bool CUpdateManager::sendMessage(int cmd, const wstring &param1, const wstring &param2, const wstring &param3)
{
    wstring str = to_wstring(cmd) + L"|" + param1 + L"|" + param2 + L"|" + param3;
    size_t sz = str.size() * sizeof(str.front());
    return m_socket->sendMessage((void*)str.c_str(), sz);
}
