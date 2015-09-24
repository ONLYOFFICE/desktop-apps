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
//  ASCTitleBarController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTitleBarController.h"
#import "ASCTitleWindow.h"
#import "ASCConstants.h"
#import "NSView+ASCView.h"
#import "ASCTabView.h"
#import "ASCHelper.h"

static float kASCWindowDefaultTrafficButtonsLeftMargin = 0;
static float kASCWindowMinTitleWidth = 320;

@interface ASCTitleBarController ()  <ASCTabsControlDelegate>
@property (nonatomic) NSArray *standardButtonsDefaults;
@property (nonatomic) NSArray *standardButtons;

@property (nonatomic, weak) NSButton *closeButton;
@property (nonatomic, weak) NSButton *miniaturizeButton;
@property (nonatomic, weak) NSButton *fullscreenButton;
@property (weak) IBOutlet NSTextField *titleLabel;
@property (weak) IBOutlet NSView *titleContainerView;
@property (weak) IBOutlet NSButton *portalButton;
@property (weak) IBOutlet NSButton *userProfileButton;
@end

@implementation ASCTitleBarController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self initialize];
}

- (void)initialize {
    NSArray * windows = [[NSApplication sharedApplication] windows];
    
    NSWindow * mainWindow = nil;
    
    for (NSWindow * window in windows) {
        if ([window isKindOfClass:[ASCTitleWindow class]]) {
            mainWindow = window;
            break;
        }
    }
    
    self.closeButton = [NSWindow standardWindowButton:NSWindowCloseButton forStyleMask:NSTitledWindowMask];
    [self.view addSubview:self.closeButton];
    self.miniaturizeButton = [NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:NSTitledWindowMask];
    [self.view addSubview:self.miniaturizeButton];
    self.fullscreenButton = [NSWindow standardWindowButton:NSWindowZoomButton forStyleMask:NSTitledWindowMask];
    [self.view addSubview:self.fullscreenButton];

    
    self.standardButtonsDefaults = @[[mainWindow standardWindowButton:NSWindowCloseButton],
                                     [mainWindow standardWindowButton:NSWindowMiniaturizeButton],
                                     [mainWindow standardWindowButton:NSWindowZoomButton]];
    
    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSButton *standardButton, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButton];
    }];
    
    self.standardButtons = @[self.closeButton, self.miniaturizeButton, self.fullscreenButton];
    
    kASCWindowDefaultTrafficButtonsLeftMargin = NSWidth(self.closeButton.frame);
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidResize:)
                                                 name:NSWindowDidResizeNotification
                                               object:mainWindow];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onWindowSetFrame:)
                                                 name:ASCEventNameMainWindowSetFrame
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFChangedTabEditorType:)
                                                 name:CEFEventNameTabEditorType
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFChangedTabEditorName:)
                                                 name:CEFEventNameTabEditorNameChanged
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFModifyChanged:)
                                                 name:CEFEventNameModifyChanged
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFLogout:)
                                                 name:CEFEventNameLogout
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFLogin:)
                                                 name:CEFEventNameLogin
                                               object:nil];
    
    [self.tabsControl.multicastDelegate addDelegate:self];
    
    [self.userProfileButton setHidden:YES];
    [self.portalButton setImage:[NSImage imageNamed:@"Documents_active_hover"]];
    
    [self.tabsControl removeAllConstraints];
    [self.titleLabel removeAllConstraints];
    
    [self doLayout];
}

- (void)windowDidResize:(NSNotification *)notification {
    [self doLayout];
}

- (void)onWindowSetFrame:(NSNotification *)notification {
    [self doLayout];
}

