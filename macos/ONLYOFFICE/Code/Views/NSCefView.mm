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
//  NSCefView.m
//  CefViewTest
//
//  Created by Oleg Korshul on 14.09.15.
//  Copyright (c) 2015 Ascensio System. All rights reserved.
//

#import "NSCefView.h"
#import "NSString+OnlyOffice.h"

class CCefViewWrapper : public CCefViewWidgetImpl
{
public:
    CCefViewWrapper(NSCefView* pView)
    {
        m_pParent = pView;
        m_pCefView = NULL;
    }
    virtual ~CCefViewWrapper()
    {
    }
    
    void SetBackgroundCefColor(unsigned char r, unsigned char g, unsigned char b)
    {
        
    }
    
    CCefView* GetCefView()
    {
        return m_pCefView;
    }
    void Create(CAscApplicationManager* pManager, CefViewWrapperType eType);
    
public:
    CCefView* m_pCefView;
    NSCefView* m_pParent;
    
    void focusInEvent()
    {
        if (NULL != m_pCefView)
            m_pCefView->focus();
    }
    
    void resizeEvent()
    {
        if (NULL != m_pCefView)
            m_pCefView->resizeEvent();
    }
    
    void moveEvent()
    {
        if (NULL != m_pCefView)
            m_pCefView->moveEvent();
    }
    
public:
    // CCefViewWidgetImpl
    virtual int parent_x()
    {
        //CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
        CGFloat koef = 1;
        return (int)(m_pParent.frame.origin.x * koef);
    }
    virtual int parent_y()
    {
        //CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
        CGFloat koef = 1;
        return (int)(m_pParent.frame.origin.y * koef);
    }
    virtual int parent_width()
    {
        //CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
        CGFloat koef = 1;
        return (int)(m_pParent.frame.size.width * koef);
    }
    virtual int parent_height()
    {
        //CGFloat koef = [[NSScreen mainScreen] backingScaleFactor];
        CGFloat koef = 1;
        return (int)(m_pParent.frame.size.height * koef);
    }
    virtual WindowHandleId parent_wid()
    {
        return (__bridge WindowHandleId)m_pParent;
    }
    virtual bool parent_window_is_empty()
    {
        return true;
    }
};

@interface NSCefView () {
    CCefViewWrapper* m_pCefView;
}
@end


@implementation NSCefView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        CALayer *viewLayer = [CALayer layer];
        [viewLayer setBackgroundColor:CGColorCreateGenericRGB(1.0, 1.0, 1.0, 1.0)]; //RGB plus Alpha Channel
        [self setWantsLayer:YES]; // view's backing store is using a Core Animation Layer
        [self setLayer:viewLayer];
        
        m_pCefView = new CCefViewWrapper(self);
    }
    return self;
}

- (void)dealloc {
    if (NULL != m_pCefView) {
        delete m_pCefView;
        m_pCefView = NULL;
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)setFrameOrigin:(NSPoint)newOrigin {
    [super setFrameOrigin:newOrigin];
    m_pCefView->moveEvent();
}

- (void) setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    m_pCefView->resizeEvent();
}

- (NSInteger)uuid {
    if (m_pCefView) {
        return m_pCefView->GetCefView()->GetId();
    }
    
    return NSNotFound;
}

- (void)Load:(NSString *)url
{
    if (m_pCefView)
        m_pCefView->GetCefView()->load([url stdwstring]);
}

- (void)Create:(CAscApplicationManager *)manager withType:(CefViewWrapperType)type {
    switch (type) {
        case cvwtSimple: {
            m_pCefView->m_pCefView = manager->CreateCefView(m_pCefView);
            break;
        }
        case cvwtEditor: {
            m_pCefView->m_pCefView = manager->CreateCefEditor(m_pCefView);
            break;
        }
        default:
            break;
    }
}

@end
