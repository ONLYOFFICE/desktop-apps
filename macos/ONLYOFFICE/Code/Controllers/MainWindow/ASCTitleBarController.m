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
//  ASCTitleBarController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTitleBarController.h"
#import "ASCTitleWindow.h"
#import "ASCConstants.h"
#import "NSView+Extensions.h"
#import "NSColor+Extensions.h"
#import "NSApplication+Extensions.h"
#import "ASCTabView.h"
#import "SFBPopover.h"
#import "ASCUserInfoViewController.h"
#import "ASCDownloadViewController.h"
#import "ASCDownloadController.h"
#import "ASCMenuButtonCell.h"

static float kASCWindowDefaultTrafficButtonsLeftMargin = 0;
static float kASCWindowMinTitleWidth = 0;

@interface ASCTitleBarController ()  <ASCTabsControlDelegate, ASCDownloadControllerDelegate>
@property (nonatomic) NSArray *standardButtonsDefaults;
@property (nonatomic) NSArray *standardButtonsFullscreen;

@property (nonatomic, weak) NSButton *closeButtonFullscreen;
@property (nonatomic, weak) NSButton *miniaturizeButtonFullscreen;
@property (nonatomic, weak) NSButton *fullscreenButtonFullscreen;
@property (nonatomic) NSImageView * miniaturizeButtonImageViewFullscreen;

