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

#import "SFBPopoverWindowFrame.h"
#import "SFBPopoverWindow.h"

@interface SFBPopoverWindowFrame (Private)
- (NSBezierPath *) popoverFramePathForContentRect:(NSRect)rect;
- (void) appendArrowToPath:(NSBezierPath *)path;
@end

@implementation SFBPopoverWindowFrame

- (id) initWithFrame:(NSRect)frame
{
	if((self = [super initWithFrame:frame])) {
		// Set the default appearance
		self.popoverPosition = SFBPopoverPositionBottom;

		self.distance = 0;

		self.borderColor = [NSColor whiteColor];
		self.borderWidth = 2;
		self.cornerRadius = 8;

		self.drawsArrow = YES;
		self.arrowWidth = 20;
		self.arrowHeight = 16;
        self.arrowOffset = 0;
		self.drawRoundCornerBesideArrow = YES;

		self.viewMargin = 2;
		self.backgroundColor = [NSColor colorWithCalibratedWhite:(CGFloat)0.1 alpha:(CGFloat)0.75];

		self.movable = NO;
		self.resizable = NO;
	}

	return self;
}

- (NSRect) frameRectForContentRect:(NSRect)contentRect
{
	NSRect frameRect = NSInsetRect(contentRect, -self.viewMargin, -self.viewMargin);

	CGFloat offset = self.arrowHeight + self.distance;
	switch(self.popoverPosition) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			frameRect.size.width += offset;
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			frameRect.size.width += offset;
			frameRect.origin.x -= offset;
			break;
			
		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			frameRect.size.height += offset;
			frameRect.origin.y += offset;
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			frameRect.size.height += offset;
			break;
	}

	return NSInsetRect(frameRect, -self.borderWidth, -self.borderWidth);
}

- (NSRect) contentRectForFrameRect:(NSRect)windowFrame
{
	NSRect contentRect = NSInsetRect(windowFrame, self.borderWidth, self.borderWidth);

	CGFloat offset = self.arrowHeight + self.distance;
	switch(self.popoverPosition) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			contentRect.size.width -= offset;
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			contentRect.size.width -= offset;
			contentRect.origin.x += offset;
			break;
			
		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			contentRect.size.height -= offset;
			contentRect.origin.y += offset;
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			contentRect.size.height -= offset;
			break;
	}
	
	return NSInsetRect(contentRect, self.viewMargin, self.viewMargin);
}

- (NSPoint) attachmentPoint
{
	return [self attachmentPointForRect:[self bounds]];
}

- (NSPoint) attachmentPointForRect:(NSRect)rect;
{
	NSPoint arrowheadPosition = NSZeroPoint;

	CGFloat minX = NSMinX(rect);
	CGFloat midX = NSMidX(rect);
	CGFloat maxX = NSMaxX(rect);

	CGFloat minY = NSMinY(rect);
	CGFloat midY = NSMidY(rect);
	CGFloat maxY = NSMaxY(rect);

    CGFloat arrowDistanceWidth  = (self.arrowWidth  / 2) + (2 * self.borderWidth);
	CGFloat arrowDistanceHeight = (self.arrowHeight / 2) + (2 * self.borderWidth);
	
    if(self.drawRoundCornerBesideArrow) {
        arrowDistanceWidth  += (self.cornerRadius + self.arrowOffset);
        arrowDistanceHeight += (self.cornerRadius + self.arrowOffset);
    }

	switch(self.popoverPosition) {
		case SFBPopoverPositionLeft:
			arrowheadPosition = NSMakePoint(maxX, midY);
			break;

		case SFBPopoverPositionLeftTop:
			arrowheadPosition = NSMakePoint(maxX, minY + arrowDistanceHeight);
			break;

		case SFBPopoverPositionLeftBottom:
			arrowheadPosition = NSMakePoint(maxX, maxY - arrowDistanceHeight);
			break;

		case SFBPopoverPositionRight:
			arrowheadPosition = NSMakePoint(minX, midY);
			break;

		case SFBPopoverPositionRightTop:
			arrowheadPosition = NSMakePoint(minX, minY + arrowDistanceHeight);
			break;

		case SFBPopoverPositionRightBottom:
			arrowheadPosition = NSMakePoint(minX, maxY - arrowDistanceHeight);
			break;
			
		case SFBPopoverPositionTop:
			arrowheadPosition = NSMakePoint(midX, minY);
			break;

		case SFBPopoverPositionTopLeft:
			arrowheadPosition = NSMakePoint(maxX - arrowDistanceWidth, minY);
			break;

		case SFBPopoverPositionTopRight:
			arrowheadPosition = NSMakePoint(minX + arrowDistanceWidth, minY);
			break;

		case SFBPopoverPositionBottom:
			arrowheadPosition = NSMakePoint(midX, maxY);
			break;

		case SFBPopoverPositionBottomLeft:
			arrowheadPosition = NSMakePoint(maxX - arrowDistanceWidth, maxY);
			break;

		case SFBPopoverPositionBottomRight:
			arrowheadPosition = NSMakePoint(minX + arrowDistanceWidth, maxY);
			break;
	}
	
	return arrowheadPosition;
}

