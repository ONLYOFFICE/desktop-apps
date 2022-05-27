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
//  main.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "mac_application.h"
#include "ASCApplicationManager.h"
#import "NSString+Extensions.h"
#import "ASCConstants.h"
#import "ASCHelper.h"
#import "ASCLinguist.h"
#import "ASCDocSignController.h"
#import "ASCExternalController.h"
#import "NSApplication+Extensions.h"

CAscApplicationManager * createASCApplicationManager() {
    return new ASCApplicationManager();
}

int main(int argc, const char * argv[]) {
//    return NSApplicationMain(argc, argv);
    
    [ASCHelper createCloudPath];
    [ASCLinguist init];

    NSAscApplicationWorker * worker = [[NSAscApplicationWorker alloc] initWithCreator:createASCApplicationManager];
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];

    // setup common user directory
    appManager->m_oSettings.SetUserDataPath([[ASCHelper applicationDataPath] stdwstring]);
    
    // setup Editors directory
    appManager->m_oSettings.local_editors_path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"editors/web-apps/apps/api/documents/index.html"] stdwstring];
    
    appManager->m_oSettings.system_plugins_path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"editors/sdkjs-plugins"] stdwstring];
    
    // setup Dictionary directory
    appManager->m_oSettings.spell_dictionaries_path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"dictionaries"] stdwstring];
    
    // setup Recovery directory
    appManager->m_oSettings.recover_path = [[ASCHelper recoveryDataPath] stdwstring];
    
    // setup Converter directory
    appManager->m_oSettings.file_converter_path = [[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"converter"] stdwstring];
    
    // setup editor fonts directory
    std::vector<std::wstring> fontsDirectories;
    fontsDirectories.push_back([[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"login/fonts"] stdwstring]);
    appManager->m_oSettings.additional_fonts_folder = fontsDirectories;
    
    // setup localization
    NSMutableArray * params = [NSMutableArray array];
    [params addObject:[NSString stringWithFormat:@"lang=%@", [ASCLinguist appLanguageCode]]];

    // setup username
    NSString * fullName = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserNameApp];
    
    if (fullName == nil) {
        fullName = NSFullUserName();
    }
    
    if (fullName) {
        [params addObject:[NSString stringWithFormat:@"username=%@", fullName]];
    }
    
    // setup ui theme
    NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
    if ( !uiTheme ) {
        uiTheme = [NSApplication isSystemDarkMode] ? uiThemeDark : uiThemeClassicLight;
        [[NSUserDefaults standardUserDefaults] setObject:uiTheme forKey:ASCUserUITheme];
    }

    [params addObject:[NSString stringWithFormat:@"uitheme=%@", uiTheme]];

    std::wstring wLocale = [[params componentsJoinedByString:@"&"] stdwstring];
    appManager->InitAdditionalEditorParams(wLocale);

    // setup doc sign
    [ASCDocSignController shared];

    //[worker Start:argc :argv];
    [worker Start:argc argv:argv];
    int result = NSApplicationMain(argc, argv);
    [worker End];
    return result;
}
