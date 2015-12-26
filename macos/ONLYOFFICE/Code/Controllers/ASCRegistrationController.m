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
//  ASCRegistrationController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/26/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCRegistrationController.h"
#import "ASCConstants.h"

@interface ASCRegistrationController ()
@property (nonatomic) NSTabViewController * tabViewController;
@property (weak) IBOutlet NSTextField *keyField;
@end

@implementation ASCRegistrationController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(nullable id)sender {
    self.tabViewController = sender;
}

#pragma mark -
#pragma mark Actions

- (IBAction)onSuccessClose:(NSButton *)sender {
    [self.view.window close];
}

- (IBAction)onHelpClick:(NSButton *)sender {
    NSString * langCode = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];
    NSString * helpUrl  = [NSString stringWithFormat:kRegHelpUrl, @""];
    
    if ([@"ru" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegHelpUrl, @"ru/"];
    } else if ([@"de" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegHelpUrl, @"de/"];
    } else if ([@"fr" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegHelpUrl, @"fr/"];
    } else if ([@"es" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegHelpUrl, @"es/"];
    }
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:helpUrl]];
}

- (IBAction)onBuyOnlineClick:(NSButton *)sender {
    NSString * langCode = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];
    NSString * helpUrl  = [NSString stringWithFormat:kRegBuyUrl, @""];
    
    if ([@"ru" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegBuyUrl, @"ru/"];
    } else if ([@"de" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegBuyUrl, @"de/"];
    } else if ([@"fr" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegBuyUrl, @"fr/"];
    } else if ([@"es" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:kRegBuyUrl, @"es/"];
    }
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:helpUrl]];
}

- (IBAction)onRegistrationClick:(NSButton *)sender {
    [self.tabViewController setSelectedTabViewItemIndex:1];
}

@end
