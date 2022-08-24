/*
 * (c) Copyright Ascensio System SIA 2010-2022
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
//  ASCThemesController.m
//  ONLYOFFICE
//
//  Created by Maxim.Kadushkin on 23/08/2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#import "ASCThemesController.h"
#import "ASCConstants.h"
#import "ASCSharedSettings.h"
#import "NSColor+Extensions.h"
#import "NSApplication+Extensions.h"


@implementation ASCThemesController

+ (NSString*)currentThemeId {
    return [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
}

+ (BOOL)isCurrentThemeDark {
    NSString * theme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
    if ([uiThemeSystem isEqualToString:theme]) {
        return [@"dark" isEqualToString:[[ASCSharedSettings sharedInstance] settingByKey:kSettingsColorScheme]];
    } else return [uiThemeDark isEqualToString:theme] || [uiThemeContrastDark isEqualToString:theme];
}

+ (NSString*)defaultThemeId:(BOOL)isdark {
    return isdark ? uiThemeDark : uiThemeClassicLight;
}

+ (NSColor*)currentThemeColor:(NSString*)name {
    return [self color:name forTheme:[self currentThemeId]];
}

+ (NSColor*)color:(NSString*)name forTheme:(NSString*)theme {
    if ( [theme isEqualToString: uiThemeSystem] )
        theme = [self defaultThemeId:[NSApplication isSystemDarkMode]];

    if ([name isEqualToString:btnPortalActiveBackgroundColor]) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x333333);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x1e1e1e);
        else {
            if ( @available(macOS 10.13, *) )
                return [NSColor colorNamed:@"tab-portal-activeColor"];
            else return kColorRGBA(255, 255, 255, 1.0);
        }
    } else if ([name isEqualToString:tabWordActiveBackgroundColor]) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x2a2a2a);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x1e1e1e);
        else return [NSColor brendDocumentEditor];
    } else if ([name isEqualToString:tabCellActiveBackgroundColor]) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x2a2a2a);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x1e1e1e);
        else return [NSColor brendSpreadsheetEditor];
    } else if ([name isEqualToString:tabSlideActiveBackgroundColor]) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x2a2a2a);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x1e1e1e);
        else return [NSColor brendPresentationEditor];
    }

    return NULL;
}

@end
