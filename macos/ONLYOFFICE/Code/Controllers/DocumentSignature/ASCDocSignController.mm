/*
 * (c) Copyright Ascensio System SIA 2010-2018
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

//
//  ASCDocSignController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 17/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "ASCDocSignController.h"
#import "NSString+Extensions.h"
#import "mac_application.h"
#import "ASCCertificateSelectDialogOpenSsl.hpp"

ASCCertificateSelectDialogOpenSsl *internalDialog = NULL;

@implementation ASCDocSignController

#pragma mark - Properties

- (void)setSignFilePath:(NSString *)signFilePath {
    _signFilePath = signFilePath;

    if (internalDialog) {
        internalDialog->m_sCertPath = [signFilePath stdwstring];
    }
}

- (void)setSignPassword:(NSString *)signPassword {
    _signPassword = signPassword;

    if (internalDialog) {
        internalDialog->m_sCertPassword = [signPassword stdwstring];
    }
}

- (void)setPrivateKeyFilePath:(NSString *)privateKeyFilePath {
    _privateKeyFilePath = privateKeyFilePath;

    if (internalDialog) {
        internalDialog->m_sKeyPath = [privateKeyFilePath stdwstring];
    }
}

- (void)setPrivateKeyPassword:(NSString *)privateKeyPassword {
    _privateKeyPassword = privateKeyPassword;

    if (internalDialog) {
        internalDialog->m_sKeyPassword = [privateKeyPassword stdwstring];
    }
}

#pragma mark - Life Cycle Methods

+ (instancetype)shared {
    static ASCDocSignController * sharedManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedManager = [[self alloc] init];
        [sharedManager initialize];
    });
    return sharedManager;
}

- (void)dealloc {
    if (NULL != internalDialog) {
        delete internalDialog;
    }
}

- (void)initialize {
    self.signFilePath = @"";
    self.signPassword = @"";
    self.privateKeyFilePath = @"";
    self.privateKeyPassword = @"";

    if (CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager]) {
        if (NULL == internalDialog) {
            internalDialog = new ASCCertificateSelectDialogOpenSsl();
            appManager->OpenSsl_SetDialog(internalDialog);
        }
    }
}

+ (void)startWizard {
    if (ASCDocSignController * signController = [ASCDocSignController shared]) {
        signController.signFilePath = @"";
        signController.signPassword = @"";
        signController.privateKeyFilePath = @"";
        signController.privateKeyPassword = @"";
    }

    NSStoryboard * storyboard = [NSStoryboard storyboardWithName:@"Document-Sign" bundle:[NSBundle mainBundle]];

    if (storyboard) {
        NSWindowController * windowController = [storyboard instantiateControllerWithIdentifier:@"DocSignWindowController"];
        NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];

        [mainWindow beginSheet:[windowController window] completionHandler:^(NSModalResponse returnCode) {
            //
        }];
    }
}

@end
