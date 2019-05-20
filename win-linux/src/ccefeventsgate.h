#ifndef CCEFEVENTSGATE_H
#define CCEFEVENTSGATE_H

#include <QObject>
#include "ctabpanel.h"

class CCefEventsGate : public QObject
{
    Q_OBJECT

public:
    explicit CCefEventsGate(QObject *parent = nullptr);

    void init(CTabPanel * const);
    CTabPanel * const panel()
    {
        return m_panel;
    }

protected:
    CTabPanel * m_panel = nullptr;

public slots:
    virtual void onPortalLogout(std::wstring portal);
    virtual void onEditorConfig(int id, std::wstring cfg) = 0;
    virtual void onDocumentName(void *);
    virtual void onDocumentChanged(int id, bool changed);
    virtual void onLocalFileSaveAs(void *) = 0;
    virtual void onDocumentSave(int id, bool cancel = false);
    virtual void onDocumentSaveInnerRequest(int id) = 0;
    virtual void onDocumentFragmented(int id, bool needbuild) = 0;
    virtual void onDocumentFragmentedBuild(int id, int error);
    virtual void onDocumentPrint(void *);
    virtual void onDocumentPrint(int current, uint count) = 0;

    virtual void onEditorAllowedClose(int) = 0;
    virtual void onKeyDown(void *);
};

#endif // CCEFEVENTSGATE_H
