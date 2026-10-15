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

#include "cdownloader.h"
#include "private/httptransaction_win32_p.h"
#include <thread>
#include <atomic>
#include <mutex>


class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    int mapToInternalError(DWORD res)
    {
        if ((res & ERROR_BIT_29) != 0)
            return CDownloader::NetworkError;

        switch (res) {
        case ERROR_SUCCESS:
            return CDownloader::Success;

        case ERROR_CANCELLED:
        case ERROR_OPERATION_ABORTED:
        case ERROR_WINHTTP_OPERATION_CANCELLED:
            return CDownloader::Aborted;

        case ERROR_OUTOFMEMORY:
        case ERROR_NOT_ENOUGH_MEMORY:
            return CDownloader::OutOfMemory;

        case ERROR_WINHTTP_INVALID_URL:
        case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
            return CDownloader::InvalidUrl;

        case ERROR_WINHTTP_NAME_NOT_RESOLVED:
        case ERROR_WINHTTP_CANNOT_CONNECT:
        case ERROR_WINHTTP_CONNECTION_ERROR:
        case ERROR_WINHTTP_TIMEOUT:
        case ERROR_WINHTTP_RESEND_REQUEST:
        case ERROR_WINHTTP_REDIRECT_FAILED:
        case ERROR_WINHTTP_HEADER_NOT_FOUND:
        case ERROR_WINHTTP_SECURE_FAILURE:
        case ERROR_WINHTTP_SECURE_CERT_DATE_INVALID:
        case ERROR_WINHTTP_SECURE_CERT_CN_INVALID:
        case ERROR_WINHTTP_SECURE_CERT_REVOKED:
        case ERROR_WINHTTP_SECURE_CERT_REV_FAILED:
            return CDownloader::NetworkError;

        case ERROR_INVALID_HANDLE:
        case ERROR_INVALID_NAME:
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_BAD_PATHNAME:
        case ERROR_ACCESS_DENIED:
        case ERROR_ALREADY_EXISTS:
        case ERROR_CANNOT_MAKE:
        case ERROR_WRITE_FAULT:
        case ERROR_WRITE_PROTECT:
        case ERROR_NOT_READY:
        case ERROR_HANDLE_DISK_FULL:
        case ERROR_DISK_FULL:
        case ERROR_DEV_NOT_EXIST:
        case ERROR_DIR_NOT_EMPTY:
        case ERROR_LOCK_VIOLATION:
        case ERROR_SHARING_VIOLATION:
            return CDownloader::FileOpError;

        default:
            return CDownloader::UnknownError;
        }
    }

    void notifyQuery(int error, uint64_t lenght)
    {
        std::lock_guard<std::mutex> lock(m_cb_mutex);
        if (m_query_callback)
            m_query_callback(error, lenght);
    }

    void notifyComplete(int error, const tstring &filePath)
    {
        std::lock_guard<std::mutex> lock(m_cb_mutex);
        if (m_complete_callback)
            m_complete_callback(error, filePath);
    }

    void notifyProgress(int percent)
    {
        std::lock_guard<std::mutex> lock(m_cb_mutex);
        if (m_progress_callback)
            m_progress_callback(percent);
    }

    void resetCallbacks()
    {
        std::lock_guard<std::mutex> lock(m_cb_mutex);
        m_query_callback = nullptr;
        m_complete_callback = nullptr;
        m_progress_callback = nullptr;
    }

    FnVoidIntInt  m_query_callback = nullptr;
    FnVoidIntStr  m_complete_callback = nullptr;
    FnVoidInt     m_progress_callback = nullptr;

    int m_prev_percent = -1;

    std::mutex        m_cb_mutex;
    std::atomic_bool  m_run{true},
                      m_lock{false};
};


CDownloader::CDownloader() :
    pimpl(std::make_shared<CDownloaderPrivate>())
{}

