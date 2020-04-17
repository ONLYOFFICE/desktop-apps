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

#ifndef CEDITORWINDOW_H
#define CEDITORWINDOW_H

#ifdef __linux__
# include "linux/csinglewindowplatform.h"
#else
# include "win/csinglewindowplatform.h"
#endif

#include "ctabpanel.h"
#include <memory>
#include <QCoreApplication>

class CEditorWindowPrivate;
class CEditorWindow : public CSingleWindowPlatform
{
    Q_DECLARE_TR_FUNCTIONS(CEditorWindow)

public:
    CEditorWindow();
    CEditorWindow(const QRect& rect, CTabPanel* view);
    ~CEditorWindow();

    bool holdView(int id) const override;
    bool holdView(const wstring& portal) const;
    void undock(bool maximized = false);
    int closeWindow();
    CTabPanel * mainView() const;
    CTabPanel * releaseEditorView() const;
    QString documentName() const;
    bool closed() const;
    AscEditorType editorType() const;

    void setReporterMode(bool);
private:
    QString m_css;
    bool m_restoreMaximized = false;

private:
    CEditorWindow(const QRect&, const QString&, QWidget *);
    QWidget * createMainPanel(QWidget * parent);
    QWidget * createMainPanel(QWidget * parent, const QString& title, bool custom) override;
    void recalculatePlaces();
    const QObject * receiver() override;

protected:
    void onCloseEvent() override;
    void onMinimizeEvent() override;
    void onMaximizeEvent() override;
    void onSizeEvent(int) override;
    void onMoveEvent(const QRect&) override;
    void onExitSizeMove() override;
    void onDpiChanged(int,int) override;

    void setScreenScalingFactor(int) override;

    void onLocalFileSaveAs(void *);

private:
    friend class CEditorWindowPrivate;
    std::unique_ptr<CEditorWindowPrivate> d_ptr;
};

#endif // CEDITORWINDOW_H
