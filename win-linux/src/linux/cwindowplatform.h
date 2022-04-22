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
#include "cmainpanelimpl.h"
#include "cx11decoration.h"
#include <memory>


class CWindowPlatform : public CX11Decoration, public CWindowBase
{
public:
    explicit CWindowPlatform(const QRect&);
    explicit CWindowPlatform(const QRect&, const QString&, QWidget*);
    explicit CWindowPlatform(const QRect&, const QString&, QCefView*);
    ~CWindowPlatform();

    QWidget * handle() const;
    void sendSertificate(int viewid);
    virtual CMainPanel * mainPanel() const final;
    virtual QRect windowRect() const final;
    virtual void show(bool) final;
    virtual void bringToTop() final;
    virtual void updateScaling() final;
    virtual void applyTheme(const std::wstring&) override;
    virtual bool holdView(int id) const override;

protected:
    void captureMouse();
    virtual void setScreenScalingFactor(double factor) override;
    virtual void onMinimizeEvent() override;
    virtual void onMaximizeEvent() override;
    virtual void onSizeEvent(int type) override;
    //virtual void onExitSizeMove() override;
    virtual void setWindowTitle(const QString &) final;

private:
    explicit CWindowPlatform(const QRect&, const WindowType, const QString&, QWidget*);

    void onScreenScalingFactor(double f);
    virtual void closeEvent(QCloseEvent *) final;
    virtual void showEvent(QShowEvent *) final;
    virtual bool event(QEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *) final;
    virtual void mousePressEvent(QMouseEvent *) final;
    virtual void mouseReleaseEvent(QMouseEvent *) final;
    virtual void mouseDoubleClickEvent(QMouseEvent *) final;

    virtual void dragEnterEvent(QDragEnterEvent *event) final;
    virtual void dropEvent(QDropEvent *event) final;
    virtual void captureMouse(int tabindex) final;
    virtual void bringToTop() const final;
    virtual void onCloseEvent() override;

    using QMainWindow::setWindowTitle;
    WindowType m_winType;
    CMainPanelImpl * _m_pMainPanel;

    class impl;
    std::unique_ptr<impl> pimpl;

public slots:
    void slot_windowChangeState(Qt::WindowState);
    void slot_windowClose();
    void slot_modalDialog(bool status, WId h);
};

#endif // CWINDOWPLATFORM_H
