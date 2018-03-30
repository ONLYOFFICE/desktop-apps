/*
 * (c) Copyright Ascensio System SIA 2010-2018
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
//  ASCEventsController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/23/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCEventsController.h"
#import "mac_application.h"
#import "ASCConstants.h"
#import "NSString+OnlyOffice.h"
#import "OfficeFileFormats.h"
#import "ASCPresentationReporter.h"


#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventListener
#pragma mark ========================================================
#pragma mark -

id stringToJson(NSString *jsonString) {
    NSData *data = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    id json = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    return json;
}

class ASCEventListener: public NSEditorApi::CAscCefMenuEventListener {
    dispatch_queue_t eventListenerQueue;
public:
    ASCEventListener() : NSEditorApi::CAscCefMenuEventListener () {
        eventListenerQueue = dispatch_queue_create("asc.onlyoffice.MenuEventListenerQueue", NULL);
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
                                                                              userInfo:stringToJson([NSString stringWithstdwstring:pData->get_Value()])];
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
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"fullscreen" : @(pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER)
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN: {
                        NSEditorApi::CAscLocalFileOpen * pData = (NSEditorApi::CAscLocalFileOpen *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenLocalFile
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"directory": [NSString stringWithstdwstring:pData->get_Directory()]
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
                        
                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_CREATE: {
                        NSEditorApi::CAscLocalFileCreate * pData = (NSEditorApi::CAscLocalFileCreate *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"action"  : @(ASCTabActionCreateLocalFile),
                                                                                     @"type"    : @(pData->get_Type()),
                                                                                     @"active"  : @(YES)
                                                                                     }];
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
                                                                                     @"suppertFormats" : supportFormats,
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
                                                                                     @"fileId"  : @(pData->get_Id())
                                                                                     }];
                        break;
                    }
                    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {                        
                        [[ASCPresentationReporter sharedInstance] create:pEvent->m_pData];
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
                    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
                        NSEditorApi::CAscExecCommand * pData = (NSEditorApi::CAscExecCommand *)pEvent->m_pData;
                        std::wstring cmd = pData->get_Command();
                        
                        if (cmd.compare(L"portal:open") == 0) {
                            NSURLComponents *urlPage      = [NSURLComponents componentsWithString:[NSString stringWithFormat:@"%@/%@", [NSString stringWithstdwstring:pData->get_Param()], @"products/files/"]];
                            NSURLQueryItem *countryCode   = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
                            NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];
                            urlPage.queryItems            = @[countryCode, portalAddress];
                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"action"  : @(ASCTabActionOpenPortal),
                                                                                         @"url"     : [urlPage string],
                                                                                         @"active"  : @(YES)
                                                                                         }];
                        } else if (cmd.compare(L"portal:login") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogin
                                                                                object:nil
                                                                              userInfo:stringToJson([NSString stringWithstdwstring:pData->get_Param()])];
                        } else if (cmd.compare(L"portal:logout") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogout
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"url": [NSString stringWithstdwstring:pData->get_Param()],
                                                                                         }];
                        } else if (cmd.compare(L"portal:create") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalCreate
                                                                                object:nil
                                                                              userInfo:nil];
                        } else if (cmd.compare(L"portal:new") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalNew
                                                                                object:nil
                                                                              userInfo:stringToJson([NSString stringWithstdwstring:pData->get_Param()])];
                        } else if (cmd.compare(L"auth:sso") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalSSO
                                                                                object:nil
                                                                              userInfo:stringToJson([NSString stringWithstdwstring:pData->get_Param()])];
                        } else if (cmd.compare(L"files:explore") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFileInFinder
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"path": [NSString stringWithstdwstring:pData->get_Param()]
                                                                                         }];
                        } else if (cmd.compare(L"files:check") == 0) {                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFilesCheck
                                                                                object:nil
                                                                              userInfo:stringToJson([NSString stringWithstdwstring:pData->get_Param()])];
                        } else if (cmd.find(L"app:onready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameStartPageReady
                                                                                object:nil
                                                                              userInfo:nil];
                        }
                        
                        break;
                    }

                    default:
                        break;
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
