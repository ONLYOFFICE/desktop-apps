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

#include "components/cfiledialog.h"
#include <QFileDialog>
#include "defines.h"
#include "utils.h"
#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"

#include "../Common/OfficeFileFormats.h"

#include <QList>
#include <QDebug>

#include "qcefview.h"

#ifdef Q_OS_WIN
# include <shobjidl.h>
#endif
#include <string>

using namespace std;

#ifdef Q_OS_WIN
static DWORD win_ver_major{0};
//static const int win_ver_major = QOperatingSystemVersion::current().majorVersion();
using VectorShellItems = vector<IShellItem *>;

auto itemsFromItemArray(IShellItemArray * items)
{
    VectorShellItems out;
    DWORD itemCount = 0;
    if (FAILED(items->GetCount(&itemCount)) || itemCount == 0)
        return out;

    out.reserve(itemCount);
    for (DWORD i = 0; i < itemCount; ++i) {
        IShellItem * item = nullptr;
        if (SUCCEEDED(items->GetItemAt(i, &item)))
            out.push_back(item);
    }

    return out;
}
#endif

class CFileDialogHelper {
    using specvector = vector<pair<wstring,wstring>>;
public:
    struct CFileDialogOpenArguments {
        CFileDialogOpenArguments(ParentHandle h, const wstring& t)
            : parent(h)
            , title(t)
        {}

        ParentHandle parent = nullptr;
        wstring title;
        wstring filter,
                startFilter;
        wstring folder;
        bool multiSelect = false;
    };

public:
    static auto stringToFilters(const wstring& wstr) -> specvector {
        specvector v;

        if ( !wstr.empty() ) {
            auto _parce_filter_string = [](const wstring& fullstr, size_t start, size_t stop) -> pair<wstring, wstring> {
                wstring filter_name = fullstr.substr(start, stop - start);
                size_t mid = filter_name.find(L"(") + 1;
                wstring filter_pattern = filter_name.substr(mid, filter_name.find(L")", mid) - mid);

                std::replace(begin(filter_pattern), end(filter_pattern), ' ', ';');
                return make_pair(filter_name, filter_pattern);
            };

            wstring _delim = L";;";
            size_t _curr = wstr.find(_delim),
                _prev = 0;
            while ( _curr != wstring::npos ) {
                v.push_back(_parce_filter_string(wstr,_prev,_curr));

                _prev = _curr + 2;
                _curr = wstr.find(_delim, _prev);
            }

            v.push_back(_parce_filter_string(wstr, _prev, wstr.size()));
        }

        return v;
    }

    static auto nativeOpenDialog(const CFileDialogOpenArguments& args) -> QStringList {
        QStringList out;

#if defined(Q_OS_WIN) && !defined(__OS_WIN_XP)
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if ( SUCCEEDED(hr) ) {
            IFileOpenDialog * pDialog = nullptr;

            // Create the FileOpenDialog object.
            hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, reinterpret_cast<void**>(&pDialog));

            if (SUCCEEDED(hr)) {
                hr = pDialog->SetTitle(args.title.c_str());

                specvector filters{stringToFilters(args.filter)};

                uint typeIndex = 1;
                COMDLG_FILTERSPEC * specOpenTypes = new COMDLG_FILTERSPEC[filters.size()];
                for (uint i{0}; i < filters.size(); ++i) {
                    specvector::const_reference iter = filters.at(i);
                    specOpenTypes[i] = {iter.first.c_str(), iter.second.c_str()};

                    if ( !args.startFilter.empty() &&
                            iter.first.find(args.startFilter) == 0 )
                        typeIndex = i + 1;
                }

                hr = pDialog->SetFileTypes(filters.size(), specOpenTypes);
                delete [] specOpenTypes;

                pDialog->SetFileTypeIndex(typeIndex);

                if ( !args.folder.empty() ) {
                    IShellItem * pItem = nullptr;
                    hr = SHCreateItemFromParsingName(args.folder.c_str(), nullptr, IID_PPV_ARGS(&pItem));

                    if (SUCCEEDED(hr)) {
                        pDialog->SetFolder(pItem);
                        pItem->Release();
                    }
                }

                DWORD dwFlags;
                hr = pDialog->GetOptions(&dwFlags);
                if (SUCCEEDED(hr)) {
                    if ( args.multiSelect ) dwFlags |= FOS_ALLOWMULTISELECT;

                    QSettings _r("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", QSettings::NativeFormat);
                    if ( _r.value("Hidden", 1) == 1 )
                        dwFlags |= FOS_FORCESHOWHIDDEN;

                    hr = pDialog->SetOptions(dwFlags | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);
                }

                hr = pDialog->Show((HWND)args.parent->winId());
                if (SUCCEEDED(hr)) {
                    IShellItemArray * items = nullptr;
                    hr = pDialog->GetResults(&items);
                    if (SUCCEEDED(hr) && items) {
                        VectorShellItems iarray = itemsFromItemArray(items);
                        for (IShellItem * item : iarray) {
                            PWSTR pszFilePath;
                            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            if (SUCCEEDED(hr)) {
                                out.append(QString::fromStdWString(pszFilePath));
                                CoTaskMemFree(pszFilePath);
                            }
                        }

                        for (IShellItem * item : iarray)
                            item->Release();
                        items->Release();
                    }
                }
                pDialog->Release();
            }
            CoUninitialize();
        }
#else
#endif

