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

#ifndef CWINDOWPLATFORM_H
#define CWINDOWPLATFORM_H

#include "cwindowbase.h"
#include <QtWidgets/QApplication>
#include <QShowEvent>
#include <QMargins>
#include <QRect>

struct CWindowGeometry
{
    CWindowGeometry() {}
    bool required = false;
    int width = 0;
    int height = 0;
};

class CWindowPlatform : public CWindowBase
{
public:
    explicit CWindowPlatform(const QRect&, const WindowType);
    virtual ~CWindowPlatform();

    QWidget * handle() const;
    void toggleBorderless(bool);
    void toggleResizeable();
    void adjustGeometry();
    void bringToTop();
    void setWindowBackgroundColor(const QColor&);
    void setWindowColors(const QColor&, const QColor& border = QColor());
    void show(bool);
    void updateScaling();
    virtual void applyTheme(const std::wstring&);

protected:
    void setMinimumSize(const int, const int);
    void setMaximumSize(const int, const int);
    void slot_modalDialog(bool,  WId);
    virtual void setScreenScalingFactor(double f);

    void captureMouse();
    void captureMouse(int);
    //virtual void onExitSizeMove();

private:
    friend auto refresh_window_scaling_factor(CWindowPlatform * window)->void;
    void setResizeable(bool);
    void setResizeableAreaWidth(int);
    void setContentsMargins(int, int, int, int);
    int dpiCorrectValue(int v) const;

    virtual void showEvent(QShowEvent*) final;
    virtual void changeEvent(QEvent*) final;
    virtual bool nativeEvent(const QByteArray&, void*, long*) final;


    WindowType m_winType;
    CWindowGeometry m_minSize;
    CWindowGeometry m_maxSize;
    Qt::WindowStates m_previousState;

    QRect m_moveNormalRect,
          m_window_rect;
    QMargins m_margins,
             m_frame;

    HWND m_hWnd,
         m_modalHwnd;

    int  m_resAreaWidth;
    bool m_borderless,
         m_closed,
         m_skipSizing,
         m_isMaximized,
         m_isResizeable,
         m_taskBarClicked,
         m_windowActivated;
};

#endif // CWINDOWPLATFORM_H
