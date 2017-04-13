/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
//  ASCConstants.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/10/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASCConstants.h"
#import "OfficeFileFormats.h"

@implementation ASCConstants

+ (NSArray *)images {
    return @[@"jpg", @"jpeg", @"png", @"gif", @"bmp", @"tif", @"tiff", @"ico"];
}

+ (NSArray *)documents {
    return @[@"doc", @"docx", @"odt", @"rtf", @"txt", @"html", @"mht", @"epub", @"pdf", @"djvu", @"xps"];
}

+ (NSArray *)spreadsheets {
    return @[@"xls", @"xlsx", @"csv", @"ods"];
}

+ (NSArray *)presentations {
    return @[@"ppt", @"pptx", @"ppsx", @"odp"];
}

+ (NSDictionary *)ascFormatsInfo {
    return @{
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX): @{
                     @"description" : NSLocalizedString(@"Word 2007 Document", nil),
                     @"extension"   : @"docx"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC): @{
                     @"description" : NSLocalizedString(@"Word 97-2003 Document", nil),
                     @"extension"   : @"doc"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT): @{
                     @"description" : NSLocalizedString(@"OpenOffice Document", nil),
                     @"extension"   : @"odt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF): @{
                     @"description" : NSLocalizedString(@"Rich Text Document", nil),
                     @"extension"   : @"rtf"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT): @{
                     @"description" : NSLocalizedString(@"Plain Text", nil),
                     @"extension"   : @"txt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML): @{
                     @"description" : NSLocalizedString(@"Web Page", nil),
                     @"extension"   : @"html"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT): @{
                     @"description" : NSLocalizedString(@"Web Page", nil),
                     @"extension"   : @"mht"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB): @{
                     @"description" : NSLocalizedString(@"Mobipocket e-book", nil),
                     @"extension"   : @"epub"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX): @{
                     @"description" : NSLocalizedString(@"PowerPoint 2007 Presentation", nil),
                     @"extension"   : @"pptx"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT): @{
                     @"description" : NSLocalizedString(@"PowerPoint 97-2003 Presentation", nil),
                     @"extension"   : @"ppt"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP): @{
                     @"description" : NSLocalizedString(@"OpenOffice Presentation", nil),
                     @"extension"   : @"odp"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX): @{
                     @"description" : NSLocalizedString(@"PowerPoint Slide Show", nil),
                     @"extension"   : @"ppsx"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX): @{
                     @"description" : NSLocalizedString(@"Excel 2007 Spreadsheet", nil),
                     @"extension"   : @"xlsx"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS): @{
                     @"description" : NSLocalizedString(@"Excel 97-2003 Spreadsheet", nil),
                     @"extension"   : @"xls"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS): @{
                     @"description" : NSLocalizedString(@"OpenOffice Spreadsheet", nil),
                     @"extension"   : @"ods"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV): @{
                     @"description" : NSLocalizedString(@"Comma-Separated Values", nil),
                     @"extension"   : @"csv"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF): @{
                     @"description" : NSLocalizedString(@"PDF File", nil),
                     @"extension"   : @"pdf"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU): @{
                     @"description" : NSLocalizedString(@"DjVu File", nil),
                     @"extension"   : @"djvu"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS): @{
                     @"description" : NSLocalizedString(@"XML Paper Specification", nil),
                     @"extension"   : @"xps"
                     }
             };
}

@end