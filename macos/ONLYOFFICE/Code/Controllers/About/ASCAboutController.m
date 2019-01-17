/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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
//  Copyright © 2017 Ascensio System SIA. All rights reserved.
//

#import "ASCAboutController.h"
#import <WebKit/WebKit.h>
#import "ASCConstants.h"
#import "ASCExternalController.h"

@interface ASCAboutController ()
@property (weak) IBOutlet NSTextField *appNameText;
@property (weak) IBOutlet NSTextField *versionText;
@property (weak) IBOutlet NSTextField *copyrightText;
@property (weak) IBOutlet NSButton *licenseButton;
@property (weak) IBOutlet WebView *eulaWebView;
@property (weak) IBOutlet NSStackView *infoStackView;
@end

@implementation ASCAboutController

- (void)viewDidLoad {
    [super viewDidLoad];

    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
    
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSDictionary * localizedInfoDictionary = [[NSBundle mainBundle] localizedInfoDictionary];
    
    NSString * locProductName   = [ASCHelper appName];
    NSString * locCopyright     = localizedInfoDictionary[@"NSHumanReadableCopyright"];

    locCopyright = locCopyright ? locCopyright : infoDictionary[@"NSHumanReadableCopyright"];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onCommercialInfo)]) {
        NSString * commercialInfo = [externalDelegate onCommercialInfo];

        if (commercialInfo) {
            NSTextField * commercialTextField;
            if (@available(macOS 10.12, *)) {
                commercialTextField = [NSTextField labelWithString:commercialInfo];
            } else {
                commercialTextField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, self.infoStackView.frame.size.width, 35)];
                [commercialTextField setStringValue:commercialInfo];
                [commercialTextField setFont:[NSFont systemFontOfSize:[NSFont systemFontSize]]];
                [commercialTextField setBezeled:NO];
                [commercialTextField setDrawsBackground:NO];
                [commercialTextField setEditable:NO];
                [commercialTextField setSelectable:NO];
            }
            [commercialTextField setAlignment:NSTextAlignmentCenter];
            [commercialTextField setLineBreakMode:NSLineBreakByWordWrapping];
            [commercialTextField setUsesSingleLineMode:NO];

            [self.infoStackView insertArrangedSubview:commercialTextField atIndex:2];
        }
    }

    // EULA View
    if (self.eulaWebView) {
        NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html"];
        [[self.eulaWebView mainFrame] loadRequest:[NSURLRequest requestWithURL:eulaUrl]];
    } else {
        // About View
        // Setup license button view
        NSMutableAttributedString * attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[self.licenseButton attributedTitle]];
        long len = [attrTitle length];
        NSRange range = NSMakeRange(0, len);
        [attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor linkColor] range:range];
        [attrTitle fixAttributesInRange:range];
        [self.licenseButton setAttributedTitle:attrTitle];
        
#ifdef _MAS
        [self.licenseButton setHidden:YES];
#endif
        
        // Product name
        [self.appNameText setStringValue:locProductName];
        
        // Version
        [self.versionText setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Version %@ (%@)", nil), [infoDictionary objectForKey:@"CFBundleShortVersionString"], [infoDictionary objectForKey:@"CFBundleVersion"]]];
        
        // Copyright
        [self.copyrightText setStringValue:locCopyright];
        
        // Window
        [self setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), locProductName]];
    }
}

- (void)viewDidAppear {
    [super viewDidAppear];
    
    // EULA View
    if (self.eulaWebView) {
        return;
    }
    
    [self.view.window setStyleMask:[self.view.window styleMask] & ~NSResizableWindowMask];
}

- (void)viewDidDisappear {
    [super viewDidDisappear];

    // EULA View
    if (self.eulaWebView) {
        return;
    }
    
    [NSApp stopModal];
}

@end
