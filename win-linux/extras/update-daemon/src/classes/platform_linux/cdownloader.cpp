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

#include "classes/cdownloader.h"
#include <curl/curl.h>
#include <future>

#define MAX_REDIRECTS_ALLOWED 0L


class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    int mapToInternalError(CURLcode res)
    {
        switch (res)
        {
        case CURLE_OK:
            return CDownloader::Success;

        case CURLE_ABORTED_BY_CALLBACK:
            return CDownloader::Aborted;

        case CURLE_OUT_OF_MEMORY:
        case CURLE_FAILED_INIT:
            return CDownloader::OutOfMemory;

        case CURLE_URL_MALFORMAT:
        case CURLE_UNSUPPORTED_PROTOCOL:
            return CDownloader::InvalidUrl;

        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_WEIRD_SERVER_REPLY:
        case CURLE_REMOTE_ACCESS_DENIED:
        case CURLE_GOT_NOTHING:
        case CURLE_LOGIN_DENIED:
        case CURLE_AUTH_ERROR:
        case CURLE_HTTP_RETURNED_ERROR:
        case CURLE_TOO_MANY_REDIRECTS:
        case CURLE_OPERATION_TIMEDOUT:
        case CURLE_SEND_ERROR:
        case CURLE_RECV_ERROR:
        case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SSL_CERTPROBLEM:
        case CURLE_SSL_CIPHER:
        case CURLE_SSL_CACERT_BADFILE:
        case CURLE_SSL_INVALIDCERTSTATUS:
        case CURLE_PEER_FAILED_VERIFICATION:
            return CDownloader::NetworkError;

        case CURLE_WRITE_ERROR:
            return CDownloader::FileOpError;

        default:
            return CDownloader::UnknownError;
        }
    }

    FnVoidIntInt  m_query_callback = nullptr;
    FnVoidIntStr  m_complete_callback = nullptr;
    FnVoidInt     m_progress_callback = nullptr;

    int m_prev_percent = -1;

    std::future<void> m_future;
    std::atomic_bool  m_run{true},
                      m_lock{false};
};


CDownloader::CDownloader() :
    pimpl(new CDownloaderPrivate)
{}

CDownloader::~CDownloader()
{
    pimpl->m_run = false;
    if (pimpl->m_future.valid())
        pimpl->m_future.wait();

    delete pimpl; pimpl = nullptr;
}

bool CDownloader::isUrlAccessible(const tstring &url)
{
    if (url.empty())
        return false;

    CURLcode res = CURLE_FAILED_INIT;
    if (CURL *curl = curl_easy_init()) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS_ALLOWED);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return res == CURLE_OK;
}

void CDownloader::queryContentLength(const tstring &url)
{
    if (url.empty())
    {
        if (pimpl->m_query_callback)
            pimpl->m_query_callback(CDownloader::InvalidUrl, 0);
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        if (pimpl->m_query_callback)
            pimpl->m_query_callback(CDownloader::Aborted, 0);
        return;
    }

    pimpl->m_future = std::async(std::launch::async, [=]() {
        curl_off_t fileSize = 0;
        CURLcode res = CURLE_FAILED_INIT;
        if (CURL *curl = curl_easy_init()) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS_ALLOWED);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
                if (fileSize < 0) fileSize = 0;
            }
            curl_easy_cleanup(curl);
        }
        int error = pimpl->mapToInternalError(res);

        if (pimpl->m_query_callback)
            pimpl->m_query_callback(error, (uint64_t)fileSize);

        pimpl->m_lock = false;
    });
}

void CDownloader::downloadFile(const tstring &url, const tstring &filePath)
{
    if (url.empty())
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(CDownloader::InvalidUrl, tstring());
        return;
    }

    if (filePath.empty())
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(CDownloader::FileOpError, tstring());
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(CDownloader::Aborted, tstring());
        return;
    }

    pimpl->m_run = true;

    pimpl->m_future = std::async(std::launch::async, [=]() {
        CURLcode res = CURLE_FAILED_INIT;
        if (FILE *fp = fopen(filePath.c_str(), "wb")) {
            if (CURL *curl = curl_easy_init()) {
                pimpl->m_prev_percent = -1;
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS_ALLOWED);
                curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                    +[](void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) -> int
                    {
                        CDownloaderPrivate *pimpl = (CDownloaderPrivate*)user_data;
                        if (dltotal > 0 && pimpl->m_progress_callback)
                        {
                            int percent = static_cast<int>((100.0 * dlnow) / dltotal);
                            if (percent != pimpl->m_prev_percent)
                            {
                                pimpl->m_progress_callback(percent);
                                pimpl->m_prev_percent = percent;
                            }
                        }

                        if (pimpl->m_run == false)
                            return CURLE_ABORTED_BY_CALLBACK;

                        return CURLE_OK;
                    });
                curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)pimpl);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
            }

            fclose(fp);
            if (res != CURLE_OK)
                remove(filePath.c_str());

        } else
            res = CURLE_WRITE_ERROR;

        int error = pimpl->mapToInternalError(res);

        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(error, filePath);

        pimpl->m_lock = false;
    });
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
