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
//  ASCAboutController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 18.02.16.
//  Copyright © 2017 Ascensio System SIA. All rights reserved.
//

#import "ASCAboutController.h"

#import "ASCConstants.h"
#import "ASCExternalController.h"
#import "ASCSharedSettings.h"
#import "ASCLicenseController.h"

static NSString * const kOfficialWebsiteURL = @"https://www.onlyoffice.com";
static NSString * const kBslLicenseURL = @"https://docspace.onlyoffice.com/s/jPfGKDyBYTRK4JM";

@interface ASCAboutController () {
    BOOL isCommercialVersion;
}
@property (weak) IBOutlet NSTextField *appNameText;
@property (weak) IBOutlet NSTextField *versionText;
@property (weak) IBOutlet NSTextField *copyrightText;
@property (weak) IBOutlet NSButton *licenseButton;
@property (weak) IBOutlet NSStackView *infoStackView;
@property (weak) IBOutlet NSStackView *licenseRow;
@property (weak) IBOutlet NSStackView *aiToolsView;
@property (weak) IBOutlet NSBox *aiToolsSeparator;
@property (weak) IBOutlet NSTextField *aiNameText;
@property (weak) IBOutlet NSTextField *aiVersionText;
@property (weak) IBOutlet NSButton *aiLicenseButton;
@property (weak) IBOutlet NSButton *websiteButton;
@end

@implementation ASCAboutController

- (void)viewDidLoad {
    [super viewDidLoad];

    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
    
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSDictionary * localizedInfoDictionary = [[NSBundle mainBundle] localizedInfoDictionary];
    
    NSString * locProductName = [NSString stringWithFormat:@"%@ %@", [ASCHelper appName], @"Desktop"];
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
            [commercialTextField setAlignment:NSTextAlignmentLeft];
            [commercialTextField setLineBreakMode:NSLineBreakByWordWrapping];
            [commercialTextField setUsesSingleLineMode:NO];

            [self.infoStackView insertArrangedSubview:commercialTextField atIndex:2];
        }
    }

    NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html" subdirectory:@"license"];
    isCommercialVersion = eulaUrl != nil;

        // About View
        // Setup link styled buttons
        [self applyLinkStyleToButton:self.licenseButton];
        [self applyLinkStyleToButton:self.aiLicenseButton];
        [self applyLinkStyleToButton:self.websiteButton];
        
#ifdef _MAS
        [self.licenseRow setHidden:YES];
#endif
        
        // Product name
        [self.appNameText setStringValue:locProductName];
        
        // Version
        [self.versionText setStringValue:[NSString stringWithFormat:
                                              [self appEdition],
                                              [self shortAppVersion]]];

        NSClickGestureRecognizer *click = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(onVersionClick:)];
        [self.versionText addGestureRecognizer:click];
        
        // If has extra features
        if ([[[ASCSharedSettings sharedInstance] settingByKey:kSettingsHasExtraFeatures] boolValue]) {
            [self.versionText setStringValue:[NSString stringWithFormat:@"%@\n%@",
                                              self.versionText.stringValue,
                                              NSLocalizedString(@"With access to pro features", nil)]];
        }
        
        // Copyright
        [self.copyrightText setStringValue:locCopyright];
        
        // AI Tools block
        if (_aiToolsVersion) {
            [self applyAiToolsVersion];
        }
        [self applyAiToolsVisibility];

        // Window
        [self setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), locProductName]];
}

- (void)viewWillAppear {
    [super viewWillAppear];

    [self adjustWindowHeight];
}

- (void)viewDidAppear {
    [super viewDidAppear];
        
    [self.view.window setStyleMask:[self.view.window styleMask] & ~NSResizableWindowMask];
}

- (void)viewDidDisappear {
    [super viewDidDisappear];
    
    [NSApp stopModal];
}

#pragma mark - AI Tools block visibility

- (void)setAiToolsHidden:(BOOL)aiToolsHidden {
    if (_aiToolsHidden == aiToolsHidden) {
        return;
    }

    _aiToolsHidden = aiToolsHidden;

    if ([self isViewLoaded]) {
        [self applyAiToolsVisibility];
    }
}

- (void)setAiToolsVersion:(NSString *)version {
    _aiToolsVersion = [version copy];

    if ([self isViewLoaded]) {
        [self applyAiToolsVersion];
    }
}

- (void)applyAiToolsVisibility {
    [self.aiToolsView setHidden:_aiToolsHidden];
    [self.aiToolsSeparator setHidden:_aiToolsHidden];

    [self adjustWindowHeight];
}

