/*
 * (c) Copyright Ascensio System SIA 2010-2018
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

#import <QuartzCore/QuartzCore.h>
#import "ASCTabViewCell.h"
#import "NSColor+OnlyOffice.h"

@interface ASCTabViewCell()
@property (nonatomic) BOOL isAnimatedIcon;
@property (nonatomic) NSImageView * animatedImageView;
@property (nonatomic, weak) NSView * parentView;
@property (nonatomic, readonly) CALayer * loaderLayer;
@property (nonatomic, readonly) CABasicAnimation * loaderAnimation;
@end

@implementation ASCTabViewCell

@synthesize loaderLayer = _loaderLayer;
@synthesize loaderAnimation = _loaderAnimation;

#pragma mark - Properties

- (CALayer *)loaderLayer {
    if (_loaderLayer) {
        return _loaderLayer;
    }

    if (nil == self.image || nil == self.parentView) {
        return nil;
    }

    NSSize size = [self.image size];
    CGRect rect = CGRectMake(
                             8,
                             (CGRectGetHeight(self.parentView.bounds) - size.width) * .5 - 1,
                             size.width,
                             size.height);


    CALayer * layer = [CALayer new];
    layer.backgroundColor = NSColor.redColor.CGColor;
    layer.bounds = rect;
    layer.anchorPoint = CGPointMake(0.5, 0.5);
    layer.position = CGPointMake(CGRectGetMidX(rect), CGRectGetMidY(rect));

    //    layer.contents = UIImage(named:"logo3")?.cgImage
    layer.contentsGravity = kCAGravityResizeAspectFill;

    _loaderLayer = layer;
    return _loaderLayer;
}

- (CABasicAnimation *)loaderAnimation {
    if (_loaderAnimation) {
        return _loaderAnimation;
    }

    CABasicAnimation * animation = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];

    animation.fromValue = 0;
    animation.toValue = [NSNumber numberWithFloat: M_PI * 4];
    animation.duration = 4;
    animation.repeatCount = HUGE_VALF;
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
    animation.removedOnCompletion = true;

    _loaderAnimation = animation;
    return animation;
}

#pragma mark - Lifecycle Methods

- (id)initTextCell:(NSString *)string {
    self = [super initTextCell:string];
    if (self) {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];

        self.inactiveColor          = UIColorFromRGBA(0xe5e5e5, 0);
        self.activeColor            = UIColorFromRGBA(0x000000, 0.1);
        self.hoverInactiveColor     = UIColorFromRGBA(0x000000, 0.1);
        self.hoverActiveColor       = UIColorFromRGB(0xe5e5e5);
        self.clickColor             = UIColorFromRGB(0xe5e5e5);
        self.activeTextColor        = UIColorFromRGB(0x000000);
        self.inactiveTextColor      = UIColorFromRGB(0x000000);
        self.inactiveBorderColor    = UIColorFromRGBA(0x000000, 0);
    }

//    [self startProcessing];
//    self.isAnimatedIcon = true;

    return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
//    [super drawWithFrame:cellFrame inView:controlView];

    self.parentView = controlView;

    CGFloat rectangleCornerRadius = 0;
    
    // Color Declarations
    NSColor * color;
    
    if (self.state) {
        color = (self.isHover) ? self.activeColor : self.activeColor;
    } else {
        color = (self.isHover) ? self.hoverInactiveColor : self.inactiveColor;
//        (self.isHover) ? NSLog(@"Hover %@ TRUE", [self className]) : NSLog(@"Hover %@ FALSE", [self className]);
    }

    // Rectangle Drawing
    NSRect rectangleRect = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y, cellFrame.size.width, cellFrame.size.height);
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
        // Bottom Line Drawing
        NSBezierPath* bottomRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rectangleRect), NSHeight(rectangleRect), NSWidth(rectangleRect), 1)];
        [self.inactiveBorderColor setFill];
        [bottomRectanglePath fill];
    }

    if (true/*!self.state*/) {
        NSBezierPath * leftRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(-1, 0, 1, NSHeight(rectangleRect))];
        NSBezierPath * rightRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMaxX(rectangleRect) - 1, 0, 1, NSHeight(rectangleRect))];
        NSColor * lineColor = kColorRGBA(0, 0, 0, 0.2);
        [lineColor setFill];
        [leftRectanglePath fill];
        [rightRectanglePath fill];
    }

    
    if (self.title) {
        [self drawTitle:[self attributedTitle] withFrame:cellFrame inView:controlView];
    }