- (void) drawRect:(NSRect)dirtyRect
{
	[NSBezierPath clipRect:dirtyRect];

	NSRect contentRect = [self contentRectForFrameRect:[self bounds]];
    NSBezierPath *path = [self popoverFramePathForContentRect:contentRect];

    [self.backgroundColor set];
    [path fill];
    
	[path setLineWidth:self.borderWidth];
    [self.borderColor set];
    [path stroke];
}

- (void) mouseDown:(NSEvent *)event
{
	NSPoint pointInView = [self convertPoint:[event locationInWindow] fromView:nil];
	NSRect contentRect = [self contentRectForFrameRect:[self bounds]];
    NSBezierPath *path = [self popoverFramePathForContentRect:contentRect];

	BOOL resize = [path containsPoint:pointInView] && !NSPointInRect(pointInView, contentRect);
	if((resize && !self.resizable) || (!resize && !self.movable))
		return;

	NSWindow *window = [self window];
    NSRect originalFrame = [window frame];

    NSRect eventLocation = { .origin = [event locationInWindow], .size = NSZeroSize };
    NSRect originalMouseLocation = [window convertRectFromScreen:eventLocation];
	
    for(;;) {
        NSEvent *newEvent = [window nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];
		
        if(NSLeftMouseUp == [newEvent type])
			break;
		
        NSRect eventLocation = { .origin = [newEvent locationInWindow], .size = NSZeroSize };
        NSRect newMouseLocation = [window convertRectFromScreen:eventLocation];
		NSPoint delta = NSMakePoint(newMouseLocation.origin.x - originalMouseLocation.origin.x, newMouseLocation.origin.y - originalMouseLocation.origin.y);
		
		NSRect newFrame = originalFrame;		
		if(!resize) {
			newFrame.origin.x += delta.x;
			newFrame.origin.y += delta.y;
		}
		else {
			newFrame.size.width += delta.x;
			newFrame.size.height -= delta.y;
			newFrame.origin.y += delta.y;
			
			NSRect newContentRect = [window contentRectForFrameRect:newFrame];

			NSSize maxSize = [window maxSize];
			NSSize minSize = [window minSize];

			if(newContentRect.size.width > maxSize.width)
				newFrame.size.width -= newContentRect.size.width - maxSize.width;
			else if(newContentRect.size.width < minSize.width)
				newFrame.size.width += minSize.width - newContentRect.size.width;

			if(newContentRect.size.height > maxSize.height) {
				newFrame.size.height -= newContentRect.size.height - maxSize.height;
				newFrame.origin.y += newContentRect.size.height - maxSize.height;
			}
			else if(newContentRect.size.height < minSize.height) {
				newFrame.size.height += minSize.height - newContentRect.size.height;
				newFrame.origin.y -= minSize.height - newContentRect.size.height;
			}
		}
		
		[window setFrame:newFrame display:YES animate:NO];
	}
}

@end

@implementation SFBPopoverWindowFrame (Private)

