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
//  ASCUpgradeController.mm
//  ONLYOFFICE
//
//  Created by Ascensio System SIA
//  Copyright © 2026 Ascensio System SIA. All rights reserved.
//

#ifdef USE_ADDON_MODE

#import "ASCUpgradeController.h"
#import "ASCThemesController.h"
#import "ASCConstants.h"
#import "NSCefView.h"
#import "ASCLinguist.h"
#import "NSDictionary+Extensions.h"
#import "PureLayout.h"
#import "mac_application.h"


static const NSSize kASCUpgradeIntroSize   = { 480, 700 };
static const NSSize kASCUpgradeInstallSize = { 480, 304 };

@interface ASCUpgradeController () <NSWindowDelegate>

@property (nonatomic, strong) NSCefView *cefView;
@property (nonatomic, assign) ASCUpgradeDialogResult modalResult;

@end

@implementation ASCUpgradeController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.modalResult = ASCUpgradeDialogResultRejected;

    self.cefView = [[NSCefView alloc] initWithFrame:self.view.bounds];

    CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
    [self.cefView create:appManager withType:cvwtSimple];
    NSColor * backColor = [ASCThemesController currentThemeColor:windowBackgroundColor];
    [self.cefView setBackgroundColor:backColor];

    [self.view addSubview:self.cefView];
    [self.cefView autoPinEdgesToSuperviewEdges];

    NSString *exeDir = [[[NSBundle mainBundle] executablePath] stringByDeletingLastPathComponent];
    NSString *htmlPath = [exeDir stringByAppendingPathComponent:@"../Resources/login/addon.html"];
    NSString *urlAdditional = [NSString stringWithFormat:@"?lang=%@&page=intro", [ASCLinguist appLanguageCode]];
    NSString *addonUrl = [NSString stringWithFormat:@"file:///%@%@", htmlPath, urlAdditional];

    [self.cefView loadWithUrl:addonUrl];
}

- (void)dealloc {
    if (self.cefView) {
        CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
        if (appManager && self.cefView.uuid != NSNotFound) {
            appManager->DestroyCefView((int)self.cefView.uuid);
        }
        [self.cefView internalClean];
    }
}

- (void)viewDidAppear {
    [super viewDidAppear];

    NSWindow *window = self.view.window;
    window.titlebarAppearsTransparent = YES;
    window.titleVisibility = NSWindowTitleHidden;
    window.styleMask &= ~NSWindowStyleMaskResizable;

    [[window standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
    [[window standardWindowButton:NSWindowZoomButton] setHidden:YES];

    window.delegate = self;

    [self resizeWindowToSize:kASCUpgradeIntroSize];
}

#pragma mark - Public Methods

- (void)accept:(ASCUpgradeDialogResult)result {
    self.modalResult = result;
    [self.view.window close];
}

- (void)reject {
    [self accept:ASCUpgradeDialogResultRejected];
}

- (NSInteger)exec {
    [NSApp runModalForWindow:self.view.window];
    return self.modalResult;
}

- (void)setProgress:(NSInteger)percent {
    [self sendAddonStatus:@{@"progress": [NSString stringWithFormat:@"%ld", (long)percent]}];
}

- (void)showStartPage {
    [self sendAddonStatus:@{@"page": @"install"}];
    [self resizeWindowToSize:kASCUpgradeInstallSize];
}

- (void)showFinalPage {
    [self sendAddonStatus:@{@"page": @"complete"}];
}

- (void)showErrorPage:(NSString *)message {
    [self sendAddonStatus:@{@"page": @"error",
                            @"message": message ?: @""}];
}


#pragma mark - NSWindowDelegate

- (void)windowWillClose:(NSNotification *)notification {
    if (NSApp.modalWindow == self.view.window) {
        [NSApp stopModal];
    }
}

#pragma mark - Private Methods

- (void)sendAddonStatus:(NSDictionary *)json {
    [self.cefView sendCommand:@"addon:status" withParam:[json jsonString]];
}

- (void)resizeWindowToSize:(NSSize)size {
    NSWindow *window = self.view.window;
    if (!window) {
        return;
    }

    NSRect frame = window.frame;
    NSRect contentRect = [window contentRectForFrameRect:frame];
    if (NSEqualSizes(contentRect.size, size)) {
        return;
    }

    NSPoint center = NSMakePoint(NSMidX(frame), NSMidY(frame));
    contentRect.size = size;
    NSRect newFrame = [window frameRectForContentRect:contentRect];
    newFrame.origin.x = center.x - newFrame.size.width / 2;
    newFrame.origin.y = center.y - newFrame.size.height / 2;

    [window setFrame:newFrame display:YES animate:NO];
}

@end

#endif
