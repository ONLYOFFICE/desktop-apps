//
//  KSNavigationController.mm
//
//  Copyright © 2016 Alex Gordiyenko. All rights reserved.
//

/*
 The MIT License (MIT)
 
 Copyright (c) 2016 A. Gordiyenko
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#import "KSNavigationController.h"
#import <Quartz/Quartz.h>

#import <stack>

@interface KSNavigationController () {
    std::stack<NSViewController<KSNavigationControllerCompatible> *__strong> _stack;
    NSViewController<KSNavigationControllerCompatible> *_rootViewController;
    NSView *_activeView;
    dispatch_once_t _addRootViewOnceToken;
}

@property (nonatomic, readonly) CATransition *transition;

@end

@implementation KSNavigationController

@synthesize transition = _transition;

#pragma mark - Life Cycle

- (instancetype)initWithRootViewController:(NSViewController<KSNavigationControllerCompatible> *)rootViewController {
    if (!rootViewController) {
        [NSException raise:NSInternalInconsistencyException format:@"`rootViewController` can't be nil"];
        return nil;
    }
    
    self = [super init];
    if (self) {
        _rootViewController = rootViewController;
        _rootViewController.navigationController = self;
    }
    return self;
}

- (void)loadView {
    self.view = [NSView new];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.wantsLayer = YES;
}

- (void)viewWillAppear {
    [super viewWillAppear];
    
    dispatch_once(&_addRootViewOnceToken, ^{
        if (_rootViewController) {
            _activeView = _rootViewController.view;
            [self addActiveViewAnimated:NO subtype:nil];
        }
    });
}

#pragma mark - Accessors

- (CATransition *)transition {
    if (!_transition) {
        _transition = [CATransition animation];
        _transition.type = kCATransitionPush;
        self.view.animations = @{@"subviews": _transition};
    }
    
    return _transition;
}

- (NSUInteger)viewControllersCount {
    return _stack.size() + 1;
}

- (NSViewController *)topViewController {
    if (_stack.size()) {
        return _stack.top();
    }
    
    return _rootViewController;
}

- (NSViewController<KSNavigationControllerCompatible> *)rootViewController {
    return _rootViewController;
}

#pragma mark - Public Methods

- (void)pushViewController:(NSViewController<KSNavigationControllerCompatible> *)viewController animated:(BOOL)animated {
    [_activeView removeFromSuperview];
    _stack.push(viewController);
    viewController.navigationController = self;
    _activeView = viewController.view;
    [self addActiveViewAnimated:animated
                        subtype:[NSApp userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionLeftToRight ? kCATransitionFromRight : kCATransitionFromLeft];
}

- (NSViewController<KSNavigationControllerCompatible> *)popViewControllerAnimated:(BOOL)animated {
    if (_stack.size() == 0) {
        return nil;
    }
    
    [_activeView removeFromSuperview];
    NSViewController<KSNavigationControllerCompatible> *retVal = _stack.top();
    _stack.pop();
    _activeView = _stack.size() > 0 ? _stack.top().view : _rootViewController.view;
    
    [self addActiveViewAnimated:animated subtype:[NSApp userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionLeftToRight ? kCATransitionFromLeft : kCATransitionFromRight];
    return retVal;
}

- (NSArray<NSViewController<KSNavigationControllerCompatible> *> *)viewControllers {
    if (!_rootViewController) {
        return nil;
    }
    
    std::stack<NSViewController<KSNavigationControllerCompatible> *__strong> tempStack = _stack;
    
    NSMutableArray *retVal = [[NSMutableArray alloc] initWithCapacity:_stack.size() + 1];
    while (!tempStack.empty()) {
        [retVal addObject:tempStack.top()];
        tempStack.pop();
    }
    
    [retVal addObject:_rootViewController];
    
    return [retVal copy];
}

- (NSArray<NSViewController<KSNavigationControllerCompatible> *> *)popToRootViewControllerAnimated:(BOOL)animated {
    if (_stack.size() == 0) {
        return nil;
    }
    
    NSMutableArray *retVal = [[NSMutableArray alloc] initWithCapacity:_stack.size()];
    for (NSUInteger i = 0; i < _stack.size(); i++) {
        [retVal addObject:[self popViewControllerAnimated:animated]];
    }
    
    return [retVal copy];
}

#pragma mark - Private Methods

- (void)addActiveViewAnimated:(BOOL)animated subtype:(NSString *)subtype {
    if (animated) {
        [CATransaction begin];
        CATransition *animation = [CATransition animation];
        animation.type = kCATransitionPush;
        animation.subtype = subtype;
        animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
        [self.view.animator addSubview:_activeView];
        [[self.view layer] addAnimation:animation forKey:@"Push"];
//        [[self topViewController] viewDidAppear]; // Revert if need synchronize
        [CATransaction commit];

        self.transition.subtype = subtype;
    } else {
        [self.view addSubview:_activeView];
    }
}

@end

