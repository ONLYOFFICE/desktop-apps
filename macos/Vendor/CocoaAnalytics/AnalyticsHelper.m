//
//  AnalyticsHelper.m
//  DraftControl
//
//  Created by Stephen Lind on 9/26/13.
//  Copyright (c) 2013 Stephen Lind. All rights reserved.
//

#import "AnalyticsHelper.h"
#import "AnalyticsEvent.h"

// User Defaults Keys
static NSString *const kMachineIdentifierKey = @"analyticsMachineIdentifier";
static NSString *const kRecordedScreensKey = @"analyticsScreens";
static NSString *const kRecordedEventsKey = @"analyticsEvents";
static NSString *const kSendErrorCountKey = @"analyticsSendErrors";
static NSString *const kAnalyticsUrl = @"http://google-analytics.com/collect";

/*
 Google will time out a session after 30 minutes of inactivity. The "real time" 
 view shows when an event has been triggered in the last 6-7 minutes. So these views 
 show the right data, send at 5 minute intervals.
 */
static const float kDefaultSendInterval = 60 * 5; // 5 minutes

/*
 In order to test this class without hitting the network, we create a network handler
 which performs the payload creation and url request tasks. In testing this is replaced
 with a testing network handler, which simulates the network success or failure.
 */
@protocol AnalyticsNetworkHandler <NSObject>
- (BOOL)sendPayload:(NSData*)payload;
- (NSData*)createEventPayload:(AnalyticsEvent*)analyticsEvent
            machineIdentifier:(NSString*)machineIdentifier
            accountIdentifier:(NSString*)accountIdentifier
                      appName:(NSString*)appName
                   appVersion:(NSString*)appVersion;

- (NSData*)createScreenViewPayload:(NSString*)viewName
                 machineIdentifier:(NSString*)machineIdentifier
                 accountIdentifier:(NSString*)accountIdentifier
                           appName:(NSString*)appName
                        appVersion:(NSString*)appVersion;
@end

@interface ProductionNetworkHandler : NSObject<AnalyticsNetworkHandler>
- (BOOL)sendPayload:(NSData *)payload;
- (NSData*)createScreenViewPayload:(NSString*)viewName
                 machineIdentifier:(NSString*)machineIdentifier
                 accountIdentifier:(NSString*)accountIdentifier
                           appName:(NSString*)appName
                        appVersion:(NSString*)appVersion;

- (NSData*)createEventPayload:(AnalyticsEvent*)analyticsEvent
            machineIdentifier:(NSString*)machineIdentifier
            accountIdentifier:(NSString*)accountIdentifier
                      appName:(NSString*)appName
                   appVersion:(NSString*)appVersion;
@end

@interface AnalyticsHelper()
@property (strong) id<AnalyticsNetworkHandler> networkHandler;
@property (strong) NSMutableArray *cachedScreens;
@property (strong) NSMutableArray *cachedEvents;
@property NSUInteger sendInterval;
@property (strong) NSString *googleAccountIdentifier;
@property (strong) NSString *appVersion;
@property (strong) NSString *appName;

@property (strong) NSDate* launchDate;
@property (strong) NSTimer* timer;
@property BOOL allowReporting;
@property BOOL reportInProgress;
@end

@implementation AnalyticsHelper

+ (id)allocWithZone:(NSZone *)zone {
    return [AnalyticsHelper sharedInstance];
}

+ (AnalyticsHelper*)sharedInstance {
    static AnalyticsHelper *analyticsHelper;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        analyticsHelper = [[super allocWithZone:nil] init];
    });
    
    return analyticsHelper;
}

- (id)init {
    self = [super init];
    if (self) {
        _launchDate = NSDate.date;
        _networkHandler = [[ProductionNetworkHandler alloc] init];
        _cachedScreens = [NSMutableArray new];
        _cachedEvents = [NSMutableArray new];
        _sendInterval = kDefaultSendInterval;
        _appVersion = @"0.0";
        _appName = @"My App";
    }
    return self;
}

- (void)setReportInterval:(NSUInteger)intervalInSeconds {
    self.sendInterval = intervalInSeconds;
    [self resetTimer];
}

