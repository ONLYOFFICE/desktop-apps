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
                     @"description" : @"Word 2007 Document",
                     @"extension"   : @"docx"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC): @{
                     @"description" : @"Word 97-2003 Document",
                     @"extension"   : @"doc"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT): @{
                     @"description" : @"OpenOffice Document",
                     @"extension"   : @"odt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF): @{
                     @"description" : @"Rich Text Document",
                     @"extension"   : @"rtf"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT): @{
                     @"description" : @"Plain Text",
                     @"extension"   : @"txt"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML): @{
                     @"description" : @"Web Page",
                     @"extension"   : @"html"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT): @{
                     @"description" : @"Web Page",
                     @"extension"   : @"mht"
                     },
             @(AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB): @{
                     @"description" : @"Mobipocket e-book",
                     @"extension"   : @"epub"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX): @{
                     @"description" : @"PowerPoint 2007 Presentation",
                     @"extension"   : @"pptx"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT): @{
                     @"description" : @"PowerPoint 97-2003 Presentation",
                     @"extension"   : @"ppt"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP): @{
                     @"description" : @"OpenOffice Presentation",
                     @"extension"   : @"odp"
                     },
             @(AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX): @{
                     @"description" : @"PowerPoint Slide Show",
                     @"extension"   : @"ppsx"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX): @{
                     @"description" : @"Excel 2007 Spreadsheet",
                     @"extension"   : @"xlsx"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS): @{
                     @"description" : @"Excel 97-2003 Spreadsheet",
                     @"extension"   : @"xls"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS): @{
                     @"description" : @"OpenOffice Spreadsheet",
                     @"extension"   : @"ods"
                     },
             @(AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV): @{
                     @"description" : @"Comma-Separated Values",
                     @"extension"   : @"csv"
                     },
             
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF): @{
                     @"description" : @"PDF File",
                     @"extension"   : @"pdf"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU): @{
                     @"description" : @"DjVu File",
                     @"extension"   : @"djvu"
                     },
             @(AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS): @{
                     @"description" : @"XML Paper Specification",
                     @"extension"   : @"xps"
                     }
             };
}

@end