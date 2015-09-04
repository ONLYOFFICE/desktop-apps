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

#include <windows.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTimer>

#include "qmainpanel.h"
#include "csavefilemessage.h"
#include "defines.h"
#include <QMenu>

#include <QJsonDocument>
#include <QJsonObject>

#include <QDir>
#include <QDialog>
#include <QWindow>
#include <QWidgetAction>
#include <QDesktopServices>
#include <QFileInfo>

#include "qascprinter.h"
#include "cprintdialog.h"
#include "cprintprogress.h"
#include "cfiledialog.h"

#define APP_TITLE "ONLYOFFICE"

//#include <QScreen>
#include <QSettings>
#include "shlobj.h"

#include <QPrinterInfo>

extern byte g_dpi_ratio;
extern QString g_lang;

struct CPrintData {
public:
    CPrintData() : _print_range(QPrintDialog::PrintRange::AllPages) {}
    QPrinterInfo _printer_info;
    QPrintDialog::PrintRange _print_range;
};


QMainPanel::QMainPanel( HWND hWnd, CAscApplicationManager* pManager )
    : QWinWidget( hWnd ),
      m_pButtonMinimize(new QPushButton), m_pButtonMaximize(new QPushButton),
      m_pButtonClose(new QPushButton), m_pButtonProfile(new QPushButton),
      m_pButtonDownload(new CPushButton),
      m_pWidgetProfile(new CUserProfileWidget), m_pWidgetDownload(new CDownloadWidget),
      m_isMaximized(false), m_printData(new CPrintData)
{        
    windowHandle = hWnd;
    m_pManager = pManager;

    setObjectName("mainPanel");

    QGridLayout *mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing( 0 );
    mainGridLayout->setMargin( 0 );
    setLayout( mainGridLayout );

    // Central widget
    QWidget *centralWidget = new QWidget( this );
    centralWidget->setObjectName( "centralWidget" );
    centralWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
//    centralWidget->setStyleSheet("background-color:#313437");

    m_pTabs = new CAscTabWidget(centralWidget);
    m_pTabs->setGeometry(0, 0, centralWidget->width(), centralWidget->height());
    m_pTabs->m_pManager = m_pManager;
    m_pTabs->activate(false);
    connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(m_pTabs, SIGNAL(tabBarClicked(int)), this, SLOT(onTabClicked(int)));
    connect(m_pTabs, SIGNAL(tabClosed(int, int)), this, SLOT(onTabClosed(int, int)));
    connect(m_pTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequest(int)));

    // Minimize
    QSize small_btn_size(16*g_dpi_ratio, 16*g_dpi_ratio);
    QSize wide_btn_size(29*g_dpi_ratio, 16*g_dpi_ratio);
    m_pButtonMinimize->setObjectName( "toolButtonMinimize" );
    m_pButtonMinimize->setProperty("class", "normal");
    m_pButtonMinimize->setFixedSize(small_btn_size);
    QObject::connect( m_pButtonMinimize, SIGNAL( clicked() ), this, SLOT( pushButtonMinimizeClicked() ) );

    // Maximize    
    m_pButtonMaximize->setObjectName( "toolButtonMaximize" );
    m_pButtonMaximize->setProperty("class", "normal");
    m_pButtonMaximize->setFixedSize(small_btn_size);
    QObject::connect( m_pButtonMaximize, SIGNAL( clicked() ), this, SLOT( pushButtonMaximizeClicked() ) );


    // Close
    m_pButtonClose->setObjectName( "toolButtonClose" );
    m_pButtonClose->setProperty("class", "normal");
    m_pButtonClose->setFixedSize(small_btn_size);
    QObject::connect( m_pButtonClose, SIGNAL( clicked() ), this, SLOT( pushButtonCloseClicked() ) );

    // profile
    m_pButtonProfile->setObjectName( "toolButtonProfile" );
    m_pButtonProfile->setFixedSize(wide_btn_size);
    m_pButtonProfile->setDisabled(true);
