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
//  ASCLinguist.m
//  ONLYOFFICE
//
//  Created by Maxim Kadushkin on 03.02.2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#import "ASCLinguist.h"
#import "ASCConstants.h"

@implementation ASCLinguist

static BOOL uiLayoutDirectionRTL = NO;

+ (void)init {
    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:[ASCLinguist appLanguageCode]] forKey:@"AppleLanguages"];
    [[NSUserDefaults standardUserDefaults] setObject:[ASCLinguist appLanguageCode] forKey:@"AppleLocale"];
    [[NSUserDefaults standardUserDefaults] synchronize];

//    bool l = [NSLocale characterDirectionForLanguage:[ASCLinguist appLanguageCode]] == NSLocaleLanguageDirectionRightToLeft;
    NSString * direction = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserUILayoutDirection];
    if ( direction != nil )
        uiLayoutDirectionRTL = [direction isEqualToString:@"rtl"];
    
    if ( uiLayoutDirectionRTL ) {
        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"AppleTextDirection"];
        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"NSForceRightToLeftWritingDirection"];

        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"NSForceLeftToRightWritingDirection"];
    } else {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"AppleTextDirection"];
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"NSForceRightToLeftWritingDirection"];

        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"NSForceLeftToRightWritingDirection"];
    }
}

+ (NSString *)appLanguageCode {
    NSString * uiLang = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserUILanguage];
    if ( !uiLang )
        uiLang = [[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode];

    if ( [uiLang length] < 3 )
        uiLang = [NSString stringWithFormat:@"%@-%@", [uiLang lowercaseString], [uiLang uppercaseString]];

    return uiLang;
}

+ (void)setAppLanguageCode:(NSString *)langCode {
    [[NSUserDefaults standardUserDefaults] setObject:langCode forKey:ASCUserUILanguage];
    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:langCode] forKey:@"AppleLanguages"];
    [[NSUserDefaults standardUserDefaults] setObject:langCode forKey:@"AppleLocale"];

    [[NSUserDefaults standardUserDefaults] synchronize];
}

+ (void)setUILayoutDirectionRtl:(BOOL)value {
    if ( value )
        [[NSUserDefaults standardUserDefaults] setObject:@"rtl" forKey:ASCUserUILayoutDirection];
    else [[NSUserDefaults standardUserDefaults] removeObjectForKey:ASCUserUILayoutDirection];

    [[NSUserDefaults standardUserDefaults] synchronize];
}

+ (BOOL)isUILayoutDirectionRtl {
    return uiLayoutDirectionRTL;
}

+ (NSDictionary *)availableLanguages {
    return @{
        @"en-US": @{@"name": @"English (United States)", @"enname": @""},
        @"en-GB": @{@"name": @"English (United Kingdom)", @"enname": @""},
        @"ru": @{@"name": @"Русский", @"enname": @"Russian"},
        @"de": @{@"name": @"Deutsch", @"enname": @"German"},
        @"fr": @{@"name": @"Français", @"enname": @"French"},
        @"es": @{@"name": @"Español", @"enname": @"Spanish"},
        @"it": @{@"name": @"Italiano", @"enname": @"Italian"},
        @"pt-BR": @{@"name": @"Português Brasileiro", @"enname": @"Portuguese (Brazil)"},
        @"pt-PT": @{@"name": @"Português (Portugal)", @"enname": @"Portuguese (Portugal)"},
        @"zh-CN": @{@"name": @"简体中文", @"enname": @"Chinese (Simplified)"},
        @"zh-TW": @{@"name": @"繁體中文", @"enname": @"Chinese (Traditional)"},
        @"sk-SK": @{@"name": @"Slovenčina", @"enname": @"Slovak"},
        @"cs-CZ": @{@"name": @"Čeština", @"enname": @"Czech"},
        @"pl-PL": @{@"name": @"Polski", @"enname":@"Polish"},
        @"ca-ES": @{@"name": @"Catalan", @"enname": @"Catalan"},
        @"da-DK": @{@"name": @"Dansk", @"enname": @"Danish"},
        @"el-GR": @{@"name": @"Ελληνικά", @"enname": @"Greek"},
        // @"et-EE": @"Eesti",
        @"fi-FI": @{@"name": @"Suomi", @"enname": @"Finnish"},
        // @"ga-IE": @"Gaeilge",
        // @"hi-IN": @"हिन्दी",
        // @"hr-HR": @"Hrvatska",
        @"hu-HU": @{@"name": @"Magyar", @"enname": @"Hungarian"},
        @"hy-AM": @{@"name": @"Հայերեն", @"enname": @"Armenian"},
        @"id-ID": @{@"name": @"Indonesian", @"enname": @"Indonesian"},
        @"no": @{@"name": @"Norsk", @"enname": @"Norwegian"},
        @"ro-RO": @{@"name": @"Romanian", @"enname": @"Romanian"},
        @"sl-SI": @{@"name": @"Slovene", @"enname": @"Slovenian"},
        @"sv-SE": @{@"name": @"Svenska", @"enname": @"Swedish"},
        @"sr-Latn-RS": @{@"name": @"Srpski (Latin)", @"enname": @"Serbian (Latin)"},
        @"sr-Cyrl-RS": @{@"name": @"Српски (ћирилица)", @"enname": @"Serbian (Cyrillic)"},
        @"tr-TR": @{@"name": @"Türkçe", @"enname": @"Turkish"},
        @"ja-JP": @{@"name": @"日本語", @"enname": @"Japanese"},
        @"ko-KR": @{@"name": @"한국어", @"enname": @"Korean"},
        @"bg-BG": @{@"name": @"Български", @"enname": @"Bulgarian"},
        @"nl-NL": @{@"name": @"Nederlands", @"enname": @"Dutch"},
        @"vi-VN": @{@"name": @"Tiếng Việt", @"enname": @"Vietnamese"},
        @"lv-LV": @{@"name": @"Latviešu valoda", @"enname": @"Latvian"},
        // @"lt-LT": @"Lietuvių kalba",
        @"be-BY": @{@"name": @"Беларуская мова", @"enname": @"Belarusian"},
        @"uk-UA": @{@"name": @"Украї́нська мо́ва", @"enname": @"Ukrainian"},
        @"lo-LA": @{@"name": @"ພາສາລາວ", @"enname": @"Lao"},
        @"gl-ES": @{@"name": @"Galego", @"enname": @"Galego"},
        @"si-LK": @{@"name": @"සිංහල", @"enname": @"Sinhala (Sri Lanka)"},
        @"ar-SA": @{@"name": @"اَلْعَرَبِيَّة", @"enname": @"Arabic"}
    };
}

@end
