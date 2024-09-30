

#ifndef XDGDESKTOPPORTAL_H
#define XDGDESKTOPPORTAL_H

#include <QWidget>
#include <QString>


namespace Xdg
{
enum Mode {
    OPEN = 0, SAVE = 1, FOLDER = 2
};

QStringList openXdgPortal(QWidget *parent,
                          Mode mode,
                          const QString &title,
                          const QString &file_name,
                          const QString &path,
                          QString filter,
                          QString *sel_filter,
                          bool sel_multiple = false);
}

#endif // XDGDESKTOPPORTAL_H
