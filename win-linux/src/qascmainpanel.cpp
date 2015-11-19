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

#include "qascmainpanel.h"

#include <QPrinterInfo>
#include <QPrintDialog>
#include <QDir>
#include <QMenu>
#include <QWidgetAction>
#include <QTimer>
#include <QStandardPaths>
#include <QApplication>
#include <QDesktopServices>

#include "defines.h"
#include "csavefilemessage.h"
#include "cprintprogress.h"
#include "cfiledialog.h"
#include "qascprinter.h"
#include "../common/libs/common/Types.h"

#ifdef _WIN32
#include "cprintdialog.h"
#else
#define VK_F4 0x73
#endif

#define APP_TITLE "ONLYOFFICE"
extern BYTE     g_dpi_ratio;
extern QString  g_lang;

struct CPrintData {
public:
    CPrintData() : _print_range(QPrintDialog::PrintRange::AllPages) {}
    QPrinterInfo _printer_info;
    QPrintDialog::PrintRange _print_range;
};

QAscMainPanel::QAscMainPanel(QWidget *parent, CAscApplicationManager *manager, bool isCustomWindow)
    : QWidget(parent), CCefEventsTransformer(this),
        m_pButtonMinimize(NULL), m_pButtonMaximize(NULL), m_pButtonClose(NULL),
        m_pButtonProfile(new QPushButton), m_pButtonDownload(new CPushButton),
        m_isMaximized(false), m_isCustomWindow(isCustomWindow),
        m_pWidgetProfile(new CUserProfileWidget), m_pWidgetDownload(new CDownloadWidget)
      , m_printData(new CPrintData)
      , m_mainWindowState(Qt::WindowNoState)
{
    m_pManager = manager;

    setObjectName("mainPanel");

    QGridLayout *mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing( 0 );
    mainGridLayout->setMargin( 0 );
    setLayout( mainGridLayout );

    // Central widget
    QWidget *centralWidget = new QWidget( this );
    centralWidget->setObjectName("centralWidget");
    centralWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QPalette palette;
    m_pTabs = new CAscTabWidget(centralWidget);
    m_pTabs->setGeometry(0, 0, centralWidget->width(), centralWidget->height());
    m_pTabs->m_pManager = m_pManager;
    m_pTabs->activate(false);
    connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(m_pTabs, SIGNAL(tabBarClicked(int)), this, SLOT(onTabClicked(int)));
    connect(m_pTabs, SIGNAL(tabClosed(int, int)), this, SLOT(onTabClosed(int, int)));
    connect(m_pTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequest(int)));

    QSize small_btn_size(16*g_dpi_ratio, 16*g_dpi_ratio);
    QSize wide_btn_size(29*g_dpi_ratio, 16*g_dpi_ratio);
    if (isCustomWindow) {
        // Minimize
        m_pButtonMinimize = new QPushButton(centralWidget);
        m_pButtonMinimize->setObjectName( "toolButtonMinimize" );
        m_pButtonMinimize->setProperty("class", "normal");
        m_pButtonMinimize->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonMinimize, SIGNAL( clicked() ), this, SLOT( pushButtonMinimizeClicked() ) );

        // Maximize
        m_pButtonMaximize = new QPushButton(centralWidget);
        m_pButtonMaximize->setObjectName( "toolButtonMaximize" );
        m_pButtonMaximize->setProperty("class", "normal");
        m_pButtonMaximize->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonMaximize, SIGNAL( clicked() ), this, SLOT( pushButtonMaximizeClicked() ) );

        // Close
        m_pButtonClose = new QPushButton(centralWidget);
        m_pButtonClose->setObjectName( "toolButtonClose" );
        m_pButtonClose->setProperty("class", "normal");
        m_pButtonClose->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonClose, SIGNAL( clicked() ), this, SLOT( pushButtonCloseClicked() ) );
    }

    // profile
    m_pButtonProfile->setObjectName( "toolButtonProfile" );
    m_pButtonProfile->setFixedSize(wide_btn_size);
    m_pButtonProfile->setDisabled(true);

    // download
    m_pButtonDownload->setObjectName("toolButtonDownload");
    m_pButtonDownload->setFixedSize(wide_btn_size);
    m_pButtonDownload->setFixedSize(QSize(35*g_dpi_ratio, 16*g_dpi_ratio));
    m_pButtonDownload->setAnimatedIcon(
                g_dpi_ratio > 1 ? ":/res/icons/downloading@2x.gif" : ":/res/icons/downloading.gif" );

    m_boxTitleBtns = new QWidget(centralWidget);
    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(APP_TITLE);
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layoutBtns->setMargin(0);
    layoutBtns->setSpacing(14*g_dpi_ratio);
    layoutBtns->addWidget(label);
    layoutBtns->addWidget(m_pButtonDownload);
    layoutBtns->addWidget(m_pButtonProfile);

    // Main
    m_pButtonMain = new QPushButton( "", centralWidget );
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    QString _tabs_stylesheet_file = g_dpi_ratio > 1 ? ":/styles@2x/" : ":/sep-styles/";
    if (isCustomWindow) {
        _tabs_stylesheet_file += "tabbar_flw.qss";
        palette.setColor(QPalette::Background, QColor("#313437"));

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

        m_boxTitleBtns->setFixedSize(282*g_dpi_ratio, 16*g_dpi_ratio);
    } else {
        _tabs_stylesheet_file += "tabbar.qss";

        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));

        palette.setBrush(QPalette::Background, QBrush(gradient));

        m_pButtonMain->setProperty("theme", "rightangle");
        m_pButtonProfile->setProperty("theme", "dark");
        label->setFixedHeight(0);
        m_boxTitleBtns->setFixedSize(342*g_dpi_ratio, 16*g_dpi_ratio);
    }

    QFile styleFile(_tabs_stylesheet_file);
    styleFile.open( QFile::ReadOnly );
    m_pTabs->setStyleSheet(QString(styleFile.readAll()));
    m_pTabs->setAutoFillBackground(true);
    m_pTabs->setPalette(palette);
    m_pTabs->applyCustomTheme(isCustomWindow);

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

    m_pButtonDownload->setMenu(menuDownload);
    m_pWidgetDownload->setManagedElements(m_pManager, m_pButtonDownload);

    eventFilter = new CProfileMenuFilter(this);
    eventFilter->setMenuButton(m_pButtonDownload);
    menuDownload->installEventFilter(eventFilter);


    QCefView * pMainWidget = new QCefView(centralWidget);
    pMainWidget->Create(m_pManager, cvwtSimple);
    pMainWidget->setObjectName( "mainPanel" );
    pMainWidget->setHidden(false);

    m_pMainWidget = (QWidget *)pMainWidget;
    m_pTabs->m_pMainButton = m_pButtonMain;

