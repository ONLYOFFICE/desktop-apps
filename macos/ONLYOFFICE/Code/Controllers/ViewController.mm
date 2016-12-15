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
//  ViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ViewController.h"
#import "applicationmanager.h"
#import "mac_application.h"
#import "nsascprinter.h"
#import "ASCTabsControl.h"
#import "ASCTabView.h"
#import "ASCTitleWindowController.h"
#import "ASCHelper.h"
#import "ASCConstants.h"
#import "ASCUserInfoViewController.h"
#import "NSView+ASCView.h"
#import "NSAlert+SynchronousSheet.h"
#import "NSString+OnlyOffice.h"
#import "AppDelegate.h"
#import "NSCefView.h"
#import "ASCEventsController.h"
#import "NSString+OnlyOffice.h"
#import "ASCDownloadController.h"
#import "ASCSavePanelWithFormatController.h"
#import "ASCSharedSettings.h"
#import "ASCReplacePresentationAnimator.h"
#import "AnalyticsHelper.h"

#define rootTabId @"1CEF624D-9FF3-432B-9967-61361B5BFE8B"

@interface ViewController() <ASCTabsControlDelegate, ASCTitleBarControllerDelegate, ASCUserInfoViewControllerDelegate> {
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
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onWindowLoaded:)
                                                 name:ASCEventNameMainWindowLoaded
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFCreateTab:)
                                                 name:CEFEventNameCreateTab
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFChangedTabEditorName:)
                                                 name:CEFEventNameTabEditorNameChanged
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFSave:)
                                                 name:CEFEventNameSave
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFOpenUrl:)
                                                 name:CEFEventNameOpenUrl
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFFullscreen:)
                                                 name:CEFEventNameFullscreen
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFKeyDown:)
                                                 name:CEFEventNameKeyboardDown
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFDownload:)
                                                 name:CEFEventNameDownload
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFStartSave:)
                                                 name:CEFEventNameStartSaveDialog
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFOnBeforePrintEnd:)
                                                 name:CEFEventNamePrintDialog
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFOnOpenLocalFile:)
                                                 name:CEFEventNameOpenLocalFile
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFSaveLocalFile:)
                                                 name:CEFEventNameSaveLocal
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFOpenLocalImage:)
                                                 name:CEFEventNameOpenImage
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFPortalLogin:)
                                                 name:CEFEventNamePortalLogin
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFPortalLogout:)
                                                 name:CEFEventNamePortalLogout
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFPortalCreate:)
                                                 name:CEFEventNamePortalCreate
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFPortalNew:)
                                                 name:CEFEventNamePortalNew
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFFileInFinder:)
                                                 name:CEFEventNameFileInFinder
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFFilesCheck:)
                                                 name:CEFEventNameFilesCheck
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFStartPageReady:)
                                                 name:CEFEventNameStartPageReady
                                               object:nil];
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
        
        [self setupTabControl];
        [self createStartPage];
        [self loadStartPage];
        
        // Create CEF event listener
        [ASCEventsController sharedInstance];
    }
}

- (void)setupTabControl {
    self.tabsControl.minTabWidth = 48;
    self.tabsControl.maxTabWidth = 135;
    
    [self.tabsControl.multicastDelegate addDelegate:self];
}

- (void)createStartPage {
    NSInteger rootTabIndex = [self.tabView indexOfTabViewItemWithIdentifier:rootTabId];
    
    if (rootTabIndex != NSNotFound) {
        NSTabViewItem * tab = [self.tabView tabViewItemAtIndex:rootTabIndex];
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        self.cefStartPageView = [[NSCefView alloc] initWithFrame:tab.view.frame];
        [self.cefStartPageView create:appManager withType:cvwtSimple];
        [tab.view addSubview:self.cefStartPageView];
        [self.cefStartPageView setupFillConstraints];
    }
}

