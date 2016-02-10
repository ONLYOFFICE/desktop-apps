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
#include <QRegularExpression>
#include <QMessageBox>

#include "defines.h"
#include "csavefilemessage.h"
#include "cprintprogress.h"
#include "cfiledialog.h"
#include "qascprinter.h"
#include "../common/libs/common/Types.h"
#include "cmessage.h"

#ifdef _WIN32
#include "cprintdialog.h"
#include "shlobj.h"

extern HWND gTopWinId;
#else
#define VK_F4 0x73
#define gTopWinId this
#endif

using namespace NSEditorApi;

#define BUTTON_MAIN_WIDTH   68
#define TITLE_HEIGHT        29
#define TOOLBTN_HEIGHT      29

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
      , m_waitActiveLic(false)
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
    connect(m_pTabs, &CAscTabWidget::closeAppRequest, this, &QAscMainPanel::onAppCloseRequest);

    QSize small_btn_size(28*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
    QSize wide_btn_size(29*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
    if (isCustomWindow) {
        // Minimize
        m_pButtonMinimize = new QPushButton(centralWidget);
        m_pButtonMinimize->setObjectName( "toolButtonMinimize" );
        m_pButtonMinimize->setProperty("class", "normal");
        m_pButtonMinimize->setProperty("act", "tool");
        m_pButtonMinimize->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonMinimize, SIGNAL( clicked() ), this, SLOT( pushButtonMinimizeClicked() ) );

        // Maximize
        m_pButtonMaximize = new QPushButton(centralWidget);
        m_pButtonMaximize->setObjectName( "toolButtonMaximize" );
        m_pButtonMaximize->setProperty("class", "normal");
        m_pButtonMaximize->setProperty("act", "tool");
        m_pButtonMaximize->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonMaximize, SIGNAL( clicked() ), this, SLOT( pushButtonMaximizeClicked() ) );

        // Close
        m_pButtonClose = new QPushButton(centralWidget);
        m_pButtonClose->setObjectName( "toolButtonClose" );
        m_pButtonClose->setProperty("class", "normal");
        m_pButtonClose->setProperty("act", "tool");
        m_pButtonClose->setFixedSize(small_btn_size);
        QObject::connect( m_pButtonClose, SIGNAL( clicked() ), this, SLOT( pushButtonCloseClicked() ) );
    }

    // profile
    m_pButtonProfile->setObjectName( "toolButtonProfile" );
    m_pButtonProfile->setFixedSize(wide_btn_size);
    m_pButtonProfile->setDisabled(true);

    // download
    m_pButtonDownload->setObjectName("toolButtonDownload");
//    m_pButtonDownload->setFixedSize(wide_btn_size);
    m_pButtonDownload->setFixedSize(QSize(33*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio));
    m_pButtonDownload->setAnimatedIcon(
                g_dpi_ratio > 1 ? ":/res/icons/downloading_2x.gif" : ":/res/icons/downloading.gif" );

    m_boxTitleBtns = new QWidget(centralWidget);
    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(APP_TITLE);
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layoutBtns->setMargin(0);
    layoutBtns->setSpacing(1*g_dpi_ratio);
    layoutBtns->addWidget(label);
    layoutBtns->addWidget(m_pButtonDownload);
