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
//  AppDelegate.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "AppDelegate.h"
#import "ASCConstants.h"
#import "ViewController.h"
#import "ASCSharedSettings.h"
#import "ASCTabView.h"
#import "NSString+OnlyOffice.h"
#import "NSCefView.h"

#ifndef MAS
    #import "PFMoveApplication.h"
#endif

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
#ifndef MAS
    PFMoveToApplicationsFolderIfNecessary();
#endif
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename {
    return YES;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray<NSString *> *)filenames {
    for (NSString * filePath in filenames) {
        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                            object:nil
                                                          userInfo:@{
                                                                     @"action"  : @(ASCTabActionOpenLocalFile),
                                                                     @"file"    : filePath,
                                                                     @"active"  : @(YES)
                                                                     }];
    }
}
    
- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)theApplication {
    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    
    if (mainWindow) {
        ViewController * controller = (ViewController *)mainWindow.contentViewController;
        
        return [controller shouldTerminateApplication] ? NSTerminateNow : NSTerminateCancel;
    }

    return NSTerminateNow;
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    
    if ([item action] == @selector(onMenuNew:)) {
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
        return YES;
    }
    
    return [super validateMenuItem:item];
}

#pragma mark -
#pragma mark Menu

- (IBAction)onShowHelp:(NSMenuItem *)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://helpcenter.onlyoffice.com/ONLYOFFICE-Editors/index.aspx"]];
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
    ViewController * controller = (ViewController *)mainWindow.contentViewController;
    NSCefView * cefView = [controller cefViewWithTab:tab];
    
    if (cefView) {
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuSaveAs:(NSMenuItem *)sender {
}

- (IBAction)onMenuPrint:(NSMenuItem *)sender {
}

@end