@property (weak) IBOutlet NSView *titleContainerView;
@property (weak) IBOutlet NSButton *portalButton;
@property (weak) IBOutlet NSButton *userProfileButton;
@property (weak) IBOutlet NSLayoutConstraint *downloadWidthConstraint;
@property (weak) IBOutlet NSImageView *downloadImageView;
@property (weak) IBOutlet NSView *downloadBackgroundView;
@property (nonatomic) SFBPopover * popover;
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

    // Standart window buttons in Fullscreen

    self.closeButtonFullscreen = [NSWindow standardWindowButton:NSWindowCloseButton forStyleMask:NSWindowStyleMaskTitled];
    self.fullscreenButtonFullscreen = [NSWindow standardWindowButton:NSWindowZoomButton forStyleMask:NSWindowStyleMaskTitled];
    NSButton * miniaturizeButtonFullscreen = [NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:NSWindowStyleMaskFullScreen];

    NSImage * miniaturizeButtonImage = [miniaturizeButtonFullscreen imageRepresentation];
    self.miniaturizeButtonImageViewFullscreen = [[NSImageView alloc] initWithFrame:CGRectMake(0, 0, miniaturizeButtonImage.size.width, miniaturizeButtonImage.size.height)];
    self.miniaturizeButtonImageViewFullscreen.image = miniaturizeButtonImage;

    self.standardButtonsFullscreen = @[self.closeButtonFullscreen, self.miniaturizeButtonImageViewFullscreen, self.fullscreenButtonFullscreen];
    [self.standardButtonsFullscreen enumerateObjectsUsingBlock:^(NSView *standardButtonView, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButtonView];
    }];

    // Standart window buttons

    if (mainWindow) {
        self.standardButtonsDefaults = @[[mainWindow standardWindowButton:NSWindowCloseButton],
                                         [mainWindow standardWindowButton:NSWindowMiniaturizeButton],
                                         [mainWindow standardWindowButton:NSWindowZoomButton]];
    }
    
    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSButton *standardButton, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButton];
    }];

    // Other window controls

    self.downloadWidthConstraint.constant = .0f;
    self.downloadImageView.canDrawSubviewsIntoLayer = YES;

    kASCWindowDefaultTrafficButtonsLeftMargin = NSWidth(self.closeButtonFullscreen.frame) - 2.0; // OSX 10.11 magic

    // Subscribe to events

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
                                             selector:@selector(onCEFLogin:)
                                                 name:CEFEventNameLogin
                                               object:nil];
    
    [[[ASCDownloadController sharedInstance] multicastDelegate] addDelegate:self];
    [self.tabsControl.multicastDelegate addDelegate:self];
    
    [self.userProfileButton setHidden:YES];
    [self.portalButton setState:NSOnState];
    
    [self.tabsControl removeAllConstraints];

    ASCMenuButtonCell * portalButtonCell = self.portalButton.cell;

    if (portalButtonCell) {
        if (@available(macOS 10.13, *)) {
            portalButtonCell.bgColor            = [NSColor colorNamed:@"tab-inactiveColor"];
            portalButtonCell.bgHoverColor       = [NSColor colorNamed:@"tab-hoverInactiveColor"];
            portalButtonCell.bgActiveColor      = [NSColor colorNamed:@"tab-portal-activeColor"];
            portalButtonCell.textColor          = [NSColor clearColor];
            portalButtonCell.textActiveColor    = [NSColor clearColor];
            portalButtonCell.lineColor          = [NSColor clearColor];
        } else {
            portalButtonCell.bgColor            = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.bgHoverColor       = kColorRGBA(255, 255, 255, 1.0);
            portalButtonCell.bgActiveColor      = kColorRGBA(255, 255, 255, 1.0);
            portalButtonCell.textColor          = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.textActiveColor    = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.lineColor          = kColorRGBA(255, 255, 255, 0.0);
        }
    }

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
    void (^layoutStandartButtons)(NSArray *, BOOL) = ^ (NSArray *views, BOOL hidden) {
        [views enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
            NSRect frame = view.frame;
            frame.origin.x = kASCWindowDefaultTrafficButtonsLeftMargin + idx * (NSWidth(frame) + 6.0);
            frame.origin.y = (int)((NSHeight(view.superview.frame) - NSHeight(view.frame)) / 2.0);
            
            [view setFrame:frame];
            [view setHidden:hidden];
            [view setNeedsDisplay:YES];
        }];
    };

    layoutStandartButtons(self.standardButtonsDefaults, [self isFullScreen]);
    layoutStandartButtons(self.standardButtonsFullscreen, ![self isFullScreen]);

    // Layout title and tabs
    CGFloat containerWidth  = CGRectGetWidth(self.titleContainerView.frame);
    CGFloat maxTabsWidth    = containerWidth - kASCWindowMinTitleWidth;
    CGFloat actualTabsWidth = self.tabsControl.maxTabWidth * [self.tabsControl.tabs count];
    
    self.tabsControl.frame  = CGRectMake(0, 0, MIN(actualTabsWidth, maxTabsWidth), CGRectGetHeight(self.tabsControl.frame));
}

- (void)viewWillTransitionToSize:(NSSize)newSize {
    [self doLayout];
}

#pragma mark -
#pragma mark CEF events handler

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
            [self.tabsControl updateTab:tab];
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

            [self.tabsControl updateTab:tab];
//            if ([tab state] == NSOnState) {
//                [self.tabsControl reloadTab:tab];
//            }
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

            [self.tabsControl updateTab:tab];
//            if ([tab state] == NSOnState) {
//                [self.tabsControl reloadTab:tab];
//            }
        }
    }
}

- (void)onCEFLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * userInfo = (NSDictionary *)notification.userInfo;
        
        [[ASCHelper localSettings] setValue:userInfo forKey:ASCUserSettingsNameUserInfo];
        [self.userProfileButton setHidden:NO];
    }
}

#pragma mark -
#pragma mark Actions
- (void)setupCustomPopover:(SFBPopover *)popover {
    popover.closesWhenPopoverResignsKey     = YES;
    popover.closesWhenApplicationBecomesInactive = YES;
    popover.drawRoundCornerBesideArrow      = YES;
    popover.borderColor                     = [NSColor clearColor];
    popover.backgroundColor                 = [NSColor whiteColor];
    popover.viewMargin                      = 0.0f;
    popover.borderWidth                     = 0.0f;
    popover.cornerRadius                    = 2.0f;
    popover.drawsArrow                      = YES;
    popover.movable                         = NO;
    popover.arrowWidth                      = 20.0f;
    popover.arrowHeight                     = 10.0f;
    popover.distance                        = 10.0f;
}