- (void)loadStartPage {
    if (self.cefStartPageView ) {
        NSUserDefaults *preferences     = [NSUserDefaults standardUserDefaults];
        NSURLComponents *loginPage      = [NSURLComponents componentsWithString:[[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"login"]];
        NSURLQueryItem *countryCode     = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
        NSURLQueryItem *portalAddress   = [NSURLQueryItem queryItemWithName:@"portal" value:[preferences objectForKey:ASCUserSettingsNamePortalUrl]];
        loginPage.queryItems            = @[countryCode, portalAddress];
        loginPage.scheme                = NSURLFileScheme;
        
        [self.cefStartPageView loadWithUrl:[loginPage string]];
    }
}

- (void)openLocalPage:(NSString *)path title:(NSString *)title {
    NSURLComponents *urlPage = [NSURLComponents componentsWithString:path];
    urlPage.scheme = NSURLFileScheme;
    
    ASCTabView * existTab = [self tabWithParam:@"url" value:[urlPage string]];
    
    if (existTab) {
        [self.tabsControl selectTab:existTab];
    } else {
        ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
        tab.title       = title;
        tab.type        = ASCTabViewPortal;
        tab.params      = [@{@"url" : [urlPage string]} mutableCopy];
        
        [self.tabsControl addTab:tab selected:YES];
    }
}

- (void)openAcknowledgments {
    [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"acknowledgments" ofType:@"html" inDirectory:@"login"] title:NSLocalizedString(@"Acknowledgments", nil)];
}

- (void)openEULA {
    [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"EULA" ofType:@"html"] title:NSLocalizedString(@"License Agreement", nil)];
}

#pragma mark -
#pragma mark Public

- (BOOL)shouldTerminateApplication {
    NSInteger unsaved = 0;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO)}];
    
    for (ASCTabView * tab in self.tabsControl.tabs) {
        if (tab.changed) {
            unsaved++;
        }
    }
    
    if (unsaved > 0) {
        NSString * productName = [ASCHelper appName];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %ld %@ documents with unconfirmed changes. Do you want to review these changes before quitting?", nil), (long)unsaved, productName]];
        [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
        [alert setAlertStyle:NSInformationalAlertStyle];
        
        NSInteger result = [alert runModal];
        
        if (result == NSAlertFirstButtonReturn) {
            // "Review Changes..." clicked
            self.shouldTerminateApp = YES;
            
            NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
            for (ASCTabView * tab in tabs) {
                if (tab.changed) {
                    [self tabs:self.tabsControl willRemovedTab:tab];
                } else {
                    [self.tabsControl removeTab:tab selected:NO];
                }
            }
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

#pragma mark -
#pragma mark Internal

- (ASCTabView *)tabWithParam:(NSString *)param value:(NSString *)value {
    if (param && value && value.length > 0) {
        for (ASCTabView * tab in self.tabsControl.tabs) {
            NSString * tabValue = tab.params[param];
            
            if (tabValue && tabValue.length > 0 && [tabValue isEqualToString:value]) {
                return tab;
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
        NSArray * formats       = params[@"suppertFormats"];
        
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
        [alert setAlertStyle:NSCriticalAlertStyle];
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
        id value = [fileList objectForKey:key];
        checkedList[key] = [[NSFileManager defaultManager] fileExistsAtPath:value] ? @"true" : @"false";
    }
    
    return checkedList;
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFCreateTab:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
        tab.title       = [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)];
        tab.type        = ASCTabViewOpeningType;
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
            [item.view enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
            
            ASCTabView * tab = [self.tabsControl selectedTab];
            
            if (tab) {
                NSCefView * cefView = [self cefViewWithTab:tab];
                [cefView focus];
            }
        } else {
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
        openPanel.directoryURL = [NSURL URLWithString:directory];

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
        openPanel.directoryURL = [NSURL URLWithString:directory];
        
        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            if (result == NSFileHandlingPanelOKButton) {
                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                
                NSEditorApi::CAscLocalOpenFileDialog * imageInfo = new NSEditorApi::CAscLocalOpenFileDialog();
                imageInfo->put_Id((int)fileId);
                imageInfo->put_Path([[[openPanel URL] path] stdwstring]);
                
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
                pEvent->m_pData = imageInfo;
                
                appManager->Apply(pEvent);
            }
        }];
    }
}

- (void)onCEFPortalLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSError * err;
        NSData * jsonData = [NSJSONSerialization dataWithJSONObject:notification.userInfo options:0 error:&err];
        NSString * jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        
        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
        pCommand->put_Command(L"portal:login");
        pCommand->put_Param([[jsonString stringByReplacingOccurrencesOfString:@"\"" withString:@"&quot;"] stdwstring]); // ¯\_(ツ)_/¯
        
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;
        
        [self.cefStartPageView apply:pEvent];
    }
}
    
