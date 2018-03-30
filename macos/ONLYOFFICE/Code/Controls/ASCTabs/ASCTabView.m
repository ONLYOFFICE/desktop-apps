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
//  ASCTabView.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTabView.h"
#import "ASCTabCloseButtonCell.h"
#import "ASCTabViewCell.h"
#import "ASCHelper.h"

static NSUInteger const kASTabViewCloseButtonSize = 12;

@interface ASCTabView()
@property (nonatomic) NSButton * close;
@property (nonatomic) NSArray * icons;
@end

@implementation ASCTabView

- (id)init {
    self = [super init];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (instancetype)copyWithZone:(NSZone *)zone
{
    ASCTabView *copy = [[ASCTabView allocWithZone:zone] initWithFrame:self.frame];
    
    copy.type = self.type;
    
    NSButtonCell *cellCopy = [self.cell copy];
    [copy setCell:cellCopy];
    
    return copy;
}

- (void)initialize {   
    _uuid = [[NSUUID UUID] UUIDString];
    
    _icons = @[
               // ASCTabViewUnknownType
               @{@"normal": @"", @"active": @""},
               // ASCTabViewOpeningType
               @{@"normal": @"tab-icon_opening", @"active": @"tab-icon_opening"},
               // ASCTabViewDocumentType
               @{@"normal": @"icon_tabs_de_inactive", @"active": @"icon_tabs_de_active"},
               // ASCTabViewSpreadsheetType
               @{@"normal": @"icon_tabs_se_inactive", @"active": @"icon_tabs_se_active"},
               // ASCTabViewPresentationType
               @{@"normal": @"icon_tabs_pe_inactive", @"active": @"icon_tabs_pe_active"},
               // ASCTabViewPortal
               @{@"normal": @"icon_tab_portal", @"active": @"icon_tab_portal"}
               ];
    
    [self setBordered:NO];
    [self setCell:[[ASCTabViewCell alloc] initTextCell:self.title]];
    [self.cell setImagePosition:NSImageLeft];
    [self.cell setBordered:NO];
        
    self.close = [[ASCButton alloc] initWithFrame:CGRectZero];
    [self.close setCell:[[ASCTabCloseButtonCell alloc] initTextCell:@""]];
    [self.close setBordered:NO];
    [self.close setTarget:self];
    [self.close setAction:@selector(onCloseTabButton:)];
    [self addSubview:self.close];
    
    [self.cell setCloseButton:self.close];
    
    [self.close setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];
    [self.close setFrame:CGRectMake(
                                    CGRectGetWidth(self.frame) - kASTabViewCloseButtonSize * 1.5,
                                    kASTabViewCloseButtonSize / 1.5,
                                    kASTabViewCloseButtonSize,
                                    kASTabViewCloseButtonSize
                                    )];
}

- (void)setFrame:(NSRect)frame {
    [super setFrame:frame];
}

- (void)setState:(NSInteger)state {
    [super setState:state];
    
    NSString * iconName = (self.state) ? _icons[self.type][@"active"] : _icons[self.type][@"normal"];
    
    if ([iconName length] > 0) {
        self.image = [NSImage imageNamed:iconName];
    }

    ASCButton * closeButton = (ASCButton *)self.close;

    if (closeButton) {
        ASCTabCloseButtonCell * cell = [closeButton cell];

        if (cell) {
            NSInteger windowNumber = [[self window] windowNumber];
            NSEvent *fakeEvent = [NSEvent enterExitEventWithType:NSEventTypeMouseExited
                                                        location:NSMakePoint(0, 0)
                                                   modifierFlags:0
                                                       timestamp:0
                                                    windowNumber:windowNumber
                                                         context:NULL
                                                     eventNumber:0
                                                  trackingNumber:0xBADFACE
                                                        userData:nil];

            [[closeButton cell] mouseExited:fakeEvent];
        }
    }

    [self setNeedsDisplay];
}

- (void)setType:(ASCTabViewType)type {
    _type = type;
    
    if (type > ASCTabViewUnknownType && type < [_icons count]) {
        NSString * iconName = (self.state) ? _icons[type][@"active"] : _icons[type][@"normal"];
        
        if ([iconName length] > 0) {
            self.image = [NSImage imageNamed:iconName];
        }
    }

    ASCTabViewCell * tabViewCell = (ASCTabViewCell *)self.cell;

    if (type == ASCTabViewPortal) {
        tabViewCell.activeColor = kColorRGB(255, 255, 255);
    } else if (type == ASCTabViewDocumentType) {
        tabViewCell.activeColor = UIColorFromRGB(0x446995);
        tabViewCell.clickColor  = UIColorFromRGB(0x446995);
        tabViewCell.activeTextColor = UIColorFromRGB(0xffffff);
    } else if (type == ASCTabViewSpreadsheetType) {
        tabViewCell.activeColor = UIColorFromRGB(0x40865c);
        tabViewCell.clickColor  = UIColorFromRGB(0x40865c);
        tabViewCell.activeTextColor = UIColorFromRGB(0xffffff);
    } else if (type == ASCTabViewPresentationType) {
        tabViewCell.activeColor = UIColorFromRGB(0xaa5252);
        tabViewCell.clickColor  = UIColorFromRGB(0xaa5252);
        tabViewCell.activeTextColor = UIColorFromRGB(0xffffff);
    }
}

- (NSString *)title {
    return _changed ? [NSString stringWithFormat:@"%@*", [super title]] : [super title];
}

- (NSMutableDictionary *)params {
    if (nil == _params) {
        _params = [NSMutableDictionary dictionary];
    }
    
    return _params;
}

- (void)onCloseTabButton:(id)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(tabDidClose:)]) {
        [_delegate tabDidClose:self];
    }
}

- (void)setChanged:(BOOL)changed {
    _changed = changed;
}

- (void)drawRect:(NSRect)dirtyRect {
//    [[NSColor greenColor] setFill];
//    NSRectFill(dirtyRect);
    
    [super drawRect:dirtyRect];
}



@end
