/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <Windows.h>
#include <Winhttp.h>
#include <string>
#include <vector>

#define ERROR_BIT_29 0x20000000
#define MAX_REDIRECTS_ALLOWED 0
#define WINHTTP_DEFAULT_IO_BUFFER 8192

typedef DWORD(*ProgressCallback_t)(void* data, ULONGLONG dltotal, ULONGLONG dlnow);


class HTTPTransaction {
public:
    HTTPTransaction() = default;
    explicit HTTPTransaction(const std::wstring& url)
        : sUrl(url)
    {}

    HTTPTransaction(const HTTPTransaction &other) = delete;
    HTTPTransaction& operator=(const HTTPTransaction &other) = delete;

    ~HTTPTransaction()
    {
        cleanup();
    }

    enum Options : DWORD
    {
        HeadOnly        = (1 << 0),
        FollowRedirects = (1 << 1),
        Writedata       = (1 << 2)
    };

    void setUrl(const std::wstring& url)
    {
        sUrl = url;
    }

    void setOption(Options opts)
    {
        nOptions |= opts;
    }

    void setFileHandle(HANDLE hFile)
    {
        this->hFile = hFile;
    }

    void setProgressCallback(ProgressCallback_t pCallback)
    {
        pProgressCallback = pCallback;
    }

    void setProgressData(void* pData)
    {
        pProgressData = pData;
    }

    DWORD getContentLength(ULONGLONG &sizeOut)
    {
        wchar_t szBuf[32] = {};
        DWORD dwBufLen = sizeof(szBuf);
        if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH,
                                 nullptr, szBuf, &dwBufLen, WINHTTP_NO_HEADER_INDEX))
        {
            sizeOut = 0;
            return GetLastError();
        }
        sizeOut = wcstoull(szBuf, nullptr, 10);
        return ERROR_SUCCESS;
    }

    DWORD perform()
    {
        cleanup();

        DWORD result = parseUrl(sUrl);
        if (result != ERROR_SUCCESS)
            return result;

        result = openSession(L"WinHTTP Example/1.0");
        if (result != ERROR_SUCCESS)
            return result;

        result = connect(sHost, nPort);
        if (result != ERROR_SUCCESS)
            return result;

        result = openRequest();
        if (result != ERROR_SUCCESS)
            return result;

        result = configureRequest();
        if (result != ERROR_SUCCESS)
            return result;

        result = sendRequest();
        if (result != ERROR_SUCCESS)
            return result;

        result = receiveResponse();
        if (result != ERROR_SUCCESS)
            return result;

        DWORD dwStatus = 0;
        result = getStatus(dwStatus);
        if (result != ERROR_SUCCESS)
            return result;

        if (dwStatus < HTTP_STATUS_OK || dwStatus >= HTTP_STATUS_AMBIGUOUS)
            return ERROR_BIT_29 | dwStatus;

        if (nOptions & Options::Writedata)
            return performWriteData();

        return ERROR_SUCCESS;
    }

    void cleanup()
    {
        if (hRequest) { WinHttpCloseHandle(hRequest); hRequest = nullptr; }
        if (hConnect) { WinHttpCloseHandle(hConnect); hConnect = nullptr; }
        if (hSession) { WinHttpCloseHandle(hSession); hSession = nullptr; }
    }

