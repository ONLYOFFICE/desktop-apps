//
//  ASCCertificateQLPreview.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import "ASCCertificateQLPreview.h"

@interface ASCCertificateQLPreview()
@property (nonatomic) NSURL * originalUrl;
@end

@implementation ASCCertificateQLPreview

- (instancetype)init:(NSURL *)url {
    self = [super init];
    if (self) {
        _fileUrl = url;
    }
    return self;
}

- (instancetype)init:(NSString *)path rename:(BOOL)rename {
    self = [super init];
    if (self) {
        if (rename) {
            NSString * linkPath = [path stringByAppendingString:@".cer"];
            if ([[NSFileManager defaultManager] linkItemAtPath:path toPath:linkPath error:nil]) {
                _originalUrl = [NSURL fileURLWithPath:path];
                _fileUrl = [NSURL fileURLWithPath:linkPath];
            }
        } else {
            _fileUrl = [NSURL fileURLWithPath:path];
        }
    }
    return self;
}

- (NSURL *)previewItemURL {
    return _fileUrl;
}

- (NSString *)previewItemTitle {
    return NSLocalizedString(@"Signature Details", nil);
}

- (void)cleanup {
    [[NSFileManager defaultManager] removeItemAtURL:_fileUrl error:nil];
    if (_originalUrl) {
        [[NSFileManager defaultManager] removeItemAtURL:_originalUrl error:nil];
    }
}

@end
