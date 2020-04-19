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

#ifndef CSINGLEWINDOWPLATFORM_H
#define CSINGLEWINDOWPLATFORM_H

#include "csinglewindowbase.h"
#include "windows.h"
#include "cwinpanel.h"
#include "cwindowbase.h"
#include <QWidget>

#define TOP_NATIVE_WINDOW_HANDLE (HWND)m_pMainPanel->winId()

class CSingleWindowPlatform : public CSingleWindowBase
{
public:
    CSingleWindowPlatform(const QRect&, const QString&, QWidget *);
    virtual ~CSingleWindowPlatform();

    HWND handle() const;
    virtual void show(bool);
    virtual void hide();
    virtual bool visible();
    virtual void bringToTop() override;

    virtual Qt::WindowState windowState();
    virtual void setWindowState(Qt::WindowState);
    virtual void setWindowTitle(const QString&) override;
    virtual QRect geometry() const;
    virtual void activateWindow();

    void toggleBorderless(bool showmax);

protected:
    HWND m_hWnd;
    HWND m_modalHwnd = nullptr;
    COLORREF m_bgColor;
    QRect m_moveNormalRect;
    bool m_borderless = true;
    bool m_visible = false;
    bool m_closed = false;
    bool m_skipSizing = false;
    CWinPanel * m_pWinPanel;
    WindowBase::CWindowGeometry m_minSize;
    WindowBase::CWindowGeometry m_maxSize;
    QMetaObject::Connection m_modalSlotConnection;

    void setMinimumSize(int width, int height);
    WindowBase::CWindowGeometry const& minimumSize() const;
    WindowBase::CWindowGeometry const& maximumSize() const;

    virtual void onSizeEvent(int);
    virtual void applyWindowState(Qt::WindowState);
    virtual void adjustGeometry() override;
    virtual void setScreenScalingFactor(int f) override;

//    virtual void focusMainPanel();

    virtual void onMinimizeEvent() override;
    virtual void onMaximizeEvent() override;
    virtual void onExitSizeMove() override;

    void captureMouse();
    void slot_modalDialog(bool status, HWND h);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // CSINGLEWINDOWPLATFORM_H
