
#include <stdio.h>
#include <glib.h>
#include "gtkutils.h"
#include "gtkfilechooser.h"
#include "cascapplicationmanagerwrapper.h"
#include <gdk/gdkx.h>


//static void find_children(GList **list, GtkWidget *wgt, const gchar *name)
//{
//    GList *children = gtk_container_get_children(GTK_CONTAINER(wgt));
//    if (!children)
//        return;
//    for (guint i = 0; i < g_list_length(children); i++) {
//        GtkWidget *child = GTK_WIDGET(g_list_nth(children, i)->data);
//        if (strcmp(name, gtk_widget_get_name(child)) == 0)
//            *list = g_list_append(*list, (gpointer)child);
//        find_children(list, child, name);
//    }
//}

//static void set_ellipsize(GtkWidget *dialog)
//{
//    GList *list = NULL;
//    find_children(&list, dialog, "GtkComboBoxText");
//    for (guint i = 0; i < g_list_length(list); i++) {
//        GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(g_list_nth(list, i)->data);
//        GtkCellRenderer *cell = gtk_cell_renderer_text_new();
//        g_object_set(cell,
//                     "width", 450,
//                     //"popup-fixed-width", FALSE,
//                     "ellipsize", PANGO_ELLIPSIZE_END,
//                     NULL);

//        gtk_cell_layout_clear(GTK_CELL_LAYOUT(combo));
//        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, TRUE);
//        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell, "text", 0, NULL);
//        //g_signal_connect(G_OBJECT(combo), "notify::popup-shown", G_CALLBACK(on_popup), NULL);
//    }
//    g_list_free(list);
//}

static char* substr(const char *src, int m, int n)
{
    int len = n - m;
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
    for (int i = m; i < n && (*(src + i) != '\0'); i++) {
        *dest = *(src + i);
        dest++;
    }
    *dest = '\0';
    return dest - len;
}

static void parseString(GSList** list,
                      const char* str,
                      const char* delim)
{
    char *_str = strdup(str);
    char *token = strtok(_str, delim);
    while (token != NULL) {
        *list = g_slist_append(*list, (gpointer)strdup(token));
        token = strtok(NULL, delim);
    }
    free(_str);
}

static void nativeFileDialog(const Window &parent_xid,
                      Gtk::Mode mode,
                      char*** filenames,
                      int* files_count,
                      const char* title,
                      const char* file,
                      const char* path,
                      const char* flt,
                      char** sel_filter,
                      bool sel_multiple)
{
    GtkFileChooserAction actions[] = {
        GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER
    };

    if (AscAppManager::isRtlEnabled())
        gtk_widget_set_default_direction(GTK_TEXT_DIR_RTL);
    GtkWidget *dialog = NULL;
    dialog = gtk_file_chooser_dialog_new(title,
                                         NULL,
                                         actions[mode],
                                         g_dgettext("gtk30", "_Cancel"),
                                         GTK_RESPONSE_CANCEL,
                                         mode == Gtk::Mode::OPEN || mode == Gtk::Mode::FOLDER  ?
                                             g_dgettext("gtk30", "_Open") : g_dgettext("gtk30", "_Save"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), TRUE);
    //g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    if (mode == Gtk::Mode::OPEN || mode == Gtk::Mode::FOLDER) {
        gtk_file_chooser_set_current_folder(chooser, path);
        gtk_file_chooser_set_select_multiple (chooser, sel_multiple ? TRUE : FALSE);
    } else {
        gtk_file_chooser_set_do_overwrite_confirmation(chooser, FALSE);
        gtk_file_chooser_set_current_folder(chooser, path);
        gtk_file_chooser_set_current_name(chooser, file);
        //gtk_file_chooser_set_filename(chooser, file);
    }

    // Filters
    GSList *list = NULL;
    if (mode != Gtk::Mode::FOLDER) {
        parseString(&list, flt, ";;");
        for (guint i = 0; i < g_slist_length(list); i++) {
            if (char *flt_name = (char*)g_slist_nth(list, i)->data) {
                GtkFileFilter *filter = gtk_file_filter_new();
                //g_print("%s\n", flt_name);
                char *start = strchr(flt_name, '(');
                char *end = strchr(flt_name, ')');
                char *short_flt_name = NULL;
                if (mode == Gtk::Mode::OPEN && strlen(flt_name) > 255 && start != NULL) {
                    int end_index = (int)(start - flt_name - 1);
                    if (end_index > 0)
                        short_flt_name = substr(flt_name, 0, end_index);
                }
                gtk_file_filter_set_name(filter, short_flt_name ? short_flt_name : flt_name);
                if (short_flt_name)
                    free(short_flt_name);

                if (start != NULL && end != NULL) {
                    int start_index = (int)(start - flt_name);
                    int end_index = (int)(end - flt_name);
                    if (start_index < end_index) {
                        char *fltrs = substr(flt_name, start_index + 1, end_index);
                        //g_print("%s\n", fltrs);
                        GSList *flt_list = NULL;
                        parseString(&flt_list, fltrs, " ");
                        free(fltrs);
                        for (guint j = 0; j < g_slist_length(flt_list); j++) {
                            char *nm = (char*)g_slist_nth(flt_list, j)->data;
                            if (nm != NULL)
                                gtk_file_filter_add_pattern(filter, nm);
                        }
                        if (flt_list)
                            g_slist_free(flt_list);
                    }
                }
                gtk_file_chooser_add_filter(chooser, filter);
                if (sel_filter && *sel_filter && strcmp(flt_name, *sel_filter) == 0)
                    gtk_file_chooser_set_filter(chooser, filter);
            }
        }
    }

