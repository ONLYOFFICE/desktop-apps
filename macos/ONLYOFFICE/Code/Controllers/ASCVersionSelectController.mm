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
//  ASCVersionSelectController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 2/16/16.
//  Copyright © 2016 Ascensio System SIA. All rights reserved.
//

#import "ASCVersionSelectController.h"
#import <WebKit/WebKit.h>
#import "applicationmanager.h"
#import "mac_application.h"
#import "NSString+OnlyOffice.h"
#import "ASCHelper.h"
#import "ASCConstants.h"

@interface ASCVersionSelectController ()
@property (weak) IBOutlet NSButton *licenseButton;
@property (weak) IBOutlet NSView *homeView;
@property (weak) IBOutlet NSView *businessView;
@property (weak) IBOutlet NSTextField *titleText;
@property (weak) IBOutlet NSTextField *subtitleText;
@property (weak) IBOutlet WebView *eulaWebView;

@property (nonatomic) BOOL shouldTerminate;
@end

@implementation ASCVersionSelectController

- (void)viewDidLoad {
    [super viewDidLoad];

    NSString * productName = [ASCHelper appName];
    
    self.shouldTerminate = YES;
    
    if (self.licenseButton) {        
        // Setup license button view
        NSMutableAttributedString * attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[self.licenseButton attributedTitle]];
        long len = [attrTitle length];
        NSRange range = NSMakeRange(0, len);
        [attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor keyboardFocusIndicatorColor] range:range];
        [attrTitle fixAttributesInRange:range];
        [self.licenseButton setAttributedTitle:attrTitle];
        
        NSTrackingAreaOptions options = NSTrackingActiveAlways | NSTrackingMouseMoved | NSTrackingCursorUpdate | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp;
        NSTrackingArea * trackingArea = [[NSTrackingArea alloc] initWithRect:self.licenseButton.bounds options:options owner:self userInfo:@{@"sender":self.licenseButton}];
        [self.licenseButton addTrackingArea:trackingArea];
    }
    
    if (self.homeView) {        
        // Setup view background
        for (NSView * view in @[self.homeView, self.businessView]) {
            view.wantsLayer = YES;
            view.layer.cornerRadius = 5.f;
            view.layer.backgroundColor = [[NSColor clearColor] CGColor];
            
            NSTrackingAreaOptions options = NSTrackingActiveAlways | NSTrackingMouseMoved | NSTrackingCursorUpdate | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp;
            NSTrackingArea * trackingArea = [[NSTrackingArea alloc] initWithRect:view.bounds options:options owner:self userInfo:@{@"sender":view}];
            [view addTrackingArea:trackingArea];
        }
    }
    
    if (self.eulaWebView) {
        NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html"];
        [[self.eulaWebView mainFrame] loadRequest:[NSURLRequest requestWithURL:eulaUrl]];
    }
    
    if (self.titleText) {
        [self.titleText setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Welcome to %@", nil), productName]];
    }
    
    if (self.subtitleText) {
        [self.subtitleText setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Select how you will use %@", nil), productName]];
    }
}

- (BOOL)shouldTerminateApplication {
    return self.shouldTerminate;
}

- (void)mouseMoved:(NSEvent *)theEvent {
    [super mouseMoved:theEvent];
    [[NSCursor pointingHandCursor] set];
}

- (void)mouseEntered:(NSEvent *)theEvent {
    [super mouseEntered:theEvent];
    
    NSDictionary * userInfo = (NSDictionary<id,id> *)[theEvent userData];
    
    if (userInfo) {
        id sender = userInfo[@"sender"];
        
        if (sender) {
            if ([sender isEqualTo:self.licenseButton]) {
                return;
            }
            
            NSView * view = (NSView *)sender;
            view.layer.backgroundColor = [[NSColor colorWithDeviceWhite:234.f/255.f alpha:1.f] CGColor];
        }
    }
}

- (void)mouseExited:(NSEvent *)theEvent {
    [super mouseExited:theEvent];
    [[NSCursor arrowCursor] set];
    
    NSDictionary * userInfo = (NSDictionary<id,id> *)[theEvent userData];
    
    if (userInfo) {
        id sender = userInfo[@"sender"];
        
        if (sender) {
            if ([sender isEqualTo:self.licenseButton]) {
                return;
            }
            
            NSView * view = (NSView *)sender;
            view.layer.backgroundColor = [[NSColor clearColor] CGColor];
        }
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    [super mouseUp:theEvent];
    
    if (1 == [theEvent clickCount]) {
        NSPoint eventLocation = [theEvent locationInWindow];

        if (NSPointInRect(eventLocation, self.homeView.frame)) {
            [[NSUserDefaults standardUserDefaults] setInteger:ASCVersionTypeForHome forKey:@"hasVersionMode"];
            [[NSUserDefaults standardUserDefaults] synchronize];
            
            [self forceSide:ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_FREE];
            
            self.shouldTerminate = NO;
            [NSApp stopModal];
        }
        
        if (NSPointInRect(eventLocation, self.businessView.frame)) {
            [[NSUserDefaults standardUserDefaults] setInteger:ASCVersionTypeForBusiness forKey:@"hasVersionMode"];
            [[NSUserDefaults standardUserDefaults] synchronize];
            
            [self forceSide:ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_DEMO];
            
            self.shouldTerminate = NO;
            [NSApp stopModal];
        }
    }
}

- (void)forceSide:(int)eventType {
    NSString * licenseDirectory = [ASCHelper licensePath];
    
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    
    // First launch
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"hasLaunchedOnce"]) {
        NSEditorApi::CAscLicenceActual * generateLicenceData = new NSEditorApi::CAscLicenceActual();
        generateLicenceData->put_Path([licenseDirectory stdwstring]);
#ifdef _PRODUCT_IVOLGA
        generateLicenceData->put_ProductId(PRODUCT_ID_IVOLGAPRO);
#else
        generateLicenceData->put_ProductId(PRODUCT_ID_ONLYOFFICE);
#endif
        
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(eventType);
        pEvent->m_pData = generateLicenceData;
        
        appManager->Apply(pEvent);
        
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"hasLaunchedOnce"];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
}

@end
