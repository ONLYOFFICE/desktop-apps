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
//  ASCEditorWindowController.m
//  ONLYOFFICE
//
//  Copyright (c) 2025 Ascensio System SIA. All rights reserved.
//

#import "ASCEditorWindowController.h"
#import "ASCEditorWindow.h"
#import "ASCEditorTitleBarController.h"
#import "AppDelegate.h"
#import "ASCConstants.h"
#import "AnalyticsHelper.h"
#import "ASCHelper.h"
#import "NSCefView.h"
#import "NSCefData.h"
#import "ASCEventsController.h"
#import "ASCPresentationReporter.h"
#import "NSString+Extensions.h"
#import "NSDictionary+Extensions.h"
#import "NSView+Extensions.h"
#import "PureLayout.h"

@interface ASCHoverButton : NSButton
@property (nonatomic) BOOL isHovered;
@end

@implementation ASCHoverButton

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    for (NSTrackingArea *area in self.trackingAreas) {
        [self removeTrackingArea:area];
    }
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                               options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow
                                                                 owner:self
                                                              userInfo:nil];
    [self addTrackingArea:trackingArea];
}

- (void)mouseEntered:(NSEvent *)event {
    _isHovered = YES;
    [self setNeedsDisplay:YES];
}

- (void)mouseExited:(NSEvent *)event {
    _isHovered = NO;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (_isHovered) {
        [[NSColor colorWithWhite:0.0 alpha:0.15] set];
        NSRectFillUsingOperation(self.bounds, NSCompositingOperationSourceOver);
    }
    [super drawRect:dirtyRect];
}

@end

@interface ASCEditorWindowController () <NSWindowDelegate>
@property (nonatomic) BOOL waitingForClose;
@property (nonatomic) ASCEditorTitleBarController *titlebarController;
@property (nonatomic) NSView *simpleEditorTitle;
@end

@implementation ASCEditorWindowController

+ (instancetype)initWithDefaultFrame {
    NSStoryboard *storyboard = [NSStoryboard storyboardWithName:StoryboardNameEditor bundle:nil];
    ASCEditorWindowController * controller = [storyboard instantiateControllerWithIdentifier:@"ASCEditorWindowControllerId"];
    return controller;
}

+ (instancetype)initWithFrame:(NSRect)frame {
    ASCEditorWindowController * controller = [self initWithDefaultFrame];
    [controller.window setFrame:frame display:NO];
    return controller;
}

- (void)windowDidLoad {
    NSString * productName = [ASCHelper appName];
    self.window.title = productName;
    
    [super windowDidLoad];
    [self setShouldCascadeWindows:YES];
    self.window.delegate = self;
    if (@available(macOS 11.0, *)) {
        self.window.toolbarStyle = NSWindowToolbarStyleUnifiedCompact;
    }
    [self setupToolbar];

    NSString *savedFrame = [[NSUserDefaults standardUserDefaults] stringForKey:@"ASCEditorWindowFrame"];
    if (savedFrame) {
        [self.window setFrameFromString:savedFrame];
    }
    
    void (^addObserverFor)(_Nullable NSNotificationName, SEL) = ^(_Nullable NSNotificationName name, SEL selector) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:selector
                                                     name:name
                                                   object:nil];
    };
    
    addObserverFor(CEFEventNameModifyChanged, @selector(onCEFModifyChanged:));
    addObserverFor(CEFEventNameTabEditorNameChanged, @selector(onCEFChangedTabEditorName:));
    addObserverFor(CEFEventNameTabEditorType, @selector(onCEFChangedTabEditorType:));
    addObserverFor(CEFEventNameSave, @selector(onCEFSave:));
    addObserverFor(CEFEventNameFullscreen, @selector(onCEFFullscreen:));
    addObserverFor(CEFEventNameEditorAppActionRequest, @selector(onCEFEditorAppActionRequest:));
    addObserverFor(CEFEventNameDocumentFragmentBuild, @selector(onCEFDocumentFragmentBuild:));
    addObserverFor(CEFEventNameDocumentFragmented, @selector(onCEFDocumentFragmented:));
    addObserverFor(CEFEventNameWebAppsEntry, @selector(onCEFWebAppsEntry:));
    addObserverFor(CEFEventNameWebTitleChanged, @selector(onWebTitleChanged:));
}