//    m_pSeparator = new QWidget(centralWidget);
//    m_pSeparator->setObjectName("separator");
//    m_pSeparator->setStyleSheet("background-color:#dadada");
//    m_pSeparator->setGeometry(0, 0, width(), 1);

//    m_pMainWidget->setVisible(false);
    loadStartPage();

    mainGridLayout->addWidget( centralWidget );

    RecalculatePlaces();

//    m_pTabs->addEditor("editor1 editor21", etDocument, L"https://testinfo.teamlab.info");
//    m_pTabs->addEditor("editor2", etPresentation, L"http://google.com");
//    m_pTabs->addEditor("editor3", etSpreadsheet, L"http://google.com");
//    m_pTabs->updateIcons();

    m_pManager->SetEventListener(this);
    m_pButtonDownload->setVisible(false, false);

//    qRegisterMetaType<std::wstring>("std::wstring");
}

void QAscMainPanel::RecalculatePlaces()
{
    int nWindowW = width();
    int nWindowH = height();
    int nCaptionH = 29 * g_dpi_ratio;
    int btnMainWidth = 108 * g_dpi_ratio;

    m_pTabs->setGeometry(0, 0, nWindowW, nWindowH);
    m_pButtonMain->setGeometry(0, 0, btnMainWidth, nCaptionH);
//    m_pSeparator->setGeometry(0, 0, nWindowW, 1*g_dpi_ratio);

    int docCaptionW = nWindowW - m_pTabs->tabBar()->width() - btnMainWidth - (24*g_dpi_ratio);
    m_boxTitleBtns->setFixedSize(docCaptionW, 16 * g_dpi_ratio);
    m_boxTitleBtns->move(nWindowW - m_boxTitleBtns->width() - (14*g_dpi_ratio), 4 * g_dpi_ratio);
    m_pMainWidget->setGeometry(0, nCaptionH, nWindowW, nWindowH - nCaptionH);
}

void QAscMainPanel::pushButtonMinimizeClicked()
{
    emit mainWindowChangeState(Qt::WindowMinimized);

//    QPrinter printer;
//    QPrintDialog *dialog = new QPrintDialog(&printer, this);
//    dialog->setWindowTitle(tr("Print Document"));

//    if (dialog->exec() != QDialog::Accepted)
//           return;
}

