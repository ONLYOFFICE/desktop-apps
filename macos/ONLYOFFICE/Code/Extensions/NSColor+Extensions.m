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
//  NSColor+Extensions.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 11/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "NSColor+Extensions.h"

#pragma mark - ONLYOFFICE

@implementation NSColor (OnlyOffice)

+ (NSColor *) brendDocumentEditor {
    if (@available(macOS 10.13, *)) {
        return [NSColor colorNamed:@"brend-DocumentEditor"];
    }
    return UIColorFromRGB(0x446995);
}

+ (NSColor *) brendSpreadsheetEditor {
    if (@available(macOS 10.13, *)) {
        return [NSColor colorNamed:@"brend-SpreadsheetEditor"];
    }
    return UIColorFromRGB(0x40865c);
}

+ (NSColor *) brendPresentationEditor {
    if (@available(macOS 10.13, *)) {
        return [NSColor colorNamed:@"brend-PresentationEditor"];
    }
    return UIColorFromRGB(0xaa5252);
}

@end

#pragma mark - Extensions

@implementation NSColor (Extensions)

- (BOOL) isLight {
    CGFloat colorBrightness = 0;
    CGColorSpaceRef colorSpace = CGColorGetColorSpace(self.CGColor);
    CGColorSpaceModel colorSpaceModel = CGColorSpaceGetModel(colorSpace);

    if(colorSpaceModel == kCGColorSpaceModelRGB){
        const CGFloat *componentColors = CGColorGetComponents(self.CGColor);

        colorBrightness = ((componentColors[0] * 299) + (componentColors[1] * 587) + (componentColors[2] * 114)) / 1000;
    } else {
        [self getWhite:&colorBrightness alpha:0];
    }

    return (colorBrightness >= .5f);
}

@end