- (NSBezierPath *) popoverFramePathForContentRect:(NSRect)contentRect
{
	contentRect = NSInsetRect(contentRect, -self.viewMargin, -self.viewMargin);
	contentRect = NSInsetRect(contentRect, -self.borderWidth / 2, -self.borderWidth / 2);

	CGFloat minX = NSMinX(contentRect);
	CGFloat midX = NSMidX(contentRect);
	CGFloat maxX = NSMaxX(contentRect);

	CGFloat minY = NSMinY(contentRect);
	CGFloat midY = NSMidY(contentRect);
	CGFloat maxY = NSMaxY(contentRect);

	NSBezierPath *path = [NSBezierPath bezierPath];
	[path setLineJoinStyle:NSRoundLineJoinStyle];

	NSPoint currentPoint = NSMakePoint(minX, maxY);
	if(0 < self.cornerRadius && (self.drawRoundCornerBesideArrow || (SFBPopoverPositionBottomRight != self.popoverPosition && SFBPopoverPositionRightBottom != self.popoverPosition)))
		currentPoint.x += (self.cornerRadius + self.arrowOffset);

	NSPoint endOfLine = NSMakePoint(maxX, maxY);
	BOOL shouldDrawNextCorner = NO;
	if(0 < self.cornerRadius && (self.drawRoundCornerBesideArrow || (SFBPopoverPositionBottomLeft != self.popoverPosition && SFBPopoverPositionLeftBottom != self.popoverPosition))) {
		endOfLine.x -= (self.cornerRadius + self.arrowOffset);
		shouldDrawNextCorner = YES;
	}

	[path moveToPoint:currentPoint];

	// If arrow should be drawn at top-left point, draw it.
	if(SFBPopoverPositionBottomRight == self.popoverPosition)
		[self appendArrowToPath:path];
	else if(SFBPopoverPositionBottom == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(midX - (self.arrowWidth / 2), maxY)];
		[self appendArrowToPath:path];
	}
	else if(SFBPopoverPositionBottomLeft == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(endOfLine.x - self.arrowWidth, maxY)];
		[self appendArrowToPath:path];
	}

	// Line to end of this side.
	[path lineToPoint:endOfLine];

	// Rounded corner on top-right.
	if(shouldDrawNextCorner)
		[path appendBezierPathWithArcFromPoint:NSMakePoint(maxX, maxY) 
									   toPoint:NSMakePoint(maxX, maxY - self.cornerRadius)
										radius:self.cornerRadius];

	// Draw the right side, beginning at the top-right.
	endOfLine = NSMakePoint(maxX, minY);
	shouldDrawNextCorner = NO;
	if(0 < self.cornerRadius && (self.drawRoundCornerBesideArrow || (SFBPopoverPositionTopLeft != self.popoverPosition && SFBPopoverPositionLeftTop != self.popoverPosition))) {
		endOfLine.y += (self.cornerRadius + self.arrowOffset);
		shouldDrawNextCorner = YES;
	}

	// If arrow should be drawn at right-top point, draw it.
	if(SFBPopoverPositionLeftBottom == self.popoverPosition)
		[self appendArrowToPath:path];
	else if(SFBPopoverPositionLeft == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(maxX, midY + (self.arrowWidth / 2))];
		[self appendArrowToPath:path];
	}
	else if(SFBPopoverPositionLeftTop == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(maxX, endOfLine.y + self.arrowWidth)];
		[self appendArrowToPath:path];
	}

	// Line to end of this side.
	[path lineToPoint:endOfLine];

	// Rounded corner on bottom-right.
	if(shouldDrawNextCorner)
		[path appendBezierPathWithArcFromPoint:NSMakePoint(maxX, minY) 
									   toPoint:NSMakePoint(maxX - self.cornerRadius, minY)
										radius:self.cornerRadius];

	// Draw the bottom side, beginning at the bottom-right.
	endOfLine = NSMakePoint(minX, minY);
	shouldDrawNextCorner = NO;
	if(0 < self.cornerRadius && (self.drawRoundCornerBesideArrow || (SFBPopoverPositionTopRight != self.popoverPosition && SFBPopoverPositionRightTop != self.popoverPosition))) {
		endOfLine.x += (self.cornerRadius + self.arrowOffset);
		shouldDrawNextCorner = YES;
	}

	// If arrow should be drawn at bottom-right point, draw it.
	if(SFBPopoverPositionTopLeft == self.popoverPosition)
		[self appendArrowToPath:path];
	else if(SFBPopoverPositionTop == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(midX + (self.arrowWidth / 2), minY)];
		[self appendArrowToPath:path];
	}
	else if(SFBPopoverPositionTopRight == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(endOfLine.x + self.arrowWidth, minY)];
		[self appendArrowToPath:path];
	}

	// Line to end of this side.
	[path lineToPoint:endOfLine];

	// Rounded corner on bottom-left.
	if(shouldDrawNextCorner)
		[path appendBezierPathWithArcFromPoint:NSMakePoint(minX, minY) 
									   toPoint:NSMakePoint(minX, minY + self.cornerRadius)
										radius:self.cornerRadius];

	// Draw the left side, beginning at the bottom-left.
	endOfLine = NSMakePoint(minX, maxY);
	shouldDrawNextCorner = NO;
	if(0 < self.cornerRadius && (self.drawRoundCornerBesideArrow || (SFBPopoverPositionRightBottom != self.popoverPosition && SFBPopoverPositionBottomRight != self.popoverPosition))) {
		endOfLine.y -= (self.cornerRadius + self.arrowOffset);
		shouldDrawNextCorner = YES;
	}

	// If arrow should be drawn at left-bottom point, draw it.
	if(SFBPopoverPositionRightTop == self.popoverPosition)
		[self appendArrowToPath:path];
	else if(SFBPopoverPositionRight == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(minX, midY - (self.arrowWidth / 2))];
		[self appendArrowToPath:path];
	}
	else if(SFBPopoverPositionRightBottom == self.popoverPosition) {
		[path lineToPoint:NSMakePoint(minX, endOfLine.y - self.arrowWidth)];
		[self appendArrowToPath:path];
	}

	// Line to end of this side.
	[path lineToPoint:endOfLine];

	// Rounded corner on top-left.
	if(shouldDrawNextCorner)
		[path appendBezierPathWithArcFromPoint:NSMakePoint(minX, maxY) 
									   toPoint:NSMakePoint(minX + self.cornerRadius, maxY)
										radius:self.cornerRadius];

	[path closePath];
	return path;
}


