
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

void CCefEventsGate::onPortalLogout(wstring portal)
{
    if ( m_panel && !portal.empty() ) {
        if ( m_panel->data()->closed() &&
                m_panel->data()->url().find(portal) != wstring::npos )
        {
            /*
             * checck if changed,
             * close window after save
            */
        }
    }
}

void CCefEventsGate::onDocumentChanged(int, bool changed)
{
    CAscTabData * doc = m_panel->data();

    /* TODO: if exists the saving error, sdk rise the changing event
     * again. maybe not good action.
    */
    if (changed && doc->closed()) doc->reuse();
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
    if ( doc->local() ) {
        if ( pData->get_Url().empty() ) {
            doc->setUrl(Utils::replaceBackslash(QString::fromStdWString(pData->get_Path())));
        }
    }

    RELEASEINTERFACE(pData);
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

void CCefEventsGate::onKeyDown(void *)
{

}

void CCefEventsGate::onDocumentFragmentedBuild(int, int error)
{
    if ( error && m_panel->data()->closed() ) {
        m_panel->data()->reuse();
    }
}
