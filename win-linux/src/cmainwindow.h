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


#ifdef _WIN32
# include "win/cwindowplatform.h"
#else
# include "linux/cwindowplatform.h"
#endif
#include "cmainpanelimpl.h"


class CMainWindow : public CWindowPlatform
{
public:
    explicit CMainWindow(const QRect&);
    virtual ~CMainWindow();

    QWidget * editor(int index);
    QRect windowRect() const;
    QString documentName(int vid);
    void selectView(int id) const;
    void selectView(const QString& url) const;
    int attachEditor(QWidget *, int index = -1);
    int attachEditor(QWidget *, const QPoint&);
    int editorsCount() const;
    int editorsCount(const std::wstring& portal) const;
    bool pointInTabs(const QPoint& pt) const;
    bool holdView(int id) const;
    virtual CMainPanel * mainPanel() const final;
    virtual void applyTheme(const std::wstring&) final;
#ifdef _UPDMODULE
    static void checkUpdates();
    static void setAutocheckUpdatesInterval(const QString&);
#endif 

private:
#ifdef _UPDMODULE
    static void updateFound();
    static void updateNotFound();
    static void updateError();
#endif    
    void setWindowState(Qt::WindowState);
    void slot_mainPageReady();
    virtual void onCloseEvent() final;
    virtual void applyWindowState(Qt::WindowState) final;
    void focus() override;

    friend class CMainPanel;
};

#endif // CMAINWINDOW_H