- (void)applyAiToolsVersion {
    NSString *version = @"";
    
    if (_aiToolsVersion.length > 0) {
        NSRegularExpression *regex =
            [NSRegularExpression regularExpressionWithPattern:@"^\\d+"
                                                      options:0
                                                        error:nil];
        NSTextCheckingResult *match =
            [regex firstMatchInString:_aiToolsVersion
                              options:0
                                range:NSMakeRange(0, _aiToolsVersion.length)];

        if (match) {
            NSString *numberString = [_aiToolsVersion substringWithRange:match.range];
            NSInteger number = numberString.integerValue;
            NSInteger replacement = number < 9 ? 1 : number - 8;
            
            version = [_aiToolsVersion stringByReplacingCharactersInRange:match.range
                                           withString:[NSString stringWithFormat:@"%ld",
                                                           (long)replacement]];
        }
    }
    
    [self.aiVersionText setStringValue:[NSString stringWithFormat:
                                            NSLocalizedString(@"Version %@", nil),
                                            version]];
}

- (void)adjustWindowHeight {
    NSWindow * window = self.view.window;
    if (!window) {
        return;
    }

    [self.view layoutSubtreeIfNeeded];

    NSSize fittingSize = [self.view fittingSize];
    NSRect contentRect = [window contentRectForFrameRect:window.frame];
    CGFloat delta = fittingSize.height - NSHeight(contentRect);
    if (fabs(delta) < 0.5) {
        return;
    }

    NSRect frame = window.frame;
    frame.size.height += delta;
    frame.origin.y -= delta / 2;

    [window setFrame:frame display:YES];
}

#pragma mark - Helpers

- (void)applyLinkStyleToButton:(NSButton *)button {
    if (!button) {
        return;
    }

    NSMutableAttributedString * attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[button attributedTitle]];
    NSRange range = NSMakeRange(0, [attrTitle length]);
    [attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor linkColor] range:range];
    [attrTitle fixAttributesInRange:range];
    [button setAttributedTitle:attrTitle];
}

- (NSString *)shortAppVersion {
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    return [infoDictionary objectForKey:@"CFBundleShortVersionString"];
}

- (NSString *)fullAppVersion {
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    return [NSString stringWithFormat:@"%@.%@",
               [infoDictionary objectForKey:@"CFBundleShortVersionString"],
               [infoDictionary objectForKey:@"ASCBundleBuildNumber"]];
}

- (NSString *)appEdition {
    return !isCommercialVersion
        ? NSLocalizedString(@"Community version %@", nil)
        : NSLocalizedString(@"Enterprise version %@", nil);
}

#pragma mark - Actions

- (void)onVersionClick:(NSTextField *)sender {
    [self.versionText setStringValue:[NSString stringWithFormat:
                                          [self appEdition],
                                          [self fullAppVersion]]];
    
#if _V8_VERSION
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ (x86)", [self.versionText stringValue]]];
#elif _X86_64_ONLY
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ (x64)", [self.versionText stringValue]]];
#elif _ARM_ONLY
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ (Apple Silicon)", [self.versionText stringValue]]];
#endif
}

- (IBAction)onLicenseButtonClick:(id)sender {
    NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html" subdirectory:@"license"];
    if ( !eulaUrl )
        eulaUrl = [[NSBundle mainBundle] URLForResource:@"LICENSE" withExtension:@"html" subdirectory:@"license"];
    
    NSWindowController * windowController = [self.storyboard instantiateControllerWithIdentifier:@"ASCLicenseWindowControllerId"];
    ASCLicenseController *licView = (ASCLicenseController *)windowController.contentViewController;
    [licView setUrl:eulaUrl];
    NSWindow *licWindow = windowController.window;
    
    NSRect parentFrame = self.view.window.frame;
    NSRect childFrame = licWindow.frame;
    [licWindow setFrameOrigin:NSMakePoint(NSMidX(parentFrame) - childFrame.size.width/2,
                                          NSMidY(parentFrame) - childFrame.size.height/2)];
    [licWindow makeKeyAndOrderFront:nil]; // Show the window first to apply the coordinates
    
    [NSApp runModalForWindow:licWindow];
}

- (IBAction)onAiLicenseButtonClick:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kBslLicenseURL]];
}

- (IBAction)onWebsiteButtonClick:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kOfficialWebsiteURL]];
}

@end
