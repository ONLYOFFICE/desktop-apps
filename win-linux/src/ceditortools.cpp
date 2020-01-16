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
#include "cprintprogress.h"
#include "cascapplicationmanagerwrapper.h"
#include "applicationmanager_events.h"
#include "cfiledialog.h"
#include "defines.h"
#include "utils.h"

#include <QDir>
#include <QDebug>

using namespace NSEditorApi;

namespace CEditorTools
{
    void print(const sPrintConf& c)
    {
        if (!(c.pagetstart < 0) || !(c.pagestop < 0)) {
            int _start = c.pagetstart < 1 ? 1 : c.pagetstart;
            int _stop = c.pagestop < 1 ? 1 : c.pagestop;
            _stop < _start && (_stop = _start);

            if ( c.context->BeginPaint() ) {
                CPrintProgress _progress(c.parent);
                _progress.startProgress();

                CAscPrintPage * pData;
                uint _count = _stop - _start;
                for (; !(_start > _stop); ++_start) {
                    c.context->AddRef();

                    _progress.setProgress(_count - (_stop - _start) + 1, _count + 1);
                    qApp->processEvents();

                    pData = new CAscPrintPage();
                    pData->put_Context(c.context);
                    pData->put_Page(_start - 1);

                    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE);
                    pEvent->m_pData = pData;

                   c.view->Apply(pEvent);
//                        RELEASEOBJECT(pData)
//                        RELEASEOBJECT(pEvent)

                    if ( _progress.isRejected() )
                        break;

                    _start < _stop && c.context->getPrinter()->newPage();
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
            QString _filter = QString::fromStdWString(pData->get_Filter());
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
            if ( _filter == "any" || _filter == "*.*" ) {
                _list = dialog.modalOpenAny(Utils::lastPath(LOCAL_PATH_OPEN), pData->get_IsMultiselect());
            } else {
                QString _sel_filter;
                _list = dialog.modalOpen(Utils::lastPath(LOCAL_PATH_OPEN), _filter, &_sel_filter, pData->get_IsMultiselect());
            }


            if ( !_list.isEmpty() ) {
                Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_list.at(0)).absolutePath());
            }

            /* data consits id of cefview */
            pData->put_IsMultiselect(true);
            vector<wstring>& _files = pData->get_Files();
            for ( const auto& f : _list ) {
                _files.push_back(f.toStdWString());
            }

            event->AddRef();
            AscAppManager::getInstance().Apply(event);
        }
    }

    QString getlocalfile(const wstring& path, int parentid)
    {
        ParentHandle parent;
        if ( !(parentid < 0) )
            parent = AscAppManager::windowHandleFromId(parentid);
        else parent = AscAppManager::topWindow()->handle();

        CFileDialogWrapper dlg(parent);

        QString _path = QString::fromStdWString(path);
        if ( _path.isEmpty() || !QDir(_path).exists() )
            _path = Utils::lastPath(LOCAL_PATH_OPEN);

        if (!(_path = dlg.modalOpenSingle(_path)).isEmpty()) {
            Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());
        }

        return _path;
    }
}
