#include <stdint.h>
#include <glib.h>
#include "gtkutils.h"
#include "gtkprintdialog.h"
#include <gdk/gdkx.h>
#include <gtk/gtkunixprint.h>

#define PDF_PRINTER_NAME "Print to File"
#define LPR_PRINTER_NAME "Print to LPR"

typedef QPrinter::Unit QUnit;
typedef uint16_t WORD;


GtkPrintDialog::GtkPrintDialog(QPrinter *printer, QWidget *parent) :
    m_printer(printer),
    m_parent(parent),
    m_title(QString()),
    m_options(PrintOptions()),
    m_print_range(PrintRange::AllPages),
    m_page_ranges(QVector<PageRanges>())
{
    m_print_range = (PrintRange)printer->printRange();
    if (m_printer->collateCopies())
        m_options |= PrintOption::PrintCollateCopies;
    m_page_ranges.append(PageRanges(m_printer->fromPage(), m_printer->toPage()));
}

GtkPrintDialog::~GtkPrintDialog()
{

}

void GtkPrintDialog::setWindowTitle(const QString &title)
{
    m_title = title;
}

void GtkPrintDialog::setEnabledOptions(PrintOptions enbl_opts)
{
    m_options = enbl_opts;
}

void GtkPrintDialog::setOptions(PrintOptions opts)
{
    m_options = opts;
}

void GtkPrintDialog::setPrintRange(PrintRange print_range)
{
    m_print_range = print_range;
}

