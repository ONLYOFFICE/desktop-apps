//
//  ASCCertificateQLPreviewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import <Quartz/Quartz.h>
#import "ASCCertificateQLPreviewController.h"
#import "ASCCertificateQLPreview.h"

@interface ASCCertificateQLPreviewController() <QLPreviewPanelDelegate, QLPreviewPanelDataSource>
@property (nonatomic) ASCCertificateQLPreview * item;
@property (nonatomic) QLPreviewPanel * panel;
@end

@implementation ASCCertificateQLPreviewController

- (void)previewBy:(NSURL *)fileUrl {
    // Set responder to the controller for QLPreviewPanelController methods
    NSResponder * aNextResponder = [[NSApp mainWindow] nextResponder];
    [[NSApp mainWindow] setNextResponder:self];
    [self setNextResponder:aNextResponder];
    
//    _item = [[ASCCertificateQLPreview alloc] init:fileUrl];
    _item = [[ASCCertificateQLPreview alloc] init:[fileUrl path] rename:true];
    _panel = [QLPreviewPanel sharedPreviewPanel];
    
    NSTimer * timer = [NSTimer scheduledTimerWithTimeInterval:0
                                                       target:self
                                                     selector:@selector(openPanelInRunLoop)
                                                     userInfo:nil
                                                      repeats:false];
    [[NSRunLoop currentRunLoop] addTimer:timer
                                 forMode:NSModalPanelRunLoopMode];
}

- (BOOL)isOpened {
    return [QLPreviewPanel sharedPreviewPanelExists] && [[QLPreviewPanel sharedPreviewPanel] isVisible];
}

- (void)openPanelInRunLoop {
    [_panel updateController];
    if (![self isOpened]) {
        [_panel makeKeyAndOrderFront:nil];
    } else {
        [_panel setCurrentPreviewItemIndex:0];
    }
}

#pragma mark - QLPreviewPanelDelegate

- (BOOL)previewPanel:(QLPreviewPanel *)panel handleEvent:(NSEvent *)event {
    return true;
}

- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *)panel {
    return true;
}

- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel {
    _panel.dataSource = self;
    _panel.delegate = self;
}

- (void)endPreviewPanelControl:(QLPreviewPanel *)panel {
    _panel.dataSource = nil;
    _panel.delegate = nil;
    
    [self.item cleanup];
}

#pragma mark - QLPreviewPanelDataSource

- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
    return 1;
}

- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index {
    return self.item;
}

@end
