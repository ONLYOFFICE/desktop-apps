//
//  ASCCertificateQLPreview.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

NS_ASSUME_NONNULL_BEGIN

@interface ASCCertificateQLPreview : NSObject <QLPreviewItem>
@property (nonatomic, strong) NSURL * fileUrl;

- (instancetype)init:(NSURL *)url;
- (instancetype)init:(NSString *)path rename:(BOOL)rename;

- (void)cleanup;

@end

NS_ASSUME_NONNULL_END
