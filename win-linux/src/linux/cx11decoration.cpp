/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include "cx11decoration.h"
#include <QX11Info>

#include <QDebug>

#include "X11/Xlib.h"
#include "X11/cursorfont.h"

const int k_NET_WM_MOVERESIZE_SIZE_TOPLEFT =     0;
const int k_NET_WM_MOVERESIZE_SIZE_TOP =         1;
const int k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT =    2;
const int k_NET_WM_MOVERESIZE_SIZE_RIGHT =       3;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOM =      5;
const int k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT =  6;
const int k_NET_WM_MOVERESIZE_SIZE_LEFT =        7;
const int k_NET_WM_MOVERESIZE_MOVE =             8;

#define MWM_HINTS_DECORATIONS   2
typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} MotifWmHints;

CX11Decoration::CX11Decoration(QWidget * w)
    : m_window(w)
    , m_title(NULL)
    , m_currentCursor(0)
    , m_decoration(true)
    , m_nBorderSize(0)
{
    createCursors();

#ifdef FORCE_LINUX_CUSTOMWINDOW_MARGINS
    m_nBorderSize = CUSTOM_BORDER_WIDTH;
#endif
}

CX11Decoration::~CX11Decoration()
{
    freeCursors();
}

void CX11Decoration::setTitleWidget(QWidget * w)
{
    m_title = w;
    m_title->setMouseTracking(true);
}

void CX11Decoration::createCursors()
{
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOPLEFT]     = XCreateFontCursor(QX11Info::display(), XC_top_left_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOP]         = XCreateFontCursor(QX11Info::display(), XC_top_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT]    = XCreateFontCursor(QX11Info::display(), XC_top_right_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_RIGHT]       = XCreateFontCursor(QX11Info::display(), XC_right_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT] = XCreateFontCursor(QX11Info::display(), XC_bottom_right_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOM]      = XCreateFontCursor(QX11Info::display(), XC_bottom_side);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT]  = XCreateFontCursor(QX11Info::display(), XC_bottom_left_corner);
    m_cursors[k_NET_WM_MOVERESIZE_SIZE_LEFT]        = XCreateFontCursor(QX11Info::display(), XC_left_side);
}

void CX11Decoration::freeCursors()
{
    Display * _display = QX11Info::display();
    std::for_each(m_cursors.begin(), m_cursors.end(),
        [_display](std::pair<int, Cursor> i) {
            XFreeCursor(_display, i.second);
        }
    );
}

int CX11Decoration::hitTest(int x, int y) const
{
    QRect rect = m_window->rect();
    int bw = CUSTOM_BORDER_WIDTH, bbw = CUSTOM_BORDER_WIDTH;
    int w = rect.width(), h = rect.height();

    QRect rc_top_left       = rect.adjusted(0, 0, -(w-bbw), -(h-bbw));
    QRect rc_top            = rect.adjusted(bbw, 0, -bbw, -(h-bw));
    QRect rc_top_right      = rect.adjusted(w-bbw, 0, 0, -(h-bbw));
    QRect rc_right          = rect.adjusted(w-bw, bbw, 0, -bbw);
    QRect rc_bottom_right   = rect.adjusted(w-bbw, h-bbw, 0, 0);
    QRect rc_bottom         = rect.adjusted(bbw, h-bw, -bbw, 0);
    QRect rc_bottom_left    = rect.adjusted(0, h-bbw, -(w-bbw), 0);
    QRect rc_left           = rect.adjusted(0, bbw, -(w-bw), -bbw);

    int _out_ret = -1;
    if (rc_top.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOP;
    } else
    if (rc_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_RIGHT;
    } else
    if (rc_bottom.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOM;
    } else
    if (rc_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_LEFT;
    } else
    if (rc_top_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOPLEFT;
    } else
    if (rc_top_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
    } else
    if (rc_bottom_left.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
    } else
    if (rc_bottom_right.contains(x, y)) {
        _out_ret = k_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
    }

    return _out_ret;
}

