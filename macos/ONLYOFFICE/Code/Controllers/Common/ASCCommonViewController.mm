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
//  ASCCommonViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCCommonViewController.h"
#import "applicationmanager.h"
#import "mac_application.h"
#import "nsascprinter.h"
#import "ASCTabsControl.h"
#import "ASCTabView.h"
#import "ASCTitleWindowController.h"
#import "ASCHelper.h"
#import "ASCConstants.h"
#import "ASCUserInfoViewController.h"
#import "NSView+Extensions.h"
#import "NSAlert+SynchronousSheet.h"
#import "NSColor+Extensions.h"
#import "NSString+Extensions.h"
#import "NSDictionary+Extensions.h"
#import "AppDelegate.h"
#import "NSCefView.h"
#import "ASCEventsController.h"
#import "ASCDownloadController.h"
#import "ASCSavePanelWithFormatController.h"
#import "ASCSharedSettings.h"
#import "ASCReplacePresentationAnimator.h"
#import "AnalyticsHelper.h"
#import "PureLayout.h"
#import "NSWindow+Extensions.h"
#import "ASCExternalController.h"
#import "ASCTouchBarController.h"
#import "ASCCertificatePreviewController.h"
#import "ASCCertificateQLPreviewController.h"
#import "ASCLinguist.h"

#define rootTabId @"1CEF624D-9FF3-432B-9967-61361B5BFE8B"
#define headerViewTag 7777

@interface ASCCommonViewController() <ASCTabsControlDelegate, ASCTitleBarControllerDelegate, ASCUserInfoViewControllerDelegate> {
    NSAscPrinterContext * m_pContext;
    NSUInteger documentNameCounter;
    NSUInteger spreadsheetNameCounter;
    NSUInteger presentationNameCounter;
}
@property (weak) ASCTabsControl *tabsControl;
@property (nonatomic) NSCefView * cefStartPageView;
@property (weak) IBOutlet NSTabView *tabView;
@property (nonatomic) BOOL shouldTerminateApp;
@property (nonatomic) BOOL shouldLogoutPortal;
@property (strong) IBOutlet NSView *headerView;
@property (nonatomic, assign) id <ASCExternalDelegate> externalDelegate;
@property (nonatomic) ASCTouchBarController *touchBarController;
@property (nonatomic) NSMutableArray<ASCTabView *> * tabsWithChanges;
@end

@implementation ASCCommonViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.tabsWithChanges = @[].mutableCopy;
    
    _externalDelegate = [[ASCExternalController shared] delegate];
    
    void (^addObserverFor)(_Nullable NSNotificationName, SEL) = ^(_Nullable NSNotificationName name, SEL selector) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:selector
                                                     name:name
                                                   object:nil];
    };
    
    addObserverFor(ASCEventNameMainWindowLoaded, @selector(onWindowLoaded:));
    addObserverFor(ASCEventNameOpenAppLinks, @selector(onOpenAppLink));
    addObserverFor(CEFEventNameCreateTab, @selector(onCEFCreateTab:));
    addObserverFor(CEFEventNameTabEditorNameChanged, @selector(onCEFChangedTabEditorName:));
    addObserverFor(CEFEventNameTabEditorType, @selector(onCEFChangedTabEditorType:));
    addObserverFor(CEFEventNameSave, @selector(onCEFSave:));
    addObserverFor(CEFEventNameOpenUrl, @selector(onCEFOpenUrl:));
    addObserverFor(CEFEventNameFullscreen, @selector(onCEFFullscreen:));
    addObserverFor(CEFEventNameKeyboardDown, @selector(onCEFKeyDown:));
    addObserverFor(CEFEventNameDownload, @selector(onCEFDownload:));
    addObserverFor(CEFEventNameStartSaveDialog, @selector(onCEFStartSave:));
    addObserverFor(CEFEventNamePrintDialog, @selector(onCEFOnBeforePrintEnd:));
    addObserverFor(CEFEventNameOpenLocalFile, @selector(onCEFOnOpenLocalFile:));
    addObserverFor(CEFEventNameSaveLocal, @selector(onCEFSaveLocalFile:));
    addObserverFor(CEFEventNameOpenImage, @selector(onCEFOpenLocalImage:));
    addObserverFor(CEFEventNameOpenFileDialog, @selector(onCEFOpenFileDialog:));
    addObserverFor(CEFEventNamePortalLogin, @selector(onCEFPortalLogin:));
    addObserverFor(CEFEventNamePortalLogout, @selector(onCEFPortalLogout:));
    addObserverFor(CEFEventNamePortalCreate, @selector(onCEFPortalCreate:));
    addObserverFor(CEFEventNamePortalNew, @selector(onCEFPortalNew:));
    addObserverFor(CEFEventNamePortalSSO, @selector(onCEFPortalSSO:));
    addObserverFor(CEFEventNameFileInFinder, @selector(onCEFFileInFinder:));
    addObserverFor(CEFEventNameFilesCheck, @selector(onCEFFilesCheck:));
    addObserverFor(CEFEventNameStartPageReady, @selector(onCEFStartPageReady:));
    addObserverFor(CEFEventNameSaveBeforSign, @selector(onCEFSaveBeforeSign:));
    addObserverFor(CEFEventNameEditorDocumentReady, @selector(onCEFEditorDocumentReady:));
    addObserverFor(CEFEventNameEditorAppReady, @selector(onCEFEditorAppReady:));
    addObserverFor(CEFEventNameEditorEvent, @selector(onCEFEditorEvent:));
    addObserverFor(CEFEventNameEditorAppActionRequest, @selector(onCEFEditorAppActionRequest:));
    addObserverFor(CEFEventNameEditorOpenFolder, @selector(onCEFEditorOpenFolder:));
    addObserverFor(CEFEventNameDocumentFragmentBuild, @selector(onCEFDocumentFragmentBuild:));
    addObserverFor(CEFEventNameDocumentFragmented, @selector(onCEFDocumentFragmented:));
    addObserverFor(CEFEventNameCertificatePreview, @selector(onCEFCertificatePreview:));
    addObserverFor(ASCEventNameChangedUITheme, @selector(onUIThemeChanged:));

    if (_externalDelegate && [_externalDelegate respondsToSelector:@selector(onCommonViewDidLoad:)]) {
        [_externalDelegate onCommonViewDidLoad:self];
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    
    // Update the view, if already loaded.
}

- (void)onWindowLoaded:(NSNotification *)notification {
    if (notification && notification.object) {
        ASCTitleWindowController *windowController = (ASCTitleWindowController *)notification.object;
        windowController.titlebarController.delegate = self;
        
        self.tabsControl = windowController.titlebarController.tabsControl;
        
        // Create CEF event listener
        [ASCEventsController sharedInstance];
        
        [self setupTabControl];
        [self createStartPage];
        [self loadStartPage];
        [self setupTouchBar];

        // External handle
        if (_externalDelegate && [_externalDelegate respondsToSelector:@selector(onMainWindowLoaded:)]) {
            [_externalDelegate onMainWindowLoaded:self];
        }
    }
}

