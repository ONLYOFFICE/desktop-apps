//
//  ASCDocumentType.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 01.06.2020.
//  Copyright © 2020 Ascensio System SIA. All rights reserved.
//

#ifndef ASCDocumentType_h
#define ASCDocumentType_h

typedef NS_ENUM(NSUInteger, ASCDocumentType) {
    ASCDocumentTypeDocument,
    ASCDocumentTypePresentation,
    ASCDocumentTypeSpreadsheet,
    ASCDocumentTypeForm,
    ASCDocumentTypeUnknown = NSUIntegerMax
};

#endif /* ASCDocumentType_h */