QDialog::DialogCode GtkPrintDialog::exec()
{
    gtk_init(NULL, NULL);
    QDialog::DialogCode exit_code = QDialog::DialogCode::Rejected;
    Window parent_xid = (m_parent) ? (Window)m_parent->winId() : 0L;

    auto qt_printer_name = m_printer->printerName();
    auto qt_resolution = m_printer->resolution();
    auto qt_orient = m_printer->orientation();
    auto qt_duplex = m_printer->duplex();
    auto qt_color_mode = m_printer->colorMode();
    auto qt_copy_count = m_printer->copyCount();
    auto qt_page_order = m_printer->pageOrder();
    auto qt_output_filename = m_printer->outputFileName();
    //auto qt_doc_name = m_printer->docName();
    //auto qt_full_page = m_printer->fullPage();
    //auto qt_color_count = m_printer->colorCount();
    //auto qt_supported_res = m_printer->supportedResolutions();
    //auto qt_supports_multi_copies = m_printer->supportsMultipleCopies();
    //auto qt_selection_option = m_printer->printerSelectionOption();
    //auto qt_output_format = m_printer->outputFormat();
    //auto qt_paper_source = m_printer->paperSource();

    // Qt-PrintOptions:
    // None = 0
    // PrintToFile = 1          - not applied
    // PrintSelection = 2       - not applied
    // PrintPageRange = 4       - not applied
    // PrintShowPageSize = 8    - not applied
    // PrintCollateCopies = 16
    // DontUseSheet = 32        - not applied
    // PrintCurrentPage = 64    - not applied
    WORD _capabilityes = (
        GTK_PRINT_CAPABILITY_PAGE_SET |
        GTK_PRINT_CAPABILITY_COPIES |
        //GTK_PRINT_CAPABILITY_SCALE |          - muted
        GTK_PRINT_CAPABILITY_REVERSE |
        GTK_PRINT_CAPABILITY_GENERATE_PDF
        //GTK_PRINT_CAPABILITY_GENERATE_PS |
        //GTK_PRINT_CAPABILITY_PREVIEW |        - muted
        //GTK_PRINT_CAPABILITY_NUMBER_UP |      - muted
        //GTK_PRINT_CAPABILITY_NUMBER_UP_LAYOUT - muted
    );

    if (m_options.testFlag(PrintOption::PrintCollateCopies))
        _capabilityes |= GTK_PRINT_CAPABILITY_COLLATE;
    GtkPrintCapabilities capabilityes = (GtkPrintCapabilities)_capabilityes;

    // Input settings
    GtkPrintSettings *settings;
    settings = gtk_print_settings_new();
    {
        gtk_print_settings_set_printer(settings, (qt_printer_name == "") ?
                                           PDF_PRINTER_NAME : qt_printer_name.toUtf8().data());
        gtk_print_settings_set_resolution(settings, qt_resolution);
        gtk_print_settings_set_use_color(settings, (qt_color_mode == QPrinter::Color) ? TRUE : FALSE);

        GtkPrintQuality quality_arr[4] = {
            GTK_PRINT_QUALITY_LOW,
            GTK_PRINT_QUALITY_NORMAL,
            GTK_PRINT_QUALITY_HIGH,
            GTK_PRINT_QUALITY_DRAFT
        };
        gtk_print_settings_set_quality(settings, quality_arr[2]);
        //gtk_print_settings_set_printer_lpi(settings, 1);
        //gtk_print_settings_set_paper_size(settings, psize);
        //gtk_print_settings_set_paper_width(settings, m_printer->widthMM(), unit);
        //gtk_print_settings_set_paper_height(settings, m_printer->heightMM(), unit);
        //gtk_print_settings_set_orientation(settings, ornt);

        // Qt-PrintRange:
        // AllPages = 0
        // Selection = 1
        // PageRange = 2
        // CurrentPage = 3
        GtkPrintPages pages_arr[4] = {
            GTK_PRINT_PAGES_ALL,
            GTK_PRINT_PAGES_SELECTION,
            GTK_PRINT_PAGES_RANGES,
            GTK_PRINT_PAGES_CURRENT
        };
        const int print_range = (int)m_print_range;
        GtkPrintPages print_pages = (print_range >= 0 && print_range <= 4) ?
                    pages_arr[m_print_range] : pages_arr[0];
        gtk_print_settings_set_print_pages(settings, print_pages);

        const gint input_num_ranges = 1;
        GtkPageRange input_range[input_num_ranges] = {
            {m_printer->fromPage() - 1, m_printer->toPage() - 1}
        };
        gtk_print_settings_set_page_ranges(settings, input_range, input_num_ranges);

        GtkPageSet page_set_arr[3] = {
            GTK_PAGE_SET_ALL,
            GTK_PAGE_SET_EVEN,
            GTK_PAGE_SET_ODD
        };
        gtk_print_settings_set_page_set(settings, page_set_arr[0]);
        gtk_print_settings_set_collate(settings, FALSE);

        // Qt-Duplex:
        // DuplexNone = 0
        // DuplexAuto = 1       - not applied
        // DuplexLongSide = 2
        // DuplexShortSide = 3
        GtkPrintDuplex duplex_arr[3] = {
            GTK_PRINT_DUPLEX_SIMPLEX,
            GTK_PRINT_DUPLEX_HORIZONTAL,
            GTK_PRINT_DUPLEX_VERTICAL
        };
        GtkPrintDuplex duplex = (qt_duplex == QPrinter::DuplexLongSide) ?  duplex_arr[1] :
                                (qt_duplex == QPrinter::DuplexShortSide) ? duplex_arr[2] :
                                                                           duplex_arr[0];
        gtk_print_settings_set_duplex(settings, duplex);
        gtk_print_settings_set_reverse(settings,
            (qt_page_order == QPrinter::LastPageFirst) ? TRUE : FALSE);

        gtk_print_settings_set_n_copies(settings, qt_copy_count);
        gtk_print_settings_set_scale(settings, 100);
        gtk_print_settings_set(settings, GTK_PRINT_SETTINGS_OUTPUT_URI, g_filename_to_uri(
                                   qt_output_filename.toUtf8().data(), NULL, NULL));
        //gtk_print_settings_set_number_up(settings, 1);
        //gtk_print_settings_set_number_up_layout(settings,);
        //gtk_print_settings_set_dither(settings, "");
        //gtk_print_settings_set_finishings(settings, "");
    }

    // Input page setup
    GtkPageSetup *page_setup;
    page_setup = gtk_page_setup_new();
    {
        GtkUnit unit = GtkUnit::GTK_UNIT_MM;
        QUnit qt_unit(QUnit::Millimeter);
        double left_in, top_in, right_in, bottom_in;
        m_printer->getPageMargins(&left_in, &top_in, &right_in, &bottom_in, qt_unit);
        gtk_page_setup_set_left_margin(page_setup, left_in, unit);
        gtk_page_setup_set_top_margin(page_setup, top_in, unit);
        gtk_page_setup_set_right_margin(page_setup, right_in, unit);
        gtk_page_setup_set_bottom_margin(page_setup, bottom_in, unit);

        const int width_in = (qt_orient == QPrinter::Portrait) ?
                    m_printer->widthMM() : m_printer->heightMM();
        const int height_in = (qt_orient == QPrinter::Portrait) ?
                    m_printer->heightMM() : m_printer->widthMM();
        GtkPaperSize *psize = gtk_paper_size_new_custom(
                    m_printer->paperName().toUtf8().data(),
                    m_printer->paperName().toUtf8().data(),
                    width_in,
                    height_in,
                    unit);
        gtk_page_setup_set_paper_size(page_setup, psize);
        gtk_paper_size_free(psize);

        // Qt-Orient:
        // Portrait = 0
        // Landscape = 1
        GtkPageOrientation ornt_arr[4]  = {
            GTK_PAGE_ORIENTATION_PORTRAIT,
            GTK_PAGE_ORIENTATION_LANDSCAPE,
            GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT,
            GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE
        };
        const int print_ornt = (int)qt_orient;
        GtkPageOrientation ornt = (print_ornt >= 0 && print_ornt <= 4) ?
                    ornt_arr[print_ornt] : ornt_arr[0];
        gtk_page_setup_set_orientation(page_setup, ornt);
    }

    // Init dialog
    GtkWidget *dialog;
    dialog = gtk_print_unix_dialog_new(m_title.toUtf8().data(), NULL);   
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    //g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(dialog), "realize", G_CALLBACK(set_parent), (gpointer)&parent_xid);
    g_signal_connect(G_OBJECT(dialog), "map_event", G_CALLBACK(set_focus), NULL);
    DialogTag tag;  // unable to send parent_xid via g_signal_connect and "focus_out_event"
    memset(&tag, 0, sizeof(tag));
    tag.dialog = dialog;
    tag.parent_xid = (ulong)parent_xid;
    g_signal_connect_swapped(G_OBJECT(dialog), "focus_out_event", G_CALLBACK(focus_out), (gpointer)&tag);

    gtk_print_unix_dialog_set_manual_capabilities(GTK_PRINT_UNIX_DIALOG(dialog), capabilityes);
    gtk_print_unix_dialog_set_embed_page_setup(GTK_PRINT_UNIX_DIALOG(dialog), TRUE);
    gtk_print_unix_dialog_set_settings(GTK_PRINT_UNIX_DIALOG(dialog), settings);
    gtk_print_unix_dialog_set_page_setup(GTK_PRINT_UNIX_DIALOG(dialog), page_setup);
    gtk_print_unix_dialog_set_has_selection(GTK_PRINT_UNIX_DIALOG(dialog), TRUE);
    gtk_print_unix_dialog_set_support_selection(GTK_PRINT_UNIX_DIALOG(dialog),
            m_options.testFlag(PrintOption::PrintSelection) ? TRUE : FALSE);
    gtk_print_unix_dialog_set_current_page(GTK_PRINT_UNIX_DIALOG(dialog), 0);

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    g_object_unref (G_OBJECT(page_setup));
    g_object_unref(G_OBJECT(settings));
    switch (res) {
    case GTK_RESPONSE_OK: {
        GtkPrinter *printer;
        printer = gtk_print_unix_dialog_get_selected_printer(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            const char* name = gtk_printer_get_name(printer);
            m_printer->setPrinterName(QString::fromUtf8(
                                          strcmp(name, PDF_PRINTER_NAME) == 0 ? "" : name));
        }

        settings = gtk_print_unix_dialog_get_settings(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            enum _ColorMode {_GrayScale, _Color}; // GrayScale is defined already
            gboolean use_color = gtk_print_settings_get_use_color(settings);
            m_printer->setColorMode(use_color == TRUE ?
                          QPrinter::ColorMode(_Color) : QPrinter::ColorMode(_GrayScale));

            GtkPrintPages print_pages = gtk_print_settings_get_print_pages(settings);
            PrintRange range_arr[4] = {
                PrintRange::AllPages,
                PrintRange::CurrentPage,
                PrintRange::PageRange,
                PrintRange::Selection
            };
            const int _print_pages = (int)print_pages;
            m_print_range = (_print_pages >= 0 && _print_pages <= 4) ?
                        range_arr[_print_pages] : range_arr[0];

            GtkPageSet page_set = gtk_print_settings_get_page_set(settings);
            GtkPageRange* page_ranges;
            gint num_ranges;
            page_ranges = gtk_print_settings_get_page_ranges(settings, &num_ranges);
            if (!m_page_ranges.isEmpty())
                m_page_ranges.clear();
            for (gint i = 0; i < num_ranges; i++) {
                int start = page_ranges[i].start + 1;
                int end = page_ranges[i].end + 1;

                if (page_set == GTK_PAGE_SET_ALL) {
                    m_page_ranges.append(PageRanges(start, end));
                } else {
                    for (int page = start; page <= end; page++) {
                        if (page % 2 == 0 && page_set == GTK_PAGE_SET_EVEN)
                            m_page_ranges.append(PageRanges(page, page));   // Filter Even pages
                        else
                        if (page % 2 != 0 && page_set == GTK_PAGE_SET_ODD)
                            m_page_ranges.append(PageRanges(page, page));   // Filter Odd pages
                    }
                }
                if (i == 0)
                    m_printer->setFromTo(start, end);
            }

            gboolean collate = gtk_print_settings_get_collate(settings);
            m_printer->setCollateCopies((bool)collate);

            GtkPrintDuplex duplex = gtk_print_settings_get_duplex(settings);
            QPrinter::DuplexMode duplex_arr[4] = {
                QPrinter::DuplexNone,
                QPrinter::DuplexAuto,
                QPrinter::DuplexLongSide,
                QPrinter::DuplexShortSide
            };
            m_printer->setDuplex(duplex == GTK_PRINT_DUPLEX_HORIZONTAL ? duplex_arr[2] :
                                 duplex == GTK_PRINT_DUPLEX_VERTICAL ?   duplex_arr[3] :
                                                                         duplex_arr[0]);

            gboolean reverse = gtk_print_settings_get_reverse(settings);
            m_printer->setPageOrder(reverse == TRUE ?
                        QPrinter::LastPageFirst : QPrinter::FirstPageFirst);

            int n_copies = gtk_print_settings_get_n_copies(settings);
            m_printer->setNumCopies(n_copies);

            const char* output_uri = gtk_print_settings_get(settings, GTK_PRINT_SETTINGS_OUTPUT_URI);
            auto path = QUrl::fromPercentEncoding(QByteArray(output_uri)).replace("file://", "");
            m_printer->setOutputFileName(path);

            //gtk_print_settings_get_quality(settings);
            //gtk_print_settings_get_scale(settings);
            //gtk_print_settings_get_number_up(settings);
            //gtk_print_settings_get_number_up_layout(settings);
            //gtk_print_settings_get_dither(settings);
            //gtk_print_settings_get_finishings(settings);
        }
        page_setup = gtk_print_unix_dialog_get_page_setup(GTK_PRINT_UNIX_DIALOG(dialog));
        {
            QUnit qt_unit(QUnit::Millimeter);
            GtkUnit unit = GtkUnit::GTK_UNIT_MM;
            gdouble left = gtk_page_setup_get_left_margin(page_setup, unit);
            gdouble top = gtk_page_setup_get_top_margin(page_setup, unit);
            gdouble right = gtk_page_setup_get_right_margin(page_setup, unit);
            gdouble bottom = gtk_page_setup_get_bottom_margin(page_setup, unit);
            m_printer->setPageMargins(left, top, right, bottom, qt_unit);

            GtkPaperSize *paper_size = gtk_page_setup_get_paper_size(page_setup);
            const char* paper_name = gtk_paper_size_get_display_name(paper_size);
            gdouble width = gtk_paper_size_get_width(paper_size, unit);
            gdouble height = gtk_paper_size_get_height(paper_size, unit);
            m_printer->setPaperName(QString::fromUtf8(paper_name));
            m_printer->setPaperSize(QSizeF(width, height), qt_unit);

            GtkPageOrientation orient = gtk_page_setup_get_orientation(page_setup);
            QPrinter::Orientation orient_arr[2] = {
                QPrinter::Portrait,
                QPrinter::Landscape
            };
            const int print_ornt = (int)orient;
            m_printer->setOrientation((print_ornt == 0 || print_ornt == 2) ?
                        orient_arr[0] : orient_arr[1]);
        }
        exit_code = QDialog::DialogCode::Accepted;
        break;
    }
    case GTK_RESPONSE_APPLY: {  // ask for preview
        break;
    }
    default:
        break;
    } 
    //gtk_window_close(GTK_WINDOW(dialog));
    gtk_widget_destroy(dialog);
    //gtk_main();
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
    return exit_code;
}

PrintRange GtkPrintDialog::printRange()
{
    return m_print_range;
}

PrintOptions GtkPrintDialog::enabledOptions()
{
    return m_options;
}

PrintOptions GtkPrintDialog::options()
{
    return m_options;
}

QVector<PageRanges> GtkPrintDialog::getPageRanges()
{
    return m_page_ranges;
}

int GtkPrintDialog::fromPage()
{
    return m_printer->fromPage();
}

int GtkPrintDialog::toPage()
{
    return m_printer->toPage();
}

void GtkPrintDialog::setFromTo(int from, int to)
{
    m_printer->setFromTo(from, to);
    if (!m_page_ranges.isEmpty())
        m_page_ranges.clear();
    m_page_ranges.append(PageRanges(m_printer->fromPage(), m_printer->toPage()));
}

void GtkPrintDialog::accept()
{

}
