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

#ifndef CSINGLEWINDOW_H
#define CSINGLEWINDOW_H

#include <QRect>
#include <QPushButton>
#include <QLabel>

#include "cwindowbase.h"
#include "cwinpanel.h"
#include "ctabpanel.h"

class CSingleWindow
{

private:
    bool m_borderless = false;
    bool m_visible = false;
    bool m_borderlessResizeable = true;
    bool m_closed = false;
    HWND m_hWnd = 0;
    double m_dpiRatio = 1;

    CWinPanel * m_pWinPanel;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_pMainView = nullptr;
    QWidget * m_boxTitleBtns = nullptr;
    WindowBase::CWindowGeometry minimumSize;
    WindowBase::CWindowGeometry maximumSize;

    QPushButton * m_pButtonMinimize;
    QPushButton * m_pButtonMaximize;
    QPushButton * m_pButtonClose;

    QLabel * m_pLabelTitle = nullptr;

    QWidget * createMainPanel(QWidget *, const QString&, bool, QWidget *);
    void recalculatePlaces();
    void pushButtonCloseClicked();
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void focusMainPanel();
    void applyWindowState(Qt::WindowState);

public:
    CSingleWindow(const QRect&);
    CSingleWindow(const QRect&, const QString&, QWidget *);
    ~CSingleWindow();

    static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    void show(bool maximized = false);
    void hide();
    bool isVisible();

    void toggleBorderless(bool);
    void toggleResizeable();
//    bool isResizeable();

    void setMinimumSize( const int width, const int height );
    void removeMinimumSize();
    int getMinimumHeight() const;
    int getMinimumWidth() const;

    void setMaximumSize( const int width, const int height );
    int getMaximumHeight();
    int getMaximumWidth();
    void removeMaximumSize();
    void adjustGeometry();
    void applyTheme(const std::wstring& themeid);

    void setScreenScalingFactor(double);
    void doClose();

    bool holdView(int id) const;

    WId handle() const
    {
        return (WId)m_hWnd;
    }

};

#endif // CSINGLEWINDOW_H