//    set_ellipsize(dialog);
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        if (sel_multiple) {
            GSList *filenames_list = gtk_file_chooser_get_filenames(chooser);
            *files_count = (int)g_slist_length(filenames_list);
            *filenames = (char**)calloc((size_t)(*files_count), sizeof(char*));
            for (guint i = 0; i < g_slist_length(filenames_list); i++)
                (*filenames)[i] = strdup((char*)g_slist_nth(filenames_list, i)->data);
            g_slist_free(filenames_list);
        } else {
            *files_count = 1;
            *filenames = (char**)calloc((size_t)(*files_count), sizeof(char*));
            **filenames = gtk_file_chooser_get_filename(chooser);
        }
    }
    if (mode != Gtk::Mode::FOLDER) {
        GtkFileFilter *s_filter = gtk_file_chooser_get_filter(chooser);
        if (sel_filter && *sel_filter)
            free(*sel_filter);
        if (s_filter && sel_filter)
            *sel_filter = strdup(gtk_file_filter_get_name(s_filter));
    }
    //gtk_window_close(GTK_WINDOW(dialog));
    gtk_widget_destroy(dialog);
    if (list)
        g_slist_free(list);
    //gtk_main();
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
}

QStringList Gtk::openGtkFileChooser(QWidget *parent,
                                    Mode mode,
                                    const QString &title,
                                    const QString &file,
                                    const QString &path,
                                    const QString &filter,
                                    QString *sel_filter,
                                    bool sel_multiple)
{
    const int pos = file.lastIndexOf('/');
    const QString _file = (pos != -1) ?
                file.mid(pos + 1) : file;
    const QString _path = (path.isEmpty() && pos != -1) ?
                file.mid(0, pos) : path;

    QStringList files;
    char **filenames = nullptr;
    char *_sel_filter = (sel_filter) ? strdup(sel_filter->toLocal8Bit().data()) : nullptr;
    int files_count = 0;
    Window parent_xid = (parent) ? (Window)parent->winId() : 0L;
    nativeFileDialog(parent_xid,
                     mode,
                     &filenames,
                     &files_count,
                     title.toLocal8Bit().data(),
                     _file.toLocal8Bit().data(),
                     _path.toLocal8Bit().data(),
                     filter.toLocal8Bit().data(),
                     &_sel_filter,
                     sel_multiple);

    for (int i = 0; i < files_count; i++) {
        //printf("\n%s  %d\n", filenames[i], files_count);
        if (filenames[i]) {
            files.append(QString::fromUtf8(filenames[i]));
            free(filenames[i]);
        }
    }
    if (filenames) {
        free(filenames);
    }
    if (_sel_filter) {
        if (sel_filter)
            *sel_filter = QString::fromUtf8(_sel_filter);
        free(_sel_filter);
    }

    return files;
}
