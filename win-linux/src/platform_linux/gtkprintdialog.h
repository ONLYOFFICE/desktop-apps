#ifndef GTKPRINTDIALOG_H
#define GTKPRINTDIALOG_H

#include <QWidget>
#include <QString>
#include <QPrinter>
#include <QUrl>
#include "components/cprintdialog.h"

typedef QAbstractPrintDialog::PrintDialogOption PrintOption;
typedef QAbstractPrintDialog::PrintDialogOptions PrintOptions;
typedef QAbstractPrintDialog::PrintRange PrintRange;


class GtkPrintDialog
{
public:
    GtkPrintDialog(QPrinter *printer, QWidget *parent);
    ~GtkPrintDialog();

    void setWindowTitle(const QString &title);
    void setEnabledOptions(PrintOptions enbl_opts);
    void setOptions(PrintOptions opts);
    void setPrintRange(PrintRange print_range);
    QDialog::DialogCode exec();
    void accept();
    PrintRange printRange();
    PrintOptions enabledOptions();
    PrintOptions options();
    QVector<PageRanges> getPageRanges();
    int fromPage();
    int toPage();
    void setFromTo(int from, int to);

private:
    QPrinter *m_printer;
    QWidget *m_parent;
    QString m_title;
    PrintOptions m_options;
    PrintRange m_print_range;
    QVector<PageRanges> m_page_ranges;
};

#endif // GTKPRINTDIALOG_H