- (NSUInteger)getErrorCount {
    return [[NSUserDefaults.standardUserDefaults objectForKey:kSendErrorCountKey] unsignedIntegerValue];
}

- (void)recordSendError {
    [self recordSendErrorWithCount:1];
}

- (void)recordSendErrorWithCount:(NSUInteger)count {
    NSUInteger prevErrorCount = [self getErrorCount];
    NSNumber *errorCount = [NSNumber numberWithUnsignedInteger:prevErrorCount + count];
    [NSUserDefaults.standardUserDefaults setObject:errorCount forKey:kSendErrorCountKey];
}

- (void)clearErrors {
    [NSUserDefaults.standardUserDefaults removeObjectForKey:kSendErrorCountKey];
}

- (void)handleApplicationWillClose {
    // We are about to close, stop any future reports
    self.allowReporting = NO;
    
    // Calculate how long the app has been open (really this class) and store an
    // event with the duration as the value.
    NSNumber *durationInSeconds = [NSNumber numberWithUnsignedInteger:(NSUInteger)[NSDate.date timeIntervalSinceDate:self.launchDate]];
    NSString *dateString = [NSDateFormatter localizedStringFromDate:self.launchDate dateStyle:NSDateFormatterShortStyle timeStyle:NSDateFormatterShortStyle];
    [self recordCachedEventWithCategory:@"Application" action:@"Close" label:dateString value:durationInSeconds];
    
    // This must be called last
    [self saveCachedScreensToDisk];
    [self saveCachedEventsToDisk];
}

- (void)recordScreenWithName:(NSString*)screenName {
    [self.cachedScreens addObject:screenName];
}

- (void)recordCachedEventWithCategory:(NSString*)eventCategory
                               action:(NSString*)eventAction
                                label:(NSString*)eventLabel
                                value:(NSNumber*)eventValue
{
    AnalyticsEvent *analyticsEvent = [[AnalyticsEvent alloc] init];
    analyticsEvent.category = eventCategory;
    analyticsEvent.action = eventAction;
    analyticsEvent.label = eventLabel;
    analyticsEvent.value = eventValue;
    
    [self.cachedEvents addObject:analyticsEvent];
}

- (void)saveCachedScreensToDisk {
    // Get the previously recorded screen names
    NSMutableArray *recordedScreens = [NSMutableArray new];
    NSArray *previouslyRecordedScreens = [self recordedEventDictionaries];
    if (previouslyRecordedScreens) {
        [recordedScreens addObjectsFromArray:previouslyRecordedScreens];
    }
    
    // Convert the events to dictionaries for storage, add to the existing events
    for (NSString *screenName in self.cachedScreens) {
        [recordedScreens addObject:screenName];
    }
    
    [NSUserDefaults.standardUserDefaults setObject:recordedScreens forKey:kRecordedScreensKey];
    [self.cachedScreens removeAllObjects];
}

- (void)saveCachedEventsToDisk {
    // Get the previously recorded events
    NSMutableArray *recordedEvents = [NSMutableArray new];
    NSArray *previouslyRecordedEvents = [self recordedEventDictionaries];
    if (previouslyRecordedEvents) {
        [recordedEvents addObjectsFromArray:previouslyRecordedEvents];
    }
    
    // Convert the events to dictionaries for storage, add to the existing events
    for (AnalyticsEvent *event in self.cachedEvents) {
        NSDictionary *eventDict = [event dictionaryRepresenation];
        [recordedEvents addObject:eventDict];
    }

    [NSUserDefaults.standardUserDefaults setObject:recordedEvents forKey:kRecordedEventsKey];
    [self.cachedEvents removeAllObjects];
}

- (void)clearRecordedScreens {
    [NSUserDefaults.standardUserDefaults removeObjectForKey:kRecordedScreensKey];
}

- (void)clearRecordedEvents {
    [NSUserDefaults.standardUserDefaults removeObjectForKey:kRecordedEventsKey];
}

- (NSArray*)recordedEventDictionaries {
    return [NSUserDefaults.standardUserDefaults objectForKey:kRecordedEventsKey];
}

- (NSArray*)recordedScreens {
    return [NSUserDefaults.standardUserDefaults objectForKey:kRecordedScreensKey];
}

