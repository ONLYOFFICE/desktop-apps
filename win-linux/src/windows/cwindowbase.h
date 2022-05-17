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

#ifndef CWINDOWBASE_H
#define CWINDOWBASE_H

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

#include <QMainWindow>
#include <QPushButton>
#include <memory>
#include "components/celipsislabel.h"

#ifdef _WIN32
# include <windows.h>
# include <windowsx.h>
# include <dwmapi.h>
#endif


class CWindowBase : public QMainWindow
{
public:
    explicit CWindowBase(const QRect&);
    virtual ~CWindowBase();   

    QWidget * handle() const;
    bool isCustomWindowStyle();
    void updateScaling();
    virtual void adjustGeometry() = 0;
    virtual void setWindowColors(const QColor&, const QColor& border = QColor());
    virtual void applyTheme(const std::wstring&);

protected:
    enum BtnType {
        Btn_Minimize, Btn_Maximize, Btn_Close
    };

    QPushButton* createToolButton(QWidget * parent, const QString& name);
    QWidget* createTopPanel(QWidget *parent);
    void setIsCustomWindowStyle(bool);
    virtual void setScreenScalingFactor(double);
    virtual void applyWindowState(Qt::WindowState);
    virtual void setWindowTitle(const QString&);
    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual void onCloseEvent();
    virtual void focus();

    QVector<QPushButton*> m_pTopButtons;
    CElipsisLabel *m_labelTitle = nullptr;
    QWidget       *m_pMainPanel = nullptr,
                  *m_boxTitleBtns = nullptr,
                  *m_pMainView = nullptr;
    double         m_dpiRatio;

private:
    virtual void showEvent(QShowEvent *) final;
    class CWindowBasePrivate;
    std::unique_ptr<CWindowBasePrivate> pimpl;
    QRect m_window_rect;
    bool  m_windowActivated;
};

#endif // CWINDOWBASE_H
