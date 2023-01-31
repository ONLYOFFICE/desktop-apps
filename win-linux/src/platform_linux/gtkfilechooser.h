#ifndef GTKFILECHOOSER_H
#define GTKFILECHOOSER_H

#include <QWidget>
#include <QString>


namespace Gtk
{
typedef enum {
    OPEN = 0, SAVE = 1, FOLDER = 2
} Mode;

QStringList openGtkFileChooser(QWidget *parent,
                               Mode mode,
                               const QString &title,
                               const QString &file,
                               const QString &path,
                               const QString &filter,
                               QString *sel_filter,
                               bool sel_multiple = false);
}

#endif // GTKFILECHOOSER_H
