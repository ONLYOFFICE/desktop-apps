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

//
//  ASCAddonManager.mm
//  ONLYOFFICE
//
//  Created by Ascensio System SIA
//  Copyright © 2026 Ascensio System SIA. All rights reserved.
//

#ifdef USE_ADDON_MODE

#import "ASCAddonManager.h"
#import "ASCApplicationManager.h"
#import "NSString+Extensions.h"
#import "ASCConstants.h"
#import "ASCHelper.h"
#include "../../../../../common/sources/cdownloader.h"

#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES @""
#endif
#ifndef URL_APPCAST_DEV_CHANNEL
# define URL_APPCAST_DEV_CHANNEL @""
#endif

NSNotificationName const ASCAddonManagerProgressChangedNotification =
    @"ASCAddonManagerProgressChangedNotification";
NSNotificationName const ASCAddonManagerInstallFinishedNotification =
    @"ASCAddonManagerInstallFinishedNotification";
NSNotificationName const ASCAddonManagerUninstallFinishedNotification =
    @"ASCAddonManagerUninstallFinishedNotification";

typedef NS_ENUM(NSInteger, ASCAddonDownloadMode) {
    ASCAddonDownloadMetadata,
    ASCAddonDownloadPackage
};

@interface ASCAddonManager ()
@property (nonatomic, assign) CDownloader *pDownloader;
@property (nonatomic, strong) NSString *tmpPath;
@property (nonatomic, strong) NSString *metadataUrl;
@property (nonatomic, assign) ASCAddonDownloadMode downloadMode;
@property (nonatomic, strong) NSString *packageHash;
@property (nonatomic, strong) NSString *packageVersion;
@property (nonatomic, assign) BOOL installing;

// Set after an install/remove in this session: the addon takes effect only
// after an application restart (plugins are read at startup).
@property (nonatomic, assign) BOOL restartRequired;
@end

@implementation ASCAddonManager

+ (instancetype)sharedManager {
    static ASCAddonManager *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _installing = NO;
        _restartRequired = NO;
        _downloadMode = ASCAddonDownloadMetadata;
        _metadataUrl = [ASCHelper processArgsContain:@"--appcast-dev-channel"]
                           ? URL_APPCAST_DEV_CHANNEL
                           : URL_APPCAST_UPDATES;
    }
    return self;
}

- (void)dealloc {
    [self resetDownloader];
}

- (BOOL)isInstalled {
    CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
    return appManager->CheckMainAddon();
}

- (BOOL)restartRequired {
    return self.restartRequired;
}

- (void)doAction:(ASCAddonManagerAction)action {
    if (self.installing)
        return;

    if (action == ASCAddonManagerActionInstall) {
        if (![self isInstalled])
            [self install];
    } else if (action == ASCAddonManagerActionUninstall) {
        if ([self isInstalled])
            [self uninstall];
    } else if (action == ASCAddonManagerActionStop) {
        [self resetDownloader];
    }
}

+ (NSString *)errorMessageForResult:(ASCAddonManagerResult)result {
    switch (result) {
    case ASCAddonManagerResultAddonVersionMismatch:
        return NSLocalizedString(@"No compatible add-on version is available for this version of the application.", nil);
    case ASCAddonManagerResultAppVersionTooOld:
        return NSLocalizedString(@"To install the add-on, update the application to the latest version.", nil);
    case ASCAddonManagerResultExtractionFailed:
        return NSLocalizedString(@"An error occurred while unpacking the archive", nil);
    case ASCAddonManagerResultInvalidMetadata:
        return NSLocalizedString(@"The add-on information is invalid or incomplete.", nil);
    case ASCAddonManagerResultMd5Mismatch:
        return NSLocalizedString(@"The add-on package is corrupted or has been modified.", nil);
    case ASCAddonManagerResultDownloadOutOfMemory:
        return NSLocalizedString(@"Not enough memory to complete the add-on download.", nil);
    case ASCAddonManagerResultDownloadNetworkError:
        return NSLocalizedString(@"Failed to download the add-on due to a network error.", nil);
    case ASCAddonManagerResultDownloadInvalidUrl:
        return NSLocalizedString(@"The add-on download URL is invalid.", nil);
    case ASCAddonManagerResultDownloadFileOpError:
        return NSLocalizedString(@"Failed to save the downloaded add-on package.", nil);
    case ASCAddonManagerResultDownloadUnknownError:
    default:
        return NSLocalizedString(@"An unexpected error occurred while installing the add-on.", nil);
    }
}