void QAscMainPanel::pushButtonMaximizeClicked()
{
    if (m_mainWindowState == Qt::WindowMaximized) {
        emit mainWindowChangeState(Qt::WindowNoState);
    } else {
        emit mainWindowChangeState(Qt::WindowMaximized);
    }
}

void QAscMainPanel::pushButtonCloseClicked()
{
    checkModified(WAIT_MODIFIED_CLOSE);
}

void QAscMainPanel::applyMainWindowState(Qt::WindowState s)
{
    m_mainWindowState = s;

    if ( m_isCustomWindow ) {
        m_pButtonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }
}

void QAscMainPanel::pushButtonMainClicked()
{
    if (m_pTabs->isActive()) {
        m_pTabs->activate(false);
        m_pMainWidget->setHidden(false);

        ((QCefView *)m_pMainWidget)->GetCefView()->focus();
        onTabChanged(m_pTabs->currentIndex());
    }
}

void QAscMainPanel::toggleButtonMain(bool toggle)
{
    if (m_pTabs->isActive() == toggle) {
        if (toggle) {
            m_pTabs->activate(false);
            m_pMainWidget->setHidden(false);

            ((QCefView *)m_pMainWidget)->GetCefView()->focus();
        } else {
            m_pTabs->activate(true);
            m_pMainWidget->setHidden(true);

            m_pTabs->setFocusedView();
        }

        onTabChanged(m_pTabs->currentIndex());
    }
}

void QAscMainPanel::focus() {
    if (m_pTabs->isActive()) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView *)m_pMainWidget)->GetCefView()->focus();
    }
}

void QAscMainPanel::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    RecalculatePlaces();
}

void QAscMainPanel::onTabClicked(int index)
{
    Q_UNUSED(index)

    if (!m_pTabs->isActive()) {
        toggleButtonMain(false);
    }
}

void QAscMainPanel::onTabClosed(int index, int curcount)
{
    Q_UNUSED(index)

    if (curcount == 0) {
        toggleButtonMain(true);
    }

    onTabChanged(m_pTabs->currentIndex());
    RecalculatePlaces();
}

CAscApplicationManager * QAscMainPanel::getAscApplicationManager()
{
    return m_pManager;
}

void QAscMainPanel::onTabChanged(int index)
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

