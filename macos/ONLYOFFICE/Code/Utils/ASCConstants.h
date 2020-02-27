/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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
//  ASCConstants.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ASCHelper.h"

#ifndef ONLYOFFICE_ASCConstants_h
#define ONLYOFFICE_ASCConstants_h

#define ADDREFINTERFACE(x) if (x) {x->AddRef();}

typedef NS_ENUM(int, ASCTabActionType) {
    ASCTabActionUnknown = -1,
    ASCTabActionOpenPortal,
    ASCTabActionOpenUrl,
    ASCTabActionCreateLocalFile,
    ASCTabActionOpenLocalFile,
    ASCTabActionOpenLocalRecentFile,
    ASCTabActionOpenLocalRecoverFile,
    ASCTabActionSaveLocalFile,
};

typedef NS_ENUM(int, CEFDocumentType) {
    CEFDocumentDocument = 0,
    CEFDocumentSpreadsheet = 2,
    CEFDocumentPresentation = 1
};

static NSString * const kRegHelpUrl                         = @"kRegHelpUrl";
static NSString * const kHelpUrl                            = @"kHelpUrl";
static NSString * const kRegistrationPortalUrl              = @"kRegistrationPortalUrl";

// Analitics
static NSString * const ASCAnalyticsCategoryApplication     = @"Application";

// Settings
static NSString * const ASCUserSettingsNamePortalUrl        = @"asc_user_portalUrl";
static NSString * const ASCUserSettingsNameUserInfo         = @"asc_user_info";
static NSString * const ASCUserLastSavePath                 = @"asc_save_path";

// Application event names
static NSString * const ASCEventNameMainWindowSetFrame      = @"UI_mainWindowSetFrame";
static NSString * const ASCEventNameMainWindowLoaded        = @"UI_mainWindowLoaded";

// CEF types
static NSString * const CEFOpenFileFilterImage              = @"images";
static NSString * const CEFOpenFileFilterPlugin             = @"plugin";
static NSString * const CEFOpenFileFilterDocument           = @"word";
static NSString * const CEFOpenFileFilterSpreadsheet        = @"cell";
static NSString * const CEFOpenFileFilterPresentation       = @"slide";
static NSString * const CEFOpenFileFilterVideo              = @"video";
static NSString * const CEFOpenFileFilterAudio              = @"audio";

// CEF event names
static NSString * const CEFEventNameCreateTab               = @"CEF_createTab";
static NSString * const CEFEventNameTabEditorType           = @"CEF_tabEditorType";
static NSString * const CEFEventNameTabEditorNameChanged    = @"CEF_tabEditorNameChanged";
static NSString * const CEFEventNameModifyChanged           = @"CEF_modifyChanged";
static NSString * const CEFEventNameLogin                   = @"CEF_login";
static NSString * const CEFEventNameSave                    = @"CEF_save";
static NSString * const CEFEventNameSaveLocal               = @"CEF_saveLocal";
static NSString * const CEFEventNameOpenUrl                 = @"CEF_openUrl";
static NSString * const CEFEventNameFullscreen              = @"CEF_fullscreen";
static NSString * const CEFEventNameKeyboardDown            = @"CEF_keyboardDown";
static NSString * const CEFEventNameDownload                = @"CEF_downloaded";
static NSString * const CEFEventNameStartSaveDialog         = @"CEF_startSaveDialog";
static NSString * const CEFEventNameEndSaveDialog           = @"CEF_endSaveDialog";
static NSString * const CEFEventNamePrintDialog             = @"CEF_printDialog";
static NSString * const CEFEventNameOpenLocalFile           = @"CEF_openLocalFile";
static NSString * const CEFEventNameOpenImage               = @"CEF_openImage";
static NSString * const CEFEventNameOpenFileDialog          = @"CEF_openFileDialog";
static NSString * const CEFEventNamePortalLogin             = @"CEF_portalLogin";
static NSString * const CEFEventNamePortalLogout            = @"CEF_portalLogout";
static NSString * const CEFEventNamePortalCreate            = @"CEF_portalCreate";
static NSString * const CEFEventNamePortalNew               = @"CEF_portalNew";
static NSString * const CEFEventNamePortalSSO               = @"CEF_portalSSO";
static NSString * const CEFEventNameFileInFinder            = @"CEF_fileOpenInFinder";
static NSString * const CEFEventNameFilesCheck              = @"CEF_filesCheck";
static NSString * const CEFEventNameStartPageReady          = @"CEF_startPageReady";
static NSString * const CEFEventNameSaveBeforSign           = @"CEF_saveBeforeSign";
static NSString * const CEFEventNameOpenSSLCertificate      = @"CEF_openSSLCertificate";
static NSString * const CEFEventNameEditorDocumentReady     = @"CEF_editorDocumentReady";
static NSString * const CEFEventNameEditorAppReady          = @"CEF_editorAppReady";
static NSString * const CEFEventNameEditorEvent             = @"CEF_editorEvent";
static NSString * const CEFEventNameEditorAppActionRequest  = @"CEF_editorAppActionRequest";
static NSString * const CEFEventNameEditorOpenFolder        = @"CEF_editorOpenFolder";
static NSString * const CEFEventNameDocumentFragmentBuild   = @"CEF_documentFragmentBuild";
static NSString * const CEFEventNameDocumentFragmented      = @"CEF_documentFragmented";

@interface ASCConstants : NSObject

//@property (nonatomic) NSSto *someProperty;

+ (id)shared;

+ (NSArray *)images;
+ (NSArray *)audios;
+ (NSArray *)videos;
+ (NSArray *)documents;
+ (NSArray *)spreadsheets;
+ (NSArray *)presentations;
+ (NSArray *)plugins;

+ (NSString *)appInfo:(NSString *)key;
+ (NSDictionary *)ascFormatsInfo;
@end

#endif
