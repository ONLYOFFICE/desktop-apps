
#include "ccefeventsgate.h"
#include "utils.h"
#include "common/Types.h"

using namespace NSEditorApi;

CCefEventsGate::CCefEventsGate(QObject *parent)
    : QObject(parent)
{
}

void CCefEventsGate::init(CTabPanel * const p)
{
    m_panel = p;
}

void CCefEventsGate::onDocumentChanged(int, bool changed)
{
    CAscTabData * doc = m_panel->data();

    /* TODO: if exists the saving error, sdk rise the changing event
     * again. maybe not a good action.
    */
    if (doc->closed() && changed) doc->reuse();
    /**/

    if (doc->hasChanges() != changed && (!doc->closed() || changed)) {
        doc->setChanged(changed);
    }
}

void CCefEventsGate::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);

    CAscTabData * doc = m_panel->data();
    doc->setTitle(QString::fromStdWString(pData->get_Name()));
    if ( doc->isLocal() ) {
        if ( pData->get_Url().empty() && !pData->get_Path().empty() ) {
            doc->setUrl(Utils::replaceBackslash(QString::fromStdWString(pData->get_Path())));
        }
    }

    RELEASEINTERFACE(pData);
}

void CCefEventsGate::onDocumentType(int id, int type)
{
    if (id) { type; }
}

void CCefEventsGate::onDocumentSave(int, bool cancel)
{
    CAscTabData * doc = m_panel->data();
    if ( doc->closed() && cancel ) {
        doc->reuse();
    }
}

void CCefEventsGate::onDocumentPrint(void * data)
{
    CAscPrintEnd * pData = reinterpret_cast<CAscPrintEnd *>(data);

    onDocumentPrint(pData->get_CurrentPage(), pData->get_PagesCount());

    RELEASEINTERFACE(pData);
}

void CCefEventsGate::onDocumentFragmentedBuild(int, int error)
{
    if ( error && m_panel->data()->closed() ) {
        m_panel->data()->reuse();
    }
}

void CCefEventsGate::onKeyDown(void *)
{

}

void CCefEventsGate::onDocumentLoadFinished(int uid)
{
}

void CCefEventsGate::onDocumentReady(int)
{
}