- (NSArray*)recordedEvents {
    // Get the list of recorded events from disk
    NSArray *eventDicts = [self recordedEventDictionaries];
    NSMutableArray *events = [[NSMutableArray alloc] init];
    for (NSDictionary *eventDict in eventDicts) {
        AnalyticsEvent *e = [AnalyticsEvent analyticsEventWithDictionary:eventDict];
        [events addObject:e];
    }
    
    return [NSArray arrayWithArray:events];
}

- (NSString*)getUniqueMachineIdentifier {
    NSString *uniqueId = [NSUserDefaults.standardUserDefaults stringForKey:kMachineIdentifierKey];
    if (uniqueId == nil) {
        uniqueId = [NSUUID.UUID UUIDString];
        [NSUserDefaults.standardUserDefaults setObject:uniqueId forKey:kMachineIdentifierKey];
    }
    
    return uniqueId;
}

- (void)sendReport {
    if (self.reportInProgress == NO) {
        self.reportInProgress = YES;
        
        // Retrieve any stored screens
        NSArray *screens = [self recordedScreens];
        
        // Retrieve any stored events
        NSArray *events = [self recordedEvents];
        
        // Retrieve analytics send error count
        NSUInteger recordedErrorCount = [self getErrorCount];
        
        NSString *machineIdentifier = [self getUniqueMachineIdentifier];
        
        // Clear non-error stored screens
        [self clearRecordedScreens];
        
        // Clear non-error stored events
        [self clearRecordedEvents];
        
        dispatch_async(dispatch_get_global_queue(0,0), ^{
            // A count for errors occurred in this report
            NSUInteger errorCount = 0;
            
            BOOL errorsSent = NO;
            
            // Send recorded screen events
            for (NSString *screenName in screens) {
                NSData *screenPayload = [self.networkHandler createScreenViewPayload:screenName
                                                                   machineIdentifier:machineIdentifier
                                                                   accountIdentifier:self.googleAccountIdentifier
                                                                             appName:self.appName
                                                                          appVersion:self.appVersion];
                BOOL success = [self.networkHandler sendPayload:screenPayload];
                if (!success) {
                    errorCount += 1;
                }
            }
            
            // Send the other recorded events
            for (AnalyticsEvent *event in events) {
                NSData *eventPayload = [self.networkHandler createEventPayload:event
                                                             machineIdentifier:machineIdentifier
                                                             accountIdentifier:self.googleAccountIdentifier
                                                                       appName:self.appName appVersion:self.appVersion];
                BOOL success = [self.networkHandler sendPayload:eventPayload];
                if (!success) {
                    errorCount += 1;
                }
            }
            
            if (recordedErrorCount > 0) {
                AnalyticsEvent *errorEvent = [[AnalyticsEvent alloc] init];
                errorEvent.category = @"Error";
                errorEvent.action = @"Analytics Send Failure";
                errorEvent.value = [NSNumber numberWithUnsignedInteger:recordedErrorCount];
                
                NSData *errorPayload = [self.networkHandler createEventPayload:errorEvent
                                                             machineIdentifier:machineIdentifier
                                                             accountIdentifier:self.googleAccountIdentifier
                                                                       appName:self.appName
                                                                    appVersion:self.appVersion];
                
                errorsSent = [self.networkHandler sendPayload:errorPayload];
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                if (errorCount > 0) {
                    [self recordSendErrorWithCount:errorCount];
                } else if (errorsSent) {
                    [self clearErrors];
                }
                self.reportInProgress = NO;
            });
        });
    }
}

- (void)beginPeriodicReportingWithAccount:(NSString *)googleAccountIdentifier
                                     name:(NSString *)appName
                                  version:(NSString *)appVersion {
    
    self.googleAccountIdentifier = googleAccountIdentifier;
    self.appName = appName;
    self.appVersion = appVersion;
    self.allowReporting = YES;
    [self resetTimer];
    [self createAndSendReport:nil];
}

- (void)createAndSendReport:(id)sender {
    if (self.allowReporting) {
        [self saveCachedScreensToDisk];
        [self saveCachedEventsToDisk];
        [self sendReport];
    }
}

