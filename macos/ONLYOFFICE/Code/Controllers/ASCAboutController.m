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
//  ASCAboutController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 18.02.16.
//  Copyright © 2016 Ascensio System SIA. All rights reserved.
//

#import "ASCAboutController.h"
#import "ASCConstants.h"

@interface ASCAboutController ()
@property (weak) IBOutlet NSTextField *appNameText;
@property (weak) IBOutlet NSTextField *versionText;
@property (weak) IBOutlet NSTextField *copyrightText;
@property (weak) IBOutlet NSButton *licenseButton;
@end

@implementation ASCAboutController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSDictionary * localizedInfoDictionary = [[NSBundle mainBundle] localizedInfoDictionary];
    
    NSString * locProductName   = localizedInfoDictionary[@"CFBundleName"];
    NSString * locCopyright     = localizedInfoDictionary[@"NSHumanReadableCopyright"];

    locProductName  = locProductName ? locProductName : infoDictionary[@"CFBundleName"];
    locCopyright    = locCopyright ? locCopyright : infoDictionary[@"NSHumanReadableCopyright"];
    
    // Setup license button view
    NSMutableAttributedString * attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[self.licenseButton attributedTitle]];
    long len = [attrTitle length];
    NSRange range = NSMakeRange(0, len);
    [attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor keyboardFocusIndicatorColor] range:range];
    [attrTitle fixAttributesInRange:range];
    [self.licenseButton setAttributedTitle:attrTitle];

    // Product name
#ifndef _MAS
    ASCVersionType versionType = (ASCVersionType)[[NSUserDefaults standardUserDefaults] integerForKey:@"hasVersionMode"];
    
    if (ASCVersionTypeForBusiness == versionType) {
        locProductName = [NSString stringWithFormat:@"%@ %@", locProductName, NSLocalizedString(@"for Business", nil)];
    } else if (ASCVersionTypeForHome == versionType) {
        locProductName = [NSString stringWithFormat:@"%@ %@", locProductName, NSLocalizedString(@"for Home", nil)];
    }
#else
    [self.licenseButton setHidden:YES];
#endif
    
    [self.appNameText setStringValue:locProductName];

    // Version
    [self.versionText setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Version %@ (%@)", nil), [infoDictionary objectForKey:@"CFBundleShortVersionString"], [infoDictionary objectForKey:@"CFBundleVersion"]]];
    
    // Copyright
    [self.copyrightText setStringValue:locCopyright];
    
    // Window
    [self setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), locProductName]];
}

- (void)viewDidAppear {
    [super viewDidAppear];
    
    [self.view.window setStyleMask:[self.view.window styleMask] & ~NSResizableWindowMask];
}

- (void)viewDidDisappear {
    [super viewDidDisappear];
    
    [NSApp stopModal];
}

@end
