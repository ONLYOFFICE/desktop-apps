//
//  AnalyticsEvent.h
//
//  Created by Stephen Lind on 9/30/13.
//  Copyright (c) 2013 Stephen Lind. All rights reserved.
//

#import <Foundation/Foundation.h>

/*!
 @brief A serializable object for storing analytics events on disk.
 
 @description   This object is an Objective-C construct to create type-safe data
 which can be stored within NSUserDefaults until it is eventually sent to the
 analytics server.
 */
@interface AnalyticsEvent : NSObject

@property (strong) NSString *category;
@property (strong) NSString *action;
@property (strong) NSString *label;
@property (strong) NSNumber *value;

+ (AnalyticsEvent*)analyticsEventWithDictionary:(NSDictionary*)dict;
- (NSDictionary*)dictionaryRepresenation;

@end
