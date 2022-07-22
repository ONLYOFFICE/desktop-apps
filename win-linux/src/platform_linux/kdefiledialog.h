#ifndef KDEFILEDIALOG_H
#define KDEFILEDIALOG_H

#include <QWidget>
#include <QString>
#include <glib.h>


namespace Kde
{
    typedef enum {
        OPEN, SAVE
    } Mode;

    QStringList openNativeDialog(QWidget *parent,
                                 Mode mode,
                                 const QString &name,
                                 const QString &file,
                                 const QString &path,
                                 const QString &filter,
                                 QString *sel_filter,
                                 bool sel_multiple = false);

    void setParent(const long &childId, const long &parentId);
    char* substr(const char* src, int m, int n);
    void parseString(GSList** list,
                     const char* str,
                     const char* delim);

    void nativeFileDialog(const long &parentId,
                          Mode mode,
                          char*** filenames,
                          int* files_count,
                          const char* name,
                          const char* file,
                          const char* path,
                          const char* filter,
                          char** sel_filter,
                          bool sel_multiple);
}

#endif
