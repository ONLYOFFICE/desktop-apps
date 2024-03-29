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

#ifndef CAPTION_H
#define CAPTION_H

#include <QWidget>
#include <QWindow>
#include <Windows.h>
#include <QPushButton>
#include <QCoreApplication>


class Caption: public QWidget
{
public:
    Caption(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()):
        QWidget(parent, f)
    {}

private:
    bool postMsg(DWORD cmd) {
        POINT pt;
        ::GetCursorPos(&pt);
        QPoint pos = mapFromGlobal(QPoint(int(pt.x), int(pt.y)));
        QPushButton *pushButton = childAt(pos) ? qobject_cast<QPushButton*>(childAt(pos)) : nullptr;
        if (!pushButton) {
            HWND hWnd = ::GetAncestor((HWND)(window()->windowHandle()->winId()), GA_ROOT);
            ::ReleaseCapture();
            ::PostMessage(hWnd, cmd, HTCAPTION, POINTTOPOINTS(pt));
            QCoreApplication::postEvent(parent(), new QEvent(QEvent::MouseButtonPress));
            return true;
        }
        return false;
    }

    bool nativeEvent(const QByteArray &eventType, void *message, long *result)
    {
    #if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
        MSG* msg = *reinterpret_cast<MSG**>(message);
    #else
        MSG* msg = reinterpret_cast<MSG*>(message);
    #endif

        switch (msg->message)
        {
        case WM_LBUTTONDOWN: {
            if (postMsg(WM_NCLBUTTONDOWN))
                return true;
            break;
        }
        case WM_LBUTTONDBLCLK: {
            if (postMsg(WM_NCLBUTTONDBLCLK))
                return true;
            break;
        }
        default:
            break;
        }
        return QWidget::nativeEvent(eventType, message, result);
    }
};

#endif