void CX11Decoration::checkCursor(QPoint & p)
{
    int _hit_test = hitTest(p.x(), p.y());

    Cursor _cursor = 0;
    if (!(_hit_test < 0)) {
        _cursor = m_cursors[_hit_test];
    }

    Display * _display = QX11Info::display();
    if (_cursor) {
        if (m_currentCursor == 0 || m_currentCursor != _cursor) {
            m_currentCursor = _cursor;
            XDefineCursor(_display, m_window->winId(), _cursor);

            XFlush(_display);
        }
    } else
    if (m_currentCursor) {
        m_currentCursor = 0;
        XUndefineCursor(_display, m_window->winId());

        XFlush(_display);
    }
}

void CX11Decoration::dispatchMouseMove(QMouseEvent * e, bool bIsPressed)
{
    if (e->buttons().testFlag(Qt::LeftButton)) {
        int _direction = -1;

        _direction = m_title->geometry().adjusted(m_nBorderSize + 1, m_nBorderSize + 1, m_nBorderSize + 1, m_nBorderSize + 1).contains(e->pos()) ?
                        k_NET_WM_MOVERESIZE_MOVE : hitTest(e->pos().x(), e->pos().y());

        if (bIsPressed && k_NET_WM_MOVERESIZE_MOVE == _direction)
            return;

        if (!(_direction < 0)) {
            Display * xdisplay_ = QX11Info::display();
            Window x_root_window_ = DefaultRootWindow(xdisplay_);

            XUngrabPointer(xdisplay_, CurrentTime);

            XEvent event;
            memset(&event, 0, sizeof(event));
            event.xclient.type = ClientMessage;
            event.xclient.display = xdisplay_;
            event.xclient.window = m_window->winId();
            event.xclient.message_type = XInternAtom(xdisplay_, "_NET_WM_MOVERESIZE", false);
            event.xclient.format = 32;
            event.xclient.data.l[0] = e->globalPos().x();
            event.xclient.data.l[1] = e->globalPos().y();
            event.xclient.data.l[2] = _direction;
            event.xclient.data.l[3] = 0;
            event.xclient.data.l[4] = 0;

            XSendEvent(xdisplay_, x_root_window_, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
            XFlush(xdisplay_);
//            XSync(xdisplay_, false);
        }
    } else {
        QPoint p(e->pos().x(), e->pos().y());
        checkCursor(p);
    }
}

void CX11Decoration::turnOn()
{
//    switchDecoration(true);

//    Display * _xdisplay = QX11Info::display();
//    Atom _atom = XInternAtom(_xdisplay, "_MOTIF_WM_HINTS", true);
//    if ( _atom != None  && XDeleteProperty(_xdisplay, m_window->winId(), _atom) < BadValue ) {
//        m_decoration = true;
//        XFlush(_xdisplay);
//    }
}

void CX11Decoration::turnOff()
{
//    switchDecoration(false);

    m_window->setWindowFlags(Qt::FramelessWindowHint);
    m_decoration = false;
}

void CX11Decoration::switchDecoration(bool on)
{
    if (m_decoration != on) {
        // Signals that the reader of the _MOTIF_WM_HINTS property should pay
        // attention to the value of |decorations|.

        MotifWmHints motif_hints = {MWM_HINTS_DECORATIONS, 0, 0, 0, 0};
        motif_hints.decorations = int(on);

        Display * _xdisplay = QX11Info::display();
        Atom hint_atom = XInternAtom(_xdisplay, "_MOTIF_WM_HINTS", false);
        if ( hint_atom != None &&
                    XChangeProperty(_xdisplay, m_window->winId(),
                               hint_atom, hint_atom, 32, PropModeReplace,
                               (unsigned char *)&motif_hints, sizeof(MotifWmHints)/sizeof(long)) < BadValue )
        {
            m_decoration = on;
            XFlush(_xdisplay);
        }
    }
}

bool CX11Decoration::isDecorated()
{
    return m_decoration;
}