//    layoutBtns->addWidget(m_pButtonProfile);

    // Main
    m_pButtonMain = new QPushButton( tr("FILE"), centralWidget );
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    m_pButtonMain->setGeometry(0, 0, BUTTON_MAIN_WIDTH * g_dpi_ratio, TITLE_HEIGHT * g_dpi_ratio);
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    QString _tabs_stylesheet_file = g_dpi_ratio > 1 ? ":/styles@2x/" : ":/sep-styles/";
    if (isCustomWindow) {
        _tabs_stylesheet_file += "tabbar.qss";
        palette.setColor(QPalette::Background, QColor("#313437"));

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

        m_boxTitleBtns->setFixedSize(282*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
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

    mainGridLayout->addWidget( centralWidget );

    RecalculatePlaces();
    loadStartPage();

//    m_pTabs->addEditor("editor1 editor21", etDocument, L"https://testinfo.teamlab.info");
//    m_pTabs->addEditor("editor2", etPresentation, L"http://google.com");
//    m_pTabs->addEditor("editor3", etSpreadsheet, L"http://google.com");
//    m_pTabs->updateIcons();

    m_pManager->SetEventListener(this);
    m_pButtonDownload->setVisible(false, false);

    GET_REGISTRY_USER(_reg_user);

    QString _path_ = _reg_user.value("openPath").value<QString>();
    m_lastOpenPath = _path_.length() > 0 && !QDir(_path_).exists() ?
        _path_ : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    _path_ = _reg_user.value("savePath").value<QString>();
    m_lastSavePath = _path_.length() > 0 && !QDir(_path_).exists() ?
        _path_ : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

//    m_savePortal;
    m_saveAction = 0; // undefined

    QString fn, ln;
    fillUserName(fn, ln);
    QString params = "lang="+g_lang+"&userfname="+fn+"&userlname="+ln;
    wstring wparams = params.toStdWString();
    m_pManager->InitAdditionalEditorParams(wparams);
}

void QAscMainPanel::RecalculatePlaces()
{
    int nWindowW = width();
    int nWindowH = height();
    int nCaptionH = TITLE_HEIGHT * g_dpi_ratio;
    int btnMainWidth = BUTTON_MAIN_WIDTH * g_dpi_ratio;

    m_pTabs->setGeometry(0, 0, nWindowW, nWindowH);
//    m_pSeparator->setGeometry(0, 0, nWindowW, 1*g_dpi_ratio);

    int docCaptionW = nWindowW - m_pTabs->tabBar()->width() - btnMainWidth - (24*g_dpi_ratio);
    m_boxTitleBtns->setFixedSize(docCaptionW, TOOLBTN_HEIGHT * g_dpi_ratio);
    m_boxTitleBtns->move(nWindowW - m_boxTitleBtns->width() - (4*g_dpi_ratio), 0 * g_dpi_ratio);
    m_pMainWidget->setGeometry(0, nCaptionH, nWindowW, nWindowH - nCaptionH);
}

void QAscMainPanel::pushButtonMinimizeClicked()
{
    emit mainWindowChangeState(Qt::WindowMinimized);
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
    // if close doesn't act
    if (m_saveAction != 2) {
        int _index_, _answ_;

        while (true) {
            // find a modified document
            _index_ = m_pTabs->findModified("");
            if ( _index_ < 0 ) {
                // no modified documents
                m_pTabs->closeAllEditors();
                QTimer::singleShot(10, this, [=]{emit mainWindowClose();});
                break;
            } else {
                // attempt to save the modified document
                _answ_ = trySaveDocument(_index_);

                // deny saving
                if ( _answ_ == MODAL_RESULT_NO ) {
                    m_pTabs->closeEditorByIndex(_index_, false);
                    continue;
                } else
                // saving in progress
                if ( _answ_ == MODAL_RESULT_YES ) {
                    m_saveAction = 2; // close portal
                    m_savePortal = "";
                    break;
                } else
                if ( _answ_ == MODAL_RESULT_CANCEL) {
                    break;
                }
            }
        }
    }
}

void QAscMainPanel::onAppCloseRequest()
{
    onFullScreen(false);
    pushButtonCloseClicked();
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
    if (trySaveDocument(index) == MODAL_RESULT_NO)
        m_pTabs->closeEditorByIndex(index, false);
}

int QAscMainPanel::trySaveDocument(int index)
{
    if (m_pTabs->closedByIndex(index)) return MODAL_RESULT_YES;

    int modal_res = MODAL_RESULT_NO;
    if (m_pTabs->modifiedByIndex(index)) {
#if defined(_WIN32)
        CSaveFileMessage saveDlg(gTopWinId);
#else
        CSaveFileMessage saveDlg(this);
#endif
        m_pTabs->setCurrentIndex(index);
        saveDlg.setFiles(m_pTabs->titleByIndex(index));

        modal_res = saveDlg.showModal();
        switch (modal_res) {
        case MODAL_RESULT_CANCEL: break;
        case MODAL_RESULT_NO: break;
        case MODAL_RESULT_YES:
        default:{
            m_pTabs->editorCloseRequest(index);

            QCefView * pView = (QCefView *)m_pTabs->widget(index);
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();

            pEvent->m_nType = ASC_MENU_EVENT_TYPE_CEF_SAVE;
            pView->GetCefView()->Apply(pEvent);

            break;}
        }
    }

    return modal_res;
}

void QAscMainPanel::onNeedCheckKeyboard()
{
    if (m_pManager) m_pManager->CheckKeyboard();
}

void QAscMainPanel::onMenuLogout()
{
//    if (m_pWidgetProfile->info()->logged()) {
//        if (checkModified(portal) != MODAL_RESULT_CANCEL) {
//            m_pManager->Logout(m_pWidgetProfile->info()->portal().toStdWString());

//            m_pButtonProfile->setDisabled(true);
//            m_pWidgetProfile->parseProfile("");

//            goStart();
//        }
//    }
}

void QAscMainPanel::doLogout(const QString& portal)
{
    m_pTabs->closePortal(portal, true);
    RecalculatePlaces();

    wstring wp = portal.toStdWString();
    m_pManager->Logout(wp);

    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(L"portal:logout");
    pCommand->put_Param(wp);

    CAscMenuEvent* pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    ((QCefView *)m_pMainWidget)->GetCefView()->Apply(pEvent);
    goStart();
}

void QAscMainPanel::onPortalLogout(QString portal)
{
    if (m_saveAction > 1) return;

    int _index_, _answ_;
    while (true) {
        _index_ = m_pTabs->findModified(portal);
        if ( _index_ < 0 ) {
            doLogout(portal);
            break;
        } else {
            _answ_ = trySaveDocument(_index_);

            if ( _answ_ == MODAL_RESULT_NO ) {
                m_pTabs->closeEditorByIndex(_index_, false);
                continue;
            } else
            if ( _answ_ == MODAL_RESULT_YES ) {
                m_saveAction = 1; // close portal
                m_savePortal = portal;
                break;
            } else
            if ( _answ_ == MODAL_RESULT_CANCEL ) {
                break;
            }
        }
    }
}

void QAscMainPanel::onCloudDocumentOpen(std::wstring url, int id, bool select)
{
    COpenOptions opts = {url};
    opts.id = id;

    m_pTabs->openCloudDocument(opts, select);

    if (id < 0)
        RecalculatePlaces();

    if (select)
        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
}

void QAscMainPanel::checkLocalUsedPath(int t)
{
    GET_REGISTRY_USER(_reg_user)

    QString _path_;
    if (t == LOCAL_PATH_OPEN) {
        _path_ = _reg_user.value("openPath").value<QString>();
        m_lastOpenPath = _path_.length() > 0 && !QDir(_path_).exists() ?
            _path_ : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    } else {
        _path_ = _reg_user.value("savePath").value<QString>();
        m_lastSavePath = _path_.length() > 0 && !QDir(_path_).exists() ?
            _path_ : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
}

void QAscMainPanel::onLocalGetImage(void * d)
{
#ifdef _WIN32
    CFileDialogWrapper dlg((HWND)parentWidget()->winId());
#else
    CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

    checkLocalUsedPath(LOCAL_PATH_OPEN);

    QString file_path = dlg.modalOpenImage(m_lastOpenPath);
    if (!file_path.isEmpty()) {
        m_lastOpenPath = QFileInfo(file_path).absoluteDir().absolutePath();
    }

    /* data consits id of cefview */
    CAscLocalOpenFileDialog * pData = static_cast<CAscLocalOpenFileDialog *>(d);

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
    pData->put_Path(file_path.toStdWString());
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);

    /* release would be made in the method Apply */
//    RELEASEINTERFACE(pData);
}

void QAscMainPanel::onLocalFileOpen(QString path)
{
#ifdef _WIN32
    CFileDialogWrapper dlg((HWND)parentWidget()->winId());
#else
    CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

    if (path.length() > 0 && QDir(path).exists()) {
        GET_REGISTRY_USER(_reg_user);

        m_lastOpenPath = QString(path);
        _reg_user.setValue("openPath", m_lastOpenPath);
    } else {
        checkLocalUsedPath(LOCAL_PATH_OPEN);
    }

    QString file_path = dlg.modalOpen(m_lastOpenPath);
    if (!file_path.isEmpty()) {
        m_lastOpenPath = QFileInfo(file_path).absoluteDir().absolutePath();

        COpenOptions opts = {"", etLocalFile, file_path};
        opts.wurl = file_path.toStdWString();
        doOpenLocalFile(opts);
    }
}

void QAscMainPanel::doOpenLocalFile(COpenOptions& opts)
{
    QFileInfo info(opts.url);
    if (!info.exists()) { return; }
    if (!info.isFile()) { return; }

    int result = m_pTabs->openLocalDocument(opts, true);
    if (!(result < 0)) {
        RecalculatePlaces();

        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
    } else
    if (result == -255) {
        CMessage mess(gTopWinId);
        mess.showModal(tr("File format not supported."), QMessageBox::Critical);
    }
}

void QAscMainPanel::onLocalFileRecent(void * d)
{
    CAscLocalOpenFileRecent_Recover * pData = static_cast<CAscLocalOpenFileRecent_Recover *>(d);

    COpenOptions opts = { pData->get_Path(),
          pData->get_IsRecover() ? etRecoveryFile : etRecentFile, pData->get_Id() };

    RELEASEINTERFACE(pData);

    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(opts.url);

    if (!match.hasMatch()) {
        if ( opts.type != etRecoveryFile && !QFileInfo(opts.url).exists() ) {
            CMessage mess(gTopWinId);
            mess.showModal(tr("File doesn't exists"), QMessageBox::Critical);
            return;
        }
    }

//    openLocalFile(opts);
    int result = m_pTabs->openLocalDocument(opts, true);
    if (!(result < 0)) {
        RecalculatePlaces();

        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
    } else
    if (result == -255) {
        CMessage mess(gTopWinId);
        mess.showModal(tr("File format not supported."), QMessageBox::Critical);
    }
}

void QAscMainPanel::onLocalFileCreate(int fformat)
{
    static short docx_count = 0;
    static short xlsx_count = 0;
    static short pptx_count = 0;

    /* check the active license */
    CAscLicenceActual * pData = new CAscLicenceActual;
    pData->AddRef();
    pData->put_Path(commonDataPath());
    pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL);
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);
    /* ************************* */

    pData->put_DaysBetween(1);
    if (!pData->get_Licence() || !pData->get_DaysLeft()) {
        doLicenseWarning(pData);
    } else {
        QString new_name;
        switch (fformat) {
        case etDocument: new_name  = tr("Document%1.docx").arg(++docx_count); break;
        case etSpreadsheet: new_name  = tr("Book%1.xlsx").arg(++xlsx_count); break;
        case etPresentation: new_name  = tr("Presentation%1.pptx").arg(++pptx_count); break;
        default: new_name = "Document.asc"; break;
        }

        COpenOptions opts = {new_name, etNewFile};
        opts.format = fformat;

        int tabIndex = m_pTabs->addEditor(opts);

        if (!(tabIndex < 0)) {
            m_pTabs->updateIcons();
            m_pTabs->setCurrentIndex(tabIndex);

            RecalculatePlaces();

            QTimer::singleShot(200, this, [=]{
                toggleButtonMain(false);
            });
        }
    }

    RELEASEINTERFACE(pData)
}