- (void)setupToolbar {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    self.titlebarController = [self.storyboard instantiateControllerWithIdentifier:@"editorTitleBarControllerID"];

    NSView * titlebar = [[window standardWindowButton:NSWindowCloseButton] superview];
    [titlebar addSubview:self.titlebarController.view];

    NSView * view = self.titlebarController.view;
    NSView * superview = view.superview;

    view.translatesAutoresizingMaskIntoConstraints = NO;

    [superview addConstraint:[NSLayoutConstraint constraintWithItem:view
                                                          attribute:NSLayoutAttributeLeading
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:superview
                                                          attribute:NSLayoutAttributeLeading
                                                         multiplier:1
                                                           constant:0]];
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:view
                                                          attribute:NSLayoutAttributeTop
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:superview
                                                          attribute:NSLayoutAttributeTop
                                                         multiplier:1
                                                           constant:0]];
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:superview
                                                          attribute:NSLayoutAttributeTrailing
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:view
                                                          attribute:NSLayoutAttributeTrailing
                                                         multiplier:1
                                                           constant:0]];
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:superview
                                                          attribute:NSLayoutAttributeHeight
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:view
                                                          attribute:NSLayoutAttributeHeight
                                                         multiplier:1
                                                           constant:0]];

    [self.titlebarController setupWithWindow:window];
}

- (void)extendableTitleToSimple {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSView *webView = window.webView;

    CGFloat stripHeight = 28.0;

    NSView *rootView = self.contentViewController.view;

    self.simpleEditorTitle = [[NSView alloc] initForAutoLayout];
    self.simpleEditorTitle.wantsLayer = YES;
    self.simpleEditorTitle.layer.backgroundColor = [[NSColor colorWithRed:170.0/255.0 green:82.0/255.0 blue:82.0/255.0 alpha:1.0] CGColor];

    [rootView addSubview:self.simpleEditorTitle];

    [self.simpleEditorTitle autoPinEdgeToSuperviewEdge:ALEdgeTop];
    [self.simpleEditorTitle autoPinEdgeToSuperviewEdge:ALEdgeLeading];
    [self.simpleEditorTitle autoPinEdgeToSuperviewEdge:ALEdgeTrailing];
    [self.simpleEditorTitle autoSetDimension:ALDimensionHeight toSize:stripHeight];

    for (NSLayoutConstraint *constraint in rootView.constraints.copy) {
        BOOL isTopConstraint = (constraint.firstItem == webView && constraint.firstAttribute == NSLayoutAttributeTop) ||
                               (constraint.secondItem == webView && constraint.secondAttribute == NSLayoutAttributeTop);
        if (isTopConstraint) {
            [rootView removeConstraint:constraint];
        }
    }

    [webView autoPinEdge:ALEdgeTop toEdge:ALEdgeBottom ofView:self.simpleEditorTitle];

    NSImage *tintedImage = [[NSImage imageNamed:@"icon-home_normal"] copy];
    tintedImage.size = NSMakeSize(20, 20);
    [tintedImage lockFocus];
    [[NSColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0] set];
    NSRectFillUsingOperation(NSMakeRect(0, 0, 20, 20), NSCompositingOperationSourceAtop);
    [tintedImage unlockFocus];

    ASCHoverButton *iconButton = [[ASCHoverButton alloc] initForAutoLayout];
    iconButton.image = tintedImage;
    iconButton.imageScaling = NSImageScaleNone;
    iconButton.imagePosition = NSImageOnly;
    iconButton.bordered = NO;
    iconButton.target = self;
    iconButton.action = @selector(onSimpleTitleIconClicked:);

    [self.simpleEditorTitle addSubview:iconButton];

    [iconButton autoPinEdgeToSuperviewEdge:ALEdgeLeading withInset:0];
    [iconButton autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
    [iconButton autoSetDimensionsToSize:CGSizeMake(40, 28)];

    NSTextField *titleLabel = [[NSTextField alloc] initForAutoLayout];
    titleLabel.stringValue = @"ONLYOFFICE";
    titleLabel.editable = NO;
    titleLabel.selectable = NO;
    titleLabel.bordered = NO;
    titleLabel.backgroundColor = [NSColor clearColor];
    titleLabel.textColor = [NSColor whiteColor];
    titleLabel.font = [NSFont systemFontOfSize:13.0 weight:NSFontWeightRegular];
    titleLabel.alignment = NSTextAlignmentCenter;

    [self.simpleEditorTitle addSubview:titleLabel];
    [titleLabel autoCenterInSuperview];
}

- (void)onSimpleTitleIconClicked:(id)sender {
    AppDelegate *app = (AppDelegate *)[NSApp delegate];
    [app presentMainWindow];
}

- (void)windowDidMove:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameEditorWindowMoving object:self.window];
}