#pragma mark - Installation Methods

- (void)install {
    self.downloadMode = ASCAddonDownloadMetadata;
    self.packageHash = nil;
    self.packageVersion = nil;
    self.tmpPath = [ASCHelper generateTmpFileName:@".json"];

    self.installing = YES;
    [self postProgressChanged:0];

    [self startDownload:self.metadataUrl filePath:self.tmpPath];
}

- (void)uninstall {
    CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
    appManager->RemoveMainAddon();
    
    // Like install, removal takes effect only after an application restart.
    self.restartRequired = YES;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCAddonManagerUninstallFinishedNotification
                                                        object:self
                                                      userInfo:nil];
}

- (void)finishInstall:(NSInteger)result {
    self.installing = NO;

    if (result == ASCAddonManagerResultSuccess) {
        CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
        int res = appManager->InstallMainAddon([self.tmpPath stdwstring]);
        self.restartRequired = (res == 0);

        if (!self.restartRequired)
            result = ASCAddonManagerResultExtractionFailed;
    }

    if (self.tmpPath.length > 0)
        [[NSFileManager defaultManager] removeItemAtPath:self.tmpPath error:nil];
    self.tmpPath = nil;

    [[NSNotificationCenter defaultCenter] postNotificationName:ASCAddonManagerInstallFinishedNotification
                                                        object:self
                                                      userInfo:@{@"result": @(result)}];
}

- (void)startDownload:(NSString *)url filePath:(NSString *)filePath {
    [self resetDownloader];

    // CDownloader writes directly to the target path and reports through its
    // own worker thread, so the callbacks are marshaled back to the main thread.
    self.pDownloader = new CDownloader;

    __weak ASCAddonManager *weakSelf = self;
    self.pDownloader->onProgress(
        [weakSelf](int percent)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [weakSelf onProgress:percent];
            });
        });

    self.pDownloader->onComplete(
        [weakSelf](int error, const tstring&)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [weakSelf onComplete:error];
            });
        });

    self.pDownloader->downloadFile([url UTF8String], [filePath UTF8String]);
}

- (void)resetDownloader {
    if (self.pDownloader) {
        self.pDownloader->stop();
        delete self.pDownloader;
        self.pDownloader = nullptr;
    }
}

#pragma mark - Download Callbacks

// Invoked (dispatched) from the downloader's worker thread onto the main thread.
- (void)onProgress:(int)percent {
    if (self.downloadMode == ASCAddonDownloadMetadata)
        return;

    [self postProgressChanged:percent];
}

- (void)onComplete:(int)error {
    // The downloader is not deleted here (it is still unwinding its worker thread);
    // it is torn down on the next install call or in dealloc.
    if (error == CDownloader::Success) {
        switch (self.downloadMode) {
        case ASCAddonDownloadMetadata:
        {
            [self onLoadMetadataFinished];
            break;
        }
        case ASCAddonDownloadPackage:
        {
            NSString *hash = [ASCHelper fileMd5Hash:self.tmpPath];
            if (self.tmpPath.length > 0 && [hash isEqualToString:self.packageHash]) {
                [self finishInstall:ASCAddonManagerResultSuccess];
            } else {
                [self finishInstall:ASCAddonManagerResultMd5Mismatch];
            }
            break;
        }
        default:
            break;
        }
    } else {
        NSInteger result = 0;
        switch (error) {
        case CDownloader::Aborted:
            result = ASCAddonManagerResultDownloadAborted;
            break;
        case CDownloader::OutOfMemory:
            result = ASCAddonManagerResultDownloadOutOfMemory;
            break;
        case CDownloader::NetworkError:
            result = ASCAddonManagerResultDownloadNetworkError;
            break;
        case CDownloader::InvalidUrl:
            result = ASCAddonManagerResultDownloadInvalidUrl;
            break;
        case CDownloader::FileOpError:
            result = ASCAddonManagerResultDownloadFileOpError;
            break;
        case CDownloader::UnknownError:
        default:
            result = ASCAddonManagerResultDownloadUnknownError;
            break;
        }

        [self finishInstall:result];
    }
}

