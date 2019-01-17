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
//  ASCTabsControl.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <objc/runtime.h>
#import "ASCTabsControl.h"
#import "ASCTabView.h"
#import "ASCTabViewCell.h"

static char kASCTabsScrollViewObservationContext;
static CGFloat const kASCTabsScrollButtonWidth = 24.f;
static NSString * const kASCTabsMulticastDelegateKey = @"asctabsmulticastDelegate";

#pragma mark -
#pragma mark ========================================================
#pragma mark ASCTabsMulticastDelegate
#pragma mark ========================================================
#pragma mark -

@implementation ASCTabsMulticastDelegate {
    // the array of observing delegates
    NSMutableArray* _delegates;
}

- (id)init {
    if (self = [super init]) {
        _delegates = [NSMutableArray array];
    }
    return self;
}

- (void)addDelegate:(id)delegate {
    [_delegates addObject:delegate];
}

- (BOOL)respondsToSelector:(SEL)aSelector {
    if ([super respondsToSelector:aSelector])
        return YES;
    
    for (id delegate in _delegates) {
        if ([delegate respondsToSelector:aSelector]) {
            return YES;
        }
    }
    return NO;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    NSMethodSignature* signature = [super methodSignatureForSelector:aSelector];
    
    if (!signature) {
        for (id delegate in _delegates) {
            if ([delegate respondsToSelector:aSelector]) {
                return [delegate methodSignatureForSelector:aSelector];
            }
        }
    }
    return signature;
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    for (id delegate in _delegates) {
        if ([delegate respondsToSelector:[anInvocation selector]]) {
            [anInvocation invokeWithTarget:delegate];
        }
    }
}

@end

#pragma mark -
#pragma mark ========================================================
#pragma mark ASCTabsControl
#pragma mark ========================================================
#pragma mark -

@interface ASCTabsControl() <ASCTabViewDelegate>
@property (nonatomic) NSScrollView *scrollView;
@property (nonatomic) NSView *tabsView;
@property (nonatomic) NSButton *scrollLeftButton;
@property (nonatomic) NSButton *scrollRightButton;
@end

@implementation ASCTabsControl

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

- (void)dealloc {
    [self stopObservingScrollView];
}

