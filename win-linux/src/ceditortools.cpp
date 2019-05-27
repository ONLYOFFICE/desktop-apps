#include "ceditortools.h"
#include "qascprinter.h"
#include "cprintprogress.h"
#include "cascapplicationmanagerwrapper.h"
#include "applicationmanager_events.h"
#include "cfiledialog.h"
#include "defines.h"
#include "utils.h"

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
}