- (void)setupTabControl {
    self.tabsControl.minTabWidth = 48;
    self.tabsControl.maxTabWidth = 135;
    
    [self.tabsControl.multicastDelegate addDelegate:self];
}

- (void)setupTouchBar {
    __weak typeof(self) weakSelf = self;
    _touchBarController = [[ASCTouchBarController alloc] init:self];
    _touchBarController.onItemTap = ^(id  _Nonnull sender, NSString * _Nonnull senderId) {
        [weakSelf onTouchBarItemTap:sender senderId:senderId];
    };
    [self.tabsControl.multicastDelegate addDelegate:_touchBarController];
    [_touchBarController invalidateTouchBar];
}

- (void)createStartPage {
    NSInteger rootTabIndex = [self.tabView indexOfTabViewItemWithIdentifier:rootTabId];
    
    if (rootTabIndex != NSNotFound) {
        NSTabViewItem * tab = [self.tabView tabViewItemAtIndex:rootTabIndex];
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        self.cefStartPageView = [[NSCefView alloc] initWithFrame:tab.view.frame];
        [self.cefStartPageView create:appManager withType:cvwtSimple];
        [tab.view addSubview:self.cefStartPageView];
        [self.cefStartPageView autoPinEdgesToSuperviewEdges];
    }
}

- (void)loadStartPage {
    if (self.cefStartPageView ) {
        id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
        
        NSUserDefaults *preferences     = [NSUserDefaults standardUserDefaults];
        NSURLComponents *loginPage      = [NSURLComponents componentsWithString:[[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"login"]];

        NSURLQueryItem *countryCode     = [NSURLQueryItem queryItemWithName:@"lang" value: [ASCLinguist appLanguageCode]];
        NSURLQueryItem *portalAddress   = [NSURLQueryItem queryItemWithName:@"portal" value:[preferences objectForKey:ASCUserSettingsNamePortalUrl]];

        if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
            countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
        }

        loginPage.queryItems            = @[countryCode, portalAddress];
        loginPage.scheme                = NSURLFileScheme;
        
        [self.cefStartPageView loadWithUrl:[loginPage string]];
    }
}

- (void)openLocalPage:(NSString *)path title:(NSString *)title {
    [self openLocalPage:path query:nil title:title];
}

- (void)openLocalPage:(NSString *)path query:(NSString *)query title:(NSString *)title {
    NSURLComponents *urlPage = [NSURLComponents componentsWithString:path];
    urlPage.scheme = NSURLFileScheme;
    urlPage.query = query;
    
    ASCTabView * existTab = [self tabWithParam:@"url" value:[urlPage string]];
    
    if (existTab) {
        [self.tabsControl selectTab:existTab];
    } else {
        ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
        tab.title       = title;
        tab.type        = ASCTabViewTypePortal;
        tab.params      = [@{@"url" : [urlPage string]} mutableCopy];
        
        [self.tabsControl addTab:tab selected:YES];
    }
}

- (void)displayPortalTabBy:(NSString *)path {
    if (NSURL * url = [NSURL URLWithString:path]) {
        NSString * urlHost = [url host];
        BOOL isFoundPortal = false;

        // Search opened tab of a portal
        for (ASCTabView * tab in self.tabsControl.tabs) {
            if (tab.type == ASCTabViewTypePortal) {
                if (NSString * portalUrlString = tab.params[@"url"]) {
                    if (NSURL * portalURL = [NSURL URLWithString:portalUrlString]) {
                        NSString * portalHost = [portalURL host];

                        if ([portalHost isEqualToString:urlHost]) {
                            [self.tabsControl selectTab:tab];

                            if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
                                NSURLComponents *urlPage      = [NSURLComponents componentsWithString:path];
                                NSURLQueryItem *countryCode   = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
                                NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];

                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
                                }

                                NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
                                [qitems addObjectsFromArray:@[countryCode, portalAddress]];
                                urlPage.queryItems = qitems;

                                [cefView loadWithUrl:[urlPage string]];
                            }

                            isFoundPortal = true;
                            break;
                        }
                    }
                }
            }
        }

        // Force open tab of a portal if not exist
        if (!isFoundPortal) {
            id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
            NSURLComponents *urlPage      = [NSURLComponents componentsWithString:path];
            NSURLQueryItem *countryCode   = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
            NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];

            if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
            }

            NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
            [qitems addObjectsFromArray:@[countryCode, portalAddress]];
            urlPage.queryItems = qitems;

            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionOpenPortal),
                                                                         @"url"     : [urlPage string],
                                                                         @"active"  : @(YES)
                                                                         }];
        }
    }
}

- (void)openAcknowledgments {
    NSString *language = [[NSLocale preferredLanguages] firstObject];
    
    if (language) {
        if ([language length] > 1) {
            language = [language substringToIndex:2];
        }
        
        NSString *query = [NSString stringWithFormat:@"lang=%@", language.lowercaseString];
        [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"acknowledgments" ofType:@"html" inDirectory:@"login"]
                      query:query
                      title:NSLocalizedString(@"Acknowledgments", nil)];
    } else {
        [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"acknowledgments" ofType:@"html" inDirectory:@"login"]
                      title:NSLocalizedString(@"Acknowledgments", nil)];
    }
}

- (void)openEULA {
    [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"EULA" ofType:@"html"] title:NSLocalizedString(@"License Agreement", nil)];
}

- (void)openPreferences {
    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
    [self.tabsControl selectTab:nil];

    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(L"panel:select");
    pCommand->put_Param(L"settings");

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    [self.cefStartPageView apply:pEvent];
}

- (void)onOpenAppLink {
    if (NSArray<NSURL *> * links = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]) {
        [links enumerateObjectsUsingBlock:^(NSURL * _Nonnull link, NSUInteger idx, BOOL * _Nonnull stop) {
            ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
            tab.type        = ASCTabViewTypeOpening;
            tab.params      = [@{
                @"action": @(ASCTabActionOpenUrl),
                @"title": [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)],
                @"url": link.absoluteString
            } mutableCopy];

            [self.tabsControl addTab:tab selected:YES];
        }];
        
        [[ASCSharedSettings sharedInstance] setSetting:nil forKey:kSettingsOpenAppLinks];
    }
}

#pragma mark -
#pragma mark TouchBar

- (NSTouchBar *)makeTouchBar {
    if (@available(macOS 10.12.2, *)) {
        if (_touchBarController) {
            return [_touchBarController makeTouchBar];
        }
    }
    return nil;
}

- (void)onTouchBarItemTap:(id)sender senderId:(NSString *)senderId {
//    NSLog(@"onTouchBarItemTap: %@", senderId);

    if (senderId) {
        if ([senderId isEqualToString:kStartPageButtonIdentifier]) {
            [self onOnlyofficeButton:nil];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"document"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionCreateLocalFile),
                                                                         @"type"    : @(CEFDocumentDocument),
                                                                         @"active"  : @(YES)
                                                                         }];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"spreadsheet"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionCreateLocalFile),
                                                                         @"type"    : @(CEFDocumentSpreadsheet),
                                                                         @"active"  : @(YES)
                                                                         }];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"presentation"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionCreateLocalFile),
                                                                         @"type"    : @(CEFDocumentPresentation),
                                                                         @"active"  : @(YES)
                                                                         }];
        } else {
            ASCTabView * tab = [self.tabsControl tabWithUUID:senderId];
            if (tab) {
                [self.tabsControl selectTab:tab];
            }
        }
    }
}

