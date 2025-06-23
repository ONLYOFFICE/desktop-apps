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
#include "gtkutils.h"
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtktogglebutton.h>
#include "gtkmessage.h"
#include "cascapplicationmanagerwrapper.h"
#include <gdk/gdkx.h>


#define toCharPtr(qstr) qstr.toLocal8Bit().data()
#define TEXT_CANCEL toCharPtr(BTN_TEXT_CANCEL)
#define TEXT_YES    toCharPtr(BTN_TEXT_YES)
#define TEXT_NO     toCharPtr(BTN_TEXT_NO)
#define TEXT_OK     toCharPtr(BTN_TEXT_OK)
#define TEXT_SKIP   toCharPtr(BTN_TEXT_SKIP)
#define TEXT_BUY    toCharPtr(BTN_TEXT_BUY)
#define TEXT_ACTIVATE   toCharPtr(BTN_TEXT_ACTIVATE)
#define TEXT_CONTINUE   toCharPtr(BTN_TEXT_CONTINUE)
#define AddButton(name, response) \
    gtk_dialog_add_button(GTK_DIALOG(dialog), name, response)
#define GrabFocus(response) \
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response))


int GtkMsg::showMessage(QWidget *parent,
                        const QString &msg,
                        MsgType msgType,
                        MsgBtns msgBtns,
                        bool   *checkBoxState,
                        const QString &chekBoxText)
{           
    QString plainText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    const int delim = plainText.indexOf('\n');
    const QString primaryText = (delim != -1) ? plainText.mid(0, delim) : plainText;
    const QString secondaryText = (delim != -1) ? plainText.mid(delim + 1) : "";
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;

    const char* img_name = NULL;
    switch (msgType) {
    case MsgType::MSG_INFO:    img_name = "dialog-information"; break;
    case MsgType::MSG_WARN:    img_name = "dialog-warning"; break;
    case MsgType::MSG_CONFIRM: img_name = "dialog-question"; break;
    case MsgType::MSG_ERROR:   img_name = "dialog-error"; break;
    default:                   img_name = "dialog-information"; break;
    }

    GtkWidget *image = NULL;
    image = gtk_image_new();
    gtk_image_set_from_icon_name(GTK_IMAGE(image), img_name, GTK_ICON_SIZE_DIALOG);

    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL,
                                    flags,
                                    GTK_MESSAGE_OTHER, // Message type doesn't show icon
                                    GTK_BUTTONS_NONE,
                                    "%s",
                                    primaryText.toLocal8Bit().data());

    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);
    //gtk_window_set_title(GTK_WINDOW(dialog), APP_TITLE);
    if (!secondaryText.isEmpty())
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.toLocal8Bit().data());
    gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
    gtk_widget_show_all(image);

    GtkWidget *chkbox = NULL;
    if (checkBoxState != nullptr) {
        //GtkWidget *cont_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        chkbox = gtk_check_button_new_with_label(chekBoxText.toLocal8Bit().data());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbox), (*checkBoxState) ? TRUE : FALSE);
        gtk_container_add(GTK_CONTAINER(msg_area), chkbox);
        gtk_widget_show_all(chkbox);
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        AddButton(TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        AddButton(TEXT_OK, GTK_RESPONSE_OK);
        AddButton(TEXT_CANCEL, GTK_RESPONSE_CANCEL);
        break;
    case MsgBtns::mbYesDefSkipNo:
        AddButton(TEXT_YES, GTK_RESPONSE_YES);
        AddButton(TEXT_SKIP, GTK_RESPONSE_REJECT);
        AddButton(TEXT_NO, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbBuy:
        AddButton(TEXT_BUY, GTK_RESPONSE_YES);
        break;
    case MsgBtns::mbActivateDefContinue:
        AddButton(TEXT_ACTIVATE, GTK_RESPONSE_YES);
        AddButton(TEXT_CONTINUE, GTK_RESPONSE_NO);
        break;
    case MsgBtns::mbContinue:
        AddButton(TEXT_CONTINUE, GTK_RESPONSE_OK);
        break;
    default:
        AddButton(TEXT_OK, GTK_RESPONSE_OK);
        break;
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo: GrabFocus(GTK_RESPONSE_NO); break;
    case MsgBtns::mbYesDefNo: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbYesNoCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbYesDefNoCancel: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbOkCancel: GrabFocus(GTK_RESPONSE_CANCEL); break;
    case MsgBtns::mbOkDefCancel: GrabFocus(GTK_RESPONSE_OK); break;
    case MsgBtns::mbYesDefSkipNo: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbBuy: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbActivateDefContinue: GrabFocus(GTK_RESPONSE_YES); break;
    case MsgBtns::mbContinue: GrabFocus(GTK_RESPONSE_OK); break;
    default: GrabFocus(GTK_RESPONSE_OK); break;
    }

    int msgboxID = gtk_dialog_run (GTK_DIALOG (dialog));
    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case GTK_RESPONSE_YES: result = (msgBtns == MsgBtns::mbBuy) ? MODAL_RESULT_BUY :
                                    (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_ACTIVATE :
                                                                                  MODAL_RESULT_YES; break;
    case GTK_RESPONSE_NO:  result = (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_CONTINUE :
                                                                                  MODAL_RESULT_NO; break;
    case GTK_RESPONSE_OK:  result = (msgBtns == MsgBtns::mbContinue) ? MODAL_RESULT_CONTINUE :
                                                                       MODAL_RESULT_OK; break;
    case GTK_RESPONSE_REJECT:  result = MODAL_RESULT_SKIP; break;
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:
    default:
        break;
    }

    if (checkBoxState != nullptr) {
        gboolean chkState = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbox));
        *checkBoxState = (chkState == TRUE);
    }

    gtk_widget_destroy(dialog);

    return result;
}