//    m_pButtonProfile->setProperty("class", "normal");

    // download
    m_pButtonDownload->setObjectName("toolButtonDownload");
    m_pButtonDownload->setFixedSize(wide_btn_size);
    m_pButtonDownload->setAnimatedIcon(
                g_dpi_ratio > 1 ? ":/res/icons/downloading@2x.gif" : ":/res/icons/downloading.gif" );

    m_boxTitleBtns = new QWidget(centralWidget);
    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(APP_TITLE);
    label->setStyleSheet("color: #fff;");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layoutBtns->addWidget(label);
    layoutBtns->addWidget(m_pButtonDownload);
    layoutBtns->addWidget(m_pButtonProfile);
    layoutBtns->addWidget(m_pButtonMinimize);
    layoutBtns->addWidget(m_pButtonMaximize);
    layoutBtns->addWidget(m_pButtonClose);
    layoutBtns->setMargin(0);
    layoutBtns->setSpacing(14*g_dpi_ratio);
    m_boxTitleBtns->setFixedSize(342*g_dpi_ratio, 16*g_dpi_ratio);

    // profile menu
    QMenu * menuProfile = new QMenu;
    QWidgetAction * waction = new QWidgetAction(menuProfile);
    waction->setDefaultWidget(m_pWidgetProfile);
    menuProfile->setObjectName("menuButtonProfile");
    menuProfile->menuAction()->setIconVisibleInMenu(false);
    menuProfile->addAction(waction);
    menuProfile->addSeparator();

    CProfileMenuFilter * eventFilter = new CProfileMenuFilter(this);
    eventFilter->setMenuButton(m_pButtonProfile);
    menuProfile->installEventFilter(eventFilter);

    QAction *action = menuProfile->addAction(tr("Logout"));
    connect(action, SIGNAL(triggered()), this, SLOT(onMenuLogout()));
    m_pButtonProfile->setMenu(menuProfile);

    // download menu
    QMenu * menuDownload = new QMenu();
    waction = new QWidgetAction(menuDownload);
    waction->setDefaultWidget(m_pWidgetDownload);
    menuDownload->setObjectName("menuButtonDownload");
    menuDownload->addAction(waction);
//    menuDownload->addSeparator();
//    menuDownload->addAction(tr("Cancel All"));

    m_pButtonDownload->setMenu(menuDownload);
    m_pWidgetDownload->setManagedElements(m_pManager, m_pButtonDownload);

    eventFilter = new CProfileMenuFilter(this);
    eventFilter->setMenuButton(m_pButtonDownload);
    menuDownload->installEventFilter(eventFilter);

    // Main
    m_pButtonMain = new QPushButton( "", centralWidget );
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    QCefView * pMainWidget = new QCefView(centralWidget);
    pMainWidget->Create(m_pManager, cvwtSimple);
    pMainWidget->setObjectName( "mainPanel" );
    pMainWidget->setHidden(false);

    m_pMainWidget = (QWidget *)pMainWidget;
    m_pTabs->m_pMainButton = m_pButtonMain;

//    m_pMainWidget->setVisible(false);
//    loadStartPage();

//    m_pSeparator = new QWidget(centralWidget);
//    m_pSeparator->setObjectName("separator");
//    m_pSeparator->setStyleSheet("background-color:#D6D6D7");
//    m_pSeparator->setGeometry(0, 28*g_dpi_ratio, width(), 1);

    mainGridLayout->addWidget( centralWidget );

    RecalculatePlaces();
    show();

//    m_pTabs->addEditor("editor1 editor21", etDocument, L"https://testinfo.teamlab.info");
//    m_pTabs->addEditor("editor2", etPresentation, L"http://google.com");
//    m_pTabs->addEditor("editor3", etSpreadsheet, L"http://google.com");
//    m_pTabs->updateIcons();

    m_pManager->SetEventListener(this);
    m_pButtonDownload->hide();

    qRegisterMetaType<std::wstring>("std::wstring");
}

void QMainPanel::RecalculatePlaces()
{
    int nWindowW = width();
    int nWindowH = height();
    int nCaptionH = 29 * g_dpi_ratio;
    int btnMainWidth = 108 * g_dpi_ratio;

    m_pTabs->setGeometry(0, 0, nWindowW, nWindowH);
    m_pButtonMain->setGeometry(0, 0, btnMainWidth, nCaptionH);
//    m_pSeparator->setGeometry(0, 28 * g_dpi_ratio, nWindowW, 1*g_dpi_ratio);

//    int nStartOffset = 12;
//    int nBetweenApp = 12;
//    int nButtonW = 12;
//    int nY = (nCaptionH - nButtonW) >> 1;
//    nY = 5;

    int docCaptionW = nWindowW - m_pTabs->tabBar()->width() - btnMainWidth - (24*g_dpi_ratio);
    m_boxTitleBtns->setFixedSize(docCaptionW, 16 * g_dpi_ratio);
    m_boxTitleBtns->move(nWindowW - m_boxTitleBtns->width() - (14*g_dpi_ratio), 4 * g_dpi_ratio);
    m_pMainWidget->setGeometry(0, nCaptionH, nWindowW, nWindowH - nCaptionH);
}

