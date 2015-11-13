#ifndef QMAINPANEL_H
#define QMAINPANEL_H

#include <QMouseEvent>
#include <QResizeEvent>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include "qwinwidget.h"

#include "applicationmanager.h"
#include <QHBoxLayout>
#include "asctabwidget.h"
#include "cuserprofilewidget.h"
#include "cdownloadwidget.h"
#include "cpushbutton.h"
#include <QLabel>

#define APP_NAME "DesktopEditors"
#define GET_REGISTRY_USER(variable) \
    QSettings variable(QSettings::NativeFormat, QSettings::UserScope, "ONLYOFFICE", APP_NAME);

struct CPrintData;

class QMainPanel : public QWinWidget, public NSEditorApi::CAscMenuEventListener
{
    Q_OBJECT

public:
    QMainPanel( HWND hWnd, CAscApplicationManager* pManager );

    bool nativeEvent(const QByteArray &, void *msg, long *result);

    void mousePressEvent( QMouseEvent *event );
    void resizeEvent(QResizeEvent* event);
    void loadStartPage();
    void toggleButtonMain(bool);
    void checkModified(byte action);

    void focus();
signals:
    void downloadEvent(NSEditorApi::CAscDownloadFileInfo *);

public slots:
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void pushButtonCloseClicked();
    void pushButtonMainClicked();

    void onTabClicked(int);
    void onTabChanged(int);
    void onTabClosed(int, int);
    void onTabCloseRequest(int);
    void onMenuLogout();
    void onAddEditor(bool, int);
    void onLogin(QString);
    void onLogout();
    void onJSMessage(QString, QString);
    void onDocumentPrint(void *);
    void onDialogSave(std::wstring sName);
    void onFullScreen(bool);
    void onKeyDown(void *);

private:
    HWND windowHandle;

    std::wstring    m_sDownloadName;

    QPushButton*    m_pButtonMain;
    QWidget*        m_pMainWidget;

    QPushButton*    m_pButtonMinimize;
    QPushButton*    m_pButtonMaximize;
    QPushButton*    m_pButtonClose;
    QPushButton*    m_pButtonProfile;
    CPushButton*    m_pButtonDownload;

    QHBoxLayout *   m_layoutBtns;
    QWidget *       m_boxTitleBtns;
    CAscTabWidget * m_pTabs;
    bool            m_isMaximized;
    CPrintData *    m_printData;

    QWidget*        m_pSeparator;
    CUserProfileWidget *    m_pWidgetProfile;
    CDownloadWidget *       m_pWidgetDownload;

    CAscApplicationManager* m_pManager;

public:
    HWND GetHwndForKeyboard()
    {
        return (HWND)((QWidget*)m_pTabs->parent())->winId();
    }

public:
    virtual void OnEvent(NSEditorApi::CAscMenuEvent* pEvent);
    virtual bool IsSupportEvent(int nEventType)
    {
        Q_UNUSED(nEventType);
        return true;
    }

private:
    void RecalculatePlaces();
};

#endif // QMAINPANEL_H
