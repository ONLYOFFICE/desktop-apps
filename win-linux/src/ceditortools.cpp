#include "ceditortools.h"
#include "qascprinter.h"
#include "cprintprogress.h"
#include "cascapplicationmanagerwrapper.h"
#include "applicationmanager_events.h"

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
}
