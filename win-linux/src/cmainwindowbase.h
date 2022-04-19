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

#ifndef CMAINWINDOWBASE_H
#define CMAINWINDOWBASE_H

#define WINDOW_MIN_WIDTH    500
#define WINDOW_MIN_HEIGHT   300

#define MAIN_WINDOW_MIN_WIDTH    960
#define MAIN_WINDOW_MIN_HEIGHT   661
#define MAIN_WINDOW_DEFAULT_SIZE QSize(1324,800)
#define EDITOR_WINDOW_MIN_WIDTH  920

#define BUTTON_MAIN_WIDTH   112
#define MAIN_WINDOW_BORDER_WIDTH 4
#define WINDOW_TITLE_MIN_WIDTH 200
#define TOOLBTN_HEIGHT      28
#define TOOLBTN_WIDTH       40
#define TITLE_HEIGHT        28

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "cmainpanel.h"

#ifdef _WIN32
# include <windows.h>
# include <windowsx.h>
# include <dwmapi.h>

namespace WindowBase
{
    struct CWindowGeometry
    {
        CWindowGeometry() {}
        bool required = false;
        int width = 0;
        int height = 0;
    };
}
#endif

enum class WindowType : uint_fast8_t
{
    MAIN, SINGLE, REPORTER
};

class CElipsisLabel : public QLabel
{
public:
    CElipsisLabel(const QString &text, QWidget *parent = Q_NULLPTR);
    CElipsisLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    auto setText(const QString&) -> void;
    auto setEllipsisMode(Qt::TextElideMode) -> void;
    auto updateText() -> void;

protected:
    void resizeEvent(QResizeEvent *event) override;
    using QLabel::setText;

private:
    QString orig_text;
    Qt::TextElideMode elide_mode = Qt::ElideRight;
};

class CMainWindowBase
{
public:
    CMainWindowBase(QRect& rect);
    virtual ~CMainWindowBase();

    QWidget * editor(int index);
    QString documentName(int vid);
    double scaling() const;
    int editorsCount() const;
    int editorsCount(const std::wstring& portal) const;
    //int attachEditor(QWidget *, int index = -1);
    //int attachEditor(QWidget *, const QPoint&);
    void selectView(int id) const;
    void selectView(const QString& url) const;
    virtual bool pointInTabs(const QPoint& pt) const;

protected:
    QWidget * createTopPanel(QWidget *, const QString&);
    QPushButton * createToolButton(QWidget * parent = nullptr, const QString& name = QString(""));
    void updateTitleCaption();
    void applyWindowState(Qt::WindowState);
    bool isCustomWindowStyle();

    virtual QWidget * createMainPanel(QWidget *, const QString&, bool custom = true, QWidget * view = nullptr);
    virtual void setScreenScalingFactor(double);
    virtual void setWindowTitle(const QString&);
    virtual void updateScaling();
    virtual void applyTheme(const std::wstring&);
    virtual void captureMouse(int tab_index);
    virtual void onSizeEvent(int);
    virtual void onMoveEvent(const QRect&) {}; // Overrides in CEditorWindow
    virtual bool holdView(int id) const;
    virtual int calcTitleCaptionWidth(); // Overrides in CEditorWindow

    virtual CMainPanel * mainPanel() const = 0;
    virtual QRect windowRect() const = 0;
    virtual void show(bool) = 0;
    virtual void bringToTop() = 0;
    virtual void onCloseEvent() = 0;
    virtual void onMinimizeEvent() = 0;
    virtual void onMaximizeEvent() = 0;
    virtual void onExitSizeMove() = 0;

    QWidget * m_boxTitleBtns;
    QWidget * m_pMainPanel;
    QWidget * m_pMainView;
    QPushButton * m_buttonMinimize;
    QPushButton * m_buttonMaximize;
    QPushButton * m_buttonClose;
    CElipsisLabel * m_labelTitle;
    double m_dpiRatio;

private:
    void initTopButtons(QWidget *parent);
    class impl;
    std::unique_ptr<impl> pimpl;  
};

#endif // CMAINWINDOWBASE_H
