//
//  ASCLinguist.h
//  ONLYOFFICE
//
//  Created by Maxim Kadushkin on 03.02.2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ASCLinguist : NSObject

+ (void)init;
+ (NSDictionary *)availableLanguages;
+ (NSString *)appLanguageCode;
+ (void)setAppLanguageCode:(NSString *)code;

@end

NS_ASSUME_NONNULL_END
