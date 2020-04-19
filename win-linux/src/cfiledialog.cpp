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

#include "cfiledialog.h"
#include <QFileDialog>
#include "defines.h"
#include "utils.h"
#include "cmessage.h"
#include "cascapplicationmanagerwrapper.h"

#include "../Common/OfficeFileFormats.h"

#include <QList>
#include <QDebug>

#include "qcefview.h"

#if defined(_WIN32)
CFileDialogWrapper::CFileDialogWrapper(HWND hParentWnd) : QWinWidget(hParentWnd)
#else
// because bug in cef - 'open/save dialog' doesn't open for second time
#if !defined(FILEDIALOG_DONT_USE_NATIVEDIALOGS) && !defined(FILEDIALOG_DONT_USE_MODAL)
#define FILEDIALOG_DONT_USE_MODAL
#endif
CFileDialogWrapper::CFileDialogWrapper(QWidget * parent) : QObject(parent)
#endif
{
    m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN]         = tr("All files (*.*)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX]   = tr("DOCX Document (*.docx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOTX]   = tr("Document template (*.dotx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC]    = tr("DOC Document (*.doc)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT]    = tr("ODT Document (*.odt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_OTT]    = tr("OpenDocument Document template (*.ott)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF]    = tr("RTF File (*.rtf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT]    = tr("TXT File (*.txt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML]   = tr("HTML File (*.html)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT]    = tr("MHT File (*.mht)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB]   = tr("EPUB File (*.epub)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX]   = tr("PPTX File (*.pptx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT]    = tr("PPT File (*.ppt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_POTX]   = tr("Presentation template (*.potx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP]    = tr("ODP File (*.odp)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_OTP]    = tr("OpenDocument Presentation Template (*.otp)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX]   = tr("PPSX File (*.ppsx)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX]    = tr("XLSX File (*.xlsx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLTX]    = tr("Spreadsheet template (*.xltx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS]     = tr("XLS File (*.xls)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS]     = tr("ODS File (*.ods)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_OTS]     = tr("OpenDocument Spreadsheet Template (*.ots)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV]     = tr("CSV File (*.csv)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF]   = tr("PDF File (*.pdf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA]  = tr("PDFA File (*.pdf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU]  = tr("DJVU File (*.djvu)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS]   = tr("XPS File (*.xps)");
}

CFileDialogWrapper::~CFileDialogWrapper()
{

}

bool CFileDialogWrapper::modalSaveAs(QString& fileName)
{
//    QString filter = tr("All files (*.*)"), ext_in;
    QString _filters, _sel_filter, _ext;

    QFileInfo info(fileName);
    _ext = info.suffix();

    QRegExp reFilter("([\\w\\s]+\\(\\*\\."+_ext+"+\\))", Qt::CaseInsensitive);
    if ( !m_filters.isEmpty() ) {
        _filters = m_filters;

        if ( !(reFilter.indexIn(m_filters) < 0) ) {
            _sel_filter = reFilter.cap(1);
        } else {
            fileName = info.absolutePath() + QDir::separator() + info.fileName();
        }
    } else {
        _filters = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];

        _sel_filter = getFilter(_ext);
        _filters.append(";;" + _sel_filter);
    }

#ifdef _WIN32
    QString _croped_name = fileName.contains(QRegExp("\\.[^\\/\\\\]+$")) ?
                                    fileName.left(fileName.lastIndexOf(".")) : fileName;

    HWND _mess_parent = QWinWidget::parentWindow();
    CInAppEventModal _event(_mess_parent);
    CRunningEventHelper _h(&_event);
#else
    QString _croped_name = fileName.left(fileName.lastIndexOf("."));
    QWidget * _mess_parent = (QWidget *)parent();
#endif
    reFilter.setPattern("\\(\\*(\\.\\w+)\\)$");

    auto _exec_dialog = [] (QWidget * p, QString n, QString f, QString& sf) {
        return QFileDialog::getSaveFileName(p, tr("Save As"), n, f, &sf,
                                            QFileDialog::DontConfirmOverwrite
#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
                                            | QFileDialog::DontUseNativeDialog
#endif
                                            );
    };

#ifdef FILEDIALOG_DONT_USE_MODAL
    QWidget * _parent = NULL;
#else
# ifdef _WIN32
    QWidget * _parent = this;
# else
    QWidget * _parent = (QWidget *)parent();
# endif
#endif

#ifndef _WIN32
    WindowHelper::CParentDisable oDisabler(qobject_cast<QWidget*>(parent()));
#endif

    while (true) {
        fileName = _exec_dialog(_parent, _croped_name, _filters, _sel_filter);

        if ( !fileName.isEmpty() ) {
            if ( !(reFilter.indexIn(_sel_filter) < 0) ) {
                _ext = reFilter.cap(1);

                if (!fileName.endsWith(_ext))
                    fileName.append(_ext);
            }

            QFileInfo info(fileName);
            if ( info.exists() ) {
                CMessage mess(_mess_parent, CMessageOpts::moButtons::mbYesNo);
                int _answ = mess.warning(tr("%1 already exists.<br>Do you want to replace it?").arg(info.fileName()));
                if ( MODAL_RESULT_CUSTOM + 1 == _answ ) {
                    continue;
                } else
                if ( MODAL_RESULT_CUSTOM + 0 != _answ ) {
                    fileName.clear();
                }
            }
        }

        break;
    }


    m_format = 0;
    if (m_filters.length()) {
        m_format = getKey(_sel_filter);
    }

    return fileName.length() > 0;
}

QString CFileDialogWrapper::getFilter(const QString& extension) const
{
    QString out = extension.toLower();
    if (extension.contains(QRegExp("^docx?$"))) {
        return tr("Word Document") + " (*." + out +")";
    } else
    if (extension.contains(QRegExp("^xlsx?$"))) {
        return tr("Excel Workbook") + " (*." + out + ")";
    } else
    if (extension.contains(QRegExp("^pptx?$"))) {
        return tr("PowerPoint Presentation") + " (*." + out + ")";
    } else {
        out.replace(0, 1, extension.left(1).toUpper());
        return tr("%1 File (*.%2)").arg(out).arg(out.toLower());
    }
}

QStringList CFileDialogWrapper::modalOpen(const QString& path, const QString& filter, QString * selected, bool multi)
{
    QString _filter_ = filter;
    if ( _filter_.isEmpty() ) {
//        _filter_ = joinFilters();
        _filter_ = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN] + ";;" +
                    tr("Text documents") + " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dotx *.dotm *.fodt *.wps *.wpt *.xml);;" +
                    tr("Spreadsheets") + " (*.xlsx *.xls *.ods *.ots *.csv *.xltx *.xltm *.fods *.et *.ett);;" +
                    tr("Presentations") + " (*.pptx *.ppt *.odp *.otp *.ppsm *.ppsx *.potx *.potm *.fodp *.dps *.dpt);;" +
                    tr("Web Page") + " (*.html *.htm *.mht);;" +
                    tr("Text files") + " (*.txt)";
    }

    QString _sel_filter = selected ? *selected : m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
//    QWidget * p = qobject_cast<QWidget *>(parent());

    QWidget * _parent =
#ifdef _WIN32
                        this;
#else
# ifdef FILEDIALOG_DONT_USE_MODAL
                        NULL;
# else
                        (QWidget *)parent();
# endif
#endif
    QFileDialog::Options _opts =
#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
            QFileDialog::DontUseNativeDialog;
#else
            QFileDialog::Options();
#endif

#ifndef _WIN32
    WindowHelper::CParentDisable oDisabler(qobject_cast<QWidget*>(parent()));
#else
    CInAppEventModal event_(QWinWidget::parentWindow());
    CRunningEventHelper h_(&event_);
#endif

    return multi ? QFileDialog::getOpenFileNames(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts) :
            QStringList(QFileDialog::getOpenFileName(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts));
}

QString CFileDialogWrapper::modalOpenSingle(const QString& path, const QString& filter, QString * selected)
{
    QStringList _list = modalOpen(path, filter, selected, false);
    return _list.isEmpty() ? QString() : _list.at(0);
}

QStringList CFileDialogWrapper::modalOpenImage(const QString& path)
{
    QString selected = tr("All Images") + " (*.jpeg *.jpg *.png *.gif *.bmp)";
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.append(";;" + selected + ";;" + tr("Jpeg (*.jpeg *.jpg);;Png (*.png);;Gif (*.gif);;Bmp (*.bmp)"));

    return modalOpen(path, filter, &selected, false);
}

QStringList CFileDialogWrapper::modalOpenImages(const QString& path)
{
    QString selected = tr("All Images") + " (*.jpeg *.jpg *.png *.gif *.bmp)";
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.append(";;" + selected + ";;" + tr("Jpeg (*.jpeg *.jpg);;Png (*.png);;Gif (*.gif);;Bmp (*.bmp)"));

    return modalOpen(path, filter, &selected, true);
}

QStringList CFileDialogWrapper::modalOpenPlugin(const QString& path)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    QString _plugins_filter = tr("Plugin file (*.plugin)");
    _filter.append(";;" + _plugins_filter);

    return modalOpen(path, _filter, &_plugins_filter, false);
}

QStringList CFileDialogWrapper::modalOpenPlugins(const QString& path)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    QString _plugins_filter = tr("Plugin file (*.plugin)");
    _filter.append(";;" + _plugins_filter);

    return modalOpen(path, _filter, &_plugins_filter, true);
}

QStringList CFileDialogWrapper::modalOpenDocuments(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Text documents") + " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dotx *.dotm *.fodt *.wps *.wpt *.xml);;");

    return modalOpen(path, filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenSpreadsheets(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Spreadsheets") + " (*.xlsx *.xls *.ods *.ots *.csv *.xltx *.xltm *.fods *.et *.ett);;");

    return modalOpen(path, filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenPresentations(const QString& path, bool multi)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Presentations") + " (*.pptx *.ppt *.odp *.otp *.ppsm *.ppsx *.potx *.potm *.fodp *.dps *.dpt);;");

    return modalOpen(path, filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenAny(const QString& path, bool multi)
{
    QString _filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    return modalOpen(path, _filter, nullptr, multi);
}

QStringList CFileDialogWrapper::modalOpenMedia(const QString& type, const QString& path, bool multi)
{
    QString selected;
    if ( type == "video" ) {
        selected = tr("Video file") + " (*.webm *.mkv *.flv *.ogg *.avi *.mov *.wmv *.mp4 *.m4v *.mpg *.mp2 *.mpeg *.mpe *.mpv *.m2v *.m4v *.3gp *.3g2 *.f4v *.m2ts *.mts)";
    } else
    if ( type == "audio" ) {
        selected = tr("Audio file") + " (*.flac *.mp3 *.ogg *.wav *.wma *.ape *.aac *.m4a *.alac)";
    }

    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN] + ";;" + selected;
    return modalOpen(path, filter, &selected, multi);
}

void CFileDialogWrapper::setFormats(std::vector<int>& vf)
{
    m_filters.clear();

    if ( vf.size() ) {
        std::vector<int>::iterator i = vf.begin();
        m_filters = m_mapFilters.value(*(i++));
        while (i != vf.end()) {
            m_filters += ";;" + m_mapFilters.value(*(i++));
        }
    }
}

int CFileDialogWrapper::getKey(const QString &value)
{
    foreach (QString v, m_mapFilters) {
        if (v == value)
           return m_mapFilters.key(value);
    }
    return -1;
}

int CFileDialogWrapper::getFormat()
{
    return m_format;
}

QString CFileDialogWrapper::joinFilters() const
{
    auto _get_all_exts = [] (const QList<QString>& l) {
        QRegExp re("[\\w\\s]+\\((\\*\\.\\w+)\\)");
        QString extns;
        for ( auto f : l ) {
            if ( !(re.indexIn(f) < 0) ) {
                if ( !extns.isEmpty() )
                    extns.append(" ");

                extns.append( re.cap(1) );
            }
        }

        return extns;
    };

    QString _out;
    QList<QString> _vl(m_mapFilters.values());
//    _vl.insert(1, tr("All supported documents") + " (" + _get_all_exts(_vl) + ")");
    for ( auto f : _vl ) {
        if ( !_out.isEmpty() )
            _out.append(";;");

        _out.append(f);
    }

    return _out;
}