- (IBAction)onOnlyofficeButton:(id)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(onOnlyofficeButton:)]) {
        [_delegate onOnlyofficeButton:sender];
    }
}

- (IBAction)onUserInfoClick:(id)sender {
    ASCUserInfoViewController * controller = [self.storyboard instantiateControllerWithIdentifier:@"ASCUserInfoControllerId"];
    self.popover = [[SFBPopover alloc] initWithContentViewController:controller];
    self.popover.arrowOffset = 8.0f;
    [self setupCustomPopover:self.popover];
    
    NSRect rectOfSender = [sender convertRect:[sender bounds] toView:nil];
    NSPoint where = rectOfSender.origin;
    where.x += rectOfSender.size.width / 2;

    [controller setPopover:self.popover];
    [self.popover displayPopoverInWindow:[sender window] atPoint:where chooseBestLocation:YES];
    
    if (_delegate && [_delegate respondsToSelector:@selector(onShowUserInfoController:)]) {
        [_delegate onShowUserInfoController:controller];
    }
}

- (IBAction)onDownloadButton:(id)sender {
    ASCDownloadViewController * controller = [self.storyboard instantiateControllerWithIdentifier:@"ASCDownloadListControllerId"];
    self.popover = [[SFBPopover alloc] initWithContentViewController:controller];
    self.popover.arrowOffset = 40.0f;
    [self setupCustomPopover:self.popover];
    
    NSRect rectOfSender = [sender convertRect:[sender bounds] toView:nil];
    NSPoint where = rectOfSender.origin;
    where.x += rectOfSender.size.width / 2;
    
    [controller setPopover:self.popover];
    [self.popover displayPopoverInWindow:[sender window] atPoint:where chooseBestLocation:YES];
}

- (IBAction)onTestButton:(NSButton *)sender {
    NSStoryboard * storyboard = [NSStoryboard storyboardWithName:@"Document-Sign" bundle:[NSBundle mainBundle]];

    if (storyboard) {
        NSWindowController * windowController = [storyboard instantiateControllerWithIdentifier:@"DocSignWindowController"];
        NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];

        [mainWindow beginSheet:[windowController window] completionHandler:^(NSModalResponse returnCode) {
            //
        }];
    }
}

#pragma mark -
#pragma mark ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didResize:(CGRect)rect {
    [self doLayout];
}

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {  
    if (tab) {
        [self.portalButton setState:NSOffState];
    } else {
        [self.portalButton setState:NSOnState];
    }

    if ([NSApplication isDarkMode]) {
        [self.portalButton setImage:(self.portalButton.state == NSOnState)
         ? [NSImage imageNamed:@"logo-tab-dark"]
         : [NSImage imageNamed:@"logo-tab-light"]
        ];
    } else {
        [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
    }
}

#pragma mark -
#pragma mark ASCDownloadController Delegate

- (void)downloadController:(ASCDownloadController *)controler didAddDownload:(id)download {
    self.downloadWidthConstraint.constant = ([[controler downloads] count] > 0) ? 30.f : .0f;
}

- (void)downloadController:(ASCDownloadController *)controler didRemovedDownload:(id)download {
    self.downloadWidthConstraint.constant = ([[controler downloads] count] > 0) ? 30.f : .0f;
}

- (void)downloadController:(ASCDownloadController *)controler didUpdatedDownload:(id)download {
    //
}

#pragma mark -
#pragma mark Navigation

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"ASCUserInfoSegueID"]) {
        if (_delegate && [_delegate respondsToSelector:@selector(onShowUserInfoController:)]) {
            [_delegate onShowUserInfoController:segue.destinationController];
        }
    }
}


@end
