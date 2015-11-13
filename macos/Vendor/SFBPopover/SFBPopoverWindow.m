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

#import "SFBPopoverWindow.h"
#import "SFBPopoverWindowFrame.h"

@interface SFBPopoverWindow ()
{
@private
	NSView * _popoverContentView;
}
@end

@interface SFBPopoverWindow (Private)
- (SFBPopoverWindowFrame *) popoverWindowFrame;
@end

@implementation SFBPopoverWindow

- (id) initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
#pragma unused(windowStyle)
	if((self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:deferCreation])) {
		[self setBackgroundColor:[NSColor clearColor]];
		[self setMovableByWindowBackground:NO];
		[self setExcludedFromWindowsMenu:YES];
		[self setAlphaValue:1];
		[self setOpaque:NO];
		[self setHasShadow:YES];
	}

	return self;
}

- (NSRect) contentRectForFrameRect:(NSRect)windowFrame
{
	return [[self popoverWindowFrame] contentRectForFrameRect:windowFrame];
}

- (NSRect) frameRectForContentRect:(NSRect)contentRect
{
	return [[self popoverWindowFrame] frameRectForContentRect:contentRect];
}

- (BOOL) canBecomeKeyWindow
{
	return YES;
}

- (BOOL) canBecomeMainWindow
{
	return NO;
}

- (NSView *) contentView
{
	return _popoverContentView;
}

- (void) setContentView:(NSView *)view
{
	if([_popoverContentView isEqualTo:view])
		return;

	SFBPopoverWindowFrame *popoverWindowFrame = [self popoverWindowFrame];
	if(nil == popoverWindowFrame) {
		popoverWindowFrame = [[SFBPopoverWindowFrame alloc] initWithFrame:NSZeroRect];
		[super setContentView:popoverWindowFrame];
	}

	// Automatically resize the window to match the size of the new content
	NSRect windowFrame = [popoverWindowFrame frameRectForContentRect:[view frame]];
	windowFrame.origin = NSZeroPoint;
	[self setFrame:windowFrame display:NO];

	if(_popoverContentView) {
		[_popoverContentView removeFromSuperview];
		_popoverContentView = nil;
	}

	_popoverContentView = view;
	NSRect viewFrame = [self contentRectForFrameRect:windowFrame];
	[_popoverContentView setFrame:viewFrame];
	[_popoverContentView setAutoresizingMask:NSViewNotSizable];

	[popoverWindowFrame addSubview:_popoverContentView];
}

- (void) setContentSize:(NSSize)size
{
	NSRect contentRect = NSMakeRect(0, 0, size.width, size.height);
	NSRect frameRect = [self frameRectForContentRect:contentRect];
	frameRect.origin = NSZeroPoint;
	[self setFrame:frameRect display:NO];
}

- (SFBPopoverPosition) popoverPosition
{
	return [[self popoverWindowFrame] popoverPosition];
}

- (void) setPopoverPosition:(SFBPopoverPosition)popoverPosition
{
	if(popoverPosition == [self popoverPosition])
		return;

	NSRect frameRect = [self frame];
	NSPoint oldOrigin = frameRect.origin;
	NSPoint oldAttachmentPoint = [[self popoverWindowFrame] attachmentPoint];

	CGFloat offset = [[self popoverWindowFrame] arrowHeight] + [[self popoverWindowFrame] distance];
	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect.size.width -= offset;
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect.size.height -= offset;
			break;
	}

	[[self popoverWindowFrame] setPopoverPosition:popoverPosition];

	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect.size.width += offset;
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect.size.height += offset;
			break;
	}

	NSRect boundsRect = frameRect;
	boundsRect.origin = NSZeroPoint;
	NSPoint newAttachmentPoint = [[self popoverWindowFrame] attachmentPointForRect:boundsRect];

	// This orderOut: then orderFront: trickery seems to be required to prevent visual artifacts when the new window position
	// overlaps the old one.  No matter what I tried (flushing, erasing, disabling screen updates) I could not get the display
	// to work properly if the window was onscreen
	BOOL isVisible = [self isVisible];
	BOOL isKey = [self isKeyWindow];
	if(isVisible) {
		NSDisableScreenUpdates();
		[self orderOut:self];
	}

	NSRect contentRect = [self contentRectForFrameRect:boundsRect];
	[_popoverContentView setFrame:contentRect];

	// Adjust the frame so the attachment point won't change
	frameRect.origin = NSMakePoint(oldOrigin.x + (oldAttachmentPoint.x - newAttachmentPoint.x), oldOrigin.y + (oldAttachmentPoint.y - newAttachmentPoint.y));

	[[self popoverWindowFrame] setNeedsDisplay:YES];
	[self setFrame:frameRect display:YES];

	if(isVisible) {
		if(isKey)
			[self makeKeyAndOrderFront:self];
		else
			[self orderFront:self];

		NSEnableScreenUpdates();
	}
}