        return out;
    }
};

/*#if defined(_WIN32)
CFileDialogWrapper::CFileDialogWrapper(HWND hParentWnd) : QWinWidget(hParentWnd)
#else*/
// because bug in cef - 'open/save dialog' doesn't open for second time
#if !defined(FILEDIALOG_DONT_USE_NATIVEDIALOGS) && !defined(FILEDIALOG_DONT_USE_MODAL)
#define FILEDIALOG_DONT_USE_MODAL
#endif
CFileDialogWrapper::CFileDialogWrapper(QWidget * parent) : QObject(parent)
//#endif
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
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_FB2]    = tr("FB2 File (*.fb2)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_MOBI]   = tr("MOBI File (*.mobi)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM]  = tr("OFORM Document (*.oform)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF]  = tr("DOCXF Document (*.docxf)");

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

    m_mapFilters[AVS_OFFICESTUDIO_FILE_IMAGE_JPG]           = tr("JPG Image (*.jpg *.jpeg)");
    m_mapFilters[AVS_OFFICESTUDIO_FILE_IMAGE_PNG]           = tr("PNG Image (*.png)");

#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
    // Set native dialog from command line arguments
    m_useNativeDialogFlag = InputArgs::contains(L"--native-file-dialog");
#endif
}

CFileDialogWrapper::~CFileDialogWrapper()
{

}

bool CFileDialogWrapper::modalSaveAs(QString& fileName, int selected)
{
//    QString filter = tr("All files (*.*)"), ext_in;
    QString _filters, _sel_filter, _ext;

    if ( !(selected < 0) && m_mapFilters.contains(selected) )
        _sel_filter = m_mapFilters[selected];

    QFileInfo info(fileName);
    _ext = info.suffix();

    QRegExp reFilter("([\\w\\s]+\\(\\*\\."+_ext+"+\\))", Qt::CaseInsensitive);
    if ( !m_filters.isEmpty() ) {
        _filters = m_filters;

        if ( !(reFilter.indexIn(m_filters) < 0) ) {
            if ( _sel_filter.isEmpty() )
                _sel_filter = reFilter.cap(1);
        } else {
            fileName = info.absolutePath() + QDir::separator() + info.fileName();
        }
    } else {
        _filters = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];

        if ( _sel_filter.isEmpty() )
            _sel_filter = getFilter(_ext);
        _filters.append(";;" + _sel_filter);
    }

#ifdef _WIN32
    QString _croped_name = fileName.contains(QRegExp("\\.[^\\/\\\\]+$")) ?
                                    fileName.left(fileName.lastIndexOf(".")) : fileName;

    QWidget * _mess_parent = (QWidget *)parent();
    /*HWND _mess_parent = QWinWidget::parentWindow();*/
    /*CInAppEventModal _event(_mess_parent->winId());
    CRunningEventHelper _h(&_event);*/
#else
    QString _croped_name = fileName.left(fileName.lastIndexOf("."));
    QWidget * _mess_parent = (QWidget *)parent();
#endif
    reFilter.setPattern("\\(\\*(\\.\\w+)\\)$");

    auto _exec_dialog = [=] (QWidget * p, QString n, QString f, QString& sf) {
        QFileDialog::Options _opts{QFileDialog::DontConfirmOverwrite};
#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
        if ( !m_useNativeDialogFlag )
            _opts |= QFileDialog::DontUseNativeDialog;
#else
#endif
        return QFileDialog::getSaveFileName(p, tr("Save As"), n, f, &sf, _opts);
    };