// Button events
void QMainPanel::pushButtonMinimizeClicked()
{
    ShowWindow( parentWindow(), SW_MINIMIZE );
}

void QMainPanel::pushButtonMaximizeClicked()
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( parentWindow(), &wp );
    if ( wp.showCmd == SW_MAXIMIZE )
    {
        ShowWindow( parentWindow(), SW_RESTORE );
    }
    else
    {
        ShowWindow( parentWindow(), SW_MAXIMIZE );
    }
}

void QMainPanel::pushButtonCloseClicked()
{
    checkModified(WAIT_MODIFIED_CLOSE);
}

void QMainPanel::pushButtonMainClicked()
{
    if (m_pTabs->isActive()) {
        m_pTabs->activate(false);
        m_pMainWidget->setHidden(false);

        ((QCefView*)m_pMainWidget)->GetCefView()->focus();
        onTabChanged(m_pTabs->currentIndex());
    }
}

void QMainPanel::toggleButtonMain(bool toggle)
{
    if (m_pTabs->isActive() == toggle) {
        if (toggle) {
            m_pTabs->activate(false);
            m_pMainWidget->setHidden(false);

            ((QCefView*)m_pMainWidget)->GetCefView()->focus();
        } else {
            m_pTabs->activate(true);
            m_pMainWidget->setHidden(true);

            m_pTabs->setFocusedView();
        }

        onTabChanged(m_pTabs->currentIndex());
    }
}

void QMainPanel::focus() {
    if (m_pTabs->isActive()) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView*)m_pMainWidget)->GetCefView()->focus();
    }
}

void QMainPanel::onTabClicked(int index)
{
    Q_UNUSED(index)

    if (!m_pTabs->isActive()) {
        toggleButtonMain(false);
    }
}

void QMainPanel::onTabClosed(int index, int curcount)
{
    Q_UNUSED(index)

    if (curcount == 0) {
        toggleButtonMain(true);
    }

    onTabChanged(m_pTabs->currentIndex());
    RecalculatePlaces();
}

void QMainPanel::onTabChanged(int index)
{
    QLabel * title = (QLabel *)m_boxTitleBtns->layout()->itemAt(0)->widget();

    if (m_pTabs->isActive() && !(index < 0) && index < m_pTabs->count()) {
        QString docName = m_pTabs->titleByIndex(index, false);
        if (!docName.length())
            docName = m_pTabs->tabBar()->tabText(index);

        title->setText(QString(APP_TITLE) + " - " + docName);
    } else {
        title->setText(APP_TITLE);
    }
}

void QMainPanel::onTabCloseRequest(int index)
{
    if (m_pTabs->modifiedByIndex(index)) {
        CSaveFileMessage saveDlg(parentWindow());
        saveDlg.setFiles(m_pTabs->titleByIndex(index));

        switch (saveDlg.showModal()) {
        case 0: return;
        case -1: break;
        case 1:
        default:{
            m_pTabs->editorCloseRequest(index);

            QCefView * pView = (QCefView *)m_pTabs->widget(index);
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();

            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SAVE;
            pView->GetCefView()->Apply(pEvent);

            return;}
        }
    }

    m_pTabs->closeEditorByIndex(index, false);
}


bool QMainPanel::nativeEvent( const QByteArray &, void * msg, long * result)
{
    Q_UNUSED(result);
    MSG *message = ( MSG * )msg;
    switch( message->message )
    {
    case WM_SYSKEYDOWN:
    {
        if ( message->wParam == VK_SPACE )
        {
            RECT winrect;
            GetWindowRect( windowHandle, &winrect );
            TrackPopupMenu( GetSystemMenu( windowHandle, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, windowHandle, NULL);
            break;
        }
    }
    case WM_KEYDOWN:
    {
        if ( message->wParam == VK_F5 || message->wParam == VK_F6 || message->wParam == VK_F7)
        {
            SendMessage( windowHandle, WM_KEYDOWN, message->wParam, message->lParam );
            break;
        }
    }

    }

    return false;
}

void QMainPanel::mousePressEvent( QMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton )
    {
        ReleaseCapture();
        SendMessage( windowHandle, WM_NCLBUTTONDOWN, HTCAPTION, 0 );
    }

    if ( event->type() == QEvent::MouseButtonDblClick )
    {
        if (event -> button() == Qt::LeftButton)
        {
            WINDOWPLACEMENT wp;
            wp.length = sizeof( WINDOWPLACEMENT );
            GetWindowPlacement( parentWindow(), &wp );
            if ( wp.showCmd == SW_MAXIMIZE )
            {
                ShowWindow( parentWindow(), SW_RESTORE );
            }
            else
            {
                ShowWindow( parentWindow(), SW_MAXIMIZE );
            }
        }
    }
}