- (void) appendArrowToPath:(NSBezierPath *)path
{
	if(!self.drawsArrow)
		return;

	NSPoint currentPoint = [path currentPoint];
	NSPoint tipPoint = currentPoint;
	NSPoint endPoint = currentPoint;

	switch(self.popoverPosition) {
		case SFBPopoverPositionLeft:
		case SFBPopoverPositionLeftTop:
		case SFBPopoverPositionLeftBottom:
			// Arrow points towards right. We're starting from the top.
			tipPoint.x += self.arrowHeight;
			tipPoint.y -= self.arrowWidth / 2;
			endPoint.y -= self.arrowWidth;
			break;

		case SFBPopoverPositionRight:
		case SFBPopoverPositionRightTop:
		case SFBPopoverPositionRightBottom:
			// Arrow points towards left. We're starting from the bottom.
			tipPoint.x -= self.arrowHeight;
			tipPoint.y += self.arrowWidth / 2;
			endPoint.y += self.arrowWidth;
			break;

		case SFBPopoverPositionTop:
		case SFBPopoverPositionTopLeft:
		case SFBPopoverPositionTopRight:
			// Arrow points towards bottom. We're starting from the right.
			tipPoint.y -= self.arrowHeight;
			tipPoint.x -= self.arrowWidth / 2;
			endPoint.x -= self.arrowWidth;
			break;

		case SFBPopoverPositionBottom:
		case SFBPopoverPositionBottomLeft:
		case SFBPopoverPositionBottomRight:
			// Arrow points towards top. We're starting from the left.
			tipPoint.y += self.arrowHeight;
			tipPoint.x += self.arrowWidth / 2;
			endPoint.x += self.arrowWidth;
			break;
	}

	[path lineToPoint:tipPoint];
	[path lineToPoint:endPoint];
}

@end
