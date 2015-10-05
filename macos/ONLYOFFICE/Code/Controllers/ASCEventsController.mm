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

#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventListener
#pragma mark ========================================================
#pragma mark -

class ASCEventListener: public NSEditorApi::CAscMenuEventListener {
    dispatch_queue_t eventListenerQueue;
public:
    ASCEventListener() : NSEditorApi::CAscMenuEventListener () {
        eventListenerQueue = dispatch_queue_create("asc.onlyoffice.MenuEventListenerQueue", NULL);
    }
    
    virtual void OnEvent(NSEditorApi::CAscMenuEvent* pRawEvent)
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
                                                                                     @"name"    : [NSString stringWithstdwstring:pData->get_Name()]
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
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameLogout
                                                                            object:nil
                                                                          userInfo:nil];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
                        NSEditorApi::CAscTypeId* pId = (NSEditorApi::CAscTypeId*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSave
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pId->get_Id()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE: {
                        NSEditorApi::CAscJSMessage * pData = (NSEditorApi::CAscJSMessage *)pEvent->m_pData;
                        
                        NSRange range = [[NSString stringWithstdwstring:pData->get_Name()] rangeOfString:@"login"];
                        
                        if (range.location == 0) {
                            NSString *jsonString = [NSString stringWithstdwstring:pData->get_Value()];
                            NSData *data = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
                            id json = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameLogin
                                                                                object:nil
                                                                              userInfo:json];
                        }
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START:
                    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
                        NSEditorApi::CAscDownloadFileInfo * pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;
                        
//                        ADDREFINTERFACE(pData);
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDownload
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"data" : [NSValue value:&pData withObjCType:@encode(void *)],
                                                                                     @"start": @(pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START)
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
                        //                    NSEditorApi::CAscPrintEnd * pData = (NSEditorApi::CAscPrintEnd *)pEvent->m_pData;
                        //
                        //                    ADDREFINTERFACE(pData)
                        //                    QMetaObject::invokeMethod(this, "onDocumentPrint", Qt::QueuedConnection, Q_ARG(void *, pData));
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
                        
                        ADDREFINTERFACE(pData);
                        
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
                        
                    default:
                        break;
                }
                
                if (NULL != pEvent)
                    delete pEvent;
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
