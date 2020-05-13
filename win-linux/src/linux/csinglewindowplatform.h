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

#ifndef CSINGLEWINDOWPLATFORM_H
#define CSINGLEWINDOWPLATFORM_H

#include "csinglewindowbase.h"
#include <QMainWindow>
#include "cx11decoration.h"
#include <memory>

class CSingleWindowPlatform : public CSingleWindowBase, public QMainWindow, public CX11Decoration
{
public:
    explicit CSingleWindowPlatform(const QRect& r, const QString& title, QWidget * panel);
    virtual ~CSingleWindowPlatform();

    void show(bool maximized);
    QWidget * handle() const;
    virtual void setWindowTitle(const QString &) override;
    virtual void bringToTop() override;

private:
    bool flag_mouse_button_left = false;
    bool flag_mouse_motion = false;

    bool event(QEvent *event);
    using QMainWindow::setWindowTitle;

    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

    void resizeEvent(QResizeEvent *);

    class impl;
    std::unique_ptr<impl> pimpl;

protected:
    void captureMouse();

    virtual void onMinimizeEvent() override;
    virtual void onMaximizeEvent() override;
    virtual void onSizeEvent(int type);
    virtual void onScreenScalingFactor(uint f);
    virtual void onExitSizeMove() override;

public slots:
    void slot_modalDialog(bool status, WId h);
};

#endif // CSINGLEWINDOWPLATFORM_H