#pragma mark -
#pragma mark Public

- (BOOL)shouldTerminateApplication {
    NSInteger unsaved = 0;
    BOOL preventTerminate = NO;

    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO)}];

    for (ASCTabView * tab in self.tabsControl.tabs) {
        if (tab.changed) {
            unsaved++;
        }

        // Blockchain check
        if (NSCefView * cefView = [self cefViewWithTab:tab]) {
            if ([cefView checkCloudCryptoNeedBuild]) {
                preventTerminate = YES;
            }
        }
    }

    if (preventTerminate) {
        return NO;
    }

    if (unsaved > 0) {
        NSString * productName = [ASCHelper appName];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %ld %@ documents with unconfirmed changes. Do you want to review these changes before quitting?", nil), (long)unsaved, productName]];
        [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
        [alert setAlertStyle:NSAlertStyleInformational];
        
        NSInteger result = [alert runModal];
        
        if (result == NSAlertFirstButtonReturn) {
            // "Review Changes..." clicked
            self.shouldTerminateApp = YES;
            
            NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
            for (ASCTabView * tab in tabs) {
                if (tab.changed) {
                    [self.tabsWithChanges addObject:tab];
                } else {
                    [self.tabsControl removeTab:tab selected:NO];
                }
            }
            
            [self safeCloseTabsWithChanges];
        } else if (result == NSAlertSecondButtonReturn) {
            // "Cancel" clicked
            return NO;
        } else {
            // "Delete and Quit" clicked
            self.shouldTerminateApp = YES;
            
            NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
            
            for (ASCTabView * tab in tabs) {
                [self.tabsControl removeTab:tab selected:NO];
            }
            
            [self.tabView selectTabViewItemWithIdentifier:rootTabId];
        }
        
        return NO;
    }
    
    return YES;
}

- (BOOL)shouldCloseMainWindow {
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO)}];
    
    if (self.tabsControl.tabs.count > 0) {
        ASCTabView * tab = [self.tabsControl selectedTab];
        
        if (tab) {
            if ([self tabs:self.tabsControl willRemovedTab:tab]) {
                [self.tabsControl removeTab:tab];
            }
            return NO;
        }
    }    
    return YES;
}

- (NSCefView *)cefViewWithTab:(ASCTabView *)tab {
    NSTabViewItem * item = nil;
    NSCefView * cefView = nil;
    
    if (tab) {
        item = [self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:tab.uuid]];
    } else {
        item = [self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:rootTabId]];
    }
    
    if (item) {
        for (NSView * view in item.view.subviews) {
            if ([view isKindOfClass:[NSCefView class]]) {
                cefView = (NSCefView *)view;
                break;
            }
        }
    }
    
    return cefView;
}

- (ASCTabView *)tabViewWithId:(int)viewId {
    return [self.tabsControl tabWithUUID:[NSString stringWithFormat:@"%d", viewId]];
}

#pragma mark -
#pragma mark Internal

- (ASCTabView *)tabWithParam:(NSString *)param value:(NSString *)value {
    if (param && value && [value isKindOfClass:[NSString class]] && value.length > 0) {
        for (ASCTabView * tab in self.tabsControl.tabs) {
            if (tab.params[param] && [tab.params[param] isKindOfClass:[NSString class]]) {
                NSString * localTabValue = [NSString stringWithString:tab.params[param]];
                NSString * localValue = [NSString stringWithString:value];

                if (NSString *provider = tab.params[@"provider"]) {
                    if ([@[@"asc", @"onlyoffice"] containsObject:provider]) {
                        if ([localTabValue isEqualToString:localValue]) {
                            return tab;
                        }
                        localValue = [localValue stringByReplacingOccurrencesOfString:@"/products/files/" withString:@""];
                    }
                }

                localTabValue = [localTabValue removeUrlQuery:@[@"lang", @"desktop"]];
                localValue = [localValue removeUrlQuery:@[@"lang", @"desktop"]];

                if (localTabValue && localTabValue.length > 0 && [localTabValue isEqualToString:localValue]) {
                    return tab;
                }
            }
        }
    }
    return nil;
}

- (void)saveLocalFileWithTab:(ASCTabView *)tab {
    if (tab) {
        NSDictionary * params   = tab.params;
        NSString * path         = params[@"path"];
        NSString * viewId       = params[@"viewId"];
        NSArray * formats       = params[@"supportedFormats"];
        
        [self.tabsControl selectTab:tab];
        
        __block NSInteger fileType = [params[@"fileType"] intValue];
        
        __block ASCSavePanelWithFormatController * saveController = [ASCSavePanelWithFormatController new];
        
        NSSavePanel * savePanel = [saveController savePanel];
        
        saveController.filters = formats;
        saveController.filterType = fileType;
        
        if (!path || path.length < 1) {
            path = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        BOOL isDir;
        if (![[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir]) {
            NSString * savedPath = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserLastSavePath];
            
            if (savedPath && savedPath.length > 0) {
                path = [savedPath stringByAppendingPathComponent:path];
            } else {
                path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject] stringByAppendingPathComponent:path];
            }
        }
        
        savePanel.directoryURL = [NSURL fileURLWithPath:[path stringByDeletingLastPathComponent]];
        savePanel.canCreateDirectories = YES;
        savePanel.nameFieldStringValue = [[path lastPathComponent] stringByDeletingPathExtension];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
            
            NSEditorApi::CAscLocalSaveFileDialog * saveData = new NSEditorApi::CAscLocalSaveFileDialog();
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            if (result == NSFileHandlingPanelOKButton) {
                [[NSUserDefaults standardUserDefaults] setObject:[[savePanel directoryURL] path] forKey:ASCUserLastSavePath];
                [[NSUserDefaults standardUserDefaults] synchronize];

                NSString * path = [NSString stringWithFormat:@"%@", [[savePanel URL] path]];
                                
                saveData->put_Path([path stdwstring]);
                saveData->put_Id([viewId intValue]);
                saveData->put_FileType((int)[saveController filterType]);
            } else {
                saveData->put_Id([viewId intValue]);
                saveData->put_Path(L"");
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
            pEvent->m_pData = saveData;
            
            appManager->Apply(pEvent);
        }];
    }
}

- (BOOL)canOpenFile:(NSString *)path tab:(ASCTabView *)tab {
    BOOL canOpen = NO;

    if (path) {
        NSURL * urlFile = [NSURL URLWithString:[path stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
        
        if (urlFile && [urlFile host]) {
            canOpen = YES;
        } else {
            int fileFormatType = CCefViewEditor::GetFileFormat([path stdwstring]);
            canOpen = (0 != fileFormatType);
        }
    }
    
    if (!canOpen) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
        [alert setMessageText:NSLocalizedString(@"File can not be open.", nil)];
        [alert setInformativeText:[NSString stringWithFormat:NSLocalizedString(@"File \"%@\" can not be open or not exist.", nil), path]];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert beginSheetModalForWindow:[NSApp mainWindow]  completionHandler:^(NSModalResponse returnCode) {
            if (tab) {
                [self.tabsControl removeTab:tab];
            }
        }];
    }
    
    return canOpen;
}

