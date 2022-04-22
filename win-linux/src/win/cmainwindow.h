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

#include "cwindowbase.h"
#include "cmainpanelimpl.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMargins>
#include <QRect>


class CMainWindow : public CWindowBase, public QMainWindow
{
public:
    explicit CMainWindow(const QRect&);
    explicit CMainWindow(const QRect&, const QString&, QWidget*);
    explicit CMainWindow(const QRect&, const QString&, QCefView*);
    virtual ~CMainWindow();

    QWidget * handle() const;
    void hide();
    void toggleBorderless(bool);
    void adjustGeometry();
    void setWindowState(Qt::WindowState);
    void setWindowBackgroundColor(const QColor&);
    void setWindowColors(const QColor&, const QColor& border = QColor());
    virtual CMainPanel * mainPanel() const final;
    virtual QRect windowRect() const final;
    virtual void show(bool) final;
    virtual void bringToTop() final;
    virtual void updateScaling() final;
    virtual void applyTheme(const std::wstring&) override;
    virtual bool holdView(int id) const override;


#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    // because of QTBUG-67211
    virtual bool pointInTabs(const QPoint& pt) const override;
#endif

#ifdef _UPDMODULE
    static void checkUpdates();
    static void setAutocheckUpdatesInterval(const QString&);
#endif

protected:
    void captureMouse();
    virtual void focus() {}; // Uses in SINGLE Mode
    virtual void onDpiChanged(double, double);
    virtual void setScreenScalingFactor(double f) override;
    virtual void onMinimizeEvent() override;
    virtual void onMaximizeEvent() override;
    virtual void onExitSizeMove() override;
    virtual void setWindowTitle(const QString&) final;

private:
    explicit CMainWindow(const QRect&, const WindowType, const QString&, QWidget*);
    friend auto refresh_window_scaling_factor(CMainWindow * window) -> void;
#ifdef _UPDMODULE
    static void updateFound();
    static void updateNotFound();
    static void updateError();
#endif
    void doClose();
    void slot_windowClose();
    void slot_mainPageReady();
    void slot_modalDialog(bool,  WId);
    void toggleResizeable();
    void setResizeable(bool);
    void setResizeableAreaWidth(int);
    void setContentsMargins(int, int, int, int);
    void setMinimumSize(const int, const int);
    void setMaximumSize(const int, const int);
    void focusMainPanel();
    int dpiCorrectValue(int v) const {return int(v * m_dpiRatio);}

    virtual void showEvent(QShowEvent*) final;
    virtual void changeEvent(QEvent*) final;
    virtual bool nativeEvent(const QByteArray&, void*, long*) final;
    virtual void captureMouse(int) final;
    virtual void onCloseEvent() override;

    WindowType m_winType;
    CWindowGeometry m_minSize;
    CWindowGeometry m_maxSize;
    QMetaObject::Connection m_modalSlotConnection;
    CMainPanelImpl *_m_pMainPanel;
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

#endif
