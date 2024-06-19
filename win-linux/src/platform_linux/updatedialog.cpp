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

#include <QTextDocumentFragment>
#include "defines.h"
#include "gtkutils.h"
#include "utils.h"
#include <gtk/gtkmessagedialog.h>
#include "updatedialog.h"
#include "cascapplicationmanagerwrapper.h"
#include <gdk/gdkx.h>
#include "res/gresource.c"

#define toCharPtr(qstr) qstr.toLocal8Bit().data()
#define TEXT_SKIP        toCharPtr(QObject::tr("Skip this version"))
#define TEXT_REMIND      toCharPtr(QObject::tr("Remind me later"))
#define TEXT_INSTALL     toCharPtr(QObject::tr("Install update"))
#define TEXT_INSLATER    toCharPtr(QObject::tr("Later"))
#define TEXT_RESTART     toCharPtr(QObject::tr("Restart Now"))
#define TEXT_SAVEANDINS  toCharPtr(QObject::tr("Save and Install Now"))
#define TEXT_DOWNLOAD    toCharPtr(QObject::tr("Download update"))
#define AddButton(name, response) \
    gtk_dialog_add_button(GTK_DIALOG(dialog), name, response)
#define GrabFocus(response) \
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response))


static void on_link_clicked(GtkWidget*, gchar *uri, gpointer)
{
    gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, NULL);
}

int WinDlg::showDialog(QWidget *parent,
                       const QString &msg,
                       const QString &content,
                       DlgBtns dlgBtns)
{
//    QString title = QString("  %1").arg(WINDOW_TITLE);
    QString primaryText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    QString linkText = !QString(RELEASE_NOTES).isEmpty() ?
                QString("<a href=\"%1\">%2</a>").arg(QString(RELEASE_NOTES), QObject::tr("Release notes")) : "";
    WindowHelper::CParentDisable oDisabler(parent);
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;

    gtk_init(NULL, NULL);
    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL,
                                    flags,
                                    GTK_MESSAGE_OTHER, // Message type doesn't show icon
                                    GTK_BUTTONS_NONE,
                                    "%s", primaryText.toLocal8Bit().data());

    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);
//    gtk_window_set_title(GTK_WINDOW(dialog), title.toLocal8Bit().data());
    if (!content.isEmpty())
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", content.toLocal8Bit().data());

    if (GtkWidget *image = gtk_image_new_from_resource("/icons/app-icon_64.png")) {
        gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
        gtk_widget_set_margin_top(image, 6);
        gtk_widget_show_all(image);
    }

    if (!linkText.isEmpty()) {
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GtkWidget *label = gtk_label_new(linkText.toLocal8Bit().data());
        gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 50);
        g_signal_connect(G_OBJECT(label), "activate-link", G_CALLBACK(on_link_clicked), NULL);
        gtk_container_add(GTK_CONTAINER(msg_area), label);
        gtk_widget_show_all(label);
    }

    { // Set text alignment
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GList *children = gtk_container_get_children(GTK_CONTAINER(msg_area));
        for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *child = GTK_WIDGET(iter->data);
            if (GTK_IS_LABEL(child))
                gtk_widget_set_halign(child, GTK_ALIGN_START);
        }
        g_list_free(children);
    }

    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:
        AddButton(TEXT_INSLATER, GTK_RESPONSE_YES);
        AddButton(TEXT_RESTART, GTK_RESPONSE_NO);
        break;
    case DlgBtns::mbSkipRemindInstall:
        AddButton(TEXT_SKIP, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_INSTALL, GTK_RESPONSE_YES);
        break;
    case DlgBtns::mbSkipRemindSaveandinstall:
        AddButton(TEXT_SKIP, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_SAVEANDINS, GTK_RESPONSE_YES);
        break;
    case DlgBtns::mbSkipRemindDownload:
        AddButton(TEXT_SKIP, GTK_RESPONSE_REJECT);
        AddButton(TEXT_REMIND, GTK_RESPONSE_NO);
        AddButton(TEXT_DOWNLOAD, GTK_RESPONSE_YES);
        break;
    default:
        break;
    }

    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:   GrabFocus(GTK_RESPONSE_NO); break;
    case DlgBtns::mbSkipRemindInstall: GrabFocus(GTK_RESPONSE_YES); break;
    case DlgBtns::mbSkipRemindSaveandinstall: GrabFocus(GTK_RESPONSE_YES); break;
    case DlgBtns::mbSkipRemindDownload: GrabFocus(GTK_RESPONSE_YES); break;
    default: break;
    }

    int msgboxID = gtk_dialog_run (GTK_DIALOG (dialog));
    int result = GTK_RESPONSE_CANCEL;
    switch (msgboxID) {
    case GTK_RESPONSE_YES: result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                                       dlgBtns == DlgBtns::mbSkipRemindSaveandinstall) ? DLG_RESULT_INSTALL :
                                    (dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_DOWNLOAD :
                                                                             DLG_RESULT_INSLATER; break;
    case GTK_RESPONSE_NO:  result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                                       dlgBtns == DlgBtns::mbSkipRemindSaveandinstall ||
                                       dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_REMIND :
                                                                             DLG_RESULT_RESTART; break;
    case GTK_RESPONSE_REJECT: result = DLG_RESULT_SKIP; break;
    case GTK_RESPONSE_CANCEL:
    default:
        break;
    }
    gtk_widget_destroy(dialog);
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);

    return result;
}