- (NSDictionary *)checkFiles:(NSDictionary *)fileList {
    NSMutableDictionary * checkedList = @{}.mutableCopy;
    
    for (NSString * key in fileList) {
        if (NSString * pathString = [fileList objectForKey:key]) {
            NSRange typeRange = [pathString rangeOfString:@"^https?://"
                                                  options:NSRegularExpressionSearch];

            if (typeRange.location != NSNotFound) {
                checkedList[key] = @"true";
            } else {
                checkedList[key] = [[NSFileManager defaultManager] fileExistsAtPath:pathString] ? @"true" : @"false";
            }
        }
    }
    
    return checkedList;
}

- (void)showHeaderPlaceholderWithIdentifier:(NSString *)uuid forType:(ASCTabViewType)type {
    NSInteger tabIndex = [self.tabView indexOfTabViewItemWithIdentifier:uuid];
    NSColor * headerColor = nil;

    switch (type) {
        case ASCTabViewTypeDocument:
            headerColor = [NSColor themedDocumentEditor];
            break;
        case ASCTabViewTypeSpreadsheet:
            headerColor = [NSColor themedSpreadsheetEditor];
            break;
        case ASCTabViewTypePresentation:
            headerColor = [NSColor themedPresentationEditor];
            break;
        default:
            break;
    }

    if (headerColor && self.headerView && tabIndex != NSNotFound) {
        NSTabViewItem * tabItem = [self.tabView tabViewItemAtIndex:tabIndex];

        if (tabItem) {
            // Remove dummy
            for (NSView * view in tabItem.view.subviews) {
                if (view.uuidTag == headerViewTag) {
                    [view removeFromSuperview];
                    break;
                }
            }

            NSView * headerView = [self.headerView duplicate];
            [tabItem.view addSubview:headerView];

            headerView.alphaValue = 1;
            headerView.uuidTag = headerViewTag;
            headerView.backgroundColor = headerColor;

            [headerView autoPinEdgeToSuperviewEdge:ALEdgeLeading];
            [headerView autoPinEdgeToSuperviewEdge:ALEdgeTop];
            [headerView autoPinEdgeToSuperviewEdge:ALEdgeTrailing];
            [headerView autoSetDimension:ALDimensionHeight toSize:56.0];
        }

        ASCTabView * tab = [self.tabsControl tabWithUUID:uuid];

        if (tab) {
            tab.isProcessing = true;
        }
    }
}

- (void)hideHeaderPlaceholderWithIdentifier:(NSString *)uuid {
    NSInteger tabIndex = [self.tabView indexOfTabViewItemWithIdentifier:uuid];
    ASCTabView * tab = [self.tabsControl tabWithUUID:uuid];

    if (tab) {
        tab.isProcessing = false;
    }

    if (tabIndex != NSNotFound) {
        NSTabViewItem * tabItem = [self.tabView tabViewItemAtIndex:tabIndex];

        if (tabItem) {
            for (NSView * view in tabItem.view.subviews) {
                if (view.uuidTag == headerViewTag) {
                    [NSAnimationContext runAnimationGroup:^(NSAnimationContext * _Nonnull context) {
                        context.duration = 0.3;
                        view.animator.alphaValue = 0;
                    } completionHandler:^{
                        [view removeFromSuperview];
                    }];
                    break;
                }
            }
        }
    }
}

- (void)requestSaveChangesForTab:(ASCTabView *)tab {
    if (tab && tab.changed) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Do you want to save the changes made to the document \"%@\"?", nil), tab.title]];
        [alert setInformativeText:NSLocalizedString(@"Your changes will be lost if you don’t save them.", nil)];
        [alert setAlertStyle:NSAlertStyleWarning];

        [self.tabsControl selectTab:tab];

        NSInteger returnCode = [alert runModalSheet];

        if(returnCode == NSAlertFirstButtonReturn) {
            NSCefView * cefView = [self cefViewWithTab:tab];

            tab.params[@"shouldClose"] = @(YES);

            if (cefView) {
                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
                [cefView apply:pEvent];
            }
        } else if (returnCode == NSAlertSecondButtonReturn) {
            [self.tabsControl removeTab:tab];
        } else if (returnCode == NSAlertThirdButtonReturn) {
            self.shouldTerminateApp = NO;
            self.shouldLogoutPortal = NO;
        }
    }
}

- (void)safeCloseTabsWithChanges {
    if ([[self tabsWithChanges] count] > 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self tabs:self.tabsControl willRemovedTab:[self.tabsWithChanges firstObject]];
        });
    }
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFCreateTab:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
        tab.title       = [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)];
        tab.type        = ASCTabViewTypeOpening;
        tab.params      = [params mutableCopy];

        ASCTabView * existTab = [self tabWithParam:@"url" value:params[@"url"]];
        
        if (!existTab) {
            existTab = [self tabWithParam:@"path" value:params[@"path"]];
        }
        
        [self.view.window makeKeyAndOrderFront:nil];
        
        if (existTab) {
            [self.tabsControl selectTab:existTab];
        } else {
            if ([params[@"action"] isEqualToNumber:@(ASCTabActionCreateLocalFile)]) {
                // Prevent add tab if necessary
            }
            
            if (params[@"external"]) {
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.9 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                    [self.tabsControl addTab:tab selected:[params[@"active"] boolValue]];
                });
            } else {
                [self.tabsControl addTab:tab selected:[params[@"active"] boolValue]];
            }
        }
    }
}

- (void)onCEFChangedTabEditorName:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
       
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
            [tab.params addEntriesFromDictionary:params];
            [self hideHeaderPlaceholderWithIdentifier:viewId];
        }
    }
}

- (void)onCEFChangedTabEditorType:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSInteger type          = [params[@"type"] integerValue];

        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];

        if (tab) {
            // Blockchain hook
            NSString * name = tab.params[@"name"];
            if (name && [name length] > 0) {
                return;
            }

            switch (type) {
                case CEFDocumentDocument:
                    [self showHeaderPlaceholderWithIdentifier:viewId forType:ASCTabViewTypeDocument];
                    break;
                case CEFDocumentSpreadsheet:
                    [self showHeaderPlaceholderWithIdentifier:viewId forType:ASCTabViewTypeSpreadsheet];
                    break;
                case CEFDocumentPresentation:
                    [self showHeaderPlaceholderWithIdentifier:viewId forType:ASCTabViewTypePresentation];
                    break;
                default:
                    break;
            }
        }
    }
}

- (void)onCEFSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab && tab.params[@"shouldClose"] && [tab.params[@"shouldClose"] boolValue]) {
            [self.tabsControl removeTab:tab];
        }
    }
}

- (void)onCEFSaveLocalFile:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];

        if (tab) {
            [tab.params addEntriesFromDictionary:params];
            [self saveLocalFileWithTab:tab];
            
            [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                     action:@"Save local file"
                                                                      label:nil
                                                                      value:nil];
        }
    }
}

