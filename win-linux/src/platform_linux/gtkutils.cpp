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
#include <string.h>
#include "platform_linux/xcbutils.h"


gboolean set_focus(GtkWidget *dialog)
{
    GdkWindow *gdk_dialog = gtk_widget_get_window(dialog);
    if (gdk_dialog) {
        xcb_window_t wnd = (xcb_window_t)gdk_x11_window_get_xid(gdk_dialog);
        XcbUtils::setNativeFocusTo(wnd);
    }
    return FALSE;
}

gboolean focus_out(gpointer data)
{
    if (data) {
        DialogTag *tag = (DialogTag*)data;
        GtkWidget *dialog = tag->dialog;
        xcb_window_t parent_xid = (xcb_window_t)tag->parent_xid;
        if (dialog && XcbUtils::isNativeFocus(parent_xid))
            set_focus(dialog);
    }
    return FALSE;
}

void set_parent(GtkWidget *dialog, gpointer data)
{
    if (dialog && data) {
        GdkDisplay *gdk_display = gdk_display_get_default();
        Window parent_xid = *(Window*)data;
        if (gdk_display && parent_xid != None) {
            GdkWindow *gdk_dialog = gtk_widget_get_window(dialog);
            GdkWindow *gdk_qtparent = gdk_x11_window_foreign_new_for_display(gdk_display, parent_xid);
            if (gdk_dialog && gdk_qtparent )
                gdk_window_set_transient_for(gdk_dialog, gdk_qtparent);
        }
    }
}

GtkWidget *find_widget_by_path(GtkWidget *parent, const gchar *widget_path)
{
    if (!parent)
        return NULL;

    gchar *str_path = NULL;
    gtk_widget_path(parent, NULL, &str_path, NULL);
    if (!str_path)
        return NULL;

    if (strcmp(str_path, widget_path) == 0) {
        g_free(str_path);
        return parent;
    }
    g_free(str_path);

    if (GTK_IS_CONTAINER(parent)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        GList *iter = children;
        while (iter) {
            if (GtkWidget *child = find_widget_by_path(GTK_WIDGET(iter->data), widget_path)) {
                g_list_free(children);
                return child;
            }
            iter = g_list_next(iter);
        }
        if (children)
            g_list_free(children);

    } else
    if (GTK_IS_BIN(parent)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
        return find_widget_by_path(child, widget_path);
    }
    return NULL;
}
