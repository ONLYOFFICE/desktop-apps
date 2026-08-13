/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CMESSAGE_GTK_H
#define CMESSAGE_GTK_H

#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include <QTextDocumentFragment>
#include <QWidget>
#include <vector>
#include <memory.h>
#pragma push_macro("signals")
# undef signals
# include "platform_linux/gtkutils.h"
#pragma pop_macro("signals")
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtktogglebutton.h>
#include <gdk/gdkx.h>
#include "res/gresource.c"

#define GTK_CUSTOM_RESPONSE_BASE 100

namespace
{
inline void addDialogButton(GtkWidget *dialog, const QString &name, int response)
{
    gtk_dialog_add_button(GTK_DIALOG(dialog), name.toLocal8Bit().constData(), response);
}

inline void focusDialogResponse(GtkWidget *dialog, int response)
{
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response));
}
}

static void onLinkClicked(GtkWidget*, gchar *uri, gpointer)
{
    gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, NULL);
}


class MessagePlatformBackend
{
public:
    explicit MessagePlatformBackend(QWidget *parent)
        : m_parent(parent)
    {}

    static int showMessage(QWidget *parent, const QString &msg, CMessage::Icon msgIcon,
                           CMessage::Buttons msgBtns, const CMessage::Config &conf)
    {
        MessagePlatformBackend dlg(parent);
        dlg.setText(msg);
        dlg.setIcon(msgIcon);
        dlg.setButtons(msgBtns);
        dlg.setConfig(conf);
        return dlg.exec();
    }

    void setButtons(std::initializer_list<QString> btns)
    {
        m_customButtons.assign(btns.begin(), btns.end());
    }

    void setButtons(CMessage::Buttons btns)
    {
        m_buttons = btns;
        m_customButtons.clear();
    }

    void setDefaultButton(CMessage::DefaultButton btn)
    {
        m_defaultButton = int(btn);
    }

    void setIcon(CMessage::Icon msgIcon)
    {
        m_icon = msgIcon;
    }

    void setText(const QString &text)
    {
        m_text = text;
    }

    void setConfig(const CMessage::Config &conf)
    {
        m_config = conf;
    }

    int exec();

private:
    QWidget *m_parent = nullptr;
    QString m_text;
    CMessage::Icon m_icon = CMessage::Icon::Info;
    CMessage::Buttons m_buttons = CMessage::Buttons::Ok;
    std::vector<QString> m_customButtons;
    int m_defaultButton = 0;
    CMessage::Config m_config;
};

