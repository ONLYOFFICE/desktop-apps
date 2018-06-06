//
//  KSNavigationController.h
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

#import <Cocoa/Cocoa.h>

@class KSNavigationController;

/**
 @brief Protocol your `NSViewController` subclass must conform to.
 @description Conform to this protocol if you want your `NSViewController` subclass to work with `KSNavigationController`. You must synthesize `navigationController` property explicitly.
 */
@protocol KSNavigationControllerCompatible <NSObject>
/**
 Navigation controller object which holds your `NSViewController` subclass.
 @attention You must synthesize this property explicitly.
 @warning Do not set this properly by yourself.
 */
@property (weak, nonatomic) KSNavigationController *navigationController;
@end

/**
 @brief This class mimics UIKit's `UINavigationController` behavior.
 @description Navigation bar is not implemented. All methods must be called from main thread.
 */
NS_CLASS_AVAILABLE(10_5, NA)
@interface KSNavigationController : NSViewController

/** The current view controller stack. */
@property (nonatomic, readonly) NSArray<__kindof NSViewController<KSNavigationControllerCompatible> *> *viewControllers;
/** Number of view controllers currently in stack. */
@property (nonatomic, readonly) NSUInteger viewControllersCount;
/** The top view controller on the stack. */
@property (nonatomic, readonly) NSViewController *topViewController;
/** The root view controller on the bottom of the stack. */
@property (nonatomic, readonly) NSViewController<KSNavigationControllerCompatible> *rootViewController;

/**
 @brief Initializes and returns a newly created navigation controller.
 @description This method throws exception if `rootViewController` is nil.
 @param rootViewController The view controller that resides at the bottom of the navigation stack.
 @return The initialized navigation controller object or nil if there was a problem initializing the object.
 */
- (instancetype)initWithRootViewController:(NSViewController<KSNavigationControllerCompatible> *)rootViewController;
/**
 Pushes a view controller onto the receiver’s stack and updates the display. Uses a horizontal slide transition.
 @param viewController The view controller to push onto the stack.
 @param animated Set this value to YES to animate the transition, NO otherwise.
 */
- (void)pushViewController:(NSViewController<KSNavigationControllerCompatible> *)viewController animated:(BOOL)animated;
/**
 Pops the top view controller from the navigation stack and updates the display.
 @param animated Set this value to YES to animate the transition, NO otherwise.
 @return The popped view controller.
 */
- (NSViewController<KSNavigationControllerCompatible> *)popViewControllerAnimated:(BOOL)animated;
/**
 Pops until there's only a single view controller left on the stack. Returns the popped view controllers.
 @param animated Set this value to YES to animate the transitions if any, NO otherwise.
 @return The popped view controllers.
 */
- (NSArray<__kindof NSViewController<KSNavigationControllerCompatible> *> *)popToRootViewControllerAnimated:(BOOL)animated;

@end
