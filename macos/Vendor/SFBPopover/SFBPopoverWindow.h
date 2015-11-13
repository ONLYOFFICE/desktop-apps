/*
 *  Copyright (C) 2011, 2012, 2013, 2014, 2015 Stephen F. Booth <me@sbooth.org>
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>

#import "SFBPopover.h"

// ========================================
// NSWindow subclass implementing a popover window
// ========================================
@interface SFBPopoverWindow : NSWindow

// ========================================
// Popover window properties
- (SFBPopoverPosition) popoverPosition;
- (void) setPopoverPosition:(SFBPopoverPosition)popoverPosition;

- (CGFloat) distance;
- (void) setDistance:(CGFloat)distance;

- (NSColor *) borderColor;
- (void) setBorderColor:(NSColor *)borderColor;
- (CGFloat) borderWidth;
- (void) setBorderWidth:(CGFloat)borderWidth;
- (CGFloat) cornerRadius;
- (void) setCornerRadius:(CGFloat)cornerRadius;

- (BOOL) drawsArrow;
- (void) setDrawsArrow:(BOOL)drawsArrow;
- (CGFloat) arrowWidth;
- (void) setArrowWidth:(CGFloat)arrowWidth;
- (CGFloat) arrowHeight;
- (void) setArrowHeight:(CGFloat)arrowHeight;
- (CGFloat) arrowOffset;
- (void) setArrowOffset:(CGFloat)arrowOffset;
- (BOOL) drawRoundCornerBesideArrow;
- (void) setDrawRoundCornerBesideArrow:(BOOL)drawRoundCornerBesideArrow;

- (CGFloat) viewMargin;
- (void) setViewMargin:(CGFloat)viewMargin;
- (NSColor *) popoverBackgroundColor;
- (void) setPopoverBackgroundColor:(NSColor *)backgroundColor;

- (BOOL) isMovable;
- (void) setMovable:(BOOL)movable;

- (BOOL) isResizable;
- (void) setResizable:(BOOL)resizable;

@end