- (void)onCEFPortalLogout:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * url = notification.userInfo[@"url"];
        
        if (url) {
            self.shouldLogoutPortal = YES;
            
            NSMutableArray * portalTabs = [NSMutableArray array];
            NSInteger unsaved = 0;
            
            for (ASCTabView * tab in self.tabsControl.tabs) {
                NSString * tabUrl = tab.params[@"url"];
                
                if (tabUrl && tabUrl.length > 0 && [tabUrl rangeOfString:url].location != NSNotFound) {
                    [portalTabs addObject:tab];
                    
                    if (tab.changed) {
                        unsaved++;
                    }
                }
            }
            
            if (unsaved > 0) {
                NSString * productName = [ASCHelper appName];
                
                NSAlert *alert = [[NSAlert alloc] init];
                [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
                [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
                [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
                [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %ld %@ documents with unconfirmed changes. Do you want to review these changes before quitting?", nil), (long)unsaved, productName]];
                [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
                [alert setAlertStyle:NSInformationalAlertStyle];
                
                NSInteger result = [alert runModal];
                
                if (result == NSAlertFirstButtonReturn) {
                    // "Review Changes..." clicked
                    
                    for (ASCTabView * tab in portalTabs) {
                        if (tab.changed) {
                            [self tabs:self.tabsControl willRemovedTab:tab];
                        } else {
                            [self.tabsControl removeTab:tab selected:NO];
                        }
                    }
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
                                                                 @"url"     : kRegistrationPortalUrl,
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
        
        ASCTabView * existTab = [self tabWithParam:@"url" value:kRegistrationPortalUrl];
        
        if (existTab) {
            existTab.params[@"url"] = domainName;
            existTab.params[@"title"] = domainName;
            existTab.title = domainName;
            [self.tabsControl selectTab:existTab];
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
                NSError * err;
                NSData * jsonData = [NSJSONSerialization  dataWithJSONObject:checkedList options:0 error:&err];
                NSString * jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
                
                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"files:checked");
                pCommand->put_Param([[jsonString stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""] stdwstring]); // ¯\_(ツ)_/¯
                
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                pEvent->m_pData = pCommand;
                
                [self.cefStartPageView apply:pEvent];

            });
        });
    }
}

- (void)onCEFStartPageReady:(NSNotification *)notification {
    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(L"app:ready");
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;
    
    [self.cefStartPageView apply:pEvent];
}


#pragma mark -
#pragma mark ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {
    [[ASCSharedSettings sharedInstance] setSetting:tab forKey:kSettingsCurrentTab];
    
    if (tab) {
        [self.tabView selectTabViewItemWithIdentifier:tab.uuid];
    }
}

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
                tab.type = ASCTabViewPortal;
                
                NSString * newTitle = [[NSURL URLWithString:tab.params[@"url"]] host];
                
                if (newTitle && newTitle.length > 0) {
                    tab.title = [[NSURL URLWithString:tab.params[@"url"]] host];
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
                    case 0: docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.docx", nil), ++documentNameCounter]; break;
                    case 1: docName = [NSString stringWithFormat:NSLocalizedString(@"Presentation %ld.pptx", nil), ++presentationNameCounter]; break;
                    case 2: docName = [NSString stringWithFormat:NSLocalizedString(@"Spreadsheet %ld.xlsx", nil), ++spreadsheetNameCounter];   break;
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
        [cefView setupFillConstraints];
    }
}

- (BOOL)tabs:(ASCTabsControl *)control willRemovedTab:(ASCTabView *)tab {
    if (tab && tab.changed) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Yes", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"No", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Do you want to save the changes made to the document \"%@\"?", nil), tab.title]];
        [alert setInformativeText:NSLocalizedString(@"Your changes will be lost if you don’t save them.", nil)];
        [alert setAlertStyle:NSWarningAlertStyle];
        
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
            [control removeTab:tab];
        } else if (returnCode == NSAlertThirdButtonReturn) {
            self.shouldTerminateApp = NO;
            self.shouldLogoutPortal = NO;
        }
        
        return NO;
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
    
    if (self.shouldTerminateApp && self.tabsControl.tabs.count < 1) {
        [NSApp terminate:nil];
    }
}

- (void)tabs:(ASCTabsControl *)control didReorderTab:(ASCTabView *)tab {
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
