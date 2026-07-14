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

#import <Foundation/Foundation.h>
#include <stdio.h>

#define MAX_REDIRECTS_ALLOWED 0

typedef NSInteger(*ProgressCallback_t)(void* data, int64_t dltotal, int64_t dlnow);

typedef NS_OPTIONS(NSUInteger, HTTPTransactionOptions) {
    HTTPTransactionHeadOnly        = (1 << 0),
    HTTPTransactionFollowRedirects = (1 << 1),
    HTTPTransactionWritedata       = (1 << 2)
};

@interface HTTPTransaction : NSObject <NSURLSessionDataDelegate>

- (instancetype)initWithUrl:(NSString*)url;

- (void)setUrl:(NSString*)url;
- (void)setOption:(HTTPTransactionOptions)opts;
- (void)setFileHandle:(FILE*)fp;
- (void)setProgressCallback:(ProgressCallback_t)pCallback;
- (void)setProgressData:(void*)pData;

- (NSInteger)getContentLength:(uint64_t*)sizeOut;
- (NSInteger)perform;

@end


@implementation HTTPTransaction
{
    NSString             *sUrl;
    FILE                 *pFile;
    ProgressCallback_t    pProgressCallback;
    void                 *pProgressData;
    HTTPTransactionOptions nOptions;

    int64_t               nContentLength;
    int64_t               nReceived;
    NSInteger             nStatusCode;
    NSInteger             nErrorCode;
    NSInteger             nRedirectCount;
    NSInteger             nMaxRedirects;
    dispatch_semaphore_t  hSemaphore;
}

- (instancetype)init
{
    if (self = [super init]) {
        sUrl = nil;
        pFile = NULL;
        pProgressCallback = NULL;
        pProgressData = NULL;
        nOptions = 0;
        nContentLength = -1;
        nReceived = 0;
    }
    return self;
}

- (instancetype)initWithUrl:(NSString*)url
{
    if (self = [self init]) {
        sUrl = url;
    }
    return self;
}

- (void)setUrl:(NSString*)url
{
    sUrl = url;
}

- (void)setOption:(HTTPTransactionOptions)opts
{
    nOptions |= opts;
}

- (void)setFileHandle:(FILE*)fp
{
    pFile = fp;
}

- (void)setProgressCallback:(ProgressCallback_t)pCallback
{
    pProgressCallback = pCallback;
}

- (void)setProgressData:(void*)pData
{
    pProgressData = pData;
}

- (NSInteger)getContentLength:(uint64_t*)sizeOut
{
    if (nContentLength < 0) {
        *sizeOut = 0;
        return NSURLErrorBadServerResponse;
    }
    *sizeOut = (uint64_t)nContentLength;
    return 0;
}

- (NSInteger)perform
{
    @autoreleasepool {
        NSURL *nsurl = sUrl ? [NSURL URLWithString:sUrl] : nil;
        if (!nsurl)
            return NSURLErrorBadURL;

        if ((nOptions & HTTPTransactionWritedata) && !pFile)
            return NSURLErrorCannotOpenFile;

        nContentLength = -1;
        nReceived = 0;
        nStatusCode = 0;
        nErrorCode = 0;
        nRedirectCount = 0;
        nMaxRedirects = (nOptions & HTTPTransactionFollowRedirects) ? MAX_REDIRECTS_ALLOWED : 0;
        hSemaphore = dispatch_semaphore_create(0);

        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:nsurl];
        request.HTTPMethod = (nOptions & HTTPTransactionHeadOnly) ? @"HEAD" : @"GET";

        NSURLSessionConfiguration *config = [NSURLSessionConfiguration ephemeralSessionConfiguration];
        NSURLSession *session = [NSURLSession sessionWithConfiguration:config
                                                              delegate:self
                                                         delegateQueue:nil];
        NSURLSessionDataTask *task = [session dataTaskWithRequest:request];
        [task resume];
        dispatch_semaphore_wait(hSemaphore, DISPATCH_TIME_FOREVER);
        [session finishTasksAndInvalidate];

        return nErrorCode;
    }
}

/* NSURLSessionDataDelegate */

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
        willPerformHTTPRedirection:(NSHTTPURLResponse *)response
        newRequest:(NSURLRequest *)request
 completionHandler:(void (^)(NSURLRequest *))completionHandler
{
    if (nRedirectCount < nMaxRedirects) {
        nRedirectCount++;
        completionHandler(request);
        return;
    }

    nStatusCode = response.statusCode;
    completionHandler(nil);
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
didReceiveResponse:(NSURLResponse *)response
 completionHandler:(void (^)(NSURLSessionResponseDisposition))completionHandler
{
    if ([response isKindOfClass:[NSHTTPURLResponse class]])
        nStatusCode = ((NSHTTPURLResponse*)response).statusCode;

    nContentLength = response.expectedContentLength;

    if (nStatusCode < 200 || nStatusCode >= 300) {
        nErrorCode = NSURLErrorBadServerResponse;
        completionHandler(NSURLSessionResponseCancel);
        return;
    }
    completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session
          dataTask:(NSURLSessionDataTask *)dataTask
    didReceiveData:(NSData *)data
{
    if (pFile && (nOptions & HTTPTransactionWritedata)) {
        __block BOOL write_ok = YES;
        [data enumerateByteRangesUsingBlock:^(const void *bytes, NSRange byteRange, BOOL *stop) {
            if (fwrite(bytes, 1, byteRange.length, self->pFile) != byteRange.length) {
                write_ok = NO;
                *stop = YES;
            }
        }];
        if (!write_ok) {
            nErrorCode = NSURLErrorCannotWriteToFile;
            [dataTask cancel];
            return;
        }
    }

    nReceived += (int64_t)data.length;

    if (pProgressCallback) {
        NSInteger result = pProgressCallback(pProgressData, nContentLength, nReceived);
        if (result != 0) {
            nErrorCode = result;
            [dataTask cancel];
        }
    }
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error
{
    if (nErrorCode == 0 && error)
        nErrorCode = error.code;

    dispatch_semaphore_signal(hSemaphore);
}

@end