void QMainPanel::resizeEvent(QResizeEvent* event)
{
    QWinWidget::resizeEvent(event);
    RecalculatePlaces();

}

void QMainPanel::OnEvent(NSEditorApi::CAscMenuEvent* pEvent)
{
    if (NULL == pEvent)
        return;

    switch (pEvent->m_nType) {
    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
        NSEditorApi::CAscCreateTab* pData = (NSEditorApi::CAscCreateTab*)pEvent->m_pData;

        bool active_tab = pData->get_Active();
        int view_id = pData->get_IdEqual();
        QMetaObject::invokeMethod(m_pTabs, "onDocumentOpen", Qt::QueuedConnection,
                                        Q_ARG(std::wstring, pData->get_Url()), Q_ARG(bool, active_tab), Q_ARG(int, view_id));

        QMetaObject::invokeMethod(this, "onAddEditor", Qt::QueuedConnection, Q_ARG(bool, active_tab), Q_ARG(int, view_id));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
        NSEditorApi::CAscTabEditorType * pData = (NSEditorApi::CAscTabEditorType*)pEvent->m_pData;

        QMetaObject::invokeMethod(m_pTabs, "onDocumentType", Qt::QueuedConnection,
                Q_ARG(int, pData->get_Id()), Q_ARG(int, pData->get_Type()));

        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONCLOSE: break;
    case ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME: {
        NSEditorApi::CAscDocumentName* pData = (NSEditorApi::CAscDocumentName*)pEvent->m_pData;

        QMetaObject::invokeMethod(m_pTabs, "onDocumentNameChanged", Qt::QueuedConnection,
                Q_ARG(int, pData->get_Id()), Q_ARG(QString, QString::fromStdWString(pData->get_Name())));

        QMetaObject::invokeMethod(this, "onTabChanged", Qt::QueuedConnection, Q_ARG(int, m_pTabs->currentIndex()));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED: {
        NSEditorApi::CAscDocumentModifyChanged * pData = (NSEditorApi::CAscDocumentModifyChanged *)pEvent->m_pData;

        QMetaObject::invokeMethod(m_pTabs, "onDocumentChanged", Qt::QueuedConnection,
                                    Q_ARG(int, pData->get_Id()), Q_ARG(bool, pData->get_Changed()));

        QMetaObject::invokeMethod(this, "onTabChanged", Qt::QueuedConnection, Q_ARG(int, m_pTabs->currentIndex()));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT: {
        QMetaObject::invokeMethod(this, "onLogout", Qt::QueuedConnection);
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
        NSEditorApi::CAscTypeId* pId = (NSEditorApi::CAscTypeId*)pEvent->m_pData;

        QMetaObject::invokeMethod(m_pTabs, "onDocumentSave", Qt::QueuedConnection,
                                        Q_ARG(int, pId->get_Id()));

        break;}

    case ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE: {
        NSEditorApi::CAscJSMessage * pData = (NSEditorApi::CAscJSMessage *)pEvent->m_pData;

//        OutputDebugString(pData->get_Value().c_str());

        if (QString::fromStdWString(pData->get_Name()).compare("login") == 0) {
                QMetaObject::invokeMethod(this, "onLogin", Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromStdWString(pData->get_Value())));
        }
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE: break;
    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS: break;

    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START:
    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
        NSEditorApi::CAscDownloadFileInfo * pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;

        ADDREFINTERFACE(pData);
        QMetaObject::invokeMethod(m_pWidgetDownload, "onDocumentDownload", Qt::QueuedConnection,
                Q_ARG(void *, pData), Q_ARG(bool, pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START));               
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
        NSEditorApi::CAscPrintEnd * pData = (NSEditorApi::CAscPrintEnd *)pEvent->m_pData;

        ADDREFINTERFACE(pData)
        QMetaObject::invokeMethod(this, "onDocumentPrint", Qt::QueuedConnection, Q_ARG(void *, pData));
        break;}
    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: {
        NSEditorApi::CAscOnOpenExternalLink * pData = (NSEditorApi::CAscOnOpenExternalLink *)pEvent->m_pData;
        QDesktopServices::openUrl(QUrl(QString().fromStdWString(pData->get_Url())));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
        NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)pEvent->m_pData;

        ADDREFINTERFACE(pData)
        QMetaObject::invokeMethod(this, "onKeyDown", Qt::QueuedConnection, Q_ARG(void *, pData));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER:
    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE:
        QMetaObject::invokeMethod(this, "onFullScreen", Qt::QueuedConnection,
                        Q_ARG(bool, pEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER));
        break;
    }

    RELEASEINTERFACE(pEvent);
}

void QMainPanel::onMenuLogout()
{
    if (m_pWidgetProfile->info()->logged()) {
        checkModified(WAIT_MODIFIED_LOGOUT);
    }
}

void QMainPanel::onAddEditor(bool select, int id)
{
    if (id < 0)
        RecalculatePlaces();

    QTimer::singleShot(200, this, [=]{
        if (select) toggleButtonMain(false);
    });
}

void QMainPanel::onLogin(QString params)
{
    m_pWidgetProfile->parseProfile(params);
    m_pButtonProfile->setDisabled(!m_pWidgetProfile->info()->portal().length());

    GET_REGISTRY_USER(_reg_user)
    _reg_user.setValue("portal", m_pWidgetProfile->info()->portal());
}

void QMainPanel::onLogout()
{
    m_pTabs->closeAllEditors();
    loadStartPage();
}

void QMainPanel::onJSMessage(QString key, QString value)
{
    if (key == "login") {
        onLogin(value);
    }
}

void QMainPanel::loadStartPage()
{
    std::wstring sAppData(L"");

#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED( SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath) ) ) {
        sAppData = std::wstring(szPath);
        NSCommon::string_replace(sAppData, L"\\", L"/");
    }
#endif

    QString data_path = sAppData.size() > 0 ?
                QString().fromStdWString(sAppData) :
                QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    data_path += "/ONLYOFFICE/DesktopEditors/webdata/local/index.html";
//    data_path = "ascdesktop://login.html";


    QString additional;

    if (!g_lang.isEmpty())
        additional.append("?lang=" + g_lang);

    GET_REGISTRY_USER(_reg_user);
    QString _portal = _reg_user.value("portal").value<QString>();
    if (!_portal.isEmpty()) {
        QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
        additional.append(arg_portal);
    }

    std::wstring start_path = (data_path + additional).toStdWString();
    ((QCefView*)m_pMainWidget)->GetCefView()->load(start_path);
}

