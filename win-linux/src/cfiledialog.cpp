/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include "../common/libs/OfficeFileFormats.h"

#include <QDebug>

#if defined(_WIN32)
CFileDialogWrapper::CFileDialogWrapper(HWND hParentWnd) : QWinWidget(hParentWnd)
#else
CFileDialogWrapper::CFileDialogWrapper(QWidget * parent) : QObject(parent)
#endif
{
    m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN]         = tr("All files (*.*)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX]   = tr("DOCX Document (*.docx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOC]    = tr("DOC Document (*.doc)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_ODT]    = tr("ODT Document (*.odt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_RTF]    = tr("RTF File (*.rtf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT]    = tr("TXT File (*.txt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_HTML]   = tr("HTML File (*.html)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MHT]    = tr("MHT File (*.mht)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_EPUB]   = tr("EPUB File (*.epub)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX]   = tr("PPTX File (*.pptx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPT]    = tr("PPT File (*.ppt)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP]    = tr("ODP File (*.odp)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_PRESENTATION_PPSX]   = tr("PPSX File (*.ppsx)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX]    = tr("XLSX File (*.xlsx)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLS]     = tr("XLS File (*.xls)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_ODS]     = tr("ODS File (*.ods)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV]     = tr("CSV File (*.csv)");

    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF]   = tr("PDF File (*.pdf)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU]  = tr("DJVU File (*.djvu)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_XPS]   = tr("XPS File (*.xps)");

}

CFileDialogWrapper::~CFileDialogWrapper()
{

}

bool CFileDialogWrapper::modalSaveAs(QString& fileName)
{
//    QString filter = tr("All files (*.*)"), ext_in;
    QString _filters, _sel_filter, ext_in;

    QRegExp re(reFileExtension);
    if (!(re.indexIn(fileName) < 0)) {
        ext_in = re.cap(1);
    }

    if (m_filters.length() > 0) {
        _filters = m_filters;

        QRegExp reFilter("([\\w\\s]+\\(\\*\\."+ext_in+"+\\))", Qt::CaseInsensitive);
        if ( !(reFilter.indexIn(m_filters) < 0) ) {
            _sel_filter = reFilter.cap(1);
        }
    } else {
        _filters = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN], ext_in;

        _sel_filter = getFilter(ext_in);
        _filters.append(";;" + _sel_filter);
    }

//    QWidget * p = qobject_cast<QWidget *>(parent());
//    fileName = QFileDialog::getSaveFileName(p, tr("Save As"), fileName, filter);

#ifdef _WIN32
    fileName = QFileDialog::getSaveFileName(this, tr("Save As"), fileName, _filters, &_sel_filter);
#else
    fileName = QFileDialog::getSaveFileName((QWidget *)parent(), tr("Save As"), fileName, _filters, &_sel_filter);
#endif

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

QString CFileDialogWrapper::modalOpen(const QString& path, const QString& filter)
{
    QString _filter_ = filter.length() ? filter : tr("All files (*.*)");
//    QWidget * p = qobject_cast<QWidget *>(parent());

#ifdef _WIN32
    return QFileDialog::getOpenFileName(this, tr("Open Document"), path, _filter_);
#else
    return QFileDialog::getOpenFileName((QWidget *)parent(), tr("Open Document"), path, _filter_);
#endif
}

QString CFileDialogWrapper::modalOpenImage(const QString& path)
{
    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
    filter.prepend(tr("Png (*.png);;Gif (*.gif);;Bmp (*.bmp);;"));

    return modalOpen(path, filter);
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
