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

#include "cdownloader.h"
#include <Windows.h>
#include <Winhttp.h>

#define DNL_OK         0
#define DNL_ABORT      1
#define DNL_URL_ERR    2
#define DNL_CONN_ERR   3
#define DNL_OUT_MEM    4
#define DNL_CREAT_ERR  5
#define DNL_OTHER_ERR  6


int downloadToFile(const wstring &url, const wstring &filePath, std::atomic_bool &run, FnVoidInt &progress_callback)
{
    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = 1;
    urlComp.dwUrlPathLength = 1;
    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp))
        return DNL_URL_ERR;

    wstring url_host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    wstring url_path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    int prev_percent = -1;
    DWORD dwProgress = 0;
    DWORD dwProgressMax = 0;
    DWORD dwSize = sizeof(DWORD);
    HANDLE hFile = INVALID_HANDLE_VALUE;

    HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return DNL_OTHER_ERR;
    //WinHttpSetStatusCallback(hSession, WinHttpCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
    DWORD dwEnabledProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwEnabledProtocols, sizeof(DWORD));

    HINTERNET hConnect = WinHttpConnect(hSession, url_host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return DNL_CONN_ERR;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", url_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return DNL_CONN_ERR;
    }

    int result = DNL_OK;
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        result = DNL_OTHER_ERR;
        goto cleanup;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        result = DNL_CONN_ERR;
        goto cleanup;
    }

    if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwProgressMax, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
        result = DNL_CONN_ERR;
        goto cleanup;
    }

    hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        result = DNL_CREAT_ERR;
        goto cleanup;
    }

    do {
        if (!run) {
            result = DNL_ABORT;
            break;
        }

        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            result = DNL_CONN_ERR;
            break;
        }

        LPSTR lpBuffer = new char[dwSize];
        if (!lpBuffer) {
            result = DNL_OUT_MEM;
            break;
        }

        DWORD dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, (LPVOID)lpBuffer, dwSize, &dwDownloaded)) {
            result = DNL_CONN_ERR;
            delete[] lpBuffer;
            break;
        }

        DWORD dwBytesWritten = 0;
        BOOL write_res = WriteFile(hFile, lpBuffer, dwDownloaded, &dwBytesWritten, NULL);
        delete[] lpBuffer;
        if (!write_res || dwBytesWritten != dwDownloaded) {
            result = DNL_OUT_MEM;
            break;
        }

        if (dwProgressMax != 0 && progress_callback) {
            dwProgress += dwDownloaded;
            int percent = static_cast<int>((100.0 * dwProgress) / dwProgressMax);
            if (percent != prev_percent) {
                progress_callback(percent);
                prev_percent = percent;
            }
        }

    } while (dwSize > 0);

cleanup :
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

CDownloader::CDownloader()
{
    m_run = true;
    m_lock = false;
}

CDownloader::~CDownloader()
{
    m_run = false;
    if (m_future.valid())
        m_future.wait();
}

void CDownloader::downloadFile(const std::wstring &url, const std::wstring &filePath)
{
    m_url.clear();
    m_filePath.clear();
    if (url.empty() || filePath.empty() || m_lock)
        return;

    m_url = url;
    m_filePath = filePath;
    start();
}

void CDownloader::start()
{
    if (m_url.empty() || m_filePath.empty() || m_lock)
        return;

    m_run = true;
    m_lock = true;
    m_future = std::async(std::launch::async, [=]() {
        int hr = downloadToFile(m_url, m_filePath, m_run, m_progress_callback);
        int error = (hr == DNL_OK) ? 0 :
                    (hr == DNL_ABORT) ? 1 :
                    (hr == DNL_OUT_MEM) ? -1 :
                    (hr == DNL_CONN_ERR) ? -2 : -3;

        if (m_complete_callback)
            m_complete_callback(error);
        m_lock = false;
    });
}

void CDownloader::stop()
{
    m_run = false;
}

wstring CDownloader::GetFilePath()
{
    return m_filePath;
}

void CDownloader::onComplete(FnVoidInt callback)
{
    m_complete_callback = callback;
}

void CDownloader::onProgress(FnVoidInt callback)
{
    m_progress_callback = callback;
}