- (void)resetTimer {
    [self.timer invalidate];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:self.sendInterval target:self selector:@selector(createAndSendReport:) userInfo:nil repeats:YES];
}

#pragma mark testing only

- (void)testSetNetworkHandler:(id<AnalyticsNetworkHandler>)handler {
    self.networkHandler = handler;
}

@end

@implementation ProductionNetworkHandler

- (NSString*)platformHeaderString {
    // Generate a platform string http header. Found here:
    //
    // https://github.com/Coppertino/AnalyticEverything/blob/master/AnalyticEverything/GATracking.m
    //
    NSDictionary *osInfo = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    
    NSLocale *currentLocale = [NSLocale autoupdatingCurrentLocale];
    NSString *headerString = [NSString stringWithFormat:@"GoogleAnalytics/2.0 (Macintosh; Intel %@ %@; %@-%@)",
                    osInfo[@"ProductName"], [osInfo[@"ProductVersion"] stringByReplacingOccurrencesOfString:@"." withString:@"_"],
                    [currentLocale objectForKey:NSLocaleLanguageCode], [currentLocale objectForKey:NSLocaleCountryCode]];
    
    return headerString;
}


- (BOOL)sendPayload:(NSData *)payload {
    BOOL ok = NO;
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc]
                                    initWithURL:[NSURL URLWithString:kAnalyticsUrl]];
    
    [request setHTTPMethod:@"POST"];
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"content-type"];
    [request setValue:[self platformHeaderString] forHTTPHeaderField:@"User-Agent"];
    [request setHTTPBody:payload];

    NSError *error = nil;
    [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:&error];
    
    if (error) {
        //NSLog(@"could not send payload error: %@", error);
    } else {
        ok = YES;
    }
    return ok;
}

- (NSData*)createEventPayload:(AnalyticsEvent*)analyticsEvent
            machineIdentifier:(NSString*)machineIdentifier
            accountIdentifier:(NSString*)accountIdentifier
                      appName:(NSString*)appName
                   appVersion:(NSString*)appVersion {
    
    /*
     SWL url constructed from:
     https://developers.google.com/analytics/devguides/collection/protocol/v1/devguide
     
     v=1             // Version.
     &tid=UA-XXXX-Y  // Tracking ID / Web property / Property ID.
     &cid=555        // Anonymous Client ID.
     
     &an=funTimes    // App name.
     
     &t=event        // Event hit type
     &ec=video       // Event Category. Required.
     &ea=play        // Event Action. Required.
     */
    
    NSString *payloadString = [NSString stringWithFormat:@"v=1&tid=%@&cid=%@&an=%@&t=event&ec=%@&ea=%@&el=%@&ev=%@",
                               accountIdentifier, // account id
                               machineIdentifier, // client id
                               appName, // app name
                               analyticsEvent.category,
                               analyticsEvent.action,
                               analyticsEvent.label,
                               analyticsEvent.value
                               ];
    
    payloadString = [payloadString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    
    return [payloadString dataUsingEncoding:NSUTF8StringEncoding];
}


- (NSData*)createScreenViewPayload:(NSString*)viewName
                 machineIdentifier:(NSString*)machineIdentifier
                 accountIdentifier:(NSString*)accountIdentifier
                           appName:(NSString*)appName
                        appVersion:(NSString*)appVersion {
    /*
     SWL url constructed from:
     https://developers.google.com/analytics/devguides/collection/protocol/v1/devguide
     
     v=1             // Version.
     &tid=UA-XXXX-Y  // Tracking ID / Web property / Property ID.
     &cid=555        // Anonymous Client ID.
     
     &t=appview      // Appview hit type.
     &an=funTimes    // App name.
     &av=4.2.0       // App version.
     
     &cd=Home        // Screen name / content description.
     */
    
    NSString *payloadString = [NSString stringWithFormat:@"v=1&tid=%@&cid=%@&t=appview&an=%@&av=%@&cd=%@",
                               accountIdentifier, // account id
                               machineIdentifier, // client id
                               appName, // app name
                               appVersion, // app version
                               viewName // Screen name
                               ];
    
    payloadString = [payloadString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    
    return [payloadString dataUsingEncoding:NSUTF8StringEncoding];
}


@end
