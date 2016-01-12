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
    NSDictionary * licenceInfo = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsLicenseInfo];
    
    NSString * title = [NSString stringWithFormat:NSLocalizedString(@"Thank you for evaluating %@!", nil), productName];
    NSString * message = [NSString stringWithFormat:NSLocalizedString(@"Your subscription has expired.\n\nThe program works in the non-activated mode.\n\nNow you can prolong or upgrade your subscription with a discount.", nil)];
    
    if (licenceInfo && licenceInfo[@"licence"] && [licenceInfo[@"licence"] boolValue]) {
        if ([licenceInfo[@"demo"] boolValue]) {
            // trial
            message = [NSString stringWithFormat:NSLocalizedString(@"You are using a trial version of %@ office.\n\nThe trial period will end in %d days, after that you will not be able to create and edit documents.", nil), productName, MAX(0, [licenceInfo[@"daysLeft"] intValue])];
            
            if ([licenceInfo[@"daysLeft"] intValue] < 1) {
                // trial is end
                message = [NSString stringWithFormat:NSLocalizedString(@"The trial period of %@ office is over.\nYou cannot create and edit documents.", nil), productName];
            }
        } else {
            if ([licenceInfo[@"daysLeft"] intValue] < 14) {
                // license is ending
                message = [NSString stringWithFormat:NSLocalizedString(@"Your subscription is about to expire\n(%d days left).\nYou cannot create and edit documents.\nWe have a special offer for you.", nil), MAX(0, [licenceInfo[@"daysLeft"] intValue])];
            } else if ([licenceInfo[@"daysLeft"] intValue] < 1) {
                // license is end
                message = [NSString stringWithFormat:NSLocalizedString(@"Your subscription has expired.\nThe program works in the non-activated mode. You cannot create and edit documents.\nNow you can prolong or upgrade your subscription with a discount.", nil)];
            }
        }
    }
    
    [self.headerField setStringValue:title];
    [self.messageField setStringValue:message];
}

#pragma mark -
#pragma mark Actions

- (IBAction)onActivation:(NSButton *)sender {
    NSViewController * activationSuccessController = [self.storyboard instantiateControllerWithIdentifier:@"ASCActivationControllerId"];
    [self presentViewController:activationSuccessController animator:[ASCReplacePresentationAnimator new]];
}

- (IBAction)onContinue:(NSButton *)sender {
    [self.view.window close];
}

@end
