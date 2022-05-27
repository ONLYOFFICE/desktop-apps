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
//  AppDelegate.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "AppDelegate.h"
#import "ASCConstants.h"
#import "ASCCommonViewController.h"
#import "ASCSharedSettings.h"
#import "ASCTabView.h"
#import "NSString+Extensions.h"
#import "NSCefView.h"
#import "ASCHelper.h"
#import "AnalyticsHelper.h"
#import "ASCExternalController.h"

#ifndef _MAS
    #import "PFMoveApplication.h"
#endif

@interface AppDelegate ()
@property (weak) IBOutlet NSMenuItem *updateMenuItem;
@property (weak) IBOutlet NSMenuItem *eulaMenuItem;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
#ifndef _MAS
    PFMoveToApplicationsFolderIfNecessary();
#endif
    
    // Remove 'Start Dictation' and 'Special Characters' from menu
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledCharacterPaletteMenuItem"];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    
    // Google Analytics
    
#ifdef _PRODUCT_ONLYOFFICE
//    [[AnalyticsHelper sharedInstance] beginPeriodicReportingWithAccount:@"UA-XXXXXXXXX-1"
//                                                                   name:@"ONLYOFFICE"
//                                                                version:[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"]];
#endif
    
    
//    // Hotkey conflict resolve
//    [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
//        NSEventModifierFlags flags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
//        if((flags - (NSShiftKeyMask | NSControlKeyMask) == 0) && ([event keyCode] == 2)) { // Shift+Ctrl+D
//            //
//        }
//        
//        return event;
//    }];

    NSArray * arguments = [[NSProcessInfo processInfo] arguments];
    for (NSString * arg in arguments) {
        if ( [arg hasPrefix:@"--new:"] || [arg hasPrefix:@"--new="] ) {
            NSString * param = [arg substringFromIndex:6];
            NSLog(@"input arg new: %@", param);

            if ( [@[@"word",@"cell",@"slide",@"form"] containsObject:param] ) {
                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                    object:nil
                                                                  userInfo:@{ @"action"  : @(ASCTabActionCreateLocalFile),
                                                                              @"type"    : param,
                                                                              @"active"  : @(YES) }];
            }
        }
    }
}

/// If your delegate implements this method, AppKit does not call the application(_:openFile:)
/// or application(_:openFiles:) methods.
- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls {

    /// Handle links
    
    NSMutableArray<NSURL *> * appLinks = @[].mutableCopy;
    
    [urls enumerateObjectsUsingBlock:^(NSURL * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([obj.scheme isEqualToString:kSchemeApp]) {
            [appLinks addObject:obj];
        }
    }];
    
    if (appLinks.count > 0) {
        if (NSArray<NSURL *> * storedAppLinks = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]) {
            NSMutableArray * extendArrayLinks = [storedAppLinks mutableCopy];
            [extendArrayLinks addObjectsFromArray:appLinks];
            [[ASCSharedSettings sharedInstance] setSetting:extendArrayLinks forKey:kSettingsOpenAppLinks];
        } else {
            [[ASCSharedSettings sharedInstance] setSetting:appLinks forKey:kSettingsOpenAppLinks];
        }
        
        [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameOpenAppLinks
                                                            object:[[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]
                                                          userInfo:nil];
    }
    
    /// Handle files
    
    NSMutableArray<NSString *> * fileNames = @[].mutableCopy;
    
    for (NSURL * url in urls) {
        if ([url isFileURL]) {
            [fileNames addObject:[url path]];
        }
    }
    
    [self application:application openFiles:fileNames];
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename {
    return YES;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray<NSString *> *)filenames {
    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onShouldOpenFile:)]) {
        NSMutableArray<NSString *> * processedFileList = [NSMutableArray array];

        for (NSString * filePath in filenames) {
            if ([externalDelegate onShouldOpenFile:filePath]) {
                [processedFileList addObject:filePath];
            }
        }

        filenames = processedFileList;
    }

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        for (NSString * filePath in filenames) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionOpenLocalFile),
                                                                         @"path"    : filePath,
                                                                         @"active"  : @(YES),
                                                                         @"external": @(YES)
                                                                         }];
        }
    });
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application

#ifdef _PRODUCT_ONLYOFFICE
    [[AnalyticsHelper sharedInstance] handleApplicationWillClose];
    [[NSUserDefaults standardUserDefaults] synchronize];
