

#ifndef XDGDESKTOPPORTAL_H
#define XDGDESKTOPPORTAL_H

#include <QWidget>
#include <QString>

typedef enum PortalMode {
    OPEN = 0, SAVE = 1, FOLDER = 2
} PortalMode;

namespace XdgPortal
{
    QStringList openNativeDialog(QWidget *parent,
                                 PortalMode mode,
                                 const QString &title,
                                 const QString &file_name,
                                 const QString &path,
                                 const QString &filter,
                                 QString *sel_filter,
                                 bool sel_multiple = false);
}

#endif // XDGDESKTOPPORTAL_H