- (void)initialize {
    self.tabs = [NSMutableArray array];
    
    [self setWantsLayer:YES];
    [self setTranslatesAutoresizingMaskIntoConstraints:NO];
    
    self.minTabWidth = 50.0;
    self.maxTabWidth = 150.0;
    
    self.scrollView = [[NSScrollView alloc] initWithFrame:self.bounds];
    [self.scrollView setDrawsBackground:NO];
    [self.scrollView setHasHorizontalScroller:NO];
    [self.scrollView setHasVerticalScroller:NO];
    [self.scrollView setUsesPredominantAxisScrolling:YES];
    [self.scrollView setHorizontalScrollElasticity:NSScrollElasticityAllowed];
    [self.scrollView setVerticalScrollElasticity:NSScrollElasticityNone];
    [self.scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    self.tabsView = [[NSView alloc] initWithFrame:self.scrollView.bounds];
    self.scrollView.documentView = self.tabsView;
    
    [self addSubview:self.scrollView];
    
    void (^initScrollButton)(NSButton *) = ^ (NSButton *button) {
        [button setBezelStyle:NSThickSquareBezelStyle];
        [button setTitle:@""];
        [button setBordered:YES];
        [button setTarget:self];
        [button setBordered:NO];
        [button.cell sendActionOn:NSLeftMouseDownMask | NSPeriodicMask];
        [button setAutoresizingMask:NSViewMinXMargin];
        
        [self addSubview:button];
    };
    
    self.scrollLeftButton = [[NSButton alloc] initWithFrame:CGRectZero];
    [self.scrollLeftButton setAction:@selector(onScrollLeftButton:)];
    [self.scrollLeftButton setImage:[NSImage imageNamed:@"change-tab_left_normal"]];
//    [self.scrollLeftButton setImagePosition:NSImageRight];
    self.scrollRightButton = [[NSButton alloc] initWithFrame:CGRectZero];
    [self.scrollRightButton setAction:@selector(onScrollRightButton:)];
    [self.scrollRightButton setImage:[NSImage imageNamed:@"change-tab_right_normal"]];
//    [self.scrollRightButton setImagePosition:NSImageLeft];

    initScrollButton(self.scrollLeftButton);
    initScrollButton(self.scrollRightButton);
    
    [self.scrollLeftButton setFrame:CGRectMake(CGRectGetMaxX(self.frame) - kASCTabsScrollButtonWidth * 2, 0, kASCTabsScrollButtonWidth, CGRectGetHeight(self.frame))];
    [self.scrollRightButton setFrame:CGRectMake(CGRectGetMaxX(self.frame) - kASCTabsScrollButtonWidth, 0, kASCTabsScrollButtonWidth, CGRectGetHeight(self.frame))];
    
    [self startObservingScrollView];
    [self updateAuxiliaryButtons];
}

- (ASCTabsMulticastDelegate *)multicastDelegate{
    id multicastDelegate = objc_getAssociatedObject(self, (__bridge const void *)(kASCTabsMulticastDelegateKey));
    if (multicastDelegate == nil) {
        
        // if not, create one
        multicastDelegate = [[ASCTabsMulticastDelegate alloc] init];
        objc_setAssociatedObject(self, (__bridge const void *)(kASCTabsMulticastDelegateKey), multicastDelegate, OBJC_ASSOCIATION_RETAIN);
        
        // and set it as the delegate
        self.delegate = multicastDelegate;
    }
    
    return multicastDelegate;
}

- (void)setFrame:(NSRect)frame {
    [super setFrame:frame];
}

- (void)setTabs:(NSMutableArray *)tabs {
    for (ASCTabView * tab in _tabs) {
        [tab removeFromSuperview];
    }
    
    _tabs = tabs;
    
    for (ASCTabView * tab in _tabs) {
        tab.delegate    = self;
        tab.target      = self;
        tab.action      = @selector(handleSelectTab:);
        
        [tab sendActionOn:NSLeftMouseDownMask];
        
        [self.tabsView addSubview:tab];
    }
        
    [self layoutTabs:nil animated:NO];
    [self updateAuxiliaryButtons];
    [self invalidateRestorableState];
}

- (ASCTabView *)tabWithUUID:(NSString *)uuid {
    NSPredicate *bPredicate = [NSPredicate predicateWithFormat:@"SELF.uuid == %@", uuid];
    NSArray * filteredArray = [self.tabs filteredArrayUsingPredicate:bPredicate];
    
    return filteredArray.firstObject;
}

- (ASCTabView *)selectedTab {
    for (ASCTabView * tab in self.tabs) {
        if (tab.state == NSOnState) {
            return tab;
        }
    }
    
    return nil;
}

#pragma mark -
#pragma mark ScrollView Observation

- (void)startObservingScrollView {
    [self.scrollView addObserver:self forKeyPath:@"frame" options:0 context:&kASCTabsScrollViewObservationContext];
    [self.scrollView addObserver:self forKeyPath:@"documentView.frame" options:0 context:&kASCTabsScrollViewObservationContext];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewDidScroll:)
                                                 name:NSViewFrameDidChangeNotification
                                               object:self.scrollView];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewDidScroll:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:self.scrollView.contentView];
}

