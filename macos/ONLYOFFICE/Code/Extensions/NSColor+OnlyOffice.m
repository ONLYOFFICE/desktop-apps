//
//  NSColor+OnlyOffice.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 11/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "NSColor+OnlyOffice.h"

@implementation NSColor (OnlyOffice)

+ (NSColor *) brendDocumentEditor {
    return UIColorFromRGB(0x446995);
}

+ (NSColor *) brendSpreadsheetEditor {
    return UIColorFromRGB(0x40865c);
}

+ (NSColor *) brendPresentationEditor {
    return UIColorFromRGB(0xaa5252);
}

@end
