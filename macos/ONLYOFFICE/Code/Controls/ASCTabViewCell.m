/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
//  ASCTabViewCell.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/10/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTabViewCell.h"
#import "ASCHelper.h"

@interface ASCTabViewCell()
@end

@implementation ASCTabViewCell

- (id)initTextCell:(NSString *)string {
    self = [super initTextCell:string];
    if (self) {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        
        self.inactiveColor          = kColorRGB(207, 207, 207);
        self.activeColor            = kColorRGB(241, 241, 241);
        self.hoverInactiveColor     = kColorRGB(221, 221, 221);
        self.hoverActiveColor       = kColorRGB(247, 247, 247);
        self.clickColor             = kColorRGB(247, 247, 247);
        self.textColor              = kColorRGB(51, 51, 51);
        self.inactiveBorderColor    = kColorRGB(175, 176, 177);
    }
    
    return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
//    [super drawWithFrame:cellFrame inView:controlView];
    
    CGFloat rectangleCornerRadius = 2;
    
    // Color Declarations
    NSColor * color;
    
    if (self.state) {
        color = (self.isHover) ? self.activeColor : self.activeColor;
    } else {
        color = (self.isHover) ? self.hoverInactiveColor : self.inactiveColor;
    }
    
    //// Rectangle Drawing
    NSRect rectangleRect = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y, cellFrame.size.width - 1, cellFrame.size.height);
    NSRect rectangleInnerRect = NSInsetRect(rectangleRect, rectangleCornerRadius, rectangleCornerRadius);
    NSBezierPath* rectanglePath = [NSBezierPath bezierPath];
    [rectanglePath appendBezierPathWithArcWithCenter: NSMakePoint(NSMinX(rectangleInnerRect), NSMinY(rectangleInnerRect)) radius: rectangleCornerRadius startAngle: 180 endAngle: 270];
    [rectanglePath appendBezierPathWithArcWithCenter: NSMakePoint(NSMaxX(rectangleInnerRect), NSMinY(rectangleInnerRect)) radius: rectangleCornerRadius startAngle: 270 endAngle: 360];
    [rectanglePath lineToPoint: NSMakePoint(NSMaxX(rectangleRect), NSMaxY(rectangleRect))];
    [rectanglePath lineToPoint: NSMakePoint(NSMinX(rectangleRect), NSMaxY(rectangleRect))];
    [rectanglePath closePath];
    [color setFill];
    [rectanglePath fill];

    if (!self.state) {
        //// Bottom Line Drawing
        NSBezierPath* bottomRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rectangleRect), NSHeight(rectangleRect) - 1, NSWidth(rectangleRect), 1)];
        [self.inactiveBorderColor setFill];
        [bottomRectanglePath fill];
    }
    
    if (self.title) {
        [self drawTitle:[self attributedTitle] withFrame:cellFrame inView:controlView];
    }
    
    if (self.image && self.imagePosition != NSNoImage) {
        [self drawImage:self.image withFrame:cellFrame inView:controlView];
    }
}

- (NSCellStyleMask)highlightsBy {
    return NSNoCellMask;
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView {
    NSSize size = [image size];
    CGRect rect = CGRectMake(8, (CGRectGetHeight(frame) - size.width) * .5 - 1, size.width, size.height);
    [super drawImage:image withFrame:rect inView:controlView];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView {
    CGFloat leftOffset  = 0.f;
    CGFloat rightOffset = 0.f;
    
    if (self.image) {
        leftOffset = 8 + self.image.size.width + 5;
    }
    
    if (self.closeButton) {
        rightOffset = CGRectGetHeight(self.closeButton.frame) * 1.5;
    }
    
    if (CGRectGetWidth(frame) - leftOffset - rightOffset > 15) {
        return [super drawTitle:title withFrame:CGRectMake(frame.origin.x + leftOffset, frame.origin.y, frame.size.width - rightOffset - leftOffset, frame.size.height) inView:controlView];
    } else {
        return [super drawTitle:[[NSAttributedString alloc] initWithString:@""] withFrame:CGRectZero inView:controlView];
    }
    
}

- (NSAttributedString *)attributedTitle {
    NSMutableAttributedString *attributedTitle  = [[super attributedTitle] mutableCopy];
    NSMutableParagraphStyle *paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    NSFont *font = [NSFont systemFontOfSize:11];
    NSColor *color = self.textColor;
    
    [paragraphStyle setAlignment:NSLeftTextAlignment];
    [paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [attributedTitle addAttributes:@{
                                     NSForegroundColorAttributeName:color,
                                     NSParagraphStyleAttributeName:paragraphStyle,
                                     NSFontAttributeName:font
                                     }
                             range:NSMakeRange(0, attributedTitle.length)];
    return attributedTitle;
}

@end
