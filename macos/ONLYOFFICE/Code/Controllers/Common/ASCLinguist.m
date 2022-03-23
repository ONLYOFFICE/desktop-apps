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

+ (void)init {
    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:[ASCLinguist appLanguageCode]] forKey:@"AppleLanguages"];
    [[NSUserDefaults standardUserDefaults] setObject:[ASCLinguist appLanguageCode] forKey:@"AppleLocale"];
    [[NSUserDefaults standardUserDefaults] synchronize];
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

+ (NSDictionary *)availableLanguages {
    return @{
        @"en": @"English",
        @"ru": @"Русский",
        @"de": @"Deutsch",
        @"fr": @"Français",
        @"es": @"Español",
        @"it": @"Italiano",
        @"pt-BR": @"Português Brasileiro",
        @"zh-CN": @"中文",
        @"sk-SK": @"Slovenčina",
        @"cs-CZ": @"Čeština",
        // @"pt-PT": @"Portuguese (Portugal)",
        @"pl-PL": @"Polski",
        // @"zh-HK": @"Chinese (Traditional)",
        @"ca-ES": @"Catalan",
        @"da-DK": @"Dansk",
        @"el-GR": @"Ελληνικά",
        // @"et-EE": @"Eesti",
        @"fi-FI": @"Suomi",
        // @"ga-IE": @"Gaeilge",
        @"hi-IN": @"हिन्दी",
        // @"hr-HR": @"Hrvatska",
        @"hu-HU": @"Magyar nyelv",
        // @"hy-AM": @"Հայաստան",
        @"id-ID": @"Indonesian",
        @"no-NO": @"Norsk",
        @"ro-RO": @"Romanian",
        @"sl-SL": @"Slovene",
        @"sv-SE": @"Svenska",
        @"tr-TR": @"Türkçe",
        @"ja-JP": @"日本語",
        @"ko-KR": @"韓國語",
        @"bg-BG": @"Български",
        @"nl-NL": @"Nederlands",
        @"vi-VN": @"Tiếng Việt",
        @"lv-LV": @"Latviešu valoda",
        // @"lt-LT": @"Lietuvių kalba",
        @"be-BY": @"Беларуская мова",
        @"uk-UA": @"Украї́нська мо́ва",
        @"lo-LA": @"ພາສາລາວ",
        @"gl-ES": @"Galego"
    };
}

@end
