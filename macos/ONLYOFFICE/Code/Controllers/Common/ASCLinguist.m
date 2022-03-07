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
        @"pl": @"Polski",
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
