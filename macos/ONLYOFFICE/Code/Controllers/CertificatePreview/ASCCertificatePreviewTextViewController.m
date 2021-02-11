//
//  ASCCertificatePreviewTextViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import "ASCCertificatePreviewTextViewController.h"

@interface ASCCertificatePreviewTextViewController ()
@property (weak) IBOutlet NSScrollView *infoTextView;

@end

@implementation ASCCertificatePreviewTextViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)setInfoText:(NSString *)infoText {
    _infoText = infoText;
    [self updateView];
}

- (void)updateView {
    [self.infoTextView.documentView setString:_infoText];
}

@end