- (void)stopObservingScrollView
{
    [self.scrollView removeObserver:self forKeyPath:@"frame" context:&kASCTabsScrollViewObservationContext];
    [self.scrollView removeObserver:self forKeyPath:@"documentView.frame" context:&kASCTabsScrollViewObservationContext];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSViewFrameDidChangeNotification
                                                  object:self.scrollView];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSViewBoundsDidChangeNotification
                                                  object:self.scrollView.contentView];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (context == &kASCTabsScrollViewObservationContext) {
        [self updateAuxiliaryButtons];
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

#pragma mark -
#pragma mark Notifiacation Handlers

- (void)scrollViewDidScroll:(NSNotification *)notification {
    [self layoutTabs:nil animated:NO];
    [self updateAuxiliaryButtons];
    [self invalidateRestorableState];
}

#pragma mark -
#pragma mark Internal

- (void)layoutTabs:(NSArray *)tabs animated:(BOOL)anim {
    if (!tabs) {
        tabs = [self tabs];
    }
    
    __block CGFloat tabsViewWidth = 0.0;
    [tabs enumerateObjectsUsingBlock:^(ASCTabView *tabView, NSUInteger idx, BOOL *stop) {
        CGFloat fullSizeWidth = (int)CGRectGetWidth(self.scrollView.frame) / tabs.count;
        CGFloat buttonWidth = MAX(MIN(self.maxTabWidth, fullSizeWidth), self.minTabWidth);
        CGRect rect = CGRectMake(idx * buttonWidth, 0, buttonWidth, CGRectGetHeight(self.tabsView.frame));
        
        // Don't animate if it is hidden, as it will screw order of tabs
        if (anim && ![tabView isHidden]) {
            [[tabView animator] setFrame:rect];
        } else {
            [tabView setFrame:rect];
        }
        
//        if ([self.delegateInterceptor.receiver respondsToSelector:@selector(tabsControl:canSelectItem:)]) {
//            [[button cell] setSelectable:[self.delegateInterceptor.receiver tabsControl:self canSelectItem:[button.cell representedObject]]];
//        }
        
        tabView.tag = idx;
        
        tabsViewWidth += CGRectGetWidth(rect);
    }];
    
    [self.tabsView setFrame:CGRectMake(0.0, 0.0, tabsViewWidth, CGRectGetHeight(self.scrollView.frame))];
}

- (void)updateAuxiliaryButtons {
    NSClipView *contentView = self.scrollView.contentView;
    
    BOOL isDocumentClipped = (contentView.subviews.count > 0) && (NSMaxX([contentView.subviews[0] frame]) > NSWidth(contentView.bounds));
    BOOL needUpdateView = ([self.scrollLeftButton isHidden] == isDocumentClipped);
    
    if (isDocumentClipped) {
        [self.scrollLeftButton  setHidden:NO];
        [self.scrollRightButton setHidden:NO];
        
        BOOL isEnableLeft   = ([self firstTabLeftOutsideVisibleRect] != nil);
        BOOL isEnableRight  = ([self firstTabRightOutsideVisibleRect] != nil);
        
        [self.scrollLeftButton setEnabled:isEnableLeft];
        [self.scrollRightButton setEnabled:isEnableRight];
        
        [self.scrollLeftButton setImage:isEnableLeft
                                        ? [NSImage imageNamed:@"change-tab_left_normal"]
                                        : [NSImage imageNamed:@"change-tab_left_disabled"]];
        [self.scrollRightButton setImage:isEnableRight
                                        ? [NSImage imageNamed:@"change-tab_right_normal"]
                                        : [NSImage imageNamed:@"change-tab_right_disabled"]];
        
    } else {
        [self.scrollLeftButton  setHidden:YES];
        [self.scrollRightButton setHidden:YES];
    }
    
    if (needUpdateView) {
        [self.scrollView setFrame:CGRectMake(
                                             self.scrollView.frame.origin.x,
                                             self.scrollView.frame.origin.y,
                                             self.frame.size.width - (isDocumentClipped ? 2 * kASCTabsScrollButtonWidth : 0),
                                             self.scrollView.frame.size.height)];
    }
}

- (void)onScrollLeftButton:(id)sender {
    NSButton *tab = [self firstTabLeftOutsideVisibleRect];
    
    if (tab != nil) {
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setAllowsImplicitAnimation:YES];
            [tab scrollRectToVisible:[tab bounds]];
        } completionHandler:^{
            [self updateAuxiliaryButtons];
        }];
    }
}

- (void)onScrollRightButton:(id)sender {
    NSButton *tab = [self firstTabRightOutsideVisibleRect];
    
    if (tab != nil) {
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setAllowsImplicitAnimation:YES];
            [tab scrollRectToVisible:[tab bounds]];
        } completionHandler:^{
            [self updateAuxiliaryButtons];
        }];
    }
}