#endif
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)theApplication {
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    
    if (mainWindow) {
        ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
        
        return [controller shouldTerminateApplication] ? NSTerminateNow : NSTerminateCancel;
    }

    return NSTerminateNow;
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {
#ifdef _MAS
    [self.updateMenuItem setHidden:YES];
    [self.eulaMenuItem setHidden:YES];
#endif
    
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    NSString * productName = [ASCHelper appName];
    
    if ([item action] == @selector(onMenuAbout:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuHide:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"Hide %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuQuit:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"Quit %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuNew:)) {
        return YES;
    } else if ([item action] == @selector(onMenuOpen:)) {
        return YES;
    } else if ([item action] == @selector(onMenuSave:)) {
        return nil != tab;
    } else if ([item action] == @selector(onMenuSaveAs:)) {
        return nil != tab;
    } else if ([item action] == @selector(onMenuPrint:)) {
        return nil != tab;
    } else if ([item action] == @selector(onShowHelp:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"%@ Help", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuAcknowledgments:)) {
        return YES;
    } else if ([item action] == @selector(onMenuEULA:)) {
        return YES;
    } else if ([item action] == @selector(onPreferences:)) {
        return YES;
    }
    
    return [super validateMenuItem:item];
}

#pragma mark -
#pragma mark Menu

- (IBAction)onShowHelp:(NSMenuItem *)sender {
    NSString * langCode = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];
    NSString * helpUrl  = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @""];

    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
        langCode = [externalDelegate onAppPreferredLanguage];
    }

    if ([@"ru" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"ru/"];
    } else if ([@"de" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"de/"];
    } else if ([@"fr" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"fr/"];
    } else if ([@"es" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"es/"];
    }
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:helpUrl]];
}

- (IBAction)onMenuNew:(NSMenuItem *)sender {
    if (100 == sender.tag) {
        return;
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                        object:nil
                                                      userInfo:@{
                                                                 @"action"  : @(ASCTabActionCreateLocalFile),
                                                                 @"type"    : @(sender.tag),
                                                                 @"active"  : @(YES)
                                                                 }];
}

- (IBAction)onMenuOpen:(NSMenuItem *)sender {
    NSString * directiry = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsLastOpenDirectory];
    
    if (!directiry || directiry.length < 1) {
        directiry = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenLocalFile
                                                        object:nil
                                                      userInfo:@{
                                                                 @"directory": directiry
                                                                 }];
}

- (IBAction)onMenuSave:(NSMenuItem *)sender {
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    NSCefView * cefView = [controller cefViewWithTab:tab];
    
    if (cefView) {
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuSaveAs:(NSMenuItem *)sender {
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    NSCefView * cefView = [controller cefViewWithTab:tab];
    
    if (cefView) {
        NSEditorApi::CAscEditorExecuteCommand * pData = new NSEditorApi::CAscEditorExecuteCommand();
        pData->put_Command(L"saveAs");
        
        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
        pEvent->m_pData = pData;
        
        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuPrint:(NSMenuItem *)sender {
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    NSCefView * cefView = [controller cefViewWithTab:tab];
    
    if (cefView) {
        NSEditorApi::CAscEditorExecuteCommand * pData = new NSEditorApi::CAscEditorExecuteCommand();
        pData->put_Command(L"print");
        
        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
        pEvent->m_pData = pData;
        
        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuAcknowledgments:(NSMenuItem *)sender {
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openAcknowledgments];
}

- (IBAction)onMenuEULA:(NSMenuItem *)sender {
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openEULA];
}

- (IBAction)onMenuAbout:(NSMenuItem *)sender {
//    [NSApp orderFrontStandardAboutPanel:sender];
    NSWindow * mainWindow = [NSApp mainWindow];
    
    if (mainWindow) {
        ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
        
        NSWindowController * windowController = [controller.storyboard instantiateControllerWithIdentifier:@"ASCAboutWindowControllerId"];
        [NSApp runModalForWindow:windowController.window];
    }
}

- (IBAction)onPreferences:(NSMenuItem *)sender {
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openPreferences];
}

- (IBAction)onMenuHide:(NSMenuItem *)sender {
    [NSApp hide:sender];
}

- (IBAction)onMenuQuit:(NSMenuItem *)sender {
    [NSApp terminate:sender];
}

@end
