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
    return @{
             kEventCategory:self.category,
             kEventActionKey:self.action,
             kEventLabelKey:self.label,
             kEventValue:self.value
             };
}

@end