- (void)onCEFOpenUrl:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * linkUrl = params[@"url"];
        
        if (linkUrl && linkUrl.length > 0) {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:linkUrl]];
        }
    }
}

- (void)onCEFFullscreen:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        BOOL isFullscreen = [params[@"fullscreen"] boolValue];
        NSTabViewItem * item = [self.tabView selectedTabViewItem];
        
        if (isFullscreen) {
            [item.view enterFullScreenMode:[[NSWindow titleWindowOrMain] screen] withOptions:@{NSFullScreenModeAllScreens: @(NO)}];

            ASCTabView * tab = [self.tabsControl selectedTab];
            
            if (tab) {
                NSCefView * cefView = [self cefViewWithTab:tab];
                [cefView focus];
            }
        } else if ([item.view isInFullScreenMode]) {
            [item.view exitFullScreenModeWithOptions:nil];
            
            ASCTabView * tab = [self.tabsControl selectedTab];
            
            if (tab) {
                NSCefView * cefView = [self cefViewWithTab:tab];
                
                if (cefView) {
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_Command(L"editor:stopDemonstration");
                    
                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                    pEvent->m_pData = pCommand;
                    
                    [cefView apply:pEvent];
                }
            }
        }
    }
}

- (void)onCEFKeyDown:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        NSValue * eventData = params[@"data"];
        
        if (eventData) {
//            NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)[eventData pointerValue];
//
//            int     keyCode     = pData->get_KeyCode();
//            bool    isCtrl      = pData->get_IsCtrl();
//            BOOL    isCommand   = pData->get_IsCommandMac();
//
//            if(isCtrl && keyCode == kVK_ANSI_W) {
//                [self tabs:self.tabsControl willRemovedTab:[self.tabsControl selectedTab]];
//            }
        }
    }
}

- (void)onCEFDownload:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        NSValue * eventData = params[@"data"];
        
        if (nil == appManager || nil == eventData)
            return;
        
        NSEditorApi::CAscDownloadFileInfo * pDownloadFileInfo = (NSEditorApi::CAscDownloadFileInfo *)[eventData pointerValue];
        
        NSString * idx = [NSString stringWithFormat:@"%d", pDownloadFileInfo->get_Id()];
        
        if (pDownloadFileInfo->get_IsComplete()) {
            [[ASCDownloadController sharedInstance] removeDownload:idx];
        } else if (pDownloadFileInfo->get_IsCanceled()) {
            [[ASCDownloadController sharedInstance] removeDownload:idx];
        } else {
            id download = [[ASCDownloadController sharedInstance] downloadWithId:idx];
            
            if (nil == download) {
                NSString * path = [NSString stringWithstdwstring:pDownloadFileInfo->get_FilePath()];
                NSString * fileName = NSLocalizedString(@"Unconfirmed", nil);
                
                if (path && [path length] > 0) {
                    fileName = [path lastPathComponent];
                }
                
                [[ASCDownloadController sharedInstance] addDownload:idx fileName:fileName];
            }
            
            [[ASCDownloadController sharedInstance] updateDownload:idx data:eventData];
        }
    }
}

- (void)onCEFStartSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * fileName = notification.userInfo[@"fileName"];
        NSNumber * idx      = notification.userInfo[@"idx"];
        
        NSSavePanel * savePanel = [NSSavePanel savePanel];
//        [savePanel setDirectoryURL:[NSURL URLWithString:[NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES) firstObject]]];
        if (fileName && fileName.length > 0) {
            [savePanel setAllowedFileTypes:@[fileName.pathExtension]];
            [savePanel setNameFieldStringValue:[fileName lastPathComponent]];
        }

        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
           
            if (result == NSFileHandlingPanelOKButton) {
                appManager->EndSaveDialog([[[savePanel URL] path] stdwstring], [idx unsignedIntValue]);
            } else {
                appManager->EndSaveDialog(L"", [idx unsignedIntValue]);
            }
        }];
    }
}

- (void)printOperationDidRun:(NSPrintOperation *)printOperation success:(BOOL)success contextInfo:(void *)contextInfo {
    if (m_pContext) {
        m_pContext->EndPaint();
    }
}

- (void)onCEFOnBeforePrintEnd:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSNumber * viewId       = notification.userInfo[@"viewId"];
        NSNumber * pagesCount   = notification.userInfo[@"countPages"];
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        if (appManager) {
            m_pContext = new NSAscPrinterContext(appManager);
            m_pContext->BeginPaint([viewId intValue], [pagesCount intValue], self, @selector(printOperationDidRun:success:contextInfo:));
        }
    }
}

- (void)onCEFOnOpenLocalFile:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"directory"];
        
        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        NSOpenPanel * openPanel = [NSOpenPanel openPanel];
        NSMutableArray * filter = [NSMutableArray array];
        [filter addObjectsFromArray:[ASCConstants documents]];
        [filter addObjectsFromArray:[ASCConstants spreadsheets]];
        [filter addObjectsFromArray:[ASCConstants presentations]];
        
        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.allowedFileTypes = filter;
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];

        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            if (result == NSFileHandlingPanelOKButton) {
                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                    object:nil
                                                                  userInfo:@{
                                                                             @"action"  : @(ASCTabActionOpenLocalFile),
                                                                             @"path"    : [[openPanel URL] path],
                                                                             @"active"  : @(YES)
                                                                             }];
            }
        }];
    }
}

- (void)onCEFOpenLocalImage:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"path"];
        NSInteger fileId = [notification.userInfo[@"fileId"] intValue];
        
        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        NSOpenPanel * openPanel = [NSOpenPanel openPanel];

        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.allowedFileTypes = [ASCConstants images];
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];
        
        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            NSEditorApi::CAscLocalOpenFileDialog * imageInfo = new NSEditorApi::CAscLocalOpenFileDialog();
            imageInfo->put_Id((int)fileId);
            
            if (result == NSFileHandlingPanelOKButton) {
                imageInfo->put_Path([[[openPanel URL] path] stdwstring]);
            } else {
                imageInfo->put_Path(L"");
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
            pEvent->m_pData = imageInfo;
            
            appManager->Apply(pEvent);
        }];
    }
}

