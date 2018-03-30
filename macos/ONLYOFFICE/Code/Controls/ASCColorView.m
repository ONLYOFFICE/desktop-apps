//
//  ASCColorView.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 27/03/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "ASCColorView.h"

@implementation ASCColorView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setNeedsLayout:YES];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self setNeedsLayout:YES];
    }
    return self;
}

- (NSColor *)getBackgroundColor {
    return [NSColor colorWithCGColor: self.layer.backgroundColor];
}

- (void)setBackgroundColor:(NSColor *)backgroundColor {
    self.wantsLayer = YES;
    self.layer.backgroundColor = backgroundColor.CGColor;
}

@end
