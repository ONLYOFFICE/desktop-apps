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

#include "xcbutils.h"
#include <QtConcurrent/QtConcurrent>
#include <QX11Info>
#include <thread>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>


void XcbUtils::moveWindow(xcb_window_t window, int x, int y)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (conn && window != XCB_WINDOW_NONE) {
        uint32_t val[2];
        val[0] = x;
        val[1] = y;
        xcb_configure_window(conn, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, val);
        xcb_flush(conn);
    }
}

bool XcbUtils::isNativeFocus(xcb_window_t window)
{
    xcb_window_t win = 0;
    xcb_connection_t *conn = QX11Info::connection();
    if (conn) {
        xcb_get_input_focus_cookie_t cookie;
        xcb_get_input_focus_reply_t *reply;
        cookie = xcb_get_input_focus(conn);
        reply = xcb_get_input_focus_reply(conn, cookie, NULL);
        if (reply) {
            win = reply->focus;
            free(reply);
        }
        xcb_flush(conn);
    }
    return window == win;
}

void XcbUtils::setNativeFocusTo(xcb_window_t window)
{
    xcb_connection_t *conn = QX11Info::connection();
    if (conn && window != XCB_WINDOW_NONE) {
        xcb_void_cookie_t cookie;
        cookie = xcb_set_input_focus(conn, XCB_INPUT_FOCUS_PARENT,
                                     window, XCB_CURRENT_TIME);
        xcb_flush(conn);
    }
}

static void SetSkipTaskbar(Display* disp, Window win)
{
    Atom wm_state = XInternAtom(disp, "_NET_WM_STATE", True);
    Atom wm_state_skip_taskbar = XInternAtom(disp, "_NET_WM_STATE_SKIP_TASKBAR", True);
    if (wm_state != None && wm_state_skip_taskbar != None)
        XChangeProperty(disp, win, wm_state, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&wm_state_skip_taskbar, 1);
}

static void GetWindowName(Display* disp, Window win, char **name) {
    XClassHint* class_hint = NULL;
    class_hint = XAllocClassHint();
    if (class_hint) {
        Status s = XGetClassHint(disp, win, class_hint);
        if (s == 1)
            *name = strdup(class_hint->res_name);
        XFree(class_hint);
    }
}

static void GetWindowList(Display *disp, Window **list, unsigned long *len) {
    int form;
    unsigned long remain;
    unsigned char *win_list;
    Atom type;
    Atom prop = XInternAtom(disp, "_NET_CLIENT_LIST", true);
    Window root = XDefaultRootWindow(disp);
    int res = XGetWindowProperty(disp, root, prop, 0, 1024, false, XA_WINDOW,
                                 &type, &form, len, &remain, &win_list);
    if (res == Success)
        *list = (Window*)win_list;
}

static bool IsVisible(Display *disp, Window wnd)
{
    xcb_connection_t *conn = XGetXCBConnection(disp);
    if (conn) {
        xcb_get_window_attributes_cookie_t cookie;
        xcb_get_window_attributes_reply_t *reply;
        cookie = xcb_get_window_attributes(conn, wnd);
        reply = xcb_get_window_attributes_reply(conn, cookie, NULL);
        if (reply) {
            uint8_t state = reply->map_state;
            free(reply);
            if (state == XCB_MAP_STATE_VIEWABLE)
                return true;
        }
    }
    return false;
}

void XcbUtils::findWindowAsync(const char *window_name, void *user_data,
                               uint timeout_ms,
                               void(*callback)(xcb_window_t, void*))
{
    QtConcurrent::run([=]() {
        Display *disp = XOpenDisplay(NULL);
        if (!disp)
            return;
        int DELAY_MS = 50;
        int RETRIES = (int)((float)timeout_ms / DELAY_MS);
        Window win_found = None;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
            Window *win_list = NULL;
            unsigned long win_list_size = 0;
            GetWindowList(disp, &win_list, &win_list_size);
            for (int i = 0; i < (int)win_list_size; i++) {
                char *name = NULL;
                GetWindowName(disp, win_list[i], &name);
                if (name) {
                    if (strstr(name, window_name) != NULL) {
                        if (IsVisible(disp, win_list[i])) {
                            win_found = win_list[i];
                            SetSkipTaskbar(disp, win_found);
                            callback((xcb_window_t)win_found, user_data);
                        }
                        free(name);
                        break;
                    }
                    free(name);
                }
            }
            if (win_list)
                XFree(win_list);
        } while (--RETRIES > 0 && win_found == None);
        XCloseDisplay(disp);
    });
}