- (void)onCEFOpenFileDialog:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"path"];
        NSString * fileTypes = notification.userInfo[@"filter"];
        NSInteger fileId = [notification.userInfo[@"fileId"] intValue];
        BOOL isMulti = [notification.userInfo[@"isMulti"] boolValue];

        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }

        NSArray * allowedFileTypes = @[];

        if ([fileTypes isEqualToString:CEFOpenFileFilterImage]) {
            allowedFileTypes = [ASCConstants images];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterAudio]) {
            allowedFileTypes = [ASCConstants audios];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterVideo]) {
            allowedFileTypes = [ASCConstants videos];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterPlugin]) {
            allowedFileTypes = [ASCConstants plugins];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterDocument]) {
            allowedFileTypes = [ASCConstants documents];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterSpreadsheet]) {
            allowedFileTypes = [ASCConstants spreadsheets];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterPresentation]) {
            allowedFileTypes = [ASCConstants presentations];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterCsvTxt]) {
            allowedFileTypes = [ASCConstants csvtxt];
        } else {
            // filters come in view "*.docx *.pptx *.xlsx"
            NSString * filters = [fileTypes stringByReplacingOccurrencesOfString:@"*." withString:@""];
            allowedFileTypes = [filters componentsSeparatedByString:@" "];
        }

        NSOpenPanel * openPanel = [NSOpenPanel openPanel];

        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];

        if (allowedFileTypes.count > 0) {
            openPanel.allowedFileTypes = allowedFileTypes;
        }

        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];

            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];

            NSEditorApi::CAscLocalOpenFileDialog * imageInfo = new NSEditorApi::CAscLocalOpenFileDialog();
            imageInfo->put_Id((int)fileId);
            imageInfo->put_Filter([fileTypes stdwstring]);
            
            if (result == NSFileHandlingPanelOKButton) {
                imageInfo->put_Path([[[openPanel URL] path] stdwstring]);
            } else {
                imageInfo->put_Path(L"");
            }
            
            if (isMulti) {
                imageInfo->put_IsMultiselect(true);
            }

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            pEvent->m_pData = imageInfo;

            appManager->Apply(pEvent);
        }];
    }
}

- (void)onCEFPortalLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        if (NSString *viewId = json[@"viewId"]) {
            if (ASCTabView * tab = [self.tabsControl tabWithUUID:viewId]) {
                if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                    if (NSString *infoString = json[@"info"]) {
                        if (NSMutableDictionary *info = [[infoString dictionary] mutableCopy]) {
                            if (NSString *originalUrl = [cefView originalUrl]) {
                                originalUrl = [[originalUrl stringByReplacingOccurrencesOfString:@"://" withString:@":////"]
                                               stringByReplacingOccurrencesOfString:@"//" withString:@"/"];

                                // Hotfix virtual path
                                if (NSString *provider = tab.params[@"provider"]) {
                                    if ([@[@"asc", @"onlyoffice"] containsObject:provider]) {
                                        originalUrl = [originalUrl stringByReplacingOccurrencesOfString:@"/products/files/" withString:@""];
                                    }
                                }
                                originalUrl = [originalUrl virtualUrl];

                                if (NSString * jsonString = [info jsonString]) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"portal:login");
                                    pCommand->put_Param([jsonString stdwstring]);

                                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;

                                    [self.cefStartPageView apply:pEvent];

                                    // Hotfix for SSO
                                    tab.params[@"url"] = originalUrl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
    
- (void)onCEFPortalLogout:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * url = json[@"domain"];
        BOOL isReload = [@"reload" isEqualToString:json[@"onsuccess"]];

        if (url) {
            self.shouldLogoutPortal = YES;

            NSMutableArray * portalTabs = [NSMutableArray array];
            NSInteger unsaved = 0;

            NSString *logoutVirtualUrl = [url virtualUrl];

            for (ASCTabView * tab in self.tabsControl.tabs) {
                NSString * tabVirtualUrl = [tab.params[@"url"] virtualUrl];

                if (tabVirtualUrl && tabVirtualUrl.length > 0 && [tabVirtualUrl rangeOfString:logoutVirtualUrl].location != NSNotFound) {
                    [portalTabs addObject:tab];

                    if (tab.changed) {
                        unsaved++;
                    }
                }
            }
            
            if (isReload) {
                for (ASCTabView * tab in portalTabs) {
                    if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                        [cefView reload];
                    }
                }
            } else if (unsaved > 0) {
                NSString * productName = [ASCHelper appName];

                NSAlert *alert = [[NSAlert alloc] init];
                [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
                [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
                [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
                [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %ld %@ documents with unconfirmed changes. Do you want to review these changes before quitting?", nil), (long)unsaved, productName]];
                [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
                [alert setAlertStyle:NSAlertStyleInformational];

                NSInteger result = [alert runModal];

                if (result == NSAlertFirstButtonReturn) {
                    // "Review Changes..." clicked

                    for (ASCTabView * tab in portalTabs) {
                        if (tab.changed) {
                            [self.tabsWithChanges addObject:tab];
                        } else {
                            [self.tabsControl removeTab:tab selected:NO];
                        }
                    }
                    
                    [self safeCloseTabsWithChanges];
                } else if (result == NSAlertSecondButtonReturn) {
                    return;
                } else {
                    // "Delete and Quit" clicked

                    for (ASCTabView * tab in portalTabs) {
                        [self.tabsControl removeTab:tab selected:NO];
                    }
                }
            } else {
                for (ASCTabView * tab in portalTabs) {
                    [self.tabsControl removeTab:tab selected:NO];
                }

                [self.tabView selectTabViewItemWithIdentifier:rootTabId];
            }
        }

        if (self.shouldLogoutPortal) {
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];

            appManager->Logout([url stdwstring]);
            if ( json[@"extra"] != nil ) {
                NSArray * urls = [json valueForKey:@"extra"];
                for ( NSString * u in urls ) {
                    appManager->Logout([u stdwstring]);
                }
            }

            NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
            pCommand->put_Command(L"portal:logout");
            pCommand->put_Param([url stdwstring]);

            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
            pEvent->m_pData = pCommand;

            [self.cefStartPageView apply:pEvent];
        }
    }
}
    
- (void)onCEFPortalCreate:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                        object:nil
                                                      userInfo:@{
                                                                 @"action"  : @(ASCTabActionOpenPortal),
                                                                 @"url"     : [ASCConstants appInfo:kRegistrationPortalUrl],
                                                                 @"title"   : NSLocalizedString(@"Create portal", nil),
                                                                 @"active"  : @(YES)
                                                                 }];
}

    
- (void)onCEFPortalNew:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * domainName = json[@"domain"];
        
        if (domainName && domainName.length > 0) {
            domainName = [[NSURL URLWithString:domainName] host];
        }
        
        ASCTabView * existTab = [self tabWithParam:@"url" value:[ASCConstants appInfo:kRegistrationPortalUrl]];
        
        if (existTab) {
            existTab.params[@"url"] = domainName;
            existTab.params[@"title"] = domainName;
            existTab.title = domainName;
            [self.tabsControl selectTab:existTab];
        }
    }
}

- (void)onCEFPortalSSO:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * portalUrl = json[@"portal"];
        NSString * providerUrl = json[@"provider"];

        if (portalUrl && providerUrl) {
            ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
            tab.title       = portalUrl;
            tab.type        = ASCTabViewTypePortal;
            tab.params      = [@{@"url" : [NSString stringWithFormat:@"sso:%@", providerUrl],
                                 @"provider": @"asc"
                                 } mutableCopy];

            [self.tabsControl addTab:tab selected:YES];
        }
    }
}

- (void)onCEFFileInFinder:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSURL * fileUrl = [NSURL fileURLWithPath:notification.userInfo[@"path"]];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[fileUrl]];
    }
}

