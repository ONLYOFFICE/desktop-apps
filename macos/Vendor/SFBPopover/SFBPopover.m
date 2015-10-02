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

#import "SFBPopover.h"
#import "SFBPopoverWindow.h"
#import "SFBPopoverWindowFrame.h"

#include <QuartzCore/QuartzCore.h>

// A custom delegate class is used since CAAnimation retains its delegate
@interface SFBPopoverAnimationDelegate : NSObject
{
@private
	__weak SFBPopoverWindow *_popoverWindow;
}

- (instancetype) initWithPopoverWindow:(SFBPopoverWindow *)popoverWindow;

@end

@implementation SFBPopoverAnimationDelegate

- (instancetype) initWithPopoverWindow:(SFBPopoverWindow *)popoverWindow
{
	if((self = [super init])) {
		_popoverWindow = popoverWindow;
	}
	return self;
}

- (void) animationDidStop:(CAAnimation *)animation finished:(BOOL)flag
{
#pragma unused(animation)
	// Detect the end of fade out and close the window
	if(flag && 0 == [_popoverWindow alphaValue]) {
		NSWindow *parentWindow = [_popoverWindow parentWindow];
		[parentWindow removeChildWindow:_popoverWindow];
		[_popoverWindow orderOut:nil];
		[_popoverWindow setAlphaValue:1];
	}
}

@end

@interface SFBPopover ()
{
@private
	NSViewController * _contentViewController;
	SFBPopoverWindow * _popoverWindow;
}
@end

@interface SFBPopoverWindow (Private)
- (SFBPopoverWindowFrame *) popoverWindowFrame;
@end

@implementation SFBPopover

- (id) init
{
	return [self initWithContentViewController:nil];
}

- (id) initWithContentView:(NSView *)contentView
{
	NSViewController *contentViewController = [[NSViewController alloc] init];
	[contentViewController setView:contentView];
	return [self initWithContentViewController:contentViewController];
}

