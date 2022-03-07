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
//  ASCEventsController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/23/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCEventsController.h"
#import "ASCConstants.h"
#import "ASCDocumentType.h"
#import "ASCExternalController.h"
#import "ASCPresentationReporter.h"
#import "ASCSharedSettings.h"
#import "NSDictionary+Extensions.h"
#import "NSString+Extensions.h"
#import "OfficeFileFormats.h"
#import "ASCLinguist.h"
#import "mac_application.h"

#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventListener
#pragma mark ========================================================
#pragma mark -

class ASCEventListener: public NSEditorApi::CAscCefMenuEventListener {
    dispatch_queue_t eventListenerQueue;
    id <ASCExternalDelegate> externalDelegate;
public:
    ASCEventListener() : NSEditorApi::CAscCefMenuEventListener () {
        eventListenerQueue = dispatch_queue_create("asc.onlyoffice.MenuEventListenerQueue", NULL);
        externalDelegate = [[ASCExternalController shared] delegate];
    }
    
    virtual void OnEvent(NSEditorApi::CAscCefMenuEvent* pRawEvent)
    {
        if (NULL == pRawEvent)
            return;
     
//        if (pRawEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START) {
//            NSLog(@"ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START");
//        }
        
        __block NSEditorApi::CAscMenuEvent * pEvent = pRawEvent;
        
        dispatch_async(eventListenerQueue, ^{
            dispatch_async(dispatch_get_main_queue(), ^{
                int senderId = -1;

                NSEditorApi::CAscCefMenuEvent * pCefEvent = dynamic_cast<NSEditorApi::CAscCefMenuEvent *>(pEvent);
                if (pCefEvent) {
                    senderId = pCefEvent->get_SenderId();
                }
                
                switch (pEvent->m_nType) {
                    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
                        NSEditorApi::CAscCreateTab *pData = (NSEditorApi::CAscCreateTab*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"action"  : @(ASCTabActionOpenUrl),
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_IdEqual()],
                                                                                     @"url"     : [NSString stringWithstdwstring:pData->get_Url()],
                                                                                     @"active"  : @(pData->get_Active())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
                        NSEditorApi::CAscTabEditorType * pData = (NSEditorApi::CAscTabEditorType*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameTabEditorType
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"type"    : @(pData->get_Type())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONCLOSE:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME: {
                        NSEditorApi::CAscDocumentName* pData = (NSEditorApi::CAscDocumentName*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameTabEditorNameChanged
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"name"    : [NSString stringWithstdwstring:pData->get_Name()],
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"url"     : [NSString stringWithstdwstring:pData->get_Url()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED: {
                        NSEditorApi::CAscDocumentModifyChanged * pData = (NSEditorApi::CAscDocumentModifyChanged *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameModifyChanged
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"сhanged" : @(pData->get_Changed())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT: {
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
                        NSEditorApi::CAscDocumentOnSaveData* saveData = (NSEditorApi::CAscDocumentOnSaveData*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSave
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", saveData->get_Id()],
                                                                                     @"cancel"  : @(saveData->get_IsCancel())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE: {
                        NSEditorApi::CAscJSMessage * pData = (NSEditorApi::CAscJSMessage *)pEvent->m_pData;
                        
                        NSRange range = [[NSString stringWithstdwstring:pData->get_Name()] rangeOfString:@"login"];
                        
                        if (range.location == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameLogin
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:pData->get_Value()] dictionary]];
                        }
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
                        NSEditorApi::CAscDownloadFileInfo * pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;
                        
//                        ADDREFINTERFACE(pData);
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDownload
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"data" : [NSValue value:&pData withObjCType:@encode(void *)]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
                        NSEditorApi::CAscPrintEnd * pData = (NSEditorApi::CAscPrintEnd *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePrintDialog
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"      : @(pData->get_Id()),
                                                                                     @"countPages"  : @(pData->get_PagesCount())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: {
                        NSEditorApi::CAscOnOpenExternalLink * pData = (NSEditorApi::CAscOnOpenExternalLink *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenUrl
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"url" : [NSString stringWithstdwstring:pData->get_Url()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
                        NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameKeyboardDown
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"data" : [NSValue value:&pData withObjCType:@encode(void *)]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER:
                    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE: {
                        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                        CCefView * pCefView = appManager->GetViewById(pRawEvent->get_SenderId());

                        if (pCefView == NULL) {
                            break;
                        }

                        if (pCefView && (pCefView->GetType() == cvwtEditor)) {
                            if (((CCefViewEditor*)pCefView)->IsPresentationReporter()) {
                                break;
                            }
                        }

                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"fullscreen" : @(pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER)
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN: {
                        NSEditorApi::CAscLocalOpenFiles * pData = (NSEditorApi::CAscLocalOpenFiles *)pEvent->m_pData;
                        
                        for(std::vector<std::wstring>::iterator it = pData->get_Files().begin(); it != pData->get_Files().end(); ++it) {
                            std::wstring filePath = *it;
                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"action"  : @(ASCTabActionOpenLocalFile),
                                                                                         @"path"    : [NSString stringWithstdwstring:filePath],
                                                                                         @"active"  : @(YES)
                                                                                         }];
                        }
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN:
                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN: {
                        NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = (NSEditorApi::CAscLocalOpenFileRecent_Recover*)pEvent->m_pData;
                        
                        BOOL isRecover = pData->get_IsRecover();
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"action"  : isRecover ? @(ASCTabActionOpenLocalRecoverFile) : @(ASCTabActionOpenLocalRecentFile),
                                                                                     @"active"  : @(YES),
                                                                                     @"fileId"  : @(pData->get_Id()),
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE: {
                        NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;
                        
                        NSMutableArray * supportFormats = [NSMutableArray array];
                        
                        for(std::vector<int>::iterator it = pData->get_SupportFormats().begin(); it != pData->get_SupportFormats().end(); ++it) {
                            int type = *it;
                            
                            NSDictionary * info = [ASCConstants ascFormatsInfo][@(type)];
                            
                            if (info) {
                                [supportFormats addObject:@{
                                                            @"type"         : @(type),
                                                            @"description"  : info[@"description"],
                                                            @"extension"    : info[@"extension"]
                                                            }];
                            }
                        }
                        
                        // Begin hotfix ODP presentation
                        
                        NSArray* pptxExtension = [supportFormats filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF['extension'] == %@", @"pptx"]];
                        NSArray* odpExtension  = [supportFormats filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF['extension'] == %@", @"odp"]];
                        
                        if ([pptxExtension count] > 0 && [odpExtension count] < 1) {
                            NSDictionary * info = [ASCConstants ascFormatsInfo][@(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP)];
                            [supportFormats addObject:@{
                                                        @"type"         : @(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP),
                                                        @"description"  : info[@"description"],
                                                        @"extension"    : info[@"extension"]
                                                        }];
                        }
                        
                        // End hotfix ODP presentation
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSaveLocal
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"fileType": @(pData->get_FileType()),
                                                                                     @"supportedFormats" : supportFormats,
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE: {
                        NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenImage
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"fileId"  : @(pData->get_Id())
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG: {
                        NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenFileDialog
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"filter"  : [NSString stringWithstdwstring:pData->get_Filter()],
                                                                                     @"fileId"  : @(pData->get_Id()),
                                                                                     @"isMulti" : @(pData->get_IsMultiselect())
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {                        
                        [[ASCPresentationReporter sharedInstance] create:pEvent->m_pData from:senderId];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
                        [[ASCPresentationReporter sharedInstance] destroy];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
                    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM: {
                        [[ASCPresentationReporter sharedInstance] apply:pEvent];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE: {
                        pEvent->AddRef();

                        CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];

                        if (appManager) {
                            appManager->Apply(pEvent);
                        }

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE: {
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenSSLCertificate
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO: {
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSaveBeforSign
                                                                            object:nil
                                                                          userInfo:nil];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS: {
                        NSEditorApi::CAscSystemExternalPlugins * pData = dynamic_cast<NSEditorApi::CAscSystemExternalPlugins *>(pEvent->m_pData);

                        if (pData) {
                            for (const NSEditorApi::CAscSystemExternalPlugins::CItem& item: pData->get_Items()) {
                                NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary:
                                                              @{
                                                                @"name": [NSString stringWithstdwstring:item.name],
                                                                @"id": [NSString stringWithstdwstring:item.id],
                                                                @"url": [NSString stringWithstdwstring:item.url]
                                                                }];

                                if (!item.nameLocale.empty()) {
                                    NSString * currentLocale = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];

                                    if (NSDictionary * nameLocales = [[NSString stringWithstdwstring:item.nameLocale] dictionary]) {
                                        if (NSString * nameLocale = nameLocales[currentLocale]) {
                                            json[@"name"] = nameLocale;
                                        }
                                    }
                                }

                                if (NSString * jsonString = [json jsonString]) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"panel:external");
                                    pCommand->put_Param([jsonString stdwstring]);

                                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;

                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->SetEventToAllMainWindows(pEvent);
                                }
                            }
                        }

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END:
                    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR: {
                        int error = pEvent->m_nType == ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR ? -1 : 0;
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDocumentFragmentBuild
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                     @"error": @(error)
                                                                                     }];

                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE: {
                        NSEditorApi::CAscX509CertificateData * pData = dynamic_cast<NSEditorApi::CAscX509CertificateData *>(pEvent->m_pData);
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCertificatePreview
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"text": [NSString stringWithstdwstring:pData->get_Data()],
                                                                                     @"path": [NSString stringWithstdwstring:pData->get_FilePath()]
                                                                                     }];

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
                        NSEditorApi::CAscExecCommand * pData = (NSEditorApi::CAscExecCommand *)pEvent->m_pData;
                        std::wstring cmd = pData->get_Command();
                        std::wstring param = pData->get_Param();

#ifdef DEBUG
                        NSLog(@"[Start page] Command: \"%@\", params: \"%@\"", [NSString stringWithstdwstring:cmd], [NSString stringWithstdwstring:param]);
#endif

                        if (cmd.compare(L"portal:open") == 0 || cmd.find(L"auth:outer") != std::wstring::npos) {
                            NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary];
                            NSString * portal = json[@"portal"];
                            NSString * provider = json[@"provider"];

                            if (portal && provider) {
                                NSURLComponents * urlPage = [NSURLComponents componentsWithString:portal];
                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

                                NSMutableString * entrypage = [NSMutableString stringWithString:[json objectForKey:@"entrypage"]];
                                if ( entrypage ) {
                                    if ( [entrypage characterAtIndex:0] != '/' )
                                        [entrypage insertString:@"/" atIndex:0];

                                    if ( urlPage.path )
                                        [entrypage insertString:urlPage.path atIndex:0];

                                    NSRange pathrange = [entrypage rangeOfString:@"?"];
                                    if ( pathrange.location != NSNotFound ) {
                                        urlPage.path = [entrypage substringToIndex:pathrange.location];
                                        urlPage.query = [entrypage substringFromIndex:pathrange.location+1];
                                    } else {
                                        urlPage.path = entrypage;
                                    }
                                }

                                NSURLQueryItem *countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];

                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
                                }

                                NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];

                                NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
                                [qitems addObjectsFromArray:@[countryCode, portalAddress]];
                                urlPage.queryItems = qitems;

                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                             @"action"  : @(ASCTabActionOpenPortal),
                                                                                             @"url"     : [urlPage string],
                                                                                             @"provider": provider,
                                                                                             @"active"  : @(YES)
                                                                                             }];
                            }
                        } else if (cmd.compare(L"portal:login") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogin
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"info": [NSString stringWithstdwstring:param]
                                                                                         }];
                        } else if (cmd.compare(L"portal:logout") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogout
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"portal:create") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalCreate
                                                                                object:nil
                                                                              userInfo:nil];
                        } else if (cmd.compare(L"portal:new") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalNew
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"auth:sso") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalSSO
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"files:explore") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFileInFinder
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"path": [NSString stringWithstdwstring:param]
                                                                                         }];
                        } else if (cmd.compare(L"files:check") == 0) {                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFilesCheck
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.find(L"app:onready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameStartPageReady
                                                                                object:nil
                                                                              userInfo:nil];
                        } else if (cmd.find(L"doc:onready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorDocumentReady
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                         }];
                        } else if (cmd.find(L"editor:ready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorAppReady
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                         }];
                        } else if (cmd.find(L"editor:event") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorEvent
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"data": [[NSString stringWithstdwstring:param] dictionary]
                                                                                         }];
                        } else if (cmd.find(L"editor:request") != std::wstring::npos) {
                            NSMutableDictionary * params = [NSMutableDictionary dictionaryWithDictionary:@{@"viewId": [NSString stringWithFormat:@"%d", senderId]}];
                            [params addEntriesFromDictionary:[[NSString stringWithstdwstring:param] dictionary]];

                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorAppActionRequest
                                                                                object:nil
                                                                              userInfo:params];
                        } else if (cmd.find(L"go:folder") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorOpenFolder
                                                                                object:nil
                                                                              userInfo: @{
                                                                                          @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                          @"path": [NSString stringWithstdwstring:param]
                                                                                          }];
                        } else if (cmd.find(L"settings:get") != std::wstring::npos) {
                            if (param.find(L"username") != std::wstring::npos) {
                                if (NSString * fullUserName = NSFullUserName()) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"settings:username");
                                    pCommand->put_Param([fullUserName stdwstring]);

                                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;

                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->SetEventToAllMainWindows(pEvent);
                                }
                            }
                        } else if (cmd.find(L"settings:apply") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                NSMutableArray * params = [NSMutableArray array];

                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    [params addObject:[NSString stringWithFormat:@"lang=%@", [externalDelegate onAppPreferredLanguage]]];
                                } else {
                                    [params addObject:[NSString stringWithFormat:@"lang=%@", [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]]];
                                }
                                
                                if (NSString * langId = json[@"langid"]) {
                                    [ASCLinguist setAppLanguageCode:langId];
                                }

                                if (NSString * userName = json[@"username"]) {
                                    if ([userName isEqualToString:@""]) {
                                        [params addObject:[NSString stringWithFormat:@"username=%@", NSFullUserName()]];
                                        [[NSUserDefaults standardUserDefaults] setObject:NSFullUserName() forKey:ASCUserNameApp];
                                    } else {
                                        [params addObject:[NSString stringWithFormat:@"username=%@", userName]];
                                        [[NSUserDefaults standardUserDefaults] setObject:userName forKey:ASCUserNameApp];
                                    }
                                    [[NSUserDefaults standardUserDefaults] synchronize];
                                } else {
                                    [params addObject:[NSString stringWithFormat:@"username=%@", NSFullUserName()]];
                                }
                                if (NSString * docopenMode = json[@"docopenmode"]) {
                                    if ([docopenMode isEqualToString:@"view"]) {
                                        [params addObject:[NSString stringWithFormat:@"mode=%@", @"view"]];
                                    }
                                }

                                if (NSString * uiTheme = json[@"uitheme"]) {
                                    if ( [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme] != uiTheme ) {
                                        [[NSUserDefaults standardUserDefaults] setObject:uiTheme forKey:ASCUserUITheme];

                                        [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedUITheme
                                                                                            object:nil
                                                                                          userInfo: @{@"uitheme": uiTheme}];
                                    }

                                    [params addObject:[NSString stringWithFormat:@"uitheme=%@", uiTheme]];
                                }

                                std::wstring wLocale = [[params componentsJoinedByString:@"&"] stdwstring];
                                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                appManager->InitAdditionalEditorParams(wLocale);
                            }
                        } else if (cmd.find(L"encrypt:isneedbuild") != std::wstring::npos) {
                            bool isFragmented = pData->get_Param() == L"true" ? true : false;
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDocumentFragmented
                                                                                object:nil
                                                                              userInfo: @{
                                                                                          @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                          @"isFragmented": @(isFragmented)
                                                                                          }];
                        } else if (cmd.find(L"create:new") != std::wstring::npos) {
                            /// Create local files
                            
                            NSString * nsParam = (NSString *)[NSString stringWithstdwstring:param];
                            ASCDocumentType docType = ASCDocumentTypeUnknown;
                            
                            if ([nsParam isEqualToString:@"word"]) {
                                docType = ASCDocumentTypeDocument;
                            } else if ([nsParam isEqualToString:@"cell"]) {
                                docType = ASCDocumentTypeSpreadsheet;
                            } else if ([nsParam isEqualToString:@"slide"]) {
                                docType = ASCDocumentTypePresentation;
                            } else if ([nsParam isEqualToString:@"form"]) {
                                docType = ASCDocumentTypeForm;
                            }

                            if (docType != ASCDocumentTypeUnknown) {
                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                      @"action"  : @(ASCTabActionCreateLocalFile),
                                                                                      @"type"    : @(docType),
                                                                                      @"active"  : @(YES)
                                                                                  }];
                            }
                        } else if (cmd.find(L"open:folder") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenLocalFile
                                                                                object:nil
                                                                              userInfo:@{
                                                                                  @"directory": [NSString stringWithstdwstring:param]
                                                                              }];
                        } else if (cmd.find(L"extra:features") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                if (NSArray * available = json[@"available"]) {
                                    [[ASCSharedSettings sharedInstance] setSetting:@([available count] > 0) forKey:kSettingsHasExtraFeatures];
                                }
                            }
                        } else if (cmd.find(L"open:document") != std::wstring::npos) {
                            if (!param.empty()) {
                                if (param.rfind(L"https://",0) == 0 || param.rfind(L"http://",0) == 0) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    CCefView * pCefView = appManager->GetViewByUrl(param);
                                    int viewId = pCefView ? pCefView->GetId() : -1;

                                    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                        object:nil
                                                                                      userInfo:@{
                                                                                             @"action"  : @(ASCTabActionOpenUrl),
                                                                                             @"viewId"  : [NSString stringWithFormat:@"%d", viewId],
                                                                                             @"url"     : [NSString stringWithstdwstring:param],
                                                                                             @"active"  : @(true)
                                                                                             }];
                                }
                            }
                        } else if (cmd.find(L"webapps:features") != std::wstring::npos) {
                            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                            CCefView * pCefView = appManager->GetViewById(senderId);

                            if (pCefView) {
                                NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme] ?: @"theme-classic-light";

                                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                pCommand->put_FrameName(L"frameEditor");
                                pCommand->put_Command(L"uitheme:changed");
                                pCommand->put_Param([uiTheme stdwstring]);

                                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                pEvent->m_pData = pCommand;

                                pCefView->Apply(pEvent);
                            }
                        }

                        break;
                    }

                    default:
                        break;
                }

                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onCefMenuEvent:)]) {
                    [externalDelegate onCefMenuEvent:pEvent];
                }
                
                if (NULL != pEvent) {
                    pEvent->Release();
                }
            });
        });
    }
    
    virtual bool IsSupportEvent(int nEventType)
    {
        return true;
    }
};


#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventsController
#pragma mark ========================================================
#pragma mark -

@interface ASCEventsController()
@property (nonatomic) ASCEventListener *listener;
@end

@implementation ASCEventsController

+ (instancetype)sharedInstance
{
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    
    return sharedInstance;
}

- (id)init {
    self = [super init];
    
    if (self) {
        _listener = new ASCEventListener();
        
        if (_listener) {
            CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
            
            if (appManager) {
                appManager->SetEventListener(_listener);
            }
        }
    }
    
    return self;
}

@end
