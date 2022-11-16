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
#include <gtk/gtk.h>
#include <gtk/gtkmessagedialog.h>
#include "gtkmessage.h"
#include <gdk/gdkx.h>

#define translated(qstr) QObject::tr(qstr).toLocal8Bit().data()
#define BTN_TEXT_CANCEL translated("Cancel")
#define BTN_TEXT_YES    translated("Yes")
#define BTN_TEXT_NO     translated("No")
#define BTN_TEXT_OK     translated("OK")
#define AddButton(name, response) \
    gtk_dialog_add_button(GTK_DIALOG(dialog), name, response)
#define GrabFocus(response) \
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response))


static gboolean set_parent(GtkWidget *dialog, gpointer data)
{
    GdkWindow *gdk_dialog = gtk_widget_get_window(dialog);
    Window parent_xid = *(Window*)data;
    GdkDisplay *gdk_display = gdk_display_get_default();
    if (parent_xid != 0L && gdk_display && gdk_dialog) {
        GdkWindow *gdk_qtparent = gdk_x11_window_foreign_new_for_display(gdk_display, parent_xid);
        if (gdk_qtparent) {
            gdk_window_set_transient_for(gdk_dialog, gdk_qtparent);
            return TRUE;
        }
    }
    return FALSE;
}

int GtkMsg::showMessage(QWidget *parent,
                        const QString &msg,
                        MsgType msgType,
                        MsgBtns msgBtns)
{           
    QString plainText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;

    const char* img_name = NULL;
    switch (msgType) {
    case MsgType::MSG_INFO:    img_name = "dialog-information"; break;
    case MsgType::MSG_WARN:    img_name = "dialog-warning"; break;
    case MsgType::MSG_CONFIRM: img_name = "dialog-question"; break;
    case MsgType::MSG_ERROR:   img_name = "dialog-error"; break;
    default:                   img_name = "dialog-information"; break;
    }

    gtk_init(NULL, NULL);   
    GtkWidget *image = NULL;
    image = gtk_image_new();
    gtk_image_set_from_icon_name(GTK_IMAGE(image), img_name, GTK_ICON_SIZE_DIALOG);

    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL,
                                    flags,
                                    GTK_MESSAGE_OTHER, // Message type doesn't show icon
                                    GTK_BUTTONS_NONE,
                                    "%s",
                                    plainText.toLocal8Bit().data());

    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    gtk_window_set_title(GTK_WINDOW(dialog), APP_TITLE);
    //gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "advanced text");
    gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
    gtk_widget_show_all(image);
    //GtkWidget *cont_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
    //GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
    //GtkWidget *chkbox = gtk_check_button_new_with_label("Apply for all");
    //gtk_container_add(GTK_CONTAINER(msg_area), chkbox);
    //gtk_widget_show_all(chkbox);

    switch (msgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        AddButton(BTN_TEXT_YES, GTK_RESPONSE_YES);
        AddButton(BTN_TEXT_NO, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        AddButton(BTN_TEXT_YES, GTK_RESPONSE_YES);
        AddButton(BTN_TEXT_NO, GTK_RESPONSE_NO);
        AddButton(BTN_TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        AddButton(BTN_TEXT_OK, GTK_RESPONSE_OK);
        AddButton(BTN_TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    default:
        AddButton(BTN_TEXT_OK, GTK_RESPONSE_OK);
        break;
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo: GrabFocus(GTK_RESPONSE_NO); break;
    case MsgBtns::mbYesDefNo: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbYesNoCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbYesDefNoCancel: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbOkCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbOkDefCancel: GrabFocus(GTK_RESPONSE_OK); break;
    default: GrabFocus(GTK_RESPONSE_OK); break;
    }

    int msgboxID = gtk_dialog_run (GTK_DIALOG (dialog));
    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case GTK_RESPONSE_YES: result = MODAL_RESULT_YES; break;
    case GTK_RESPONSE_NO:  result = MODAL_RESULT_NO; break;
    case GTK_RESPONSE_OK:  result = MODAL_RESULT_OK; break;
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:
    default:
        break;
    }

    gtk_widget_destroy(dialog);
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);

    return result;
}
