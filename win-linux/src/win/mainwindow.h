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

#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <windows.h>

#include "cwinpanel.h"
#include "cmainpanelimpl.h"
#include "qwinwidget.h"
#include "cwindowbase.h"
#include "cmainwindowbase.h"

#include <QtWidgets/QApplication>

class CMainWindow : public CMainWindowBase
{

public:
    HWND                    hWnd;
    HINSTANCE               hInstance;

    explicit CMainWindow(QRect&);
    ~CMainWindow() override;
    static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    void show(bool);
    void hide();
    bool isVisible();

    void toggleBorderless(bool);
    void toggleResizeable();

    void setMinimumSize( const int width, const int height );
    bool isSetMinimumSize();
    void removeMinimumSize();
    int getMinimumHeight() const;
    int getMinimumWidth() const;

    void setMaximumSize( const int width, const int height );
    bool isSetMaximumSize();
    int getMaximumHeight();
    int getMaximumWidth();
    void removeMaximumSize();
    void adjustGeometry();
    void applyTheme(const std::wstring&) override;
    void updateScaling() override;

    CMainPanel * mainPanel() const;
    QRect windowRect() const;
    bool isMaximized() const;
    HWND handle() const;
    void bringToTop() const override;

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    // because of QTBUG-67211
    bool pointInTabs(const QPoint& pt) const override;
#endif

/*#ifdef _UPDMODULE
    static void checkUpdates();
    static void setAutocheckUpdatesInterval(const QString&);
#endif*/

private:
    void captureMouse(int tabindex) override;
    void setScreenScalingFactor(double);
    void doClose();

    void slot_windowChangeState(Qt::WindowState);
    void slot_windowClose();
    void slot_mainPageReady();
    void slot_modalDialog(bool, HWND);

    friend auto refresh_window_scaling_factor(CMainWindow * window) -> void;

/*#ifdef _UPDMODULE
    static void updateFound();
    static void updateNotFound();
    static void updateError();
#endif*/

public:
    CWinPanel * m_pWinPanel;

private:
    bool closed;
    bool visible;
    bool skipsizing = false;

    bool borderless;
    bool borderlessResizeable;

    CMainPanelImpl * m_pMainPanel;

    WindowBase::CWindowGeometry minimumSize;
    WindowBase::CWindowGeometry maximumSize;

    double m_dpiRatio = 1;
    HWND m_modalHwnd;

    QRect m_moveNormalRect;
};

#endif