- (void)windowWillClose:(NSNotification *)notification {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    [[NSUserDefaults standardUserDefaults] setObject:[window stringWithSavedFrame] forKey:@"ASCEditorWindowFrame"];
    
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView) {
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        appManager->DestroyCefView((int)cefView.uuid);
        [cefView internalClean];
        window.webView = nil;
    }
    
    AppDelegate *app = (AppDelegate *)[NSApp delegate];
    [app.editorWindowControllers removeObject:self];
    
    if (app.waitingForTerminateApp && app.editorWindowControllers.count > 0) {
        [app safeCloseEditorWindows];
    }
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    if (self.waitingForClose)
        return NO;
    
    return [self shouldCloseWindow];
}

- (BOOL)shouldCloseWindow {
    BOOL hasUnsaved = NO;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO),
                                                                 @"terminate"  : @(YES)
                                                               }];
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (!cefView) {
        return YES;
    }
    
    if ([cefView.data hasChanges]) {
        hasUnsaved = YES;
    }
    
    // Blockchain check
    if ([cefView checkCloudCryptoNeedBuild]) {
        self.waitingForClose = YES;
        return NO;
        
    } else {
        if ([cefView isSaveLocked]) {
            hasUnsaved = YES;
        }
    }
    
    if (hasUnsaved) {
        [window makeKeyAndOrderFront:nil];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Do you want to save the changes made to the document \"%@\"?", nil), [cefView.data title:YES]]];
        [alert setInformativeText:NSLocalizedString(@"Your changes will be lost if you don't save them.", nil)];
        [alert setAlertStyle:NSAlertStyleWarning];

        NSInteger returnCode = [alert runModal];
        if (returnCode == NSAlertFirstButtonReturn) {
            // Save
            self.waitingForClose = YES;
            
            NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
            [cefView apply:pEvent];
            return NO;

        } else
        if (returnCode == NSAlertSecondButtonReturn) {
            // Don't Save

        } else
        if (returnCode == NSAlertThirdButtonReturn) {
            // Cancel
            AppDelegate *app = (AppDelegate *)[NSApp delegate];
            app.waitingForTerminateApp = NO;
            return NO;
        }
    }
    return YES;
}

- (BOOL)holdView:(NSString *)viewId {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView && cefView.uuid == [viewId intValue]) {
        return YES;
    }
    return NO;
}

- (NSCefData *)cefData {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView) {
        return cefView.data;
    }
    return nil;
}

- (NSMutableDictionary *)params {
    if (nil == _params) {
        _params = [NSMutableDictionary dictionary];
    }
    
    return _params;
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFModifyChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        BOOL changed = [params[@"сhanged"] boolValue];
        if ([self holdView:viewId]) {
            [self.cefData setChanged:changed];
        }
    }
}

- (void)onCEFChangedTabEditorType:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSInteger type          = [params[@"type"] integerValue];
        
        if ([self holdView:viewId]) {
            [self.cefData setContentType:AscEditorType(type)];
        }
    }
}

- (void)onCEFChangedTabEditorName:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSString * name         = params[@"name"];
        NSString * path         = params[@"path"];
        
        if ([self holdView:viewId]) {
            [self.cefData setTitle:name];
            [self.window setTitle:name];

            if ( !(path == nil) && !(path.length == 0) ) {
                self.cefData.path = path;
            }
        }
    }
}