- (NSButton *)firstTabLeftOutsideVisibleRect {
    NSView *tabView = self.scrollView.documentView;
    NSRect visibleRect = tabView.visibleRect;
    
    for (NSUInteger index = [[tabView subviews] count]; index > 0; index--) {
        NSButton *button = [[tabView subviews] objectAtIndex:index - 1];
        
        if (NSMinX(button.frame) < NSMinX(visibleRect)) {
            return button;
        }
    }
    return nil;
}

- (NSButton *)firstTabRightOutsideVisibleRect {
    NSView *tabView = self.scrollView.documentView;
    NSRect visibleRect = tabView.visibleRect;
    
    for (NSButton *button in tabView.subviews) {
        if (NSMaxX(button.frame) > NSMaxX(visibleRect)) {
            return button;
        }
    }
    
    return nil;
}

- (void)setMinTabWidth:(CGFloat)minTabWidth {
    _minTabWidth = minTabWidth;
    
    [self layoutTabs:nil animated:NO];
    [self updateAuxiliaryButtons];
}

- (void)setMaxTabWidth:(CGFloat)maxTabWidth {
    if (maxTabWidth <= self.minTabWidth) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Max width '%.1f' must be larger than min width (%.1f)!", maxTabWidth, self.minTabWidth];
    }
    _maxTabWidth = maxTabWidth;
    
    [self layoutTabs:nil animated:NO];
    [self updateAuxiliaryButtons];
}

- (void)handleSelectTab:(ASCTabView *)selectedTab {
    for (ASCTabView * tab in self.tabs) {
        [tab setState:(tab == selectedTab) ? NSOnState : NSOffState];
    }
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [context setAllowsImplicitAnimation:YES];
//        [selectedTab.superview scrollRectToVisible:[selectedTab bounds]];
    } completionHandler:nil];
    
    if (_delegate && [_delegate respondsToSelector:@selector(tabs:didSelectTab:)]) {
        [_delegate tabs:self didSelectTab:selectedTab];
    }
    
    NSEvent *currentEvent = [NSApp currentEvent];
    
    if (currentEvent.clickCount > 1) {
        // On double click...
    } else {
        // watch for a drag event and initiate dragging if a drag is found...
        if ([self.window nextEventMatchingMask:NSLeftMouseUpMask | NSLeftMouseDraggedMask
                                     untilDate:[NSDate distantFuture]
                                        inMode:NSEventTrackingRunLoopMode dequeue:NO].type == NSLeftMouseDragged) {
            [self reorderTab:selectedTab withEvent:currentEvent];
            return; // no autoscroll
        }
    }
    
    // scroll to visible if either editing or selecting...
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [context setAllowsImplicitAnimation:YES];
        [selectedTab.superview scrollRectToVisible:selectedTab.frame];
    } completionHandler:nil];
    
    [self invalidateRestorableState];
}

- (void)selectTab:(ASCTabView *)selectedTab {
    for (ASCTabView * tab in self.tabs) {
        [tab setState:(tab == selectedTab) ? NSOnState : NSOffState];
    }
    
    if (selectedTab) {
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setAllowsImplicitAnimation:YES];
            [selectedTab.superview scrollRectToVisible:[selectedTab bounds]];
        } completionHandler:nil];
        
        if (_delegate && [_delegate respondsToSelector:@selector(tabs:didSelectTab:)]) {
            [_delegate tabs:self didSelectTab:selectedTab];
        }
        
        // scroll to visible if either editing or selecting...
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setAllowsImplicitAnimation:YES];
            [selectedTab.superview scrollRectToVisible:selectedTab.frame];
        } completionHandler:nil];
        
        [self invalidateRestorableState];
    } else {
        if (_delegate && [_delegate respondsToSelector:@selector(tabs:didSelectTab:)]) {
            [_delegate tabs:self didSelectTab:nil];
        }
    }
}

- (void)updateTab:(ASCTabView *)tab {
    if (_delegate && [_delegate respondsToSelector:@selector(tabs:didUpdateTab:)]) {
        [_delegate tabs:self didUpdateTab:tab];
    }
}