void QMainPanel::checkModified(byte action)
{
    QMap<int, QString> mapModified;
    for (int i = 0; i < m_pTabs->count(); i++) {
        if (m_pTabs->modifiedByIndex(i)) {
            mapModified.insert(m_pTabs->viewByIndex(i), m_pTabs->titleByIndex(i, true));
        }
    }

    if (mapModified.size()) {
        CSaveFileMessage saveDlg(parentWindow());
        saveDlg.setFiles(&mapModified);

        switch (saveDlg.showModal()) {
        case 0: return;
        case -1: break;
        case 1:
        default:{
            if (mapModified.size()) {
                CCefView * pView;
                QMapIterator<int,QString> i(mapModified);
                while (i.hasNext()) {
                    i.next();

                    pView = m_pManager->GetViewById(i.key());
                    pView->Apply(new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));
                }
            }

            break;}
        }
    }

    if (action == WAIT_MODIFIED_CLOSE) {
        PostQuitMessage(0);
    } else
    if (action == WAIT_MODIFIED_LOGOUT) {
        m_pManager->Logout(m_pWidgetProfile->info()->portal().toStdWString());

        m_pButtonProfile->setDisabled(true);
        m_pWidgetProfile->parseProfile("");
    }
}

void QMainPanel::onDocumentPrint(void * opts)
{
    static bool printInProcess = false;
    if (!printInProcess)
        printInProcess = true; else
        return;

    NSEditorApi::CAscPrintEnd * pData = (NSEditorApi::CAscPrintEnd *)opts;

    int id = pData->get_Id(),
    pagesCount = pData->get_PagesCount(),
    currentPage = pData->get_CurrentPage();

    if (pagesCount < 1) return;

//#ifdef _WIN32
    QAscPrinterContext * pContext = m_printData->_printer_info.isNull() ?
                new QAscPrinterContext() : new QAscPrinterContext(m_printData->_printer_info);

    QPrinter * printer = pContext->getPrinter();
    printer->setOutputFileName("");
    printer->setFromTo(1, pagesCount);

#ifdef _WIN32
    CPrintDialogWinWrapper wrapper(printer, parentWindow());
    QPrintDialog * dialog = wrapper.q_dialog();
#else
#endif // _WIN32

    dialog->setWindowTitle(tr("Print Document"));
    dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintCurrentPage);
    if (!(currentPage < 0))
        currentPage++, dialog->setOptions(dialog->options() | QPrintDialog::PrintCurrentPage);
    dialog->setPrintRange(m_printData->_print_range);

    int start = -1, finish = -1;
    int res = dialog->exec();
    if (res == QDialog::Accepted) {
        m_printData->_printer_info = QPrinterInfo::printerInfo(printer->printerName());
        m_printData->_print_range = dialog->printRange();

        switch(dialog->printRange()) {
        case QPrintDialog::AllPages: start = 1, finish = pagesCount; break;
        case QPrintDialog::PageRange:
            start = dialog->fromPage(), finish = dialog->toPage(); break;
        case QPrintDialog::Selection: break;
        case QPrintDialog::CurrentPage: start = currentPage, finish = currentPage; break;
        }

        if (!(start < 0) || !(finish < 0)) {
            start < 1 && (start = 1);
            finish < 1 && (finish = 1);
            finish < start && (finish = start);

            CCefView * pView = m_pManager->GetViewById(id);
            if ( pView ) {
                NSEditorApi::CAscPrintPage * pData;
                NSEditorApi::CAscMenuEvent * pEvent;

                EnableWindow(parentWindow(), FALSE);

                CPrintProgress progressDlg(parentWindow());
                progressDlg.startProgress();

                uint count = finish - start;
                pContext->BeginPaint();
                for (; !(start > finish); ++start) {
                    pContext->AddRef();

                    progressDlg.setProgress(count - (finish - start) + 1, count + 1);
                    qApp->processEvents();

                    pData = new NSEditorApi::CAscPrintPage();
                    pData->put_Context(pContext);
                    pData->put_Page(start - 1);

                    pEvent = new NSEditorApi::CAscMenuEvent();
                    pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE;
                    pEvent->m_pData = pData;

                    pView->Apply(pEvent);

                    if (progressDlg.isRejected())
                        break;

                    if (start < finish) {
                        printer->newPage();
                    }
                }
                pContext->EndPaint();

                pEvent = new NSEditorApi::CAscMenuEvent();
                pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_PRINT_END;

                pView->Apply(pEvent);
                EnableWindow(parentWindow(), TRUE);
            }
        } else {
            // TODO: show error message
        }
    }

    printInProcess = false;
    pContext->Release();
    RELEASEINTERFACE(pData)
}