- (void)onCEFSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        if ([self holdView:viewId]) {
            if ( ![params[@"cancel"] boolValue] ) {
                if (self.waitingForClose) {
                    [self.window close];
                }
            } else {
                self.waitingForClose = NO;
            }
        }
    }
}

- (void)onCEFFullscreen:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;

        BOOL isFullscreen = [params[@"fullscreen"] boolValue];
        
        ASCEditorWindow *window = (ASCEditorWindow *)self.window;
        NSCefView *cefView = (NSCefView *)window.webView;
        if (!cefView) {
            return;
        }
        
        if ([params objectForKey:@"viewId"]) {
            NSString * viewId = params[@"viewId"];
            if (![self holdView:viewId]) {
                return;
            }
        } else
        if ([params objectForKey:@"terminate"] and [params[@"terminate"] boolValue]) {
            [ASCEventsController resetFullscreenState];
        }
        
        if (isFullscreen) {
            NSScreen * targetScreen = [self.window screen];
            NSArray<NSScreen *> * screens = [NSScreen screens];
            
            if ( [[ASCPresentationReporter sharedInstance] isVisible] && [screens count] > 1 ) {
                for ( NSScreen * screen in screens ) {
                    if ( screen != [NSScreen mainScreen] ) {
                        targetScreen = screen;
                        break;
                    }
                }
            }

            NSApplicationPresentationOptions options = NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar;
            [cefView enterFullScreenMode:targetScreen withOptions:@{
                NSFullScreenModeApplicationPresentationOptions: @(options),
                NSFullScreenModeAllScreens: @(NO)
            }];
            
            [cefView focus];
            [self.window setIsVisible:NO];
            
        } else if ([cefView isInFullScreenMode]) {
            [self.window setIsVisible:YES];
            [cefView exitFullScreenModeWithOptions:nil];

            NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
            pCommand->put_Command(L"editor:stopDemonstration");

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
            pEvent->m_pData = pCommand;

            [cefView apply:pEvent];
        }
    }
}

- (void)onCEFDocumentFragmentBuild:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        int error = [json[@"error"] intValue];
                
        if ([self holdView:viewId]) {
            if (error == 0 && self.waitingForClose) {
                [self.window close];
            }
        }
    }
}

- (void)onCEFDocumentFragmented:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        BOOL isFragmented = [json[@"isFragmented"] boolValue];

        if ([self holdView:viewId]) {
            if (isFragmented) {
                ASCEditorWindow *window = (ASCEditorWindow *)self.window;
                NSCefView *cefView = (NSCefView *)window.webView;
                if (cefView) {
                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD);
                    [cefView apply:pEvent];
                    return;
                }
            }

            [self.window close];
        }
    }
}

- (void)onCEFEditorAppActionRequest:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        NSString * action = json[@"action"];
        NSString * path = json[@"url"];

        if (viewId && action) {
            if ([self holdView:viewId]) {
                if ([action isEqualToString:@"close"]) {
                    [self.window close];
                }
            }
        }
    }
}

- (void)onCEFWebAppsEntry:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        if ([self holdView:viewId]) {
            ASCEditorWindow *window = (ASCEditorWindow *)self.window;
            NSCefView *cefView = (NSCefView *)window.webView;
            if (cefView) {
                NSDictionary *windowFeatures = @{@"skiptoparea": TOOLBTN_HEIGHT, @"singlewindow": @YES};
                [cefView sendCommand:@"window:features" withParam:[windowFeatures jsonString]];
            }
        }
    }
}

- (void)onWebTitleChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        if ([self holdView:viewId]) {
            if (NSString * infoString = json[@"info"]) {
                if (NSMutableDictionary *info = [[infoString dictionary] mutableCopy]) {
                    NSString *clickAction = info[@"click"];
                    if (clickAction && [clickAction isEqualToString:@"home"]) {
                        AppDelegate *app = (AppDelegate *)[NSApp delegate];
                        [app presentMainWindow];
                    }
                }
            }
        }
    }
}

@end
