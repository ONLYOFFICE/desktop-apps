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

#include "cx11decoration.h"

#include <QMainWindow>
#include "applicationmanager.h"
#include "cmainpanelimpl.h"
#include "cwindowbase.h"
#include "cmainwindowbase.h"


class CMainWindow : public QMainWindow, public CX11Decoration, public CMainWindowBase
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget *parent = 0);
    explicit CMainWindow(const QRect&);
    ~CMainWindow();

    void parseInputArgs(const QStringList&);
    CMainPanel * mainPanel() const;
    QRect windowRect() const;
    bool isMaximized() const;
    void sendSertificate(int viewid);
    QWidget * handle() const;
    void bringToTop() const override;

protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    bool event(QEvent *event);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void captureMouse(int tabindex) override;

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void setScreenScalingFactor(uchar factor);
private:
    CMainPanelImpl *   m_pMainPanel;
    uchar m_dpiRatio = 1;

signals:
public slots:
    void slot_windowChangeState(Qt::WindowState);
    void slot_windowClose();
    void slot_modalDialog(bool status, WId h);
};

#endif // CMAINWINDOW_H