void QAscMainPanel::onTabCloseRequest(int index)
{
    if (m_pTabs->modifiedByIndex(index)) {
#if defined(_WIN32)
        CSaveFileMessage saveDlg((HWND)parentWidget()->winId());
#else
        CSaveFileMessage saveDlg(this);
#endif
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

void QAscMainPanel::onNeedCheckKeyboard()
{
    if (m_pManager) m_pManager->CheckKeyboard();
}

void QAscMainPanel::onMenuLogout()
{
    if (m_pWidgetProfile->info()->logged()) {
        checkModified(WAIT_MODIFIED_LOGOUT);
    }
}

void QAscMainPanel::onDocumentOpen(std::wstring url, int id, bool select)
{
    m_pTabs->openDocument(url, id, select);

    if (id < 0)
        RecalculatePlaces();

    if (select)
        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
}

void QAscMainPanel::onDocumentType(int id, int type)
{
    m_pTabs->applyDocumentChanging(id, type);
}

void QAscMainPanel::onDocumentName(int id, QString name)
{
    m_pTabs->applyDocumentChanging(id, name);
    onTabChanged(m_pTabs->currentIndex());
}

void QAscMainPanel::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void QAscMainPanel::onDocumentDownload(void * info)
{
    m_pWidgetDownload->downloadProcess(info);

    NSEditorApi::CAscDownloadFileInfo * pData = reinterpret_cast<NSEditorApi::CAscDownloadFileInfo *>(info);
    RELEASEINTERFACE(pData);
}

void QAscMainPanel::onLogin(QString params)
{
    m_pWidgetProfile->parseProfile(params);
    m_pButtonProfile->setDisabled(!m_pWidgetProfile->info()->portal().length());

    GET_REGISTRY_USER(_reg_user)
    _reg_user.setValue("portal", m_pWidgetProfile->info()->portal());
}

void QAscMainPanel::onLogout()
{
    m_pTabs->closeAllEditors();
    loadStartPage();
}

void QAscMainPanel::onJSMessage(QString key, QString value)
{
    if (key == "login") {
        onLogin(value);
    }
}

void QAscMainPanel::loadStartPage()
{
    QString data_path = QString().fromStdWString(m_pManager->m_oSettings.app_data_path) + "/webdata/local/index.html";
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

    std::wstring start_path = ("file://" + data_path + additional).toStdWString();
    ((QCefView*)m_pMainWidget)->GetCefView()->load(start_path);
}

void QAscMainPanel::goStart()
{
    loadStartPage();
    toggleButtonMain(true);
}

void QAscMainPanel::checkModified(BYTE action)
{
    QMap<int, QString> mapModified;
    for (int i = 0; i < m_pTabs->count(); i++) {
        if (m_pTabs->modifiedByIndex(i)) {
            mapModified.insert(m_pTabs->viewByIndex(i), m_pTabs->titleByIndex(i, true));
        }
    }

    if (mapModified.size()) {
#ifdef _WIN32
        CSaveFileMessage saveDlg((HWND)parentWidget()->winId());
#else
        CSaveFileMessage saveDlg(this);
#endif
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
        emit mainWindowClose();
    } else
    if (action == WAIT_MODIFIED_LOGOUT) {
        m_pManager->Logout(m_pWidgetProfile->info()->portal().toStdWString());

        m_pButtonProfile->setDisabled(true);
        m_pWidgetProfile->parseProfile("");

        goStart();
    }
}

void QAscMainPanel::onDocumentPrint(void * opts)
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
    CPrintDialogWinWrapper wrapper(printer, (HWND)parentWidget()->winId());
    QPrintDialog * dialog = wrapper.q_dialog();
#else
    QPrintDialog * dialog =  new QPrintDialog(printer, this);
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

#if defined(_WIN32)
//                EnableWindow(parentWindow(), FALSE);
                EnableWindow((HWND)parentWidget()->winId(), FALSE);

                CPrintProgress progressDlg((HWND)parentWidget()->winId());
#else
                CPrintProgress progressDlg(qobject_cast<QWidget *>(parent()));
#endif
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
#if defined(_WIN32)
//                EnableWindow(parentWindow(), TRUE);
                EnableWindow((HWND)parentWidget()->winId(), TRUE);
#endif
            }
        } else {
            // TODO: show error message
        }
    }

    printInProcess = false;
    pContext->Release();
    RELEASEINTERFACE(pData)

#ifndef _WIN32
    RELEASEOBJECT(dialog)
#endif
}

void QAscMainPanel::onDialogSave(std::wstring sName, uint id)
{
    GET_REGISTRY_USER(_reg_user);

    qDebug() << "NAME: " << QString().fromStdWString(sName);

    QString savePath = _reg_user.value("savePath").value<QString>();
    if (savePath.isEmpty() || !QDir(savePath).exists())
        savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    static bool saveInProcess = false;
    if (!saveInProcess) {
        saveInProcess = true;

        if (sName.size()) {
            QString fullPath = savePath + "/" + QString().fromStdWString(sName);
#ifdef _WIN32
            CFileDialogWrapper dlg((HWND)parentWidget()->winId());
#else
            CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

            if (dlg.showModal(fullPath)) {
                savePath = QFileInfo(fullPath).absolutePath();
                _reg_user.setValue("savePath", savePath);
            }

            m_pManager->EndSaveDialog(fullPath.toStdWString(), id);
        }

        saveInProcess = false;
    }
}

void QAscMainPanel::onFullScreen(bool apply)
{
    if (apply) {
        m_isMaximized = m_mainWindowState == Qt::WindowMaximized;

        m_pTabs->setFullScreen(apply);
        emit mainWindowChangeState(Qt::WindowFullScreen);
    } else {
        emit mainWindowChangeState(m_isMaximized ? Qt::WindowMaximized : Qt::WindowNoState);
        m_pTabs->setFullScreen(apply);
    }

//    if (!apply) {
//        ShowWindow(parentWindow(), m_isMaximized ? SW_MAXIMIZE : SW_SHOW);
//        m_pTabs->setFullScreen(apply);
//    } else {
//        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
//        GetWindowPlacement(parentWindow(), &wp);

//        m_isMaximized = wp.showCmd == SW_MAXIMIZE;

//        m_pTabs->setFullScreen(apply);
//        ShowWindow(parentWindow(), SW_HIDE);
//    }
}

void QAscMainPanel::onKeyDown(void * eventData)
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

void QAscMainPanel::onLink(QString url)
{
    QDesktopServices::openUrl(QUrl(url));
}