void QAscMainPanel::onLocalFilesOpen(void * data)
{
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    vector<wstring> vctFiles = pData->get_Files();

    doOpenLocalFiles(&vctFiles);

    RELEASEINTERFACE(pData);
}

void QAscMainPanel::doOpenLocalFiles(const vector<wstring> * vec)
{
    for (vector<wstring>::const_iterator i = vec->begin(); i != vec->end(); i++) {
        COpenOptions opts = {(*i), etLocalFile};
        m_lastOpenPath = QFileInfo(opts.url).absoluteDir().absolutePath();
        doOpenLocalFile(opts);
    }
}

void QAscMainPanel::doOpenLocalFiles(const QStringList& list)
{
    QStringListIterator i(list);
    while (i.hasNext()) {
        COpenOptions opts = {i.next().toStdWString(), etLocalFile};

        m_lastOpenPath = QFileInfo(opts.url).absoluteDir().absolutePath();
        doOpenLocalFile(opts);
    }
}

void QAscMainPanel::onDocumentType(int id, int type)
{
    m_pTabs->applyDocumentChanging(id, type);
}

void QAscMainPanel::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);

    QString name = QString::fromStdWString(pData->get_Name());
    QString descr = pData->get_Url().size() > 0 ? name : QString::fromStdWString(pData->get_Path());

    if (!descr.length()) descr = name;
    m_pTabs->applyDocumentChanging(pData->get_Id(), name, descr);
    onTabChanged(m_pTabs->currentIndex());

    RELEASEINTERFACE(pData);
}