#ifdef FILEDIALOG_DONT_USE_MODAL
    QWidget * _parent = NULL;
#else
# ifdef _WIN32
    QWidget * _parent = (QWidget *)parent();
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
                    tr("Text documents") + " (*.docx *.doc *.odt *.ott *.rtf *.docm *.dotx *.dotm *.fodt *.wps *.wpt *.xml *.pdf *.epub *.djv *.djvu *.docxf *.oform);;" +
                    tr("Spreadsheets") + " (*.xlsx *.xls *.ods *.ots *.csv *.xltx *.xltm *.fods *.et *.ett);;" +
                    tr("Presentations") + " (*.pptx *.ppt *.odp *.otp *.ppsm *.ppsx *.potx *.potm *.fodp *.dps *.dpt);;" +
                    tr("Web Page") + " (*.html *.htm *.mht);;" +
                    tr("Text files") + " (*.txt)";
    }

    QString _sel_filter = selected ? *selected : m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN];
//    QWidget * p = qobject_cast<QWidget *>(parent());

    QWidget * _parent =
#ifdef _WIN32
                        (QWidget *)parent();
#else
# ifdef FILEDIALOG_DONT_USE_MODAL
                        NULL;
# else
                        (QWidget *)parent();
# endif
#endif
    QFileDialog::Options _opts;
#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
    if ( !m_useNativeDialogFlag )
        _opts = QFileDialog::DontUseNativeDialog;
#else
    _opts = QFileDialog::Options();
#endif

#ifndef _WIN32
    WindowHelper::CParentDisable oDisabler(qobject_cast<QWidget*>(parent()));

    return multi ? QFileDialog::getOpenFileNames(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts) :
                QStringList(QFileDialog::getOpenFileName(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts));
#else
    /*CInAppEventModal event_(qobject_cast<QWidget*>(parent())->winId());
    CRunningEventHelper h_(&event_);*/

    CFileDialogHelper::CFileDialogOpenArguments args{_parent,tr("Open Document").toStdWString()};
    args.filter = _filter_.toStdWString();
    args.startFilter = _sel_filter.toStdWString();
    args.multiSelect = multi;
    args.folder = path.toStdWString();

    if ( win_ver_major == 0 ) {
        OSVERSIONINFO osvi;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&osvi);
        win_ver_major = osvi.dwMajorVersion;
    }

    // Win XP doesn't support IFileOpenDialog
    if ( win_ver_major > 5 ) {
        return CFileDialogHelper::nativeOpenDialog(args);
    } else {
        return multi ? QFileDialog::getOpenFileNames(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts) :
                    QStringList(QFileDialog::getOpenFileName(_parent, tr("Open Document"), path, _filter_, &_sel_filter, _opts));
    }
#endif
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
    QString selected, extra;
    if ( type == "video" ) {
        selected = tr("Video file") + " (*.mp4 *.mkv *.avi *.mpg *.mpeg *.mpe *.mpv *.mov *.wmv *.m2v *.m4v *.webm *.ogg *.f4v *.m2ts *.mts)";
        extra = "Avi (*.avi);;Mpeg (*.mpg *.mpeg *.mpe *.mpv *.m2v *.m4v *.mp4);;Mkv (*.mkv);;Mts (*.m2ts *.mts);;Webm (*.webm);;Mov (*.mov)"
                                      ";;Wmv (*.wmv);;F4v (*.f4v);;Ogg (*.ogg)";
    } else
    if ( type == "audio" ) {
        selected = tr("Audio file") + " (*.mp3 *.mp2 *.ogg *.wav *.wma *.flac *.ape *.aac *.m4a)";
        extra = "Mp3 (*.mp3);;Mp2 (*.mp2);;Wav (*.wav);;Flac (*.flac);;Wma (*.wma);;Ogg (*.ogg);;Ape (*.ape);;Aac (*.aac);;M4a (*.m4a)";
    }

    QString filter = m_mapFilters[AVS_OFFICESTUDIO_FILE_UNKNOWN] + ";;" + selected + ";;" + extra;
    return modalOpen(path, filter, &selected, multi);
}

QString CFileDialogWrapper::selectFolder(const QString& folder)
{
    QWidget * _parent =
#ifdef _WIN32
                        (QWidget *)parent();
#else
# ifdef FILEDIALOG_DONT_USE_MODAL
                        NULL;
# else
                        (QWidget *)parent();
# endif
#endif

    return QFileDialog::getExistingDirectory(_parent, "", folder);
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
