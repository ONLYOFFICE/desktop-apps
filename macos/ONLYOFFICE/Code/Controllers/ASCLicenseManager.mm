/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
//  ASCLicenseManager.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 25.02.16.
//  Copyright © 2016 Ascensio System SIA. All rights reserved.
//

#import "ASCLicenseManager.h"
#import "applicationmanager.h"
#import "mac_application.h"
#import "ASCHelper.h"
#import "NSString+OnlyOffice.h"
#import "ASCConstants.h"

@implementation ASCLicenseInfo

- (instancetype)initWithDictionary:(NSDictionary *)dictionary {
    if (dictionary) {        
        ASCVersionType licenseType = (ASCVersionType)[[NSUserDefaults standardUserDefaults] integerForKey:@"hasVersionMode"];
        
        self.path           = dictionary[@"path"];
        self.productId      = [dictionary[@"product"] integerValue];
        self.daysLeft       = [dictionary[@"daysLeft"] integerValue];
        self.daysBetween    = [dictionary[@"daysBetween"] integerValue];
        self.exist          = [dictionary[@"licence"] boolValue];
        self.demo           = [dictionary[@"demo"] boolValue];
        self.free           = [dictionary[@"free"] boolValue] && (licenseType == ASCVersionTypeForHome);
        self.serverError    = [dictionary[@"serverUnavailable"] boolValue];
        self.ending         = self.daysLeft < 31;
        self.business       = self.exist && !self.free && !self.demo;
    }
    
    return self;
}

@end


@implementation ASCLicenseManager

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.licence = [ASCLicenseInfo new];
    }
    return self;
}

+ (instancetype)sharedInstance
{
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    
    return sharedInstance;
}

- (void)readLicense {
    NSString * licenseDirectory = [ASCHelper licensePath];
    
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    
    ASCVersionType licenseType = (ASCVersionType)[[NSUserDefaults standardUserDefaults] integerForKey:@"hasVersionMode"];
    
    // Checking license
    NSEditorApi::CAscLicenceActual * licenceData = new NSEditorApi::CAscLicenceActual();
    licenceData->put_Path([licenseDirectory stdwstring]);
#ifdef _PRODUCT_IVOLGA
    licenceData->put_ProductId(PRODUCT_ID_IVOLGAPRO);
#else
    licenceData->put_ProductId(PRODUCT_ID_ONLYOFFICE);
#endif
    
    ADDREFINTERFACE(licenceData);
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL);
    pEvent->m_pData = licenceData;
    
    appManager->Apply(pEvent);
    
    NSEditorApi::CAscLicenceActual * resultLicenceData = licenceData;
    
    if (resultLicenceData) {
        self.licence.path           = [NSString stringWithstdwstring:resultLicenceData->get_Path()];
        self.licence.productId      = resultLicenceData->get_ProductId();
        self.licence.daysLeft       = resultLicenceData->get_DaysLeft();
        self.licence.daysBetween    = resultLicenceData->get_DaysBetween();
        self.licence.exist          = resultLicenceData->get_Licence();
        self.licence.demo           = resultLicenceData->get_IsDemo();
        self.licence.serverError    = resultLicenceData->get_IsServerUnavailable();
        self.licence.free           = (resultLicenceData->get_IsFree() && licenseType == ASCVersionTypeForHome);
        self.licence.ending         = self.licence.daysLeft < 31;
        self.licence.business       = self.licence.exist && !self.licence.free && !self.licence.demo;
    }
}

- (void)createLicense:(int)type {
    NSString * licenseDirectory = [ASCHelper licensePath];
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSEditorApi::CAscLicenceActual * generateLicenceData = new NSEditorApi::CAscLicenceActual();
    
    generateLicenceData->put_Path([licenseDirectory stdwstring]);
#ifdef _PRODUCT_IVOLGA
    generateLicenceData->put_ProductId(PRODUCT_ID_IVOLGAPRO);
#else
    generateLicenceData->put_ProductId(PRODUCT_ID_ONLYOFFICE);
#endif
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(type);
    pEvent->m_pData = generateLicenceData;
    
    appManager->Apply(pEvent);
}

- (void)sendKey:(NSString *)key {
    NSString * licenseDirectory = [ASCHelper licensePath];
    
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    
    NSEditorApi::CAscLicenceKey * keyData = new NSEditorApi::CAscLicenceKey();
    keyData->put_Path([licenseDirectory stdwstring]);
#ifdef _PRODUCT_IVOLGA
    keyData->put_ProductId(PRODUCT_ID_IVOLGAPRO);
#else
    keyData->put_ProductId(PRODUCT_ID_ONLYOFFICE);
#endif
    keyData->put_Key([key stdstring]);
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_SEND_KEY);
    pEvent->m_pData = keyData;
    
    appManager->Apply(pEvent);
}

@end