- (void)reorderTab:(ASCTabView *)tab withEvent:(NSEvent *)event {
    NSMutableArray *orderedTabs = [[NSMutableArray alloc] initWithArray:[self tabs]];
    
    CGFloat tabX = NSMinX(tab.frame);
    NSPoint dragPoint = [self.tabsView convertPoint:event.locationInWindow fromView:nil];
    
    ASCTabView * draggingTab = [tab copy];

    [self addSubview:draggingTab];
    [tab setHidden:YES];

    CGPoint prevPoint = dragPoint;
    
    while (1) {
        event = [self.window nextEventMatchingMask:NSLeftMouseDraggedMask | NSLeftMouseUpMask];
        
        CGFloat scrollPosition = [[self.scrollView contentView] documentVisibleRect].origin.x;
        
        if (event.type == NSLeftMouseUp) {
            [[NSAnimationContext currentContext] setCompletionHandler:^{
                [draggingTab removeFromSuperview];
                [tab setHidden:NO];

                // Calculate indexes
                NSString * uuidTab = tab.uuid;

                NSInteger oldIndex = [self.tabs indexOfObjectPassingTest:^BOOL(ASCTabView *  _Nonnull tabView, NSUInteger idx, BOOL * _Nonnull stop) {
                    if ([tabView.uuid isEqualToString:uuidTab]) {
                        *stop = YES;
                        return YES;
                    }
                    return NO;
                }];
                NSInteger newIndex = [orderedTabs indexOfObjectPassingTest:^BOOL(ASCTabView *  _Nonnull tabView, NSUInteger idx, BOOL * _Nonnull stop) {
                    if ([tabView.uuid isEqualToString:uuidTab]) {
                        *stop = YES;
                        return YES;
                    }
                    return NO;
                }];

                self.tabs = orderedTabs;

                if (oldIndex != newIndex && oldIndex != NSNotFound && newIndex != NSNotFound) {
                    if (_delegate && [_delegate respondsToSelector:@selector(tabs:didReorderTab:from:to:)]) {
                        [_delegate tabs:self didReorderTab:tab from:oldIndex to:newIndex];
                    }
                }
            }];
            
            [[draggingTab animator] setFrame:CGRectOffset(tab.frame, -scrollPosition, 0)];
            
            break;
        };
        
        NSPoint nextPoint = [self.tabsView convertPoint:event.locationInWindow fromView:nil];
        CGFloat nextX = tabX + (nextPoint.x - dragPoint.x);
        CGRect newRect = draggingTab.frame;
        
        if (nextX > CGRectGetWidth(self.scrollView.frame) - CGRectGetWidth(draggingTab.frame) + scrollPosition) {
            nextX = CGRectGetWidth(self.scrollView.frame) - CGRectGetWidth(draggingTab.frame) + scrollPosition;
        } else if (nextX < 0) {
            nextX = 0;
        }
        
        newRect.origin.x = nextX;
        draggingTab.frame = CGRectOffset(newRect, -scrollPosition, 0);
        
        BOOL movingLeft = (nextPoint.x < prevPoint.x);
        BOOL movingRight = (nextPoint.x > prevPoint.x);
        
        prevPoint = nextPoint;
        
        if (movingLeft && NSMidX(newRect) < NSMinX(tab.frame) && tab != orderedTabs.firstObject) {
            // shift left
            NSUInteger index = [orderedTabs indexOfObject:tab];
            [orderedTabs exchangeObjectAtIndex:index withObjectAtIndex:index - 1];
            [self layoutTabs:orderedTabs animated:YES];
        }
        else if (movingRight && NSMidX(newRect) > NSMaxX(tab.frame) && tab != orderedTabs.lastObject) {
            // shift right
            NSUInteger index = [orderedTabs indexOfObject:tab];
            [orderedTabs exchangeObjectAtIndex:index + 1 withObjectAtIndex:index];
            [self layoutTabs:orderedTabs animated:YES];
        }
    }
}