void QMainPanel::onDialogSave(std::wstring sName)
{
    GET_REGISTRY_USER(_reg_user);

    QString savePath = _reg_user.value("savePath").value<QString>();
    if (savePath.isEmpty() || !QDir(savePath).exists())
        savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    static bool saveInProcess = false;
    if (!saveInProcess) {
        saveInProcess = true;

        if (sName.size()) {
            QString fullPath = savePath + "\\" + QString().fromStdWString(sName);
            CFileDialogWinWrapper dlg(parentWindow());

            if (dlg.showModal(fullPath)) {
                savePath = QFileInfo(fullPath).absolutePath();
                _reg_user.setValue("savePath", savePath);
            }

            m_pManager->EndSaveDialog(fullPath.toStdWString());
        }

        saveInProcess = false;
    }
}

void QMainPanel::onFullScreen(bool apply)
{    
    if (!apply) {
        ShowWindow(parentWindow(), m_isMaximized ? SW_MAXIMIZE : SW_SHOW);
        m_pTabs->setFullScreen(apply);
    } else {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(parentWindow(), &wp);

        m_isMaximized = wp.showCmd == SW_MAXIMIZE;

        m_pTabs->setFullScreen(apply);
        ShowWindow(parentWindow(), SW_HIDE);
    }
}

void QMainPanel::onKeyDown(void * eventData)
{
    NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)eventData;

    int key = pData->get_KeyCode();
    bool _is_ctrl = pData->get_IsCtrl();

    RELEASEINTERFACE(pData)

    switch (key) {
    case VK_F4:
        if (_is_ctrl && m_pTabs->isActive()) {
            m_pTabs->closeEditorByIndex(m_pTabs->currentIndex());
        }
        break;
    }
}
