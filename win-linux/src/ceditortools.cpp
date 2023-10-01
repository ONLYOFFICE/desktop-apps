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

#include "ceditortools.h"
#include "qascprinter.h"
#include "components/cprintprogress.h"
#include "cascapplicationmanagerwrapper.h"
#include "components/cfiledialog.h"
#include "defines.h"
#include "utils.h"
#include "cfilechecker.h"
#include "OfficeFileFormats.h"
#include "components/cmessage.h"
#include <QApplication>
#include <QDir>

using namespace NSEditorApi;

namespace CEditorTools
{
    void print(const sPrintConf& c)
    {
        if (!c.page_ranges->isEmpty()) {
            if ( c.context->BeginPaint() ) {
                CPrintProgress _progress(c.parent);
                _progress.startProgress();

                CAscPrintPage * pData;
                int curr = 1;
                int count = 0;
                foreach (PageRanges range, *c.page_ranges)
                    count += range.toPage - range.fromPage + 1;

                foreach (PageRanges range, *c.page_ranges) {
                    int start = range.fromPage, finish = range.toPage;
                    start < 1 && (start = 1);
                    finish < 1 && (finish = 1);
                    finish < start && (finish = start);
                    c.context->SetPageOrientation(c.view->GetPrintPageOrientation(start - 1));
                    while (start <= finish) {
                        c.context->AddRef();

                        _progress.setProgress(curr, count);
                        PROCESSEVENTS();

                        pData = new CAscPrintPage();
                        pData->put_Context(c.context);
                        pData->put_Page(start - 1);

                        CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE);
                        pEvent->m_pData = pData;

                        c.view->Apply(pEvent);
//                        RELEASEOBJECT(pData)
//                        RELEASEOBJECT(pEvent)

                        if ( _progress.isRejected() )
                            break;
                        if (curr < count) {
                            c.context->SetPageOrientation(c.view->GetPrintPageOrientation(start));
                            c.context->getPrinter()->newPage();
                        }
                        curr++;
                        start++;
                    }
                }
                c.context->EndPaint();
            }
        } else {
            // TODO: show error message
        }
    }

    void getlocalfile(void * e)
    {
        CAscCefMenuEvent * event = static_cast<CAscCefMenuEvent *>(e);
        ParentHandle parent = AscAppManager::windowHandleFromId(event->get_SenderId());
        if ( parent ) {
            CFileDialogWrapper dialog(parent);

            CAscLocalOpenFileDialog * pData = static_cast<CAscLocalOpenFileDialog *>(event->m_pData);
            const QString _filter = QString::fromStdWString(pData->get_Filter());
            QStringList _list;

            if ( _filter == "plugin" ) {
                _list = pData->get_IsMultiselect() ? dialog.modalOpenPlugins(Utils::lastPath(LOCAL_PATH_OPEN)) :
                                                     dialog.modalOpenPlugin(Utils::lastPath(LOCAL_PATH_OPEN));
            } else
            if ( _filter == "image" || _filter == "images" ) {
                _list = pData->get_IsMultiselect() ? dialog.modalOpenImages(Utils::lastPath(LOCAL_PATH_OPEN)) :
                                                        dialog.modalOpenImage(Utils::lastPath(LOCAL_PATH_OPEN));
            } else
            if ( _filter == "word" ) {
                _list = dialog.modalOpenDocuments(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else
            if ( _filter == "cell" ) {
                _list = dialog.modalOpenSpreadsheets(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else
            if ( _filter == "slide" ) {
                _list = dialog.modalOpenPresentations(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else
            if ( _filter == "video" || _filter == "audio" ) {
                _list = dialog.modalOpenMedia(_filter, Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else
            if ( _filter == "csv/txt" ) {
                QString _sel_filter;
                const QString _txt_filter = CFileDialogWrapper::tr("All supported files") + " (*.txt *.csv);;" + QObject::tr("All files (*.*)");

                _list = dialog.modalOpen(Utils::lastPath(LOCAL_PATH_OPEN), _txt_filter, &_sel_filter, pData->get_IsMultiselect());
            } else
            if ( _filter == "cryptofiles" ) {
                _list = dialog.modalOpenForEncrypt(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else
            if ( _filter == "(*xml)" ) {
                QString _sel_filter;
                const QString _txt_filter = QObject::tr("XML File (*.xml)") + ";;" + QObject::tr("All files (*.*)");

                _list = dialog.modalOpen(Utils::lastPath(LOCAL_PATH_OPEN), _txt_filter, &_sel_filter, pData->get_IsMultiselect());
            } else
            if ( _filter == "any" || _filter == "*.*" ) {
                _list = dialog.modalOpenAny(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else {
                QString _sel_filter, _file_filter{_filter};
                if ( !_filter.isEmpty() ) {
                    QRegularExpression re("^\\*\\.[\\w]");
                    QRegularExpressionMatch match = re.match(_filter);
                    if ( match.hasMatch() ) {
                         _file_filter = "(" + _filter + ")";
                    }
                }


                _list = dialog.modalOpen(Utils::lastPath(LOCAL_PATH_OPEN), _file_filter, &_sel_filter, pData->get_IsMultiselect());
            }


            if ( !_list.isEmpty() ) {
                Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_list.at(0)).absolutePath());
            }

            /* data consits id of cefview */
            pData->put_IsMultiselect(true);
            std::vector<std::wstring>& _files = pData->get_Files();
            for ( const auto& f : _list ) {
                _files.push_back(f.toStdWString());
            }

            event->AddRef();
            AscAppManager::getInstance().Apply(event);
        }
    }

    QString getlocalfile(const std::wstring& path, int parentid)
    {
        ParentHandle parent;
        if ( !(parentid < 0) )
            parent = AscAppManager::windowHandleFromId(parentid);
        else parent = qApp->activeWindow();

        CFileDialogWrapper dlg(parent);

        QString _path = QString::fromStdWString(path);
        if ( _path.isEmpty() || !QDir(_path).exists() )
            _path = Utils::lastPath(LOCAL_PATH_OPEN);

        if (!(_path = dlg.modalOpenSingle(_path)).isEmpty()) {
            Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());
        }

        return _path;
    }

    QString getlocaltemplate(const std::wstring& editor, int parentid)
    {
        ParentHandle parent;
        if ( !(parentid < 0) )
            parent = AscAppManager::windowHandleFromId(parentid);
        else parent = qApp->activeWindow();

        QString _filter;
        if ( editor.compare(L"cell") == 0 ) {
            _filter = CFileDialogWrapper::tr("Spreadsheet template") + " (*.xltx *.xltm *.ots)";
        } else
        if ( editor.compare(L"slide") == 0 ) {
            _filter = CFileDialogWrapper::tr("Presentation template") + " (*.potx *.otp)";
        } else {
            _filter = CFileDialogWrapper::tr("Document template") + " (*.dotx *.ott)";
        }

        CFileDialogWrapper dlg(parent);

        QString _path = Utils::lastPath(LOCAL_PATH_OPEN);
        if ( !(_path = dlg.modalOpenSingle(_path, _filter)).isEmpty() ) {
            Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());
        }

        return _path;
    }

    std::wstring getFolder(const std::wstring& path, int parentid)
    {
        ParentHandle parent;
        if ( !(parentid < 0) )
            parent = AscAppManager::windowHandleFromId(parentid);
        else parent = qApp->activeWindow();

        QString sel_path = path.empty() ? QString::fromStdWString(path) : Utils::lastPath(LOCAL_PATH_OPEN);

        CFileDialogWrapper dlg(parent);
        return dlg.selectFolder(sel_path).toStdWString();
    }

    auto createEditorPanel(const COpenOptions& opts, const QRect& rect) -> CTabPanel *
    {
        int _file_format{0};
        if ( opts.srctype == etLocalFile ) {
            _file_format = CCefViewEditor::GetFileFormat(opts.wurl);
            if ( _file_format == 0 )
                return nullptr;
        } else
        if (opts.srctype == etRecentFile) {
            if (CFileInspector::isLocalFile(QString::fromStdWString(opts.wurl))) {
                QFileInfo info(opts.url);
                if (!info.isReadable()) {
                    CMessage::error(AscAppManager::getInstance().mainWindow(), QObject::tr("Access to file '%1' is denied!").arg(opts.url));
                    return nullptr;
                }
            }
        }

        CTabPanel * panel = CTabPanel::createEditorPanel();

        bool result = true;
        if (opts.srctype == etLocalFile) {
            std::wstring params{InputArgs::webapps_params()};
            if ( opts.mode == COpenOptions::eOpenMode::review ) {
                params.append(L"&mode=review");
            } else
            if ( opts.mode == COpenOptions::eOpenMode::view ) {
                params.append(L"&mode=view");
            }

            panel->openLocalFile(opts.wurl, _file_format, params);
        } else
        if (opts.srctype == etRecoveryFile) {
            result = panel->openRecoverFile(opts.id);
        } else
        if (opts.srctype == etRecentFile) {
            result = panel->openRecentFile(opts.id);
        } else
        if (opts.srctype == etNewFile) {
            panel->createLocalFile(editorTypeFromFormat(opts.format), opts.name.toStdWString());
        } else
        if (opts.srctype == etTemplateFile) {
            panel->createLocalFile(opts.wurl, opts.name.toStdWString());
        } else {
            panel->cef()->load(opts.wurl);
        }

        if ( result ) {
            CAscTabData * data = new CAscTabData(opts.name);
            data->setUrl(opts.wurl);
            data->setIsLocal( opts.srctype == etLocalFile || opts.srctype == etNewFile || opts.srctype == etTemplateFile ||
                           (opts.srctype == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

            if ( opts.srctype == etNewFile )
                data->setContentType(editorTypeFromFormat(opts.format));



            panel->setData(data);
            //if ( !rect.isEmpty() )
                //panel->setGeometry(rect);
        } else {
            AscAppManager::getInstance().DestroyCefView(panel->cef()->GetId());
            panel->hide();
            panel->deleteLater();
            panel = nullptr;
        }

        return panel;
    }

    auto editorTypeFromFormat(int format) -> AscEditorType {
        if ( format == AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF ) {
            return AscEditorType::etDocumentMasterForm;
        } else
        if ( (format > AVS_OFFICESTUDIO_FILE_DOCUMENT && format < AVS_OFFICESTUDIO_FILE_PRESENTATION) ||
                format == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDF || format == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_PDFA ||
                    format == AVS_OFFICESTUDIO_FILE_CROSSPLATFORM_DJVU )
            return AscEditorType::etDocument;
        else
        if ( format > AVS_OFFICESTUDIO_FILE_PRESENTATION && format < AVS_OFFICESTUDIO_FILE_SPREADSHEET )
            return AscEditorType::etPresentation;
        else
        if (format > AVS_OFFICESTUDIO_FILE_SPREADSHEET && format < AVS_OFFICESTUDIO_FILE_CROSSPLATFORM ) {
            return AscEditorType::etSpreadsheet;
        }
        else
        if (format > AVS_OFFICESTUDIO_FILE_CROSSPLATFORM && format < AVS_OFFICESTUDIO_FILE_IMAGE )
            return AscEditorType::etPdf;

        return AscEditorType::etUndefined;
    }

    auto processLocalFileSaveAs(const CAscCefMenuEvent * event) -> void {
        CAscLocalSaveFileDialog * pData = static_cast<CAscLocalSaveFileDialog *>(event->m_pData);

        QFileInfo info(QString::fromStdWString(pData->get_Path()));
        if ( !info.fileName().isEmpty() ) {
            bool _keep_path = false;
            QString _full_path;
            if ( info.exists() )
                _full_path = info.absoluteFilePath();
            else _full_path = Utils::lastPath(LOCAL_PATH_SAVE) + "/" + info.fileName(),
                    _keep_path = true;

            ParentHandle _parent = AscAppManager::windowHandleFromId(event->m_nSenderId);
            CFileDialogWrapper dlg(_parent);
            dlg.setFormats(pData->get_SupportFormats());

            CAscLocalSaveFileDialog * pSaveData = new CAscLocalSaveFileDialog();
            pSaveData->put_Id(pData->get_Id());
            pSaveData->put_Path(L"");

            if ( dlg.modalSaveAs(_full_path) ) {
                if ( _keep_path )
                    Utils::keepLastPath(LOCAL_PATH_SAVE, QFileInfo(_full_path).absoluteDir().absolutePath());

                bool _allowed = true;
                switch ( dlg.getFormat() ) {
                case AVS_OFFICESTUDIO_FILE_DOCUMENT_TXT:
                case AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV: {
                    int res = CMessage::showMessage(_parent,
                                                    QCoreApplication::translate("CEditorWindow", "Some data will lost.<br>Continue?"),
                                                    MsgType::MSG_WARN, MsgBtns::mbOkDefCancel);
                    _allowed = (MODAL_RESULT_OK == res);
                    break; }
                default: break;
                }

                if ( _allowed ) {
                    pSaveData->put_Path(_full_path.toStdWString());
                    int format = dlg.getFormat() > 0 ? dlg.getFormat() :
                            AscAppManager::GetFileFormatByExtentionForSave(pSaveData->get_Path());

                    pSaveData->put_FileType(format > -1 ? format : 0);
                }
            }

            CAscMenuEvent* pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
            pEvent->m_pData = pSaveData;

            AscAppManager::getInstance().Apply(pEvent);

    //        RELEASEINTERFACE(pData)
    //        RELEASEINTERFACE(pEvent)
        }
    }
}