- (CGFloat) distance
{
	return [[self popoverWindowFrame] distance];
}

- (void) setDistance:(CGFloat)distance
{
	CGFloat delta = distance - [[self popoverWindowFrame] distance];
	if(0 == delta)
		return;

	NSRect frameRect = [self frame];
	NSRect boundsRect = frameRect;
	boundsRect.origin = NSZeroPoint;
	NSRect contentRect = [self contentRectForFrameRect:boundsRect];

	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			frameRect.origin.x -= delta;
			frameRect.size.width += delta;
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect.size.width += delta;
			contentRect.origin.x += delta;
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			frameRect.size.height += delta;
			contentRect.origin.y += delta;
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect.origin.y -= delta;
			frameRect.size.height += delta;
			break;
	}

	[[self popoverWindowFrame] setDistance:distance];
	[_popoverContentView setFrame:contentRect];
	[self setFrame:frameRect display:YES];
}

- (NSColor *) borderColor
{
	return [[self popoverWindowFrame] borderColor];
}

- (void) setBorderColor:(NSColor *)borderColor
{
	[[self popoverWindowFrame] setBorderColor:borderColor];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (CGFloat) borderWidth
{
	return [[self popoverWindowFrame] borderWidth];
}

- (void) setBorderWidth:(CGFloat)borderWidth
{
	CGFloat delta = borderWidth - [[self popoverWindowFrame] borderWidth];
	if(0 == delta)
		return;

	NSRect frameRect = NSInsetRect([self frame], -delta, -delta);

	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			frameRect = NSOffsetRect(frameRect, -delta, 0);
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect = NSOffsetRect(frameRect, delta, 0);
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			frameRect = NSOffsetRect(frameRect, 0, delta);
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect = NSOffsetRect(frameRect, 0, -delta);
			break;
	}
	
	[[self popoverWindowFrame] setBorderWidth:borderWidth];

	NSRect boundsRect = frameRect;
	boundsRect.origin = NSZeroPoint;
	NSRect contentRect = [self contentRectForFrameRect:boundsRect];

	[_popoverContentView setFrame:contentRect];
	[self setFrame:frameRect display:YES];
}

- (CGFloat) cornerRadius
{
	return [[self popoverWindowFrame] cornerRadius];
}

