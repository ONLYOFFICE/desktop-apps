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
#include "private/httptransaction_mac_p.h"
#include <thread>
#include <atomic>


class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    int mapToInternalError(int errorCode)
    {
        switch (errorCode) {
        case 0:
            return CDownloader::Success;

        case NSURLErrorCancelled:
            return CDownloader::Aborted;

        case NSURLErrorBadURL:
        case NSURLErrorUnsupportedURL:
            return CDownloader::InvalidUrl;

        case NSURLErrorCannotCreateFile:
        case NSURLErrorCannotOpenFile:
        case NSURLErrorCannotWriteToFile:
            return CDownloader::FileOpError;

        case NSURLErrorTimedOut:
        case NSURLErrorCannotFindHost:
        case NSURLErrorCannotConnectToHost:
        case NSURLErrorNetworkConnectionLost:
        case NSURLErrorDNSLookupFailed:
        case NSURLErrorHTTPTooManyRedirects:
        case NSURLErrorResourceUnavailable:
        case NSURLErrorNotConnectedToInternet:
        case NSURLErrorRedirectToNonExistentLocation:
        case NSURLErrorBadServerResponse:
        case NSURLErrorFileDoesNotExist:
        case NSURLErrorUserCancelledAuthentication:
        case NSURLErrorUserAuthenticationRequired:
        case NSURLErrorZeroByteResource:
        case NSURLErrorCannotDecodeRawData:
        case NSURLErrorCannotDecodeContentData:
        case NSURLErrorCannotParseResponse:
        case NSURLErrorInternationalRoamingOff:
        case NSURLErrorCallIsActive:
        case NSURLErrorDataNotAllowed:
        case NSURLErrorSecureConnectionFailed:
        case NSURLErrorServerCertificateHasBadDate:
        case NSURLErrorServerCertificateUntrusted:
        case NSURLErrorServerCertificateHasUnknownRoot:
        case NSURLErrorServerCertificateNotYetValid:
        case NSURLErrorClientCertificateRejected:
        case NSURLErrorClientCertificateRequired:
            return CDownloader::NetworkError;

        case ENOMEM:
            return CDownloader::OutOfMemory;

        default:
            return CDownloader::UnknownError;
        }
    }

    FnVoidIntInt  m_query_callback = nullptr;
    FnVoidIntStr  m_complete_callback = nullptr;
    FnVoidInt     m_progress_callback = nullptr;

    int m_prev_percent = -1;

    std::atomic_bool  m_run{true},
                      m_lock{false};
};


CDownloader::CDownloader() :
    pimpl(std::make_shared<CDownloaderPrivate>())
{}

CDownloader::~CDownloader()
{
    pimpl->m_run = false;
}

bool CDownloader::isUrlAccessible(const tstring &url)
{
    if (url.empty())
        return false;

    @autoreleasepool {
        HTTPTransaction *tsn = [[HTTPTransaction alloc]
                                initWithUrl:[NSString stringWithUTF8String:url.c_str()]];
        [tsn setOption:HTTPTransactionHeadOnly];
        [tsn setOption:HTTPTransactionFollowRedirects];
        if ([tsn perform] != 0)
            return false;

        return true;
    }
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

    auto pimpl_copy = pimpl;
    std::thread([pimpl_copy, url]() {
        @autoreleasepool {
            uint64_t fileSize = 0;

            HTTPTransaction *tsn = [[HTTPTransaction alloc]
                                    initWithUrl:[NSString stringWithUTF8String:url.c_str()]];
            [tsn setOption:HTTPTransactionHeadOnly];
            [tsn setOption:HTTPTransactionFollowRedirects];
            NSInteger res = [tsn perform];
            if (res == 0)
                res = [tsn getContentLength:&fileSize];

            int error = pimpl_copy->mapToInternalError((int)res);

            if (pimpl_copy->m_run && pimpl_copy->m_query_callback)
                pimpl_copy->m_query_callback(error, fileSize);

            pimpl_copy->m_lock = false;
        }
    }).detach();
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

    auto pimpl_copy = pimpl;
    std::thread([pimpl_copy, url, filePath]() {
        @autoreleasepool {
            pimpl_copy->m_prev_percent = -1;
            NSInteger res = 0;

            if (FILE *fp = fopen(filePath.c_str(), "wb")) {
                HTTPTransaction *tsn = [[HTTPTransaction alloc]
                                        initWithUrl:[NSString stringWithUTF8String:url.c_str()]];
                [tsn setFileHandle:fp];
                [tsn setProgressCallback:
                    +[](void* pData, int64_t dltotal, int64_t dlnow) -> NSInteger
                    {
                        CDownloaderPrivate *pimpl = (CDownloaderPrivate*)pData;
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
                            return NSURLErrorCancelled;

                        return 0;
                    }];
                [tsn setProgressData:pimpl_copy.get()];
                [tsn setOption:HTTPTransactionWritedata];
                [tsn setOption:HTTPTransactionFollowRedirects];
                res = [tsn perform];

                fclose(fp);
                if (res != 0)
                    remove(filePath.c_str());

            } else
                res = NSURLErrorCannotOpenFile;

            int error = pimpl_copy->mapToInternalError((int)res);

            if (pimpl_copy->m_run && pimpl_copy->m_complete_callback)
                pimpl_copy->m_complete_callback(error, filePath);

            pimpl_copy->m_lock = false;
        }
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