void QAscMainPanel::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void QAscMainPanel::onDocumentSave(int id, bool cancel)
{
    m_pTabs->applyDocumentSave(id, cancel);

    if (!cancel && m_saveAction != 0) {
        if (m_saveAction == 1) {
            m_saveAction = 0;
            onPortalLogout(m_savePortal);
        } else
        if (m_saveAction == 2) {
            m_saveAction = 0;
            pushButtonCloseClicked();
        }
    } else {
        m_saveAction = 0;
    }
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
//    m_pTabs->closeAllEditors();
//    loadStartPage();
}

void QAscMainPanel::onActivate(QString key)
{
    doActivate(key);
}

void QAscMainPanel::onActivated(void * data)
{
    doLicenseWarning(data);

    CAscLicenceActual * pData = static_cast<CAscLicenceActual *>(data);
    RELEASEINTERFACE(pData)
}

void QAscMainPanel::doActivate(const QString& key)
{
    wstring sAppData = commonDataPath();
    if (sAppData.size()) {
        QDir().mkpath(QString::fromStdWString(sAppData));
        m_waitActiveLic = true;

        CAscLicenceKey * pData = new CAscLicenceKey;
        pData->AddRef();
        pData->put_Key(key.toStdString());
        pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);
        pData->put_Path(sAppData);

        CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_SEND_KEY);
        pEvent->m_pData = pData;
        m_pManager->Apply(pEvent);
    } else {
        CMessage mess(gTopWinId);
        mess.showModal(tr("Internal activation error"), QMessageBox::Critical);
    }
}