- (void) setCornerRadius:(CGFloat)cornerRadius
{
	[[self popoverWindowFrame] setCornerRadius:cornerRadius];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (BOOL) drawsArrow
{
	return [[self popoverWindowFrame] drawsArrow];
}

- (void) setDrawsArrow:(BOOL)drawsArrow
{
	[[self popoverWindowFrame] setDrawsArrow:drawsArrow];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (CGFloat) arrowWidth
{
	return [[self popoverWindowFrame] arrowWidth];
}

- (void) setArrowWidth:(CGFloat)arrowWidth
{
	[[self popoverWindowFrame] setArrowWidth:arrowWidth];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (CGFloat) arrowHeight
{
	return [[self popoverWindowFrame] arrowHeight];
}

- (void) setArrowHeight:(CGFloat)arrowHeight
{
	CGFloat delta = arrowHeight - [[self popoverWindowFrame] arrowHeight];
	if(0 == delta)
		return;

	NSRect frameRect = [self frame];
	NSRect boundsRect = frameRect;
	boundsRect.origin = NSZeroPoint;
	NSRect contentRect = [self contentRectForFrameRect:boundsRect];

	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			frameRect.origin.x -= delta;
			frameRect.size.width += delta;
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect.size.width += delta;
			contentRect.origin.x += delta;
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			frameRect.size.height += delta;
			contentRect.origin.y += delta;
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect.origin.y -= delta;
			frameRect.size.height += delta;
			break;
	}

	[[self popoverWindowFrame] setArrowHeight:arrowHeight];
	[_popoverContentView setFrame:contentRect];
	[self setFrame:frameRect display:YES];
}

- (CGFloat) arrowOffset
{
    return [[self popoverWindowFrame] arrowOffset];
}

- (void) setArrowOffset:(CGFloat)arrowOffset
{
    [[self popoverWindowFrame] setArrowOffset:arrowOffset];
    [[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (BOOL) drawRoundCornerBesideArrow
{
	return [[self popoverWindowFrame] drawRoundCornerBesideArrow];
}

- (void)setDrawRoundCornerBesideArrow:(BOOL)drawRoundCornerBesideArrow
{
	[[self popoverWindowFrame] setDrawRoundCornerBesideArrow:drawRoundCornerBesideArrow];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (CGFloat) viewMargin
{
	return [[self popoverWindowFrame] viewMargin];
}

- (void) setViewMargin:(CGFloat)viewMargin
{
	CGFloat delta = viewMargin - [[self popoverWindowFrame] viewMargin];
	if(0 == delta)
		return;

	NSRect frameRect = NSInsetRect([self frame], -delta, -delta);

	switch([[self popoverWindowFrame] popoverPosition]) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			frameRect = NSOffsetRect(frameRect, -delta, 0);
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect = NSOffsetRect(frameRect, delta, 0);
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			frameRect = NSOffsetRect(frameRect, 0, delta);
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect = NSOffsetRect(frameRect, 0, -delta);
			break;
	}

	[[self popoverWindowFrame] setViewMargin:viewMargin];

	NSRect boundsRect = frameRect;
	boundsRect.origin = NSZeroPoint;
	NSRect contentRect = [self contentRectForFrameRect:boundsRect];
	
	[_popoverContentView setFrame:contentRect];
	[self setFrame:frameRect display:YES];
}

- (NSColor *) popoverBackgroundColor
{
	return [[self popoverWindowFrame] backgroundColor];
}

- (void) setPopoverBackgroundColor:(NSColor *)backgroundColor
{
	[[self popoverWindowFrame] setBackgroundColor:backgroundColor];
	[[self popoverWindowFrame] setNeedsDisplay:YES];
}

- (BOOL) isMovable
{
	return [[self popoverWindowFrame] isMovable];
}

- (void) setMovable:(BOOL)movable
{
	[[self popoverWindowFrame] setMovable:movable];
}

- (BOOL) isResizable
{
	return [[self popoverWindowFrame] isResizable];
}

- (void) setResizable:(BOOL)resizable
{
	[[self popoverWindowFrame] setResizable:resizable];
//	if(resizable)
//		[self setStyleMask:NSBorderlessWindowMask | NSResizableWindowMask];
//	else
//		[self setStyleMask:NSBorderlessWindowMask];
}

@end

@implementation SFBPopoverWindow (Private)

- (SFBPopoverWindowFrame *) popoverWindowFrame
{
	// The window's content view is the popover frame
	return (SFBPopoverWindowFrame *)[super contentView];
}

@end