- (id) initWithContentViewController:(NSViewController *)contentViewController
{
	if((self = [super init])) {
		_contentViewController = contentViewController;
		self.animates = YES;

		NSView *contentView = [_contentViewController view];
		_popoverWindow = [[SFBPopoverWindow alloc] initWithContentRect:[contentView frame] styleMask:0 backing:NSBackingStoreBuffered defer:YES];
		[_popoverWindow setContentView:contentView];
		[_popoverWindow setMinSize:[contentView frame].size];

		CAAnimation *animation = [CABasicAnimation animation];
		[animation setDelegate:[[SFBPopoverAnimationDelegate alloc] initWithPopoverWindow:_popoverWindow]];
		[_popoverWindow setAnimations:[NSDictionary dictionaryWithObject:animation forKey:@"alphaValue"]];

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:_popoverWindow];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidResignActive:) name:NSApplicationDidResignActiveNotification object:nil];
	}
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (SFBPopoverPosition) bestPositionInWindow:(NSWindow *)window atPoint:(NSPoint)point
{
	// Get all relevant geometry in screen coordinates.
	NSRect screenFrame = NSZeroRect;
	if(window && [window screen])
		screenFrame = [[window screen] visibleFrame];
	else
		screenFrame = [[NSScreen mainScreen] visibleFrame];

	NSPoint pointOnScreen = point;
	if(window) {
		NSRect rect = { .origin = point, .size = NSZeroSize };
		NSRect rectOnScreen = [window convertRectToScreen:rect];
		pointOnScreen = rectOnScreen.origin;
	}

	SFBPopoverWindow *popoverWindow = _popoverWindow;
	NSSize popoverSize = [popoverWindow frame].size;
	popoverSize.width += 2 * [popoverWindow viewMargin];
	popoverSize.height += 2 * [popoverWindow viewMargin];

	// By default, position us centered below.
	SFBPopoverPosition side = SFBPopoverPositionBottom;
	CGFloat distance = [popoverWindow arrowHeight] + [popoverWindow distance];

	// We'd like to display directly below the specified point, since this gives a 
	// sense of a relationship between the point and this window. Check there's room.
	if(pointOnScreen.y - popoverSize.height - distance < NSMinY(screenFrame)) {
		// We'd go off the bottom of the screen. Try the right.
		if(pointOnScreen.x + popoverSize.width + distance >= NSMaxX(screenFrame)) {
			// We'd go off the right of the screen. Try the left.
			if(pointOnScreen.x - popoverSize.width - distance < NSMinX(screenFrame)) {
				// We'd go off the left of the screen. Try the top.
				if (pointOnScreen.y + popoverSize.height + distance < NSMaxY(screenFrame))
					side = SFBPopoverPositionTop;
			}
			else
				side = SFBPopoverPositionLeft;
		}
		else
			side = SFBPopoverPositionRight;
	}

	CGFloat halfWidth = popoverSize.width / 2;
	CGFloat halfHeight = popoverSize.height / 2;

	NSRect parentFrame = window ? NSIntersectionRect([window frame], screenFrame) : screenFrame;
	CGFloat arrowInset = ([popoverWindow arrowWidth] / 2) + ([popoverWindow drawRoundCornerBesideArrow] ? [popoverWindow cornerRadius] : 0);

	// We're currently at a primary side.
	// Try to avoid going outwith the parent area in the secondary dimension,
	// by checking to see if an appropriate corner side would be better.
	switch(side) {
		case SFBPopoverPositionBottom:
		case SFBPopoverPositionTop:
			// Check to see if we go beyond the left edge of the parent area.
			if(pointOnScreen.x - halfWidth < NSMinX(parentFrame)) {
				// We go beyond the left edge. Try using right position.
				if(pointOnScreen.x + popoverSize.width - arrowInset < NSMaxX(screenFrame)) {
					// We'd still be on-screen using right, so use it.
					if(SFBPopoverPositionBottom == side)
						side = SFBPopoverPositionBottomRight;
					else
						side = SFBPopoverPositionTopRight;
				}
			}
			else if(pointOnScreen.x + halfWidth >= NSMaxX(parentFrame)) {
				// We go beyond the right edge. Try using left position.
				if(pointOnScreen.x - popoverSize.width + arrowInset >= NSMinX(screenFrame)) {
					// We'd still be on-screen using left, so use it.
					if(SFBPopoverPositionBottom == side)
						side = SFBPopoverPositionBottomLeft;
					else
						side = SFBPopoverPositionTopLeft;
				}
			}
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionLeft:
			// Check to see if we go beyond the bottom edge of the parent area.
			if(pointOnScreen.y - halfHeight < NSMinY(parentFrame)) {
				// We go beyond the bottom edge. Try using top position.
				if(pointOnScreen.y + popoverSize.height - arrowInset < NSMaxY(screenFrame)) {
					// We'd still be on-screen using top, so use it.
					if(SFBPopoverPositionRight == side)
						side = SFBPopoverPositionRightTop;
					else
						side = SFBPopoverPositionLeftTop;
				}
			}
			else if(pointOnScreen.y + halfHeight >= NSMaxY(parentFrame)) {
				// We go beyond the top edge. Try using bottom position.
				if(pointOnScreen.y - popoverSize.height + arrowInset >= NSMinY(screenFrame)) {
					// We'd still be on-screen using bottom, so use it.
					if(SFBPopoverPositionRight == side)
						side = SFBPopoverPositionRightBottom;
					else
						side = SFBPopoverPositionLeftBottom;
				}
			}
			break;

		default:
			break;
	}

	return side;
}

- (void) displayPopoverInWindow:(NSWindow *)window atPoint:(NSPoint)point
{
	[self displayPopoverInWindow:window atPoint:point chooseBestLocation:NO];
}

- (void) displayPopoverInWindow:(NSWindow *)window atPoint:(NSPoint)point chooseBestLocation:(BOOL)chooseBestLocation
{
	if([_popoverWindow isVisible])
		return;

	if(chooseBestLocation)
		[_popoverWindow setPopoverPosition:[self bestPositionInWindow:window atPoint:point]];

	NSPoint attachmentPoint = [[_popoverWindow popoverWindowFrame] attachmentPoint];
	NSPoint pointOnScreen = point;
	if(window) {
		NSRect rect = { .origin = point, .size = NSZeroSize };
		NSRect rectOnScreen = [window convertRectToScreen:rect];
		pointOnScreen = rectOnScreen.origin;
	}

	pointOnScreen.x -= attachmentPoint.x;
	pointOnScreen.y -= attachmentPoint.y;

	[_popoverWindow setFrameOrigin:pointOnScreen];

	if(self.animates)
		[_popoverWindow setAlphaValue:0];

	[window addChildWindow:_popoverWindow ordered:NSWindowAbove];

	[_popoverWindow makeKeyAndOrderFront:nil];

	if(self.animates)
		[[_popoverWindow animator] setAlphaValue:1];
}

- (void) movePopoverToPoint:(NSPoint)point
{
	NSPoint attachmentPoint = [[_popoverWindow popoverWindowFrame] attachmentPoint];
	NSWindow *window = [_popoverWindow parentWindow];

	NSPoint pointOnScreen = point;
	if(window) {
		NSRect rect = { .origin = point, .size = NSZeroSize };
		NSRect rectOnScreen = [window convertRectToScreen:rect];
		pointOnScreen = rectOnScreen.origin;
	}

	pointOnScreen.x -= attachmentPoint.x;
	pointOnScreen.y -= attachmentPoint.y;

	[_popoverWindow setFrameOrigin:pointOnScreen];
}

