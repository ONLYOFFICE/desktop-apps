/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
//  ASCTrialController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/26/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTrialController.h"
#import "ASCConstants.h"
#import "ASCSharedSettings.h"
#import "ASCHelper.h"
#import "ASCReplacePresentationAnimator.h"
#import "applicationmanager.h"
#import "mac_application.h"
#import "NSString+OnlyOffice.h"
#import "ASCLicenseManager.h"

@interface ASCTrialController ()
@property (weak) IBOutlet NSTextField *headerField;
@property (weak) IBOutlet NSTextField *messageField;
@property (weak) IBOutlet NSButton *tryButton;
@property (weak) IBOutlet NSButton *activateButton;
@end

@implementation ASCTrialController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSString * productName = [ASCHelper appName];
    ASCLicenseInfo * license = [[ASCLicenseManager sharedInstance] licence];
        
    NSString * title = [NSString stringWithFormat:NSLocalizedString(@"Thank you for evaluating %@!", nil), productName];
    NSString * message = [NSString stringWithFormat:NSLocalizedString(@"Unregistered application version.\nYou cannot create and edit local files.", nil)];
    
    if (license.serverError) {
        title = NSLocalizedString(@"Application activation failed", nil);
        message = NSLocalizedString(@"Check your Internet connection settings and retry activation the application.", nil);
    } else if (license.demo) {
        // trial
        message = [NSString stringWithFormat:NSLocalizedString(@"You are using a trial version of the application.\nThe trial period will end in %d days, after that you will not be able to create and edit documents.", nil), MAX(0, license.daysLeft)];
        
        if (license.daysLeft < 1) {
            // trial is end
            message = [NSString stringWithFormat:NSLocalizedString(@"The trial period is over.\nYou cannot create and edit documents.", nil)];
        }
    } else if (license.business) {
        title = [NSString stringWithFormat:NSLocalizedString(@"Thank you for using %@!", nil), productName];
        
        if (license.daysLeft < 1) {
            // license is end
            message = [NSString stringWithFormat:NSLocalizedString(@"The license expired.\nYou cannot create and edit local files.", nil)];
        } else if (license.ending) {
            // license is ending
            message = [NSString stringWithFormat:NSLocalizedString(@"%d days are left until the license expiration.", nil), MAX(0, license.daysLeft)];
        }
    }
    
    [self.headerField setStringValue:title];
    [self.messageField setStringValue:message];
}

#pragma mark -
#pragma mark Actions

- (IBAction)onActivation:(NSButton *)sender {
    ASCVersionType licenseType = (ASCVersionType)[[NSUserDefaults standardUserDefaults] integerForKey:@"hasVersionMode"];
    
    if (licenseType == ASCVersionTypeForBusiness) {
        NSViewController * activationSuccessController = [self.storyboard instantiateControllerWithIdentifier:@"ASCActivationControllerId"];
        [self presentViewController:activationSuccessController animator:[ASCReplacePresentationAnimator new]];
    } else {
        [[ASCLicenseManager sharedInstance] createLicense:ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_FREE];
        [self.view.window close];
    }
}

- (IBAction)onContinue:(NSButton *)sender {
    [self.view.window close];
}

@end