void QAscMainPanel::checkActivation()
{
    CAscLicenceActual * pData = new CAscLicenceActual;
    pData->AddRef();
    pData->put_Path(commonDataPath());
    pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL);
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);

    doLicenseWarning(pData);
    RELEASEINTERFACE(pData)
}

void QAscMainPanel::doLicenseWarning(void * data)
{
    CAscLicenceActual * pData = static_cast<CAscLicenceActual *>(data);

    CMessage mess(gTopWinId);
    if (m_waitActiveLic) {
        QString desr;
        if (pData->get_Licence()) {
            desr = tr("Activation successfully finished!");
            syncLicenseToJS(true);
        } else desr = tr("Activation failed!");

        mess.showModal(desr, QMessageBox::Information);
    } else
    if (!pData->get_Licence()) {
        syncLicenseToJS(false);

        mess.setButtons(tr("Buy Now"), "");
        if (201 == mess.showModal(tr("The program is unregistered"), QMessageBox::Information)) {
            onLink(URL_BUYNOW);
        }
    } else {
        if (pData->get_IsDemo()) {
            syncLicenseToJS(false, false);
            mess.setButtons(tr("Activate"), tr("Continue"));

            int res = 0;
            if (pData->get_DaysLeft() == 0)
                res = mess.showModal(tr("The trial period is over.").arg(pData->get_DaysLeft()), QMessageBox::Information); else
                res = mess.showModal(tr("Trial period expired for %1 days.").arg(pData->get_DaysLeft()), QMessageBox::Information);

            if (res == 201) {
                syncLicenseToJS(false, true);
                pushButtonMainClicked();
            }
        } else
        if (pData->get_DaysBetween() > 0) {
            // the license checked more then 1 day before
            if (pData->get_DaysLeft() == 0) {
                syncLicenseToJS(false, false);

                mess.setButtons(tr("Activate"), tr("Continue"));
                if (201 == mess.showModal(tr("The program is non-activated!"), QMessageBox::Information)) {
                    syncLicenseToJS(false, true);
                    pushButtonMainClicked();
                }
            } else
            if (pData->get_DaysLeft() < 15) {
                syncLicenseToJS(false);

                QString text = tr("%1 days left before the license end").arg(pData->get_DaysLeft());

                CMessage mess(gTopWinId);
                mess.setButtons(tr("Continue"), "");
                mess.showModal(text, QMessageBox::Information);
            } else {
                syncLicenseToJS(true);
            }
        } else {
            syncLicenseToJS(true);
        }
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

#ifndef QT_DEBUG
    std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
#else
  #ifdef _WIN32
//    std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
    std::wstring start_path = QString("file:///E:/Work/Projects/ASCDocumentEditor/common/loginpage/src/index.html").toStdWString();
//    std::wstring start_path = QString("file:///E:/Work/Projects/ASCDocumentEditor/common/loginpage/deploy/index.html").toStdWString();
  #elif __linux__
    #ifdef _IVOLGA_PRO
    std::wstring start_path = QString("file:///" + qgetenv("HOME") + "/QTProject/Desktop/common/loginpage/deploy/index.ivolga.html").toStdWString();
    #else
    std::wstring start_path = QString("file:///" + qgetenv("HOME") + "/QTProject/Desktop/common/loginpage/deploy/index.html").toStdWString();
    #endif

    start_path.append(additional.toStdWString());
  #endif
#endif
    ((QCefView*)m_pMainWidget)->GetCefView()->load(start_path);
}

void QAscMainPanel::goStart()
{
//    loadStartPage();
    toggleButtonMain(true);
}

int QAscMainPanel::checkModified(const QString& portalname)
{
    QMap<int, QString> mapModified = m_pTabs->modified(portalname);

    int out_res = MODAL_RESULT_YES;
    if (mapModified.size()) {
#ifdef _WIN32
        CSaveFileMessage saveDlg(gTopWinId);
#else
        CSaveFileMessage saveDlg(this);
#endif
        saveDlg.setFiles(&mapModified);

        out_res = saveDlg.showModal();
        switch (out_res) {
        case MODAL_RESULT_NO: break;
        case MODAL_RESULT_CANCEL: break;
        case MODAL_RESULT_YES:
        default:{
            if (mapModified.size()) {
                CCefView * pView;
                QMapIterator<int,QString> i(mapModified);
                while (i.hasNext()) {
                    i.next();

                    pView = m_pManager->GetViewById(i.key());
                    pView->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));
                }
            }

            break;}
        }
    }

    return out_res;
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

    QString savePath = _reg_user.value("savePath").value<QString>();
    if (savePath.isEmpty() || !QDir(savePath).exists())
        savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    static bool saveInProcess = false;
    if (!saveInProcess) {
        saveInProcess = true;

        if (sName.size()) {
            QString fullPath = savePath + "/" + QString().fromStdWString(sName);
#ifdef _WIN32
            CFileDialogWrapper dlg(gTopWinId);
#else
            CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

            if (dlg.modalSaveAs(fullPath)) {
                savePath = QFileInfo(fullPath).absolutePath();
                _reg_user.setValue("savePath", savePath);
            }

            m_pManager->EndSaveDialog(fullPath.toStdWString(), id);
        }

        saveInProcess = false;
    }
}