inline int MessagePlatformBackend::exec()
{
    using Buttons = CMessage::Buttons;
    using Type = CMessage::Icon;
    using Result = CMessage::Result;

    WindowHelper::CParentDisable oDisabler(m_parent);
    QString primaryText = QTextDocumentFragment::fromHtml(m_text).toPlainText();
    QString secondaryText = m_config.contentText;
    if (secondaryText.isEmpty())
    {
        const int delim = primaryText.indexOf('\n');
        if (delim != -1)
        {
            secondaryText = primaryText.mid(delim + 1);
            primaryText = primaryText.mid(0, delim);
        }
    }
    Window parent_xid = (m_parent) ? (Window)m_parent->winId() : 0L;

    const char* img_name = NULL;
    switch (m_icon)
    {
    case Type::Info:    img_name = "dialog-information"; break;
    case Type::Warning: img_name = "dialog-warning"; break;
    case Type::Confirm: img_name = "dialog-question"; break;
    case Type::Error:   img_name = "dialog-error"; break;
    case Type::Brand:   img_name = "/icons/app-icon_64.png"; break;
    default:            img_name = "dialog-information"; break;
    }

    GtkWidget *image = NULL;
    if (m_icon == Type::Brand)
    {
        image = gtk_image_new_from_resource(img_name);
    }
    else
    {
        image = gtk_image_new();
        gtk_image_set_from_icon_name(GTK_IMAGE(image), img_name, GTK_ICON_SIZE_DIALOG);
    }

    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL, flags,
                                    GTK_MESSAGE_OTHER, // Message type doesn't show icon
                                    GTK_BUTTONS_NONE, "%s",
                                    primaryText.toLocal8Bit().data());

    // gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);
    // gtk_window_set_title(GTK_WINDOW(dialog), APP_TITLE);
    if (!secondaryText.isEmpty())
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.toLocal8Bit().data());
    if (image)
    {
        gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(dialog), image);
        if (m_icon == Type::Brand)
            gtk_widget_set_margin_top(image, 6);
        gtk_widget_show_all(image);
    }
    if (!m_config.linkText.isEmpty())
    {
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GtkWidget *label = gtk_label_new(m_config.linkText.toLocal8Bit().data());
        gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 50);
        g_signal_connect(G_OBJECT(label), "activate-link", G_CALLBACK(onLinkClicked), NULL);
        gtk_container_add(GTK_CONTAINER(msg_area), label);
        gtk_widget_show_all(label);
    }
    GtkWidget *chkbox = NULL;
    if (m_config.checkBoxState != nullptr)
    {
        // GtkWidget *cont_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        chkbox = gtk_check_button_new_with_label(m_config.checkBoxText.toLocal8Bit().data());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbox), *m_config.checkBoxState);
        gtk_container_add(GTK_CONTAINER(msg_area), chkbox);
        gtk_widget_show_all(chkbox);
    }
    if (m_icon == Type::Brand)
    {
        // Set text alignment
        GtkWidget *msg_area = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));
        GList *children = gtk_container_get_children(GTK_CONTAINER(msg_area));
        for (GList *iter = children; iter != NULL; iter = g_list_next(iter))
        {
            GtkWidget *child = GTK_WIDGET(iter->data);
            if (GTK_IS_LABEL(child))
                gtk_widget_set_halign(child, GTK_ALIGN_START);
        }
        g_list_free(children);
    }

    if (!m_customButtons.empty())
    {
        int index = 0;
        for (const QString &btn : m_customButtons)
        {
            addDialogButton(dialog, btn, GTK_CUSTOM_RESPONSE_BASE + index);
            index++;
        }
        focusDialogResponse(dialog, GTK_CUSTOM_RESPONSE_BASE +
                                    (m_defaultButton < index ? m_defaultButton : 0));
    }
    else
    {
        switch (m_buttons)
        {
        case Buttons::YesNo:
        case Buttons::YesDefNo:
            addDialogButton(dialog, CMessage::ButtonText::yes(), GTK_RESPONSE_YES);
            addDialogButton(dialog, CMessage::ButtonText::no(), GTK_RESPONSE_NO);
            focusDialogResponse(dialog, m_buttons == Buttons::YesDefNo
                                            ? GTK_RESPONSE_YES
                                            : GTK_RESPONSE_NO);
            break;
        case Buttons::YesNoCancel:
        case Buttons::YesDefNoCancel:
            addDialogButton(dialog, CMessage::ButtonText::yes(), GTK_RESPONSE_YES);
            addDialogButton(dialog, CMessage::ButtonText::no(), GTK_RESPONSE_NO);
            addDialogButton(dialog, CMessage::ButtonText::cancel(), GTK_RESPONSE_CANCEL);
            focusDialogResponse(dialog, m_buttons == Buttons::YesDefNoCancel
                                            ? GTK_RESPONSE_YES
                                            : GTK_RESPONSE_CANCEL);
            break;
        case Buttons::OkCancel:
        case Buttons::OkDefCancel:
            addDialogButton(dialog, CMessage::ButtonText::ok(), GTK_RESPONSE_OK);
            addDialogButton(dialog, CMessage::ButtonText::cancel(), GTK_RESPONSE_CANCEL);
            focusDialogResponse(dialog, m_buttons == Buttons::OkDefCancel
                                            ? GTK_RESPONSE_OK
                                            : GTK_RESPONSE_CANCEL);
            break;
        default:
            addDialogButton(dialog, CMessage::ButtonText::ok(), GTK_RESPONSE_OK);
            focusDialogResponse(dialog, GTK_RESPONSE_OK);
            break;
        }
    }

    int msgboxID = gtk_dialog_run (GTK_DIALOG (dialog));
    int result = int(Result::Cancel);
    switch (msgboxID)
    {
    case GTK_RESPONSE_YES: result = int(Result::Yes); break;
    case GTK_RESPONSE_NO:  result = int(Result::No); break;
    case GTK_RESPONSE_OK:  result = int(Result::Ok); break;
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_CANCEL:
        break;
    default:
        if (msgboxID >= GTK_CUSTOM_RESPONSE_BASE)
            result = int(Result::FirstButton) + (msgboxID - GTK_CUSTOM_RESPONSE_BASE);
        break;
    }

    if (m_config.checkBoxState != nullptr)
    {
        gboolean chkState = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbox));
        *m_config.checkBoxState = (chkState == TRUE);
    }

    gtk_widget_destroy(dialog);

    return result;
}

#endif // CMESSAGE_GTK_H

