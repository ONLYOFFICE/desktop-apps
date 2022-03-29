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
//#include <QObject>
#include <QMouseEvent>
#include <QTimer>
#include <Windows.h>
//#include <QDebug>


class Caption: public QWidget
{
    //Q_OBJECT
public:
    Caption(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()):
        QWidget(parent, f),
        m_code(0)
    {
        installEventFilter(this);
        _pTimer = new QTimer(this);
        _pTimer->setSingleShot(true);
        _pTimer->setInterval(240);
        connect(_pTimer, &QTimer::timeout, this, [this]() {
            HWND hWnd = ::GetAncestor((HWND)(window()->windowHandle()->winId()), GA_ROOT);
            POINT pt;
            ::GetCursorPos(&pt);
            ::ReleaseCapture();
            if (m_code == 1) {
                ::SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, POINTTOPOINTS(pt));
            } else
            if (m_code == 2) {
                ::SendMessage(hWnd, WM_NCLBUTTONDBLCLK, HTCAPTION, POINTTOPOINTS(pt));
            }
        });
    }

private:
    int m_code;
    QTimer *_pTimer;
    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                //qDebug() << "Press";
                m_code = 1;
                _pTimer->stop();
                _pTimer->start();
                return true;
            }

        } else
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                //qDebug() << "DbClick";
                m_code = 2;
                _pTimer->stop();
                _pTimer->start();
                return true;
            }

        } /*else
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                //qDebug() << "Release";
                //_pTimer->stop();
                //_pTimer->start();
                //return true;
            }
        }*/
        return QWidget::eventFilter(object, event);
    }

    /*void mousePressEvent(QMouseEvent* event) override
    {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            qDebug() << "Press";
            HWND hWnd = ::GetAncestor((HWND)(window()->windowHandle()->winId()), GA_ROOT);
            POINT pt;
            ::GetCursorPos(&pt);
            ::ReleaseCapture();
            ::SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, POINTTOPOINTS(pt));
        }
        //QWidget::mousePressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            qDebug() << "DbClick";
            HWND hWnd = ::GetAncestor((HWND)(window()->windowHandle()->winId()), GA_ROOT);
            POINT pt;
            ::GetCursorPos(&pt);
            ::ReleaseCapture();
            ::SendMessage(hWnd, WM_NCLBUTTONDBLCLK, HTCAPTION, POINTTOPOINTS(pt));
        }
        //QWidget::mouseDoubleClickEvent(event);
    }*/


};

#endif
