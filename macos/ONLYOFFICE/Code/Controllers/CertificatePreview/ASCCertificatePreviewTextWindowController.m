//
//  ASCCertificatePreviewTextWindowController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import "ASCCertificatePreviewTextWindowController.h"

@interface ASCCertificatePreviewTextWindowController () <NSWindowDelegate>

@end

@implementation ASCCertificatePreviewTextWindowController

- (void)windowDidLoad {
    [super windowDidLoad];
}

- (void)windowWillClose:(NSNotification *)notification {
    [NSApp stopModal];
}

@end
