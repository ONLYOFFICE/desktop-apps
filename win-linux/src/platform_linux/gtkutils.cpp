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

#include "gtkutils.h"
#include <gdk/gdkx.h>
#include <xcb/xcb.h>
#include <X11/Xlib-xcb.h>


void set_focus(GtkWidget *dialog)
{
    Display *disp = NULL;
    disp = XOpenDisplay(NULL);
    if (disp) {
        xcb_connection_t *conn = NULL;
        conn = XGetXCBConnection(disp);
        GdkWindow *gdk_dialog = gtk_widget_get_window(dialog);
        if (conn && gdk_dialog) {
            xcb_window_t wnd = (xcb_window_t)gdk_x11_window_get_xid(gdk_dialog);
            if (wnd != 0L)
                xcb_set_input_focus(conn, XCB_INPUT_FOCUS_PARENT, wnd, XCB_CURRENT_TIME);
        }
        XCloseDisplay(disp);
    }
}

void set_parent(GtkWidget *dialog, gpointer data)
{
    GdkWindow *gdk_dialog = gtk_widget_get_window(dialog);
    Window parent_xid = *(Window*)data;
    GdkDisplay *gdk_display = gdk_display_get_default();
    if (parent_xid != 0L && gdk_display && gdk_dialog) {
        GdkWindow *gdk_qtparent = gdk_x11_window_foreign_new_for_display(gdk_display, parent_xid);
        if (gdk_qtparent) {
            gdk_window_set_transient_for(gdk_dialog, gdk_qtparent);
        }
    }
}