void QAscMainPanel::onLocalFileSaveAs(void * d)
{
    CAscLocalSaveFileDialog * pData = static_cast<CAscLocalSaveFileDialog *>(d);

    QFileInfo info(QString::fromStdWString(pData->get_Path()));
    if (info.absoluteDir().exists()) {
        m_lastSavePath = info.absoluteDir().absolutePath();
    }

    if (!QDir(m_lastSavePath).exists()) {
        m_lastSavePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    if (info.fileName().size()) {
        QString fullPath = m_lastSavePath + "/" + info.fileName();

#ifdef _WIN32
        CFileDialogWrapper dlg(gTopWinId);
#else
        CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif
        dlg.setFormats(pData->get_SupportFormats());

        CAscLocalSaveFileDialog * pSaveData = new CAscLocalSaveFileDialog();
        pSaveData->put_Id(pData->get_Id());
        pSaveData->put_Path(L"");

        if (dlg.modalSaveAs(fullPath)) {
            GET_REGISTRY_USER(_reg_user);

            m_lastSavePath = QDir(fullPath).absolutePath();
            _reg_user.setValue("savePath", m_lastSavePath);

            pSaveData->put_Path(fullPath.toStdWString());
            int format = dlg.getFormat() > 0 ? dlg.getFormat() :
                    CAscApplicationManager::GetFileFormatByExtentionForSave(pSaveData->get_Path());

            pSaveData->put_FileType(format > -1 ? format : 0);
        }

        CAscMenuEvent* pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
        pEvent->m_pData = pSaveData;
        m_pManager->Apply(pEvent);

//        RELEASEINTERFACE(pData)
//        RELEASEINTERFACE(pEvent)
    }

    RELEASEINTERFACE(pData);
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

void QAscMainPanel::onPortalOpen(QString url)
{
    int res = m_pTabs->openPortal(url);
    if (res == 2) { RecalculatePlaces(); }

    if (!(res < 0)) {
        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
    }
}

void QAscMainPanel::fillUserName(QString& firstname, QString& lastname)
{
    QString _full_name = qgetenv("USER");
    if (_full_name.isEmpty())
        _full_name = qgetenv("USERNAME");

    if (_full_name.isEmpty())
        _full_name = "Unknown.User";

    QRegularExpression re(reUserName);
    QRegularExpressionMatch match = re.match(_full_name);

    if (match.hasMatch()) {
        firstname = match.captured(1);
        lastname = match.captured(2);
    }
}

wstring QAscMainPanel::commonDataPath() const
{
    std::wstring sAppData(L"");
#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
        sAppData = std::wstring(szPath);
        std::replace(sAppData.begin(), sAppData.end(), '\\', '/');
        sAppData.append(QString(APP_LICENSE_PATH).toStdWString());
    }

    if (sAppData.size()) {
        QDir().mkpath(QString::fromStdWString(sAppData));
    }

#else
    sAppData = QString("/var/lib").append(APP_DATA_PATH).toStdWString();
    QFileInfo fi(QString::fromStdWString(sAppData));
    if (fi.isDir() && !fi.isWritable()) {
        // TODO: check directory permissions and warn the user
        qDebug() << "directory permission error";
    }
#endif

    return sAppData;
}

void QAscMainPanel::syncLicenseToJS(bool active, bool proceed)
{
    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(L"lic:active");
    pCommand->put_Param(QString::number(active).toStdWString());

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    ((QCefView *)m_pMainWidget)->GetCefView()->Apply(pEvent);

    if (!active && proceed) {
        pCommand = new CAscExecCommandJS;
        pCommand->put_Command(L"lic:selectpanel");

        pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;

        ((QCefView *)m_pMainWidget)->GetCefView()->Apply(pEvent);
    }
}

void QAscMainPanel::selfActivation()
{
    CAscLicenceActual * pData = new CAscLicenceActual;
    pData->put_Path(commonDataPath());
    pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_DEMO);
    pEvent->m_pData = pData;

    m_pManager->Apply(pEvent);
}
