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

#ifndef CX11DECORATION_H
#define CX11DECORATION_H

#include "qtextstream.h"

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>

#define FORCE_LINUX_CUSTOMWINDOW_MARGINS

class CX11Decoration
{
public:
    CX11Decoration(QWidget *);
    ~CX11Decoration();

    void setTitleWidget(QWidget *);
    void dispatchMouseDown(QMouseEvent *);
    void dispatchMouseMove(QMouseEvent *);
    void dispatchMouseUp(QMouseEvent *);
    void setCursorPos(int x, int y);

    void turnOn();
    void turnOff();
    bool isDecorated();
    void setMaximized(bool);
    void raiseWindow();

    static int devicePixelRatio();
    static int customWindowBorderWith();

    bool checkButtonState(Qt::MouseButton);
    int m_nDirection;

private:
    QWidget * m_window;
    QWidget * m_title;
    QTimer * m_motionTimer;
    ulong m_currentCursor;
    bool m_decoration;
    int m_nBorderSize;
    bool m_bIsMaximized;
    bool need_to_check_motion = false;

    std::map<int, ulong> m_cursors;

    void createCursors();
    void freeCursors();
    int  hitTest(int x, int y) const;
    void checkCursor(QPoint & p);
    void switchDecoration(bool);
    void sendButtonRelease();
};

class CX11Caption : public QWidget
{
public:
    Q_OBJECT

signals:
    void mouseDoubleClicked();

public:
    explicit CX11Caption(QWidget* parent);
    virtual ~CX11Caption();

    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // CX11DECORATION_H
