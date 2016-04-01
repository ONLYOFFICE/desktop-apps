//
//  AnalyticsEvent.m
//  DraftControl
//
//  Created by Stephen Lind on 9/30/13.
//  Copyright (c) 2013 Stephen Lind. All rights reserved.
//

#import "AnalyticsEvent.h"

static NSString *const kEventActionKey = @"eventAction";
static NSString *const kEventCategory = @"eventCategory";
static NSString *const kEventLabelKey = @"eventLabel";
static NSString *const kEventValue = @"eventValue";

@implementation AnalyticsEvent

+ (AnalyticsEvent*)analyticsEventWithDictionary:(NSDictionary*)dict {
    AnalyticsEvent *analyticsEvent = [[AnalyticsEvent alloc] init];
    
    analyticsEvent.action = [dict objectForKey:kEventActionKey];
    analyticsEvent.category = [dict objectForKey:kEventCategory];
    analyticsEvent.value = [dict objectForKey:kEventValue];
    analyticsEvent.label = [dict objectForKey:kEventLabelKey];
    
    return analyticsEvent;
}

- (NSDictionary*)dictionaryRepresenation {
    // We expect all of the values to be filled in here. If not, BOOM.
    
    NSMutableDictionary * dictionary = [NSMutableDictionary dictionary];
    
    if (self.category) {
        [dictionary setObject:self.category forKey:kEventCategory];
    }
    
    if (self.action) {
        [dictionary setObject:self.action forKey:kEventActionKey];
    }
    
    if (self.label) {
        [dictionary setObject:self.label forKey:kEventLabelKey];
    }
    
    if (self.value) {
        [dictionary setObject:self.value forKey:kEventValue];
    }
    
    return dictionary;
}

@end