- (BOOL)isFullScreen {
    NSArray * windows = [[NSApplication sharedApplication] windows];
    NSWindow * mainWindow = [[[NSApplication sharedApplication] windows] firstObject];
    
    for (NSWindow * window in windows) {
        if ([window isKindOfClass:[ASCTitleWindow class]]) {
            mainWindow = window;
            break;
        }
    }
    
	return (([mainWindow styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);
}

- (void)doLayout {
    void (^layoutStandartButtons)(NSArray *, BOOL) = ^ (NSArray *buttons, BOOL hidden) {
        [buttons enumerateObjectsUsingBlock:^(NSButton *button, NSUInteger idx, BOOL *stop) {
            NSRect frame = button.frame;
            frame.origin.x = kASCWindowDefaultTrafficButtonsLeftMargin + idx * (NSWidth(frame) + 6);
            frame.origin.y = (NSHeight(button.superview.frame) - NSHeight(button.frame)) / 2;
            
            [button setFrame:frame];
            [button setHidden:hidden];
            [button setNeedsDisplay:YES];
        }];

    };
    
    layoutStandartButtons(self.standardButtonsDefaults, [self isFullScreen]);
    layoutStandartButtons(self.standardButtons, ![self isFullScreen]);
    [self.miniaturizeButton setEnabled:![self isFullScreen]];
    
    // Layout title and tabs
    CGFloat containerWidth  = CGRectGetWidth(self.titleContainerView.frame);
    CGFloat maxTabsWidth    = containerWidth - kASCWindowMinTitleWidth;
    CGFloat actualTabsWidth = self.tabsControl.maxTabWidth * [self.tabsControl.tabs count];
    
    self.tabsControl.frame  = CGRectMake(0, 0, MIN(actualTabsWidth, maxTabsWidth), CGRectGetHeight(self.tabsControl.frame));
    self.titleLabel.frame   = CGRectMake(CGRectGetWidth(self.tabsControl.frame), self.titleLabel.frame.origin.y, containerWidth - CGRectGetWidth(self.tabsControl.frame), self.titleLabel.frame.size.height);
}

- (void)viewWillTransitionToSize:(NSSize)newSize {
    [self doLayout];
}

#pragma mark -
#pragma mark - CEF events handler

- (void)onCEFChangedTabEditorType:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSInteger type          = [params[@"type"] integerValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
            ASCTabViewType docType = ASCTabViewUnknownType;
            switch (type) {
                case 0: docType = ASCTabViewDocumentType;       break;
                case 1: docType = ASCTabViewPresentationType;   break;
                case 2: docType = ASCTabViewSpreadsheetType;    break;
                    
                default:
                    break;
            }
            [tab setType:docType];
        }
    }
}

- (void)onCEFChangedTabEditorName:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSString * name         = params[@"name"];

        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
            [tab setTitle:name];
            [tab setToolTip:name];
            [self.tabsControl selectTab:tab];
        }
    }
}

- (void)onCEFModifyChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        BOOL changed            = [params[@"сhanged"] boolValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
            tab.changed = changed;
            
            if ([tab state] == NSOnState) {
                [self.tabsControl selectTab:tab];
            }
        }
    }
}

- (void)onCEFLogout:(NSNotification *)notification {
    [self.userProfileButton setHidden:YES];
}

- (void)onCEFLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * userInfo = (NSDictionary *)notification.userInfo;
        
        [[ASCHelper localSettings] setValue:userInfo forKey:ASCUserSettingsNameUserInfo];
        [self.userProfileButton setHidden:NO];
    }
}

#pragma mark -
#pragma mark - Actions

- (IBAction)onOnlyofficeButton:(id)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(onOnlyofficeButton:)]) {
        [_delegate onOnlyofficeButton:sender];
    }
}


#pragma mark -
#pragma mark - ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didResize:(CGRect)rect {
    [self doLayout];
}

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {
    if (tab) {
        NSButton * btn = (NSButton *)tab;
        [self.titleLabel setStringValue:[NSString stringWithFormat:@"ONLYOFFICE  ▸  %@", btn.title]];
        [self.portalButton setImage:[NSImage imageNamed:@"Documents_inactive_normal"]];
    } else {
        [self.titleLabel setStringValue:@"ONLYOFFICE"];
        [self.portalButton setImage:[NSImage imageNamed:@"Documents_active_hover"]];
    }
}

#pragma mark -
#pragma mark - Navigation

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"ASCUserInfoSegueID"]) {
        if (_delegate && [_delegate respondsToSelector:@selector(onShowUserInfoController:)]) {
            [_delegate onShowUserInfoController:segue.destinationController];
        }
    }
}


@end
