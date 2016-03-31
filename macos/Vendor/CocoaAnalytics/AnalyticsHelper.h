//
//  AnalyticsHelper.h
//
//  Created by Stephen Lind on 9/26/13.
//

#import <Foundation/Foundation.h>

/*
 SWL Sept 2013
 
 AnalyticsHelper is a singleton interface for recording and sending events to your google analytics account.
 
 This class is *not* thread safe. All calls to it should be performed from the main thread. Reports are sent on a background thread. If the send fails, an error event will be sent the next time analytics are reported.
 */

@interface AnalyticsHelper : NSObject

/*!
 @brief create/access the AnalyticsHelper singleton object.
 
 @description This object should *only* be accessed through this method.
 */
+ (AnalyticsHelper*)sharedInstance;


/*!
 @brief Begin the session with your Google account + app data
 
 @desc  This begins the reporting interval, which will allow the app to appear used
 in the Google Analytics real-time views. This also begins the periodic reporting
 interval which will send cached events to the server.
 */
- (void)beginPeriodicReportingWithAccount:(NSString*)googleAccountIdentifier
                                     name:(NSString*)appName
                                  version:(NSString*)appVersion;


/*!
 @brief Shut-down method to keep track of application uptime
 
 @desc  Calculates how long the app has been open and stores the in-memory data for later reports.
        This should be called when applicationWillTerminate, and should be accompanied by a
        [NSUserDefaults.standardUserDefaults synchronize];
 */
- (void)handleApplicationWillClose;


/*!
 @brief OS X version of the user's current "screen"
 
 @desc  On OS X, the user can actually have multiple screens up. So it is
        the caller's job to use this call wisely in order to get sensible
        data on the Google Analytics reporting screens.
 */
- (void)recordScreenWithName:(NSString*)screenName;



/*!
 @brief Record a Google Analytics "event" with the supplied data
 
 @desc  These events are cached locally, not sent immediately. Once
        the reporting interval elapses, they will be sent all at once.
 */
- (void)recordCachedEventWithCategory:(NSString*)eventCategory
                                 action:(NSString*)eventAction
                                  label:(NSString*)eventLabel
                                  value:(NSNumber*)eventValue;


@end