- (void)onCEFFilesCheck:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id paths = notification.userInfo;
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            NSDictionary * checkedList = [self checkFiles:paths];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                if (NSString * jsonString = [checkedList jsonString]) {
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_Command(L"files:checked");
                    pCommand->put_Param([jsonString stdwstring]);

                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                    pEvent->m_pData = pCommand;

                    [self.cefStartPageView apply:pEvent];
                }
            });
        });
    }
}

- (void)onCEFSaveBeforeSign:(NSNotification *)notification {
    ASCTabView * tab = [self.tabsControl selectedTab];
    NSCefView * cefView = NULL;

    if (tab) {
        cefView = [self cefViewWithTab:tab];
    }

    if (NULL == cefView) {
        return;
    }

    NSAlert *alert = [NSAlert new];
    [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
    [[alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)] setKeyEquivalent:@"\e"];
    [alert setMessageText:NSLocalizedString(@"Before signing the document, it must be saved.", nil)];
    [alert setInformativeText:NSLocalizedString(@"Save the document?", nil)];
    [alert setAlertStyle:NSAlertStyleWarning];

    NSInteger returnCode = [alert runModalSheet];

    if (returnCode == NSAlertFirstButtonReturn) {
        NSEditorApi::CAscEditorSaveQuestion * pEventData = new NSEditorApi::CAscEditorSaveQuestion();
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);

        if (pEvent && pEventData) {
            pEventData->put_Value(true);
            pEvent->m_pData = pEventData;

            [cefView apply:pEvent];
        }
    }
}

- (void)onCEFStartPageReady:(NSNotification *)notification {
    
    NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme] ?: @"theme-classic-light";

    NSMutableDictionary * json_langs = @{
        @"uitheme": uiTheme
    }.mutableCopy;

    NSDictionary * langs = [ASCLinguist availableLanguages];
    if ( langs ) {
        [json_langs setObject:@{
                @"current": [ASCLinguist appLanguageCode],
                @"langs": langs,
                @"restart": @true
            } forKey:@"locale"];
    }

    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(L"settings:init");
    pCommand->put_Param([[json_langs jsonString] stdwstring]);

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;
    pEvent->AddRef();

    [self.cefStartPageView apply:pEvent];

    pCommand->put_Command(L"app:ready");
    pCommand->put_Param(L"");

    [self.cefStartPageView apply:pEvent];
    
    [self onOpenAppLink];
}

- (void)onCEFEditorDocumentReady:(NSNotification *)notification {
    //
}

- (void)onCEFEditorAppReady:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        if (NSString * viewId = json[@"viewId"]) {
            [self hideHeaderPlaceholderWithIdentifier:viewId];
        }
    }
}

- (void)onCEFEditorEvent:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        NSDictionary * data = json[@"data"];

        if (viewId && data) {
            NSString * action = data[@"action"];

            if ( action ) {
                if ( [action isEqualToString:@"file:close"] ) {
                    if (ASCTabView * tab = [self.tabsControl tabWithUUID:viewId]) {
                        [self.tabsControl removeTab:tab selected:NO];
                    }
                } else
                if ( [action isEqualToString:@"file:open"] ){
                    NSNotification * notification = [NSNotification notificationWithName: CEFEventNameOpenLocalFile
                                                                         object: nil
                                                                       userInfo: @{@"directory":@""}];
                    [self onCEFOnOpenLocalFile: notification];
                }
            }
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
            if (ASCTabView * tab = [self.tabsControl tabWithUUID:viewId]) {
                if ([action isEqualToString:@"close"]) {
                    [self.tabsControl removeTab:tab selected:NO];
                }
            }

            // Open start page or portal
            if (path) {
                if ([path isEqualToString:@"onlyoffice.com"]) { // onlyoffice.com equal "offline"
                    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
                    [self.tabsControl selectTab:nil];
                } else {
                    [self displayPortalTabBy:path];
                }
            }
        }
    }
}

- (void)onCEFEditorOpenFolder:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        NSString * path = json[@"path"];

        if (viewId && path) {
            if ([path isEqualToString:@"offline"]) {
                int cefViewId = [viewId intValue];
                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                CCefViewEditor * cefView = (CCefViewEditor *)appManager->GetViewById(cefViewId);

                if (cefView) {
                    NSString * urlString = [NSString stringWithstdwstring:cefView->GetLocalFilePath()];

                    if (urlString && urlString.length > 0) {
                        // Offline file is exist
                        if (NSURL * url = [NSURL fileURLWithPath:urlString]) {
                            if (NSURL * folder = [url URLByDeletingLastPathComponent]) {
                                [[NSWorkspace sharedWorkspace] openURL:folder];
                            }
                        }
                    } else {
                        // Offline file is new

                        NSAlert *alert = [NSAlert new];
                        [alert addButtonWithTitle:@"OK"];
                        [alert setMessageText:NSLocalizedString(@"Cannot open folder of the file location.", nil)];
                        [alert setInformativeText:NSLocalizedString(@"To open the file location, it must be saved.", nil)];
                        [alert setAlertStyle:NSAlertStyleWarning];

                        [alert runModalSheet];
                    }
                }
            } else {
                [self displayPortalTabBy:path];
            }
        }
    }
}

- (void)onCEFDocumentFragmentBuild:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        int error = [json[@"error"] intValue];

        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        if (tab) {
            if (error == 0) {
                [self.tabsControl removeTab:tab selected:YES];
            }
        }
    }
}

- (void)onCEFDocumentFragmented:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * viewId = json[@"viewId"];
        BOOL isFragmented = [json[@"isFragmented"] boolValue];

        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        if (tab) {
            if (isFragmented) {
                NSAlert * alert = [NSAlert new];
                [alert addButtonWithTitle:NSLocalizedString(@"Yes", nil)];
                [alert addButtonWithTitle:NSLocalizedString(@"No", nil)];
                [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
                [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"The document \"%@\" must be built. Continue?", nil), [tab title]]];
                [alert setAlertStyle:NSAlertStyleInformational];

                NSInteger returnCode = [alert runModalSheet];

                if (returnCode == NSAlertFirstButtonReturn) {
                    NSCefView * cefView = [self cefViewWithTab:tab];

                    if (cefView) {
                        self.shouldTerminateApp = NO;

                        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD);
                        [cefView apply:pEvent];
                        return;
                    }
                } else if (returnCode == NSAlertSecondButtonReturn) {
                    //
                } else {
                    return;
                }
            }

            [self.tabsControl removeTab:tab selected:YES];
        }
    }
}

- (void)onCEFCertificatePreview:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;

        NSString * text = json[@"text"];
        NSString * path = json[@"path"];
        
        if (path && path.length > 0) {
            ASCCertificateQLPreviewController * controller = [ASCCertificateQLPreviewController new];
            [controller previewBy:[NSURL fileURLWithPath:path]];
        } else if (text && text.length > 0) {
            ASCCertificatePreviewController * previewController = [[ASCCertificatePreviewController alloc] init:self];
            [previewController presentTextInfo:text];
        }
    }
}