//    if (self.animatedImageView == nil) {
//        NSSize size = [self.image size];
//        CGRect rect = CGRectMake(8, (CGRectGetHeight(cellFrame) - size.width) * .5 - 1, size.width, size.height);
//
//        self.animatedImageView = [[NSImageView alloc] initWithFrame:rect];
//        self.animatedImageView.wantsLayer = true;
//        self.animatedImageView.layer.backgroundColor = NSColor.redColor.CGColor;
//
//        [self.animatedImageView.layer setBounds:rect];
//        [self.animatedImageView.layer setPosition:CGPointMake(NSMidX(self.animatedImageView.frame), NSMidY(self.animatedImageView.frame))];
//        [self.animatedImageView.layer setAnchorPoint:CGPointMake(0.5, 0.5)];
////        [self.animatedImageView.layer setPosition:CGPointMake(NSMidX(self.animatedImageView.frame), NSMidY(self.animatedImageView.frame))];
//
//        [controlView addSubview:self.animatedImageView];
//
//
//        CABasicAnimation * rotate =  [CABasicAnimation animationWithKeyPath:@"transform.rotation.z"];
//        rotate.removedOnCompletion = FALSE;
//        rotate.fillMode = kCAFillModeForwards;
//
//        [rotate setToValue: [NSNumber numberWithFloat: M_PI / 2]];
//        rotate.repeatCount = HUGE_VALF;
//
//        rotate.duration = 0.25;
//        rotate.cumulative = TRUE;
//        rotate.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
//
//
//        [self.animatedImageView.layer addAnimation:rotate forKey:@"rotateAnimation"];
//    }

//    if ([self.loaderLayer animationForKey:@"rotateAnimation"] == nil) {
//        [controlView.layer addSublayer:self.loaderLayer];
//
//        ASCTabViewCell * __weak weakSelf = self;
//
//        [CATransaction begin];
////        [CATransaction setCompletionBlock:^{
////            if (weakSelf) {
////                [[weakSelf loaderLayer] removeAnimationForKey:@"rotateAnimation"];
////                [[weakSelf loaderLayer] removeFromSuperlayer];
////            }
////        }];
//        [self.loaderLayer addAnimation:self.loaderAnimation forKey:@"rotateAnimation"];
//        [CATransaction commit];
//    }

//    [self startProcessing];

    if (self.image && self.imagePosition != NSNoImage) {
        if (self.isAnimatedIcon) {
            //
        } else {
            [self drawImage:self.image withFrame:cellFrame inView:controlView];
        }
    }
}

- (void)startProcessing {
//    if (self.isAnimatedIcon) {
//        return;
//    }

    self.isAnimatedIcon = true;

    if ([self.loaderLayer animationForKey:@"rotateAnimation"]) {
        [self.loaderLayer removeAnimationForKey:@"rotateAnimation"];
        [self.loaderLayer removeFromSuperlayer];
    }

    if (self.parentView) {
        [self.parentView.layer addSublayer:self.loaderLayer];
    }

    [CATransaction begin];
    [self.loaderLayer addAnimation:self.loaderAnimation forKey:@"rotateAnimation"];
    [CATransaction commit];
}

- (void)stopProcessing {
    self.isAnimatedIcon = false;

    if ([self.loaderLayer animationForKey:@"rotateAnimation"]) {
        [self.loaderLayer removeAnimationForKey:@"rotateAnimation"];
        [self.loaderLayer removeFromSuperlayer];
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
    NSColor *color = self.inactiveTextColor;

    if (self.state) {
        color = self.activeTextColor;
    }
    
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