- (IBAction) closePopover:(id)sender
{
	if(![_popoverWindow isVisible])
		return;

//	[NSAnimationContext beginGrouping];
//	[[NSAnimationContext currentContext] setDuration:0];
//	[[_popoverWindow animator] setAlphaValue:0];
//	[NSAnimationContext endGrouping];

	if(self.animates)
		[[_popoverWindow animator] setAlphaValue:0];
	else {
		NSWindow *parentWindow = [_popoverWindow parentWindow];
		[parentWindow removeChildWindow:_popoverWindow];
		[_popoverWindow orderOut:sender];
	}
}

- (NSWindow *) popoverWindow
{
	return _popoverWindow;
}

- (BOOL) isVisible
{
	return [_popoverWindow isVisible];
}

- (SFBPopoverPosition) position
{
	return [_popoverWindow popoverPosition];
}

- (void) setPosition:(SFBPopoverPosition)position
{
	[_popoverWindow setPopoverPosition:position];
}

- (CGFloat) distance
{
	return [_popoverWindow distance];
}

- (void) setDistance:(CGFloat)distance
{
	[_popoverWindow setDistance:distance];
}

- (NSColor *) borderColor
{
	return [_popoverWindow borderColor];
}

- (void) setBorderColor:(NSColor *)borderColor
{
	[_popoverWindow setBorderColor:borderColor];
}

- (CGFloat) borderWidth
{
	return [_popoverWindow borderWidth];
}

- (void) setBorderWidth:(CGFloat)borderWidth
{
	[_popoverWindow setBorderWidth:borderWidth];
}

- (CGFloat) cornerRadius
{
	return [_popoverWindow cornerRadius];
}

- (void) setCornerRadius:(CGFloat)cornerRadius
{
	[_popoverWindow setCornerRadius:cornerRadius];
}

- (BOOL) drawsArrow
{
	return [_popoverWindow drawsArrow];
}

- (void) setDrawsArrow:(BOOL)drawsArrow
{
	[_popoverWindow setDrawsArrow:drawsArrow];
}

- (CGFloat) arrowWidth
{
	return [_popoverWindow arrowWidth];
}

- (void) setArrowWidth:(CGFloat)arrowWidth
{
	[_popoverWindow setArrowWidth:arrowWidth];
}

- (CGFloat) arrowHeight
{
	return [_popoverWindow arrowHeight];
}

- (void) setArrowHeight:(CGFloat)arrowHeight
{
	[_popoverWindow setArrowHeight:arrowHeight];
}

- (CGFloat) arrowOffset
{
    return [_popoverWindow arrowOffset];
}

- (void) setArrowOffset:(CGFloat)arrowOffset
{
    [_popoverWindow setArrowOffset:arrowOffset];
}

- (BOOL) drawRoundCornerBesideArrow
{
	return [_popoverWindow drawRoundCornerBesideArrow];
}

- (void) setDrawRoundCornerBesideArrow:(BOOL)drawRoundCornerBesideArrow
{
	[_popoverWindow setDrawRoundCornerBesideArrow:drawRoundCornerBesideArrow];
}

- (CGFloat) viewMargin
{
	return [_popoverWindow viewMargin];
}

- (void) setViewMargin:(CGFloat)viewMargin
{
	[_popoverWindow setViewMargin:viewMargin];
}

- (NSColor *) backgroundColor
{
	return [_popoverWindow popoverBackgroundColor];
}

- (void) setBackgroundColor:(NSColor *)backgroundColor
{
	[_popoverWindow setPopoverBackgroundColor:backgroundColor];
}

- (BOOL) isMovable
{
	return [_popoverWindow isMovable];
}

- (void) setMovable:(BOOL)movable
{
	[_popoverWindow setMovable:movable];
}

- (BOOL) isResizable
{
	return [_popoverWindow isResizable];
}

- (void) setResizable:(BOOL)resizable
{
	[_popoverWindow setResizable:resizable];
}

@end

@implementation SFBPopover (NSWindowDelegateMethods)

- (void) windowDidResignKey:(NSNotification *)notification
{
	if(self.closesWhenPopoverResignsKey)
		[self closePopover:notification];
}

- (void) applicationDidResignActive:(NSNotification *)notification
{
	if(self.closesWhenApplicationBecomesInactive)
		[self closePopover:notification];
}

@end
