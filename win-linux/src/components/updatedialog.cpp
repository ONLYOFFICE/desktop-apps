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
#include "updatedialog.h"
#include "platform_win/resource.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <string.h>
#include <QTimer>
#ifdef __linux__
# pragma push_macro("signals")
# undef signals
# include "platform_linux/gtkutils.h"
# pragma pop_macro("signals")
# include <gtk/gtkmessagedialog.h>
# include <gdk/gdkx.h>
# include "res/gresource.c"
# define toCharPtr(qstr) qstr.toLocal8Bit().data()
#else
# include <Windows.h>
# include <CommCtrl.h>
# define toCharPtr(qstr) _wcsdup(qstr.toStdWString().c_str())
#endif

#define DLG_PADDING 7
#define BTN_SPACING 5
#define BTN_PADDING 13
#define DLG_PREF_WIDTH 240

#define BTN_TEXT_SKIPVER    QObject::tr("Skip this version")
#define BTN_TEXT_REMIND     QObject::tr("Remind me later")
#define BTN_TEXT_INSTALL    QObject::tr("Install update")
#define BTN_TEXT_INSLATER   QObject::tr("Later")
#define BTN_TEXT_RESTART    QObject::tr("Restart Now")
#define BTN_TEXT_SAVEANDINS QObject::tr("Save and Install Now")
#define BTN_TEXT_DOWNLOAD   QObject::tr("Download update")

#define TEXT_SKIP        toCharPtr(BTN_TEXT_SKIPVER)
#define TEXT_REMIND      toCharPtr(BTN_TEXT_REMIND)
#define TEXT_INSTALL     toCharPtr(BTN_TEXT_INSTALL)
#define TEXT_INSLATER    toCharPtr(BTN_TEXT_INSLATER)
#define TEXT_RESTART     toCharPtr(BTN_TEXT_RESTART)
#define TEXT_SAVEANDINS  toCharPtr(BTN_TEXT_SAVEANDINS)
#define TEXT_DOWNLOAD    toCharPtr(BTN_TEXT_DOWNLOAD)

#ifdef __linux__
# define AddButton(name, response) \
    gtk_dialog_add_button(GTK_DIALOG(dialog), name, response)
# define GrabFocus(response) \
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), response))

static void on_link_clicked(GtkWidget*, gchar *uri, gpointer)
{
    gtk_show_uri(NULL, uri, GDK_CURRENT_TIME, NULL);
}
#else
static int calcApproxMinWidth(TASKDIALOG_BUTTON *pButtons, uint cButtons)
{
    int width = 0;
    HDC hdc = GetDC(NULL);
    long units = GetDialogBaseUnits();
    HGDIOBJ hFont = GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(hdc, hFont);
    for (uint i = 0; i < cButtons; i++) {
        SIZE textSize = {0,0};
        const wchar_t *text = pButtons[i].pszButtonText;
        GetTextExtentPoint32(hdc, text, (int)std::wcslen(text), &textSize);
        width += MulDiv(textSize.cx, 4, LOWORD(units));
    }
    ReleaseDC(NULL, hdc);
    width +=  (2 * DLG_PADDING) + (2 * BTN_PADDING * cButtons) + (BTN_SPACING * (cButtons - 1));
    return width;
}

static HRESULT CALLBACK Pftaskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    switch (msg) {
    case TDN_HYPERLINK_CLICKED:
        ShellExecute(NULL, L"open", (PCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
        break;
    case TDN_DIALOG_CONSTRUCTED: {
        QTimer::singleShot(0, [=]() {
            if (hwnd)
                WindowHelper::bringToTop(hwnd);
        });
        break;
    }
    default:
        break;
    }
    return S_OK;
}
#endif

#ifdef _WIN32
int WinDlg::showDialog(QWidget *parent, const QString &msg, const QString &content, DlgBtns dlgBtns)
{
    std::wstring lpCaption = QString("  %1").arg(WINDOW_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    QString linkText = !QString(RELEASE_NOTES).isEmpty() ?
                QString("\n<a href=\"%1\">%2</a>").arg(QString(RELEASE_NOTES), QObject::tr("Release notes")) : "";
    std::wstring lpContent = QString("%1\n%2").arg(content, linkText).toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : NULL;

    int msgboxID = 0;
    PCWSTR pIcon = MAKEINTRESOURCE(IDI_MAINICON);
    TASKDIALOG_BUTTON *pButtons = NULL;
    uint cButtons = 0;
    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_INSLATER};
        pButtons[1] = {IDNO,  TEXT_RESTART};
        break;
    case DlgBtns::mbSkipRemindInstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_INSTALL};
        break;
    case DlgBtns::mbSkipRemindSaveandinstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_SAVEANDINS};
        break;
    case DlgBtns::mbSkipRemindDownload:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_DOWNLOAD};
        break;
    default:
        break;
    }

    int nDefltBtn{0};
    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:   nDefltBtn = IDNO; break;
    case DlgBtns::mbSkipRemindInstall: nDefltBtn = IDYES; break;
    case DlgBtns::mbSkipRemindSaveandinstall: nDefltBtn = IDYES; break;
    case DlgBtns::mbSkipRemindDownload: nDefltBtn = IDYES; break;
    default: break;
    }

    TASKDIALOGCONFIG config = {sizeof(TASKDIALOGCONFIG)};
    config.dwFlags            = TDF_ENABLE_HYPERLINKS |
                                TDF_POSITION_RELATIVE_TO_WINDOW |
                                TDF_ALLOW_DIALOG_CANCELLATION;
    if (AscAppManager::isRtlEnabled())
        config.dwFlags |= TDF_RTL_LAYOUT;
    config.hwndParent         = parent_hwnd;
    config.hInstance          = GetModuleHandle(NULL);
    config.pfCallback         = (PFTASKDIALOGCALLBACK)Pftaskdialogcallback;
    config.pButtons           = pButtons;
    config.cButtons           = cButtons;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = lpContent.c_str();
    config.cxWidth            = calcApproxMinWidth(pButtons, cButtons) > DLG_PREF_WIDTH ? 0 : DLG_PREF_WIDTH;

    TaskDialogIndirect(&config, &msgboxID, NULL, NULL);
    for (int i = 0; i < (int)cButtons; i++)
        free((void*)pButtons[i].pszButtonText);
    delete[] pButtons;

    int result = -1;
    switch (msgboxID) {
    case IDYES: result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                          dlgBtns == DlgBtns::mbSkipRemindSaveandinstall) ? DLG_RESULT_INSTALL :
                         (dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_DOWNLOAD :
                                                                      DLG_RESULT_INSLATER; break;
    case IDNO:  result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                          dlgBtns == DlgBtns::mbSkipRemindSaveandinstall ||
                          dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_REMIND :
                                                                      DLG_RESULT_RESTART; break;
    case IDRETRY: result = DLG_RESULT_SKIP; break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
#else
int WinDlg::showDialog(QWidget *parent, const QString &msg, const QString &content, DlgBtns dlgBtns)
{
//    QString title = QString("  %1").arg(WINDOW_TITLE);
    QString primaryText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    QString linkText = !QString(RELEASE_NOTES).isEmpty() ?
                           QString("<a href=\"%1\">%2</a>").arg(QString(RELEASE_NOTES), QObject::tr("Release notes")) : "";
    WindowHelper::CParentDisable oDisabler(parent);
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;

    GtkDialogFlags flags;
    flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new(NULL, flags,
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

    return result;
}
#endif