- (void)addTab:(ASCTabView *)tab selected:(BOOL)selected {
    if (tab) {
        tab.hidden = YES;
        [self.tabs addObject:tab];
        
        if (_delegate && [_delegate respondsToSelector:@selector(tabs:didResize:)]) {
            [_delegate tabs:self didResize:CGRectZero];
        }
        
        [self layoutTabs:nil animated:YES];
        
        tab.hidden = NO;
        tab.frame = CGRectOffset(tab.frame, 0, -CGRectGetHeight(self.scrollView.frame));
        
        [self.tabsView setFrame:CGRectMake(0.0, 0.0, CGRectGetMaxX(tab.frame), CGRectGetHeight(self.scrollView.frame))];
        
        tab.delegate    = self;
        tab.target      = self;
        tab.action      = @selector(handleSelectTab:);
        [tab sendActionOn:NSLeftMouseDownMask];
        
        [self.tabsView addSubview:tab];
        
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [context setAllowsImplicitAnimation:YES];
            tab.animator.frame = CGRectOffset(tab.frame, 0, CGRectGetHeight(self.scrollView.frame));
            [tab.superview scrollRectToVisible:tab.frame];
        } completionHandler:^{
            [self layoutTabs:nil animated:NO];
            [self updateAuxiliaryButtons];
            [self invalidateRestorableState];
        }];
        
        if (_delegate && [_delegate respondsToSelector:@selector(tabs:didAddTab:)]) {
            [_delegate tabs:self didAddTab:tab];
        }
        
        if (selected) {
            [self selectTab:tab];
        }
    }
}

- (void)addTab:(ASCTabView *)tab {
    [self addTab:tab selected:YES];
}

- (void)removeTab:(ASCTabView *)tab selected:(BOOL)selected {
    if (tab) {
        NSInteger tabIndex = tab.tag;
        
        [self.tabs removeObject:tab];
        
        if (_delegate && [_delegate respondsToSelector:@selector(tabs:didRemovedTab:)]) {
            [_delegate tabs:self didRemovedTab:tab];
        }
        
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            tab.animator.frame = CGRectOffset(tab.frame, 0, -CGRectGetHeight(self.scrollView.frame));
        } completionHandler:^{
            [tab removeFromSuperview];
            
            if (_delegate && [_delegate respondsToSelector:@selector(tabs:didResize:)]) {
                [_delegate tabs:self didResize:CGRectZero];
            }
            
            [self layoutTabs:nil animated:NO];
            [self updateAuxiliaryButtons];
            [self invalidateRestorableState];
            
            if (selected) {                
                NSInteger tabsCount = [self.tabs count];
                
                if (tabsCount > tabIndex) {
                    [self selectTab:[self.tabs objectAtIndex:tabIndex]];
                } else if (tabsCount > tabIndex - 1 && tabIndex - 1 >= 0) {
                    [self selectTab:[self.tabs objectAtIndex:tabIndex - 1]];
                } else if (tabsCount > 0) {
                    [self selectTab:[self.tabs objectAtIndex:tabsCount - 1]];
                } else {
                    [self selectTab:nil];
                }
            }
        }];
    }
}

- (void)removeTab:(ASCTabView *)tab {
    [self removeTab:tab selected:YES];
}

- (void)removeAllTabs {
    for (ASCTabView * tab in self.tabs) {
        [tab removeFromSuperview];
    }
    
    [self.tabs removeAllObjects];
    
    [self layoutTabs:nil animated:NO];
    [self updateAuxiliaryButtons];
    [self invalidateRestorableState];
    
    [self selectTab:nil];
}

#pragma mark -
#pragma mark ASCTabView Delegate

- (void)tabDidClose:(ASCTabView *)tab {
    if (_delegate && [_delegate respondsToSelector:@selector(tabs:didRemovedTab:)]) {
        if (![_delegate tabs:self willRemovedTab:tab]) {
            return;
        }
    }
    [self removeTab:tab];
}

- (void)tabDidUpdate:(ASCTabView *)tab {
    if (_delegate && [_delegate respondsToSelector:@selector(tabs:didUpdateTab:)]) {
        [_delegate tabs:self didUpdateTab:tab];
    }
}

#pragma mark -
#pragma mark Drawning

- (void)drawRect:(NSRect)dirtyRect {
//    [[NSColor redColor] setFill];
//    NSRectFill(dirtyRect);
    
    [super drawRect:dirtyRect];
}

@end