#pragma mark - Metadata Handling

- (BOOL)isNewerVersion:(NSString *)currentVer than:(NSString *)targetVer {
    NSArray<NSString *> *oldVer = [currentVer componentsSeparatedByString:@"."];
    NSArray<NSString *> *newVer = [targetVer componentsSeparatedByString:@"."];
    NSUInteger size = MAX(newVer.count, oldVer.count);

    for (NSUInteger i = 0; i < size; i++) {
        const NSInteger oldPart = i < oldVer.count ? [oldVer[i] integerValue] : 0;
        const NSInteger newPart = i < newVer.count ? [newVer[i] integerValue] : 0;

        if (newPart > oldPart)
            return YES;

        if (newPart < oldPart)
            return NO;
    }

    return NO;
}

- (void)onLoadMetadataFinished {
    NSDictionary *root = [ASCHelper parseJsonFile:self.tmpPath];
    do {
        if (root.count == 0)
            break;

        NSDictionary *addon = root[@"addon"];
        if (![addon isKindOfClass:[NSDictionary class]] || addon.count == 0)
            break;

        NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
        NSString *currVersion = [NSString stringWithFormat:@"%@.%@",
                                 [infoDictionary objectForKey:@"CFBundleShortVersionString"],
                                 [infoDictionary objectForKey:@"ASCBundleBuildNumber"]];
        NSString *version = addon[@"version"];
        if (![version isKindOfClass:[NSString class]] || version.length == 0)
            break;

        NSDictionary *package = addon[@"package"];
        if (![package isKindOfClass:[NSDictionary class]] || package.count == 0)
            break;

#if defined(__aarch64__) || defined(__arm64__)
        NSDictionary *platform = package[@"macos_arm"];
#elif defined(__x86_64__)
        NSDictionary *platform = nil;
        if (@available(macOS 10.13, *)) {
            platform = package[@"macos_x86_64"];
        } else {
            platform = package[@"macos_v8"];
        }
#else
# error Unsupported architecture
#endif
        if (![platform isKindOfClass:[NSDictionary class]] || platform.count == 0)
            break;

        if ([self isNewerVersion:currVersion than:version]) {
            [self finishInstall:ASCAddonManagerResultAppVersionTooOld];

        } else if ([self isNewerVersion:version than:currVersion]) {
            [self finishInstall:ASCAddonManagerResultAddonVersionMismatch];
            
        } else {
            NSString *url = [platform[@"url"] isKindOfClass:[NSString class]] ? platform[@"url"] : @"";
            NSString *url2 = [platform[@"url2"] isKindOfClass:[NSString class]] ? platform[@"url2"] : @"";
            if (url.length == 0 && url2.length == 0)
                break;

            NSLog(@"Primary package URL: %@\nSecondary package URL: %@", url, url2);
            BOOL primary_bad = (url.length == 0 || !self.pDownloader->isUrlAccessible([url UTF8String]));
            BOOL prefer_secondary = [ASCHelper processArgsContain:@"--prefer-alternate-url"];
            NSString *packageUrl = (url2.length > 0 && (primary_bad || prefer_secondary)) ? url2 : url;
            NSLog(@"Download from URL:\n%@", packageUrl);

            NSString *hash = [platform[@"md5"] isKindOfClass:[NSString class]] ? platform[@"md5"] : @"";

            self.packageVersion = version;
            self.packageHash = [hash lowercaseString];

            [[NSFileManager defaultManager] removeItemAtPath:self.tmpPath error:nil];
            self.downloadMode = ASCAddonDownloadPackage;
            self.tmpPath = [ASCHelper generateTmpFileName:@".zip"];
            [self startDownload:packageUrl filePath:self.tmpPath];
        }

        return;
    } while (false);

    [self finishInstall:ASCAddonManagerResultInvalidMetadata];
}

#pragma mark - Helper Methods

- (void)postProgressChanged:(int)progress {
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCAddonManagerProgressChangedNotification
                                                        object:self
                                                      userInfo:@{@"progress": @(progress)}];
}

@end

#endif