- (void)onUIThemeChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        std::wstring theme = [params[@"uitheme"] stdwstring];
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];

        for (ASCTabView * tab in self.tabsControl.tabs) {
            if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                CCefView * cef = appManager->GetViewById((int)cefView.uuid);
                if (cef && cef->GetType() == cvwtEditor) {
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_FrameName(L"frameEditor");
                    pCommand->put_Command(L"uitheme:changed");
                    pCommand->put_Param(theme);

                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                    pEvent->m_pData = pCommand;

                    [cefView apply:pEvent];
                }
            }
        }
    }
}

#pragma mark -
#pragma mark ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {
    [[ASCSharedSettings sharedInstance] setSetting:tab forKey:kSettingsCurrentTab];
    
    if (tab) {
        [self.tabView selectTabViewItemWithIdentifier:tab.uuid];
    }
}

//- (void)tabs:(ASCTabsControl *)control didUpdateTab:(ASCTabView *)tab {
//    NSLog(@"didUpdateTab");
//}

- (void)tabs:(ASCTabsControl *)control didAddTab:(ASCTabView *)tab {
    if (tab.params) {
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        NSCefView * cefView = [[NSCefView alloc] initWithFrame:CGRectZero];
        
        ASCTabActionType action = (ASCTabActionType)[tab.params[@"action"] intValue];
        
        if (action == ASCTabActionOpenPortal) {
            [cefView create:appManager withType:cvwtSimple];
        } else {
            [cefView create:appManager withType:cvwtEditor];
        }
        
        [self.view.window makeKeyAndOrderFront:nil];
        
        switch (action) {
            case ASCTabActionOpenPortal: {
                tab.type = ASCTabViewTypePortal;
                
                NSString * newTitle = [[NSURL URLWithString:tab.params[@"url"]] host];
                NSString * provider = tab.params[@"provider"];
                
                if (newTitle && newTitle.length > 0) {
                    tab.title = [[NSURL URLWithString:tab.params[@"url"]] host];
                }

                if (provider && [provider length] > 0) {
                    [cefView setExternalCloud:provider];
                }
            }
            case ASCTabActionOpenUrl: {
                [cefView loadWithUrl:tab.params[@"url"]];
                
                if (tab.params[@"title"] && [tab.params[@"title"] length] > 0) {
                    tab.title = tab.params[@"title"];
                }
                
                break;
            }
                
            case ASCTabActionCreateLocalFile: {
                int docType = [tab.params[@"type"] intValue];
                
                NSString * docName = NSLocalizedString(@"Untitled", nil);
                
                switch (docType) {
                    case CEFDocumentDocument:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.docx", nil), ++documentNameCounter];
                        break;
                    case CEFDocumentSpreadsheet:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Spreadsheet %ld.xlsx", nil), ++spreadsheetNameCounter];
                        break;
                    case CEFDocumentPresentation:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Presentation %ld.pptx", nil), ++presentationNameCounter];
                        break;
                    case CEFDocumentForm:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.docxf", nil), ++documentNameCounter];
                        break;
                }
                
                [cefView createFileWithName:docName type:docType];
                break;
            }
                
            case ASCTabActionOpenLocalFile: {
                NSString * filePath = tab.params[@"path"];
                
                if ([self canOpenFile:filePath tab:tab]) {
                    int fileFormatType = CCefViewEditor::GetFileFormat([filePath stdwstring]);
                    [cefView openFileWithName:filePath type:fileFormatType];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Open local file"
                                                                              label:nil
                                                                              value:nil];
                }
                
                break;
            }
            case ASCTabActionOpenLocalRecoverFile: {
                NSInteger docId = [tab.params[@"fileId"] intValue];               
                [cefView openRecoverFileWithId:docId];
                
                [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                         action:@"Open local recover file"
                                                                          label:nil
                                                                          value:nil];
                
                break;
            }
            case ASCTabActionOpenLocalRecentFile: {
                NSInteger docId = [tab.params[@"fileId"] intValue];
                NSString * filePath = tab.params[@"path"];
                
                if ([self canOpenFile:filePath tab:tab]) {
                    [cefView openRecentFileWithId:docId];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Open local file"
                                                                              label:nil
                                                                              value:nil];
                }
                
                break;
            }

            default:
                break;
        }
        
        
        tab.uuid = [NSString stringWithFormat:@"%ld", (long)cefView.uuid];
        
        NSTabViewItem * item = [[NSTabViewItem alloc] initWithIdentifier:tab.uuid];
        item.label = tab.title;
        [self.tabView addTabViewItem:item];
        [item.view addSubview:cefView];
        [cefView autoPinEdgesToSuperviewEdges];
    }
}

- (BOOL)tabs:(ASCTabsControl *)control willRemovedTab:(ASCTabView *)tab {
    if (tab) {
        NSCefView * cefView = [self cefViewWithTab:tab];
        if (cefView && ([cefView checkCloudCryptoNeedBuild] || [cefView checkBuilding])) {
            self.shouldTerminateApp = NO;
            return NO;
        }

        if (tab.changed) {
            [self requestSaveChangesForTab:tab];
            return NO;
        }
    }
    return YES;
}

- (void)tabs:(ASCTabsControl *)control didRemovedTab:(ASCTabView *)tab {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    appManager->DestroyCefView([tab.uuid intValue]);

    NSCefView * cefView = [self cefViewWithTab:tab];

    if (cefView) {
        [cefView internalClean];
    }
    
    [self.tabView removeTabViewItem:[self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:tab.uuid]]];
    
    if ([self.tabsWithChanges containsObject:tab]) {
        [self.tabsWithChanges removeObject:tab];
    }
    if (self.tabsWithChanges.count > 0) {
        [self safeCloseTabsWithChanges];
    }
    
    if (self.shouldTerminateApp && self.tabsControl.tabs.count < 1) {
        [NSApp terminate:nil];
    }
}

- (void)tabs:(ASCTabsControl *)control didReorderTab:(ASCTabView *)tab from:(NSInteger)oldIndex to:(NSInteger)newIndex {
    //
}

#pragma mark -
#pragma mark ASCTitleBarController Delegate

- (void)onOnlyofficeButton:(id)sender {
    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
    [self.tabsControl selectTab:nil];
}

- (void)onShowUserInfoController:(NSViewController *)controller {
    ASCUserInfoViewController *userInfoController = (ASCUserInfoViewController *)controller;
    userInfoController.delegate = self;
}

#pragma mark -
#pragma mark ASCUserInfoViewController Delegate

- (void)onLogoutButton:(ASCUserInfoViewController *)controller {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSDictionary * userInfo = [[ASCHelper localSettings] valueForKey:ASCUserSettingsNameUserInfo];
    
    if (userInfo) {
        NSString * portal = userInfo[@"portal"];
        
        if (portal) {
            appManager->Logout([portal stdwstring]);
            
            [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                     action:@"Portal Logout"
                                                                      label:nil
                                                                      value:nil];
        }
    }
}

#pragma mark -
#pragma mark Debug

//- (IBAction)onAddTab:(id)sender {
//    ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
//    tab.title = [NSString stringWithFormat:@"Tab %lu", (unsigned long)rand() % 10000];
//    [self.tabsControl addTab:tab];
//}
//
//- (IBAction)onRemoveTab:(id)sender {
//    [self.tabsControl removeTab:[[self.tabsControl tabs] firstObject]];
//}
@end