CDownloader::~CDownloader()
{
    pimpl->m_run = false;
    pimpl->resetCallbacks();
}

bool CDownloader::isUrlAccessible(const tstring &url)
{
    if (url.empty())
        return false;

    HTTPTransaction tsn(url);
    tsn.setOption(HTTPTransaction::HeadOnly);
    tsn.setOption(HTTPTransaction::FollowRedirects);
    if (tsn.perform() != ERROR_SUCCESS)
        return false;

    return true;
}

void CDownloader::queryContentLength(const tstring &url)
{
    if (url.empty())
    {
        pimpl->notifyQuery(CDownloader::InvalidUrl, 0);
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        pimpl->notifyQuery(CDownloader::Aborted, 0);
        return;
    }

    auto pimpl_copy = pimpl;
    std::thread([pimpl_copy, url]() {
        ULONGLONG dwFileSize = 0;

        HTTPTransaction tsn(url);
        tsn.setOption(HTTPTransaction::HeadOnly);
        tsn.setOption(HTTPTransaction::FollowRedirects);
        DWORD res = tsn.perform();
        if (res == ERROR_SUCCESS)
            res = tsn.getContentLength(dwFileSize);

        int error = pimpl_copy->mapToInternalError(res);

        pimpl_copy->notifyQuery(error, (uint64_t)dwFileSize);

        pimpl_copy->m_lock = false;
    }).detach();
}

void CDownloader::downloadFile(const std::wstring &url, const std::wstring &filePath)
{
    if (url.empty())
    {
        pimpl->notifyComplete(CDownloader::InvalidUrl, tstring());
        return;
    }

    if (filePath.empty())
    {
        pimpl->notifyComplete(CDownloader::FileOpError, tstring());
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        pimpl->notifyComplete(CDownloader::Aborted, tstring());
        return;
    }

    pimpl->m_run = true;

    auto pimpl_copy = pimpl;
    std::thread([pimpl_copy, url, filePath]() {
        pimpl_copy->m_prev_percent = -1;
        DWORD res = ERROR_SUCCESS;

        HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE | DELETE, 0, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            HTTPTransaction tsn(url);
            tsn.setFileHandle(hFile);
            tsn.setProgressCallback(
                +[](void* pData, ULONGLONG dltotal, ULONGLONG dlnow) -> DWORD
                {
                    CDownloaderPrivate *pimpl = (CDownloaderPrivate*)pData;
                    if (dltotal > 0)
                    {
                        int percent = static_cast<int>((100.0 * dlnow) / dltotal);
                        if (percent != pimpl->m_prev_percent)
                        {
                            pimpl->notifyProgress(percent);
                            pimpl->m_prev_percent = percent;
                        }
                    }

                    if (pimpl->m_run == false)
                        return ERROR_CANCELLED;

                    return ERROR_SUCCESS;
                });
            tsn.setProgressData(pimpl_copy.get());
            tsn.setOption(HTTPTransaction::Writedata);
            tsn.setOption(HTTPTransaction::FollowRedirects);
            res = tsn.perform();

            if (res != ERROR_SUCCESS)
            {
                FILE_DISPOSITION_INFO fdi = { TRUE };
                SetFileInformationByHandle(hFile, FileDispositionInfo, &fdi, sizeof(fdi));
            }
            CloseHandle(hFile);

        } else res = GetLastError();

        int error = pimpl_copy->mapToInternalError(res);

        pimpl_copy->notifyComplete(error, filePath);

        pimpl_copy->m_lock = false;
    }).detach();
}

void CDownloader::stop()
{
    pimpl->m_run = false;
}

void CDownloader::onQueryResponse(FnVoidIntInt callback)
{
    pimpl->m_query_callback = callback;
}

void CDownloader::onComplete(FnVoidIntStr callback)
{
    pimpl->m_complete_callback = callback;
}

void CDownloader::onProgress(FnVoidInt callback)
{
    pimpl->m_progress_callback = callback;
}
