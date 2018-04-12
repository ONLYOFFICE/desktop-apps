//
//  NSColor+OnlyOffice.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 11/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define kColorRGBA(r, g, b, a)      [NSColor colorWithRed:(r)/255.f green:(g)/255.f blue:(b)/255.f alpha:(a)]
#define kColorRGB(r, g, b)          [NSColor colorWithRed:(r)/255.f green:(g)/255.f blue:(b)/255.f alpha:1.f]
#define UIColorFromRGB(rgbValue)    [NSColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 \
                                                    green:((float)((rgbValue & 0x00FF00) >>  8))/255.0 \
                                                     blue:((float)((rgbValue & 0x0000FF) >>  0))/255.0 \
                                                    alpha:1.0]
#define UIColorFromRGBA(rgbValue, a) [NSColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 \
                                                     green:((float)((rgbValue & 0x00FF00) >>  8))/255.0 \
                                                      blue:((float)((rgbValue & 0x0000FF) >>  0))/255.0 \
                                                     alpha:a]

@interface NSColor (OnlyOffice)
+ (NSColor *) brendDocumentEditor;
+ (NSColor *) brendSpreadsheetEditor;
+ (NSColor *) brendPresentationEditor;
@end