private:
    static void CALLBACK WinHttpStatusCallback(HINTERNET hInternet,
                                               DWORD_PTR dwContext,
                                               DWORD dwInternetStatus,
                                               LPVOID lpvStatusInformation,
                                               DWORD dwStatusInformationLength)
    {
        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_REDIRECT)
        {
            LPCWSTR redirectUrl = (LPCWSTR)lpvStatusInformation;

            std::wstring msg = L"[WinHTTP] REDIRECT: ";
            msg += (redirectUrl ? redirectUrl : L"(null)");
            msg += L"\n";
            OutputDebugStringW(msg.c_str());
        }
    }

    DWORD parseUrl(const std::wstring& url)
    {
        URL_COMPONENTS uc = {};
        uc.dwStructSize = sizeof(uc);
        uc.dwHostNameLength = (DWORD)-1;
        uc.dwUrlPathLength = (DWORD)-1;
        uc.dwExtraInfoLength = (DWORD)-1;

        if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc))
            return GetLastError();

        sHost.assign(uc.lpszHostName, uc.dwHostNameLength);
        sUrlPath.assign(uc.lpszUrlPath, uc.dwUrlPathLength);
        if (uc.lpszExtraInfo && uc.dwExtraInfoLength > 0)
            sUrlPath.append(uc.lpszExtraInfo, uc.dwExtraInfoLength);

        nPort = (uc.nScheme == INTERNET_SCHEME_HTTPS)
                    ? INTERNET_DEFAULT_HTTPS_PORT
                    : INTERNET_DEFAULT_HTTP_PORT;

        bSecure = (uc.nScheme == INTERNET_SCHEME_HTTPS);

        return ERROR_SUCCESS;
    }

    DWORD openSession(const wchar_t* userAgent)
    {
        hSession = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession)
        {
            return GetLastError();
        }

        DWORD dwEnabledProtocols =
            WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 |
            WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 |
            WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 |
            WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;

        WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS,
                         &dwEnabledProtocols, sizeof(dwEnabledProtocols));
        return ERROR_SUCCESS;
    }

    DWORD connect(const std::wstring& host, INTERNET_PORT port)
    {
        hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
        if (!hConnect)
        {
            return GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD openRequest()
    {
        DWORD flags = bSecure ? WINHTTP_FLAG_SECURE : 0;
        const wchar_t* method = (nOptions & Options::HeadOnly) ? L"HEAD" : L"GET";
        hRequest = WinHttpOpenRequest(hConnect, method, sUrlPath.c_str(), nullptr,
                                      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest)
        {
            return GetLastError();
        }

        WinHttpSetStatusCallback(hRequest, WinHttpStatusCallback,
                                 WINHTTP_CALLBACK_FLAG_REDIRECT, 0);
        return ERROR_SUCCESS;
    }

    DWORD configureRequest()
    {
        DWORD maxRedirects = 0;
        if (nOptions & Options::FollowRedirects)
            maxRedirects = MAX_REDIRECTS_ALLOWED;

        if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_MAX_HTTP_AUTOMATIC_REDIRECTS,
                              &maxRedirects, sizeof(maxRedirects)))
        {
            return GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD sendRequest()
    {
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        {
            return GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD receiveResponse()
    {
        if (!WinHttpReceiveResponse(hRequest, nullptr))
        {
            return GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD getStatus(DWORD &status)
    {
        DWORD size = sizeof(status);
        if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                 nullptr, &status, &size, WINHTTP_NO_HEADER_INDEX))
        {
            return GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD performWriteData()
    {
        if (!hFile || hFile == INVALID_HANDLE_VALUE)
            return ERROR_INVALID_HANDLE;

        ULONGLONG dwProgress = 0;
        ULONGLONG dwProgressMax = 0;
        getContentLength(dwProgressMax);

        std::vector<BYTE> ioBuffer(WINHTTP_DEFAULT_IO_BUFFER);

        while (true)
        {
            DWORD dwBytesAvailable = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwBytesAvailable))
                return GetLastError();

            if (dwBytesAvailable == 0)
                break;

            if (ioBuffer.size() < dwBytesAvailable)
                ioBuffer.resize(dwBytesAvailable);

            DWORD dwBytesRead = 0;
            if (!WinHttpReadData(hRequest, (LPVOID)ioBuffer.data(), dwBytesAvailable, &dwBytesRead))
                return GetLastError();

            if (dwBytesRead == 0)
                break;

            DWORD dwBytesWritten = 0;
            if (!WriteFile(hFile, ioBuffer.data(), dwBytesRead, &dwBytesWritten, NULL))
                return GetLastError();

            if (dwBytesWritten != dwBytesRead)
                return ERROR_WRITE_FAULT;

            dwProgress += dwBytesRead;

            if (pProgressCallback)
            {
                DWORD result = pProgressCallback(pProgressData, dwProgressMax, dwProgress);
                if (result != ERROR_SUCCESS)
                    return result;
            }
        }

        return ERROR_SUCCESS;
    }

    std::wstring sUrl;
    std::wstring sHost;
    std::wstring sUrlPath;

    HANDLE hFile = nullptr;

    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;

    ProgressCallback_t pProgressCallback = nullptr;
    void* pProgressData = nullptr;

    DWORD nOptions = 0;
    INTERNET_PORT nPort = 0;
    bool bSecure = false;
};
