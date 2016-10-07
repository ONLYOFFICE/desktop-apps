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

#include "cmainpanel.h"

#include <QPrinterInfo>
#include <QPrintDialog>
#include <QDir>
#include <QMenu>
#include <QWidgetAction>
#include <QTimer>
#include <QStandardPaths>
#include <QApplication>
#include <QRegularExpression>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <regex>
#include <QStorageInfo>

#include "defines.h"
#include "cprintprogress.h"
#include "cfiledialog.h"
#include "qascprinter.h"
#include "common/Types.h"
#include "utils.h"
#include "version.h"
#include "cmessage.h"

#ifdef _WIN32
#include "win/cprintdialog.h"
#include "shlobj.h"
#include "lmcons.h"

extern HWND gTopWinId;
#else
#define VK_F4 0x73
#define gTopWinId this
#include "linux/cx11decoration.h"
#endif

using namespace NSEditorApi;

#define BUTTON_MAIN_WIDTH   68
#define TITLE_HEIGHT        29
#define TOOLBTN_HEIGHT      29

#define HTML_QUOTE "\\u005c&quot;" // \" symbols

extern BYTE     g_dpi_ratio;
extern QString  g_lang;

struct printdata {
public:
    printdata() : _print_range(QPrintDialog::PrintRange::AllPages) {}
    QPrinterInfo _printer_info;
    QPrintDialog::PrintRange _print_range;
};

CMainPanel::CMainPanel(QWidget *parent, CAscApplicationManager *manager, bool isCustomWindow)
    : QWidget(parent),
        m_pButtonMinimize(NULL), m_pButtonMaximize(NULL), m_pButtonClose(NULL),
        m_pButtonDownload(new CPushButton),
        m_isMaximized(false), m_isCustomWindow(isCustomWindow),
        m_pWidgetDownload(new CDownloadWidget)
      , m_printData(new printdata)
      , m_mainWindowState(Qt::WindowNoState)
      , m_inFiles(NULL)
      , m_saveAction(0)
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
    connect(m_pTabs, &CAscTabWidget::closeAppRequest, this, &CMainPanel::onAppCloseRequest);

    QSize small_btn_size(28*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
//    QSize wide_btn_size(29*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);

    // download
    m_pButtonDownload->setObjectName("toolButtonDownload");
//    m_pButtonDownload->setFixedSize(wide_btn_size);
    m_pButtonDownload->setFixedSize(QSize(33*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio));
    m_pButtonDownload->setAnimatedIcon(
                g_dpi_ratio > 1 ? ":/res/icons/downloading_2x.gif" : ":/res/icons/downloading.gif" );

#ifdef __linux__
    m_boxTitleBtns = new CX11Caption(centralWidget);
#else
    m_boxTitleBtns = new QWidget(centralWidget);
#endif
    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(APP_TITLE);
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    layoutBtns->setContentsMargins(0,0,4*g_dpi_ratio,0);
    layoutBtns->setSpacing(1*g_dpi_ratio);
    layoutBtns->addWidget(label);
    layoutBtns->addWidget(m_pButtonDownload);

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

        auto _creatToolButton = [small_btn_size](const QString& name, QWidget * parent) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");
            btn->setFixedSize(small_btn_size);

            return btn;
        };

        // Minimize
        m_pButtonMinimize = _creatToolButton("toolButtonMinimize", centralWidget);
        QObject::connect(m_pButtonMinimize, &QPushButton::clicked, this, &CMainPanel::pushButtonMinimizeClicked);

        // Maximize
        m_pButtonMaximize = _creatToolButton("toolButtonMaximize", centralWidget);
        QObject::connect(m_pButtonMaximize, &QPushButton::clicked, this, &CMainPanel::pushButtonMaximizeClicked);

        // Close
        m_pButtonClose = _creatToolButton("toolButtonClose", centralWidget);
        QObject::connect(m_pButtonClose, &QPushButton::clicked, this, &CMainPanel::pushButtonCloseClicked);

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

#ifdef __linux__
        mainGridLayout->setMargin( CX11Decoration::customWindowBorderWith() );

        QPalette _palette(parent->palette());
        _palette.setColor(QPalette::Background, QColor(0x31, 0x34, 0x37));
        parent->setAutoFillBackground(true);
        parent->setPalette(_palette);

        connect(m_boxTitleBtns, SIGNAL(mouseDoubleClicked()), this, SLOT(pushButtonMaximizeClicked()));
#endif

        m_boxTitleBtns->setFixedSize(282*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
    } else {
#ifdef __linux__
        _tabs_stylesheet_file += "tabbar.nix.qss";
#endif
        m_pButtonMain->setProperty("theme", "light");

        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));

        palette.setBrush(QPalette::Background, QBrush(gradient));

        label->setFixedHeight(0);
        m_boxTitleBtns->setFixedSize(342*g_dpi_ratio, 16*g_dpi_ratio);
    }

    QFile styleFile(_tabs_stylesheet_file);
    styleFile.open( QFile::ReadOnly );
    m_pTabs->setStyleSheet(QString(styleFile.readAll()));
    m_pTabs->setAutoFillBackground(true);
    m_pTabs->setPalette(palette);
    m_pTabs->applyCustomTheme(isCustomWindow);

    styleFile.close();

    // download menu
    QMenu * menuDownload = new QMenu();
    QWidgetAction * waction = new QWidgetAction(menuDownload);
    waction->setDefaultWidget(m_pWidgetDownload);
    menuDownload->setObjectName("menuButtonDownload");
    menuDownload->addAction(waction);

    m_pButtonDownload->setMenu(menuDownload);
    m_pWidgetDownload->setManagedElements(m_pManager, m_pButtonDownload);

    CProfileMenuFilter * eventFilter = new CProfileMenuFilter(this);
    eventFilter->setMenuButton(m_pButtonDownload);
    menuDownload->installEventFilter(eventFilter);

    QCefView * pMainWidget = new QCefView(centralWidget);
    pMainWidget->Create(m_pManager, cvwtSimple);
    pMainWidget->setObjectName( "mainPanel" );
    pMainWidget->setHidden(false);

    m_pMainWidget = (QWidget *)pMainWidget;
    m_pTabs->m_pMainButton = m_pButtonMain;

//    m_pMainWidget->setVisible(false);

    mainGridLayout->addWidget( centralWidget );

    RecalculatePlaces();
    loadStartPage();

//    m_pTabs->addEditor("editor1 editor21", etDocument, L"https://testinfo.teamlab.info");
//    m_pTabs->addEditor("editor2", etPresentation, L"http://google.com");
//    m_pTabs->addEditor("editor3", etSpreadsheet, L"http://google.com");
//    m_pTabs->updateIcons();

    m_pButtonDownload->setVisible(false, false);

    wstring first_name, last_name;
    readSystemUserName(first_name, last_name);

    QString params = QString("lang=%1&userfname=%3&userlname=%4&location=%2")
                        .arg(g_lang, Utils::systemLocationCode());

    wstring wparams = params.toStdWString();
    wparams.replace(wparams.find(L"%3"), 2, first_name);
    wparams.replace(wparams.find(L"%4"), 2, last_name);
    m_pManager->InitAdditionalEditorParams(wparams);

    m_saveDocMessage = tr("%1 is modified.<br>Do you want to keep changes?");
}

void CMainPanel::RecalculatePlaces()
{
    int cbw = 0;

#ifdef __linux
    QWidget * cw = findChild<QWidget *>("centralWidget");
    int windowW = cw->width(),
        windowH = cw->height(),
#else
    int windowW = width(),
        windowH = height(),
#endif
        captionH = TITLE_HEIGHT * g_dpi_ratio,
        btnMainWidth = BUTTON_MAIN_WIDTH * g_dpi_ratio;

    m_pTabs->setGeometry(cbw, cbw, windowW, windowH);

    int docCaptionW = windowW - m_pTabs->tabBar()->width() - btnMainWidth;
    int contentH = windowH - captionH;

    if (docCaptionW < 1)
        docCaptionW = 1;
    if (contentH < 1)
        contentH = 1;

    m_boxTitleBtns->setFixedSize(docCaptionW, TOOLBTN_HEIGHT * g_dpi_ratio);
    m_boxTitleBtns->move(windowW - m_boxTitleBtns->width() + cbw, cbw);
    m_pMainWidget->setGeometry(cbw, captionH + cbw, windowW, contentH);
}

#ifdef __linux
QWidget * CMainPanel::getTitleWidget()
{
    return m_boxTitleBtns;
}

void CMainPanel::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
    findChild<QWidget *>("centralWidget")->setMouseTracking(enable);
    findChild<QLabel *>("labelAppTitle")->setMouseTracking(enable);

    m_boxTitleBtns->setMouseTracking(enable);
    m_pTabs->setMouseTracking(enable);
    m_pTabs->tabBar()->setMouseTracking(enable);
    m_pButtonMain->setMouseTracking(enable);
    m_pButtonClose->setMouseTracking(enable);
    m_pButtonMinimize->setMouseTracking(enable);
    m_pButtonMaximize->setMouseTracking(enable);
    m_pMainWidget->setMouseTracking(enable);
}
#endif

void CMainPanel::pushButtonMinimizeClicked()
{
    emit mainWindowChangeState(Qt::WindowMinimized);
}

void CMainPanel::pushButtonMaximizeClicked()
{
    if (m_mainWindowState == Qt::WindowMaximized) {
        emit mainWindowChangeState(Qt::WindowNoState);
    } else {
        emit mainWindowChangeState(Qt::WindowMaximized);
    }
}

void CMainPanel::pushButtonCloseClicked()
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
                if (m_mainWindowState == Qt::WindowMinimized)
                    emit mainWindowChangeState(Qt::WindowNoState);

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

void CMainPanel::onAppCloseRequest()
{
    onFullScreen(false);
    pushButtonCloseClicked();
}

void CMainPanel::applyMainWindowState(Qt::WindowState s)
{
    m_mainWindowState = s;

    if ( m_isCustomWindow ) {
#ifdef __linux__
        layout()->setMargin(s == Qt::WindowMaximized ? 0 : CX11Decoration::customWindowBorderWith());
#endif

        m_pButtonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }
}

void CMainPanel::pushButtonMainClicked()
{
    if (m_pTabs->isActive()) {
        m_pTabs->activate(false);
        m_pMainWidget->setHidden(false);

        ((QCefView *)m_pMainWidget)->GetCefView()->focus();
        onTabChanged(m_pTabs->currentIndex());
    }
}

void CMainPanel::toggleButtonMain(bool toggle)
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

void CMainPanel::focus() {
    if (m_pTabs->isActive()) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView *)m_pMainWidget)->GetCefView()->focus();
    }
}

void CMainPanel::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    RecalculatePlaces();
}

void CMainPanel::onTabClicked(int index)
{
    Q_UNUSED(index)

    if (!m_pTabs->isActive()) {
        toggleButtonMain(false);
    }
}

void CMainPanel::onTabClosed(int index, int curcount)
{
    Q_UNUSED(index)

    if (curcount == 0) {
        toggleButtonMain(true);
    }

    onTabChanged(m_pTabs->currentIndex());
    RecalculatePlaces();
}

CAscApplicationManager * CMainPanel::getAscApplicationManager()
{
    return m_pManager;
}

void CMainPanel::onTabChanged(int index)
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

void CMainPanel::onTabCloseRequest(int index)
{
    if (trySaveDocument(index) == MODAL_RESULT_NO)
        m_pTabs->closeEditorByIndex(index, false);
}

int CMainPanel::trySaveDocument(int index)
{
    if (m_pTabs->closedByIndex(index)) return MODAL_RESULT_YES;

    int modal_res = MODAL_RESULT_NO;
    if (m_pTabs->modifiedByIndex(index)) {
#if defined(_WIN32)
        CMessage mess(gTopWinId);
#else
        CMessage mess(this);
#endif
        m_pTabs->setCurrentIndex(index);

        mess.setButtons({tr("Yes")+":default", tr("No"), tr("Cancel")});
        modal_res = mess.warning(m_saveDocMessage.arg(m_pTabs->titleByIndex(index)));

        switch (modal_res) {
        case MODAL_RESULT_CANCEL: break;
        case MODAL_RESULT_CUSTOM + 1: modal_res = MODAL_RESULT_NO; break;
        case MODAL_RESULT_CUSTOM + 2: modal_res = MODAL_RESULT_CANCEL; break;
        case MODAL_RESULT_CUSTOM + 0:
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

void CMainPanel::onNeedCheckKeyboard()
{
    if (m_pManager) m_pManager->CheckKeyboard();
}

void CMainPanel::doLogout(const QString& portal, bool allow)
{
    wstring wp = portal.toStdWString();
    wstring wcmd = L"portal:logout";

    if (allow) {
        m_pTabs->closePortal(portal, true);
        RecalculatePlaces();

        m_pManager->Logout(wp);
    } else
        wcmd.append(L":cancel");

    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(wcmd);
    pCommand->put_Param(wp);

    CAscMenuEvent* pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    ((QCefView *)m_pMainWidget)->GetCefView()->Apply(pEvent);
    goStart();
}

void CMainPanel::onPortalLogout(QString portal)
{
    if (m_saveAction > 1) return;

    int _index_, _answ_;
    bool _allow = true;
    while (true) {
        _index_ = m_pTabs->findModified(portal);
        if ( _index_ < 0 ) {
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
                return;
            } else
            if ( _answ_ == MODAL_RESULT_CANCEL ) {
                _allow = false;
                break;
            }
        }
    }

    doLogout(portal, _allow);
}

void CMainPanel::onCloudDocumentOpen(std::wstring url, int id, bool select)
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

void CMainPanel::onLocalGetImage(void * d)
{
#ifdef _WIN32
    CFileDialogWrapper dlg((HWND)parentWidget()->winId());
#else
    CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

    QString file_path = dlg.modalOpenImage(Utils::lastPath(LOCAL_PATH_OPEN));
    if (!file_path.isEmpty()) {
        Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(file_path).absolutePath());
    }

    /* data consits id of cefview */
    CAscLocalOpenFileDialog * pData = static_cast<CAscLocalOpenFileDialog *>(d);
    pData->put_Path(file_path.toStdWString());

    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
    pEvent->m_pData = pData;
    m_pManager->Apply(pEvent);

    /* release would be made in the method Apply */
//    RELEASEINTERFACE(pData);
}

void CMainPanel::onLocalFileOpen(const QString& inpath)
{
#ifdef _WIN32
    CFileDialogWrapper dlg((HWND)parentWidget()->winId());
#else
    CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

    QString _path = !inpath.isEmpty() && QDir(inpath).exists() ?
                        inpath : Utils::lastPath(LOCAL_PATH_OPEN);

    if (!(_path = dlg.modalOpen(_path)).isEmpty()) {
        Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());

        COpenOptions opts = {"", etLocalFile, _path};
        opts.wurl = _path.toStdWString();
        doOpenLocalFile(opts);
    }
}

void CMainPanel::doOpenLocalFile(COpenOptions& opts)
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
        CMessage::error(gTopWinId, tr("File format not supported."));
    }
}

void CMainPanel::onLocalFileRecent(void * d)
{
    CAscLocalOpenFileRecent_Recover * pData = static_cast<CAscLocalOpenFileRecent_Recover *>(d);

    COpenOptions opts = { pData->get_Path(),
          pData->get_IsRecover() ? etRecoveryFile : etRecentFile, pData->get_Id() };

    RELEASEINTERFACE(pData);

    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(opts.url);

    if (!match.hasMatch()) {
        if ( opts.type != etRecoveryFile && !QFileInfo(opts.url).exists() ) {
            CMessage::error(gTopWinId, tr("File doesn't exists"));
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
        CMessage::error(gTopWinId, tr("File format not supported."));
    }
}

void CMainPanel::onLocalFileCreate(int fformat)
{
    static short docx_count = 0;
    static short xlsx_count = 0;
    static short pptx_count = 0;

    {
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
}

void CMainPanel::onLocalFilesOpen(void * data)
{
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    vector<wstring> vctFiles = pData->get_Files();

    doOpenLocalFiles(&vctFiles);

    RELEASEINTERFACE(pData);
}

void CMainPanel::onLocalFilesCheck(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();

        if ( objRoot.size() ) {
            QTimer::singleShot(10, this, [=]{
                QJsonObject _json_obj;
                QString _file_name;
                foreach (QString s, objRoot.keys() ) {
                    _file_name = objRoot[s].toString();

                    QStorageInfo storage(_file_name);
                    // check file is local
#ifdef Q_OS_WIN
                    if (storage.device().startsWith("\\\\?\\")) {
#else
                    if (storage.device().startsWith("/dev/")) {
#endif
                        QFileInfo info(_file_name);
                        if (!info.exists()) _json_obj[s] = "false";
                    }
                }

                if ( _json_obj.size() ) {
                    cmdMainPage("files:checked", Utils::encodeJson(_json_obj));
                }
            });
        }
    }
}

void CMainPanel::doOpenLocalFiles(const vector<wstring> * vec)
{
    if (qApp->activeModalWidget()) return;

    for (wstring wstr : (*vec)) {
        COpenOptions opts = {wstr, etLocalFile};
        doOpenLocalFile(opts);
    }

    if (vec->size())
        Utils::keepLastPath(LOCAL_PATH_OPEN,
                    QFileInfo(QString::fromStdWString(vec->back())).absolutePath());
}

void CMainPanel::doOpenLocalFiles(const QStringList& list)
{
    if (qApp->activeModalWidget()) return;

    QStringListIterator i(list);
    while (i.hasNext()) {
        COpenOptions opts = {i.next().toStdWString(), etLocalFile};
        doOpenLocalFile(opts);
    }

    i.toBack();
    if (i.hasPrevious()) {
        Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(i.peekPrevious()).absolutePath());
    }
}

void CMainPanel::doOpenLocalFiles()
{
    if ( m_inFiles ) {
        if ( m_inFiles->size() )
            doOpenLocalFiles( *m_inFiles );

        RELEASEOBJECT(m_inFiles)
    }
}

void CMainPanel::onDocumentType(int id, int type)
{
    m_pTabs->applyDocumentChanging(id, type);
}

void CMainPanel::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);

    QString name = QString::fromStdWString(pData->get_Name());
    QString descr = pData->get_Url().size() > 0 ? name : QString::fromStdWString(pData->get_Path());

    if (!descr.length()) descr = name;
    m_pTabs->applyDocumentChanging(pData->get_Id(), name, descr);
    onTabChanged(m_pTabs->currentIndex());

    RELEASEINTERFACE(pData);
}

void CMainPanel::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void CMainPanel::onDocumentSave(int id, bool cancel)
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

void CMainPanel::onDocumentDownload(void * info)
{
    m_pWidgetDownload->downloadProcess(info);

    NSEditorApi::CAscDownloadFileInfo * pData = reinterpret_cast<NSEditorApi::CAscDownloadFileInfo *>(info);
    RELEASEINTERFACE(pData);
}

void CMainPanel::onLogin(QString params)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(params.toUtf8(), &jerror);

    QString _portal;
    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();
        _portal = objRoot["portal"].toString();
    }

    GET_REGISTRY_USER(_reg_user)
    _reg_user.setValue("portal", _portal);
}

void CMainPanel::loadStartPage()
{
    GET_REGISTRY_USER(_reg_user);

#if defined(QT_DEBUG)
    QString data_path = _reg_user.value("startpage").value<QString>();
#else
# ifdef _WIN32
    QString data_path = QString().fromStdWString(m_pManager->m_oSettings.app_data_path) + "/webdata/local/index.html";
//    data_path = "ascdesktop://login.html";
# elif __linux__
    QString data_path = qApp->applicationDirPath() + "/index.html";
# endif
#endif

    QString additional = "?waitingloader=yes";
    if (!g_lang.isEmpty())
        additional.append("&lang=" + g_lang);

    QString _portal = _reg_user.value("portal").value<QString>();
    if (!_portal.isEmpty()) {
        QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
        additional.append(arg_portal);
    }


    std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
    ((QCefView*)m_pMainWidget)->GetCefView()->load(start_path);
}

void CMainPanel::goStart()
{
//    loadStartPage();
    toggleButtonMain(true);
}

void CMainPanel::onDocumentPrint(void * opts)
{
    static bool printInProcess = false;
    if (!printInProcess)
        printInProcess = true; else
        return;

    CAscPrintEnd * pData = (CAscPrintEnd *)opts;
    CCefView * pView = m_pManager->GetViewById(pData->get_Id());

    int pagesCount = pData->get_PagesCount(),
        currentPage = pData->get_CurrentPage();

    if (pView && !(pagesCount < 1)) {
//#ifdef _WIN32
        NSEditorApi::CAscMenuEvent * pEvent;
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
        dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintCurrentPage | QPrintDialog::PrintToFile);
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

                CAscPrintPage * pData;

#if defined(_WIN32)
//                EnableWindow(parentWindow(), FALSE);
                EnableWindow((HWND)parentWidget()->winId(), FALSE);

                CPrintProgress progressDlg((HWND)parentWidget()->winId());
#else
                CPrintProgress progressDlg(qobject_cast<QWidget *>(parent()));
#endif
                progressDlg.startProgress();

                uint count = finish - start;
                if (pContext->BeginPaint()) {
                    for (; !(start > finish); ++start) {
                        pContext->AddRef();

                        progressDlg.setProgress(count - (finish - start) + 1, count + 1);
                        qApp->processEvents();

                        pData = new NSEditorApi::CAscPrintPage();
                        pData->put_Context(pContext);
                        pData->put_Page(start - 1);

                        pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_PAGE);
                        pEvent->m_pData = pData;

                        pView->Apply(pEvent);
//                        RELEASEOBJECT(pData)
//                        RELEASEOBJECT(pEvent)

                        if (progressDlg.isRejected())
                            break;

                        start < finish && printer->newPage();
                    }
                    pContext->EndPaint();
                }

#if defined(_WIN32)
//                EnableWindow(parentWindow(), TRUE);
                EnableWindow((HWND)parentWidget()->winId(), TRUE);
#endif
            } else {
                // TODO: show error message
            }
        }

        pContext->Release();

        pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_END);
        pView->Apply(pEvent);
//        RELEASEOBJECT(pEvent)

#ifndef _WIN32
        RELEASEOBJECT(dialog)
#endif
    }

    printInProcess = false;
    RELEASEINTERFACE(pData)
}

void CMainPanel::onDialogSave(std::wstring sName, uint id)
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

void CMainPanel::onLocalFileSaveAs(void * d)
{
    CAscLocalSaveFileDialog * pData = static_cast<CAscLocalSaveFileDialog *>(d);

    QFileInfo info( QString::fromStdWString(pData->get_Path()) );
    if ( info.fileName().size() ) {
        QString _lastSavePath = Utils::lastPath(LOCAL_PATH_SAVE);
        if ( !QDir(_lastSavePath).exists() ) {
            _lastSavePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }

        QString fullPath = _lastSavePath + "/" + info.fileName();

#ifdef _WIN32
        CFileDialogWrapper dlg(gTopWinId);
#else
        CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif
        dlg.setFormats(pData->get_SupportFormats());

        CAscLocalSaveFileDialog * pSaveData = new CAscLocalSaveFileDialog();
        pSaveData->put_Id(pData->get_Id());
        pSaveData->put_Path(L"");

        if ( dlg.modalSaveAs(fullPath) ) {
            Utils::keepLastPath(LOCAL_PATH_SAVE, QFileInfo(fullPath).absoluteDir().absolutePath());

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

void CMainPanel::onFullScreen(bool apply)
{
    if (apply) {
        m_isMaximized = m_mainWindowState == Qt::WindowMaximized;

        m_pTabs->setFullScreen(apply);
        emit mainWindowChangeState(Qt::WindowFullScreen);
    } else {
        emit mainWindowChangeState(m_isMaximized ? Qt::WindowMaximized : Qt::WindowNoState);
        m_pTabs->setFullScreen(apply);
    }
}

void CMainPanel::onKeyDown(void * eventData)
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

void CMainPanel::onLink(QString url)
{
    Utils::openUrl(url);
}

void CMainPanel::onPortalOpen(QString url)
{
    int res = m_pTabs->openPortal(url);
    if (res == 2) { RecalculatePlaces(); }

    if (!(res < 0)) {
        QTimer::singleShot(200, this, [=]{
            toggleButtonMain(false);
        });
    }
}

void CMainPanel::onPortalNew(QString in)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(in.toLatin1(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();

        QString _f_name = objRoot["firstName"].toString(),
                _l_name = objRoot["lastName"].toString(),
                _lang = objRoot["language"].toString(),
                _full_name;

        if ( _f_name.isEmpty() )
            _full_name = _l_name; else
        if ( _l_name.isEmpty() )
            _full_name = _f_name; else
        {
            _full_name = _lang.contains("ru", Qt::CaseInsensitive) ?
                                _l_name + ' ' + _f_name : _f_name + ' ' + _l_name;
        }

        QString _domain = Utils::getPortalName(objRoot["domain"].toString());
        int _tab_index = m_pTabs->tabIndexByEditorType(etNewPortal);
        if ( !(_tab_index < 0)) {
            int _uid = m_pTabs->viewByIndex(_tab_index);
            m_pTabs->applyDocumentChanging(_uid, _domain, _domain);
            m_pTabs->applyDocumentChanging(_uid, etPortal);

            onTabChanged(m_pTabs->currentIndex());
        }

        QJsonObject _json_obj;
        _json_obj["displayName"]    = _full_name;
        _json_obj["email"]          = objRoot.value("email").toString();
        _json_obj["domain"]         = _domain;

        QTimer::singleShot(20, this, [=]{
            cmdMainPage("portal:new", Utils::encodeJson(_json_obj));
        });
    }
}

void CMainPanel::onPortalCreate()
{
    int res = m_pTabs->newPortal(URL_SIGNUP, tr("Sign Up"));
    if (res == 2) { RecalculatePlaces(); }

    QTimer::singleShot(200, this, [=]{
        toggleButtonMain(false);
    });
}

void CMainPanel::readSystemUserName(wstring& first, wstring& last)
{
#ifdef Q_OS_WIN
    WCHAR _env_name[UNLEN + 1]{0};
    DWORD _size = UNLEN + 1;

    wstring _full_name = GetUserName(_env_name, &_size) ?
                            wstring(_env_name) : L"Unknown.User";
#else
    QString _env_name = qgetenv("USER");
    if (_env_name.isEmpty())
        _env_name = qgetenv("USERNAME");

    if (_env_name.isEmpty())
        _env_name = "Unknown.User";

    wstring _full_name = _env_name.toStdWString();
#endif
//    std::wregex _rexp(QString(reUserName).toStdWString());
//    std::wsmatch _res;
//    if (std::regex_search(_full_name, _res, _rexp)) {
//        first = _res.str(1),
//        last = _res.str(2);
//    }

    auto i = _full_name.find('.');
    i == wstring::npos ? first.assign(_full_name) :
                (first.assign(_full_name.substr(0, i)), last.assign(_full_name.substr(++i)));
}

void CMainPanel::onMainPageReady()
{
    QTimer::singleShot(20, this, [=]{
        refreshAboutVersion();
        emit mainPageReady();

        cmdMainPage("app:ready", "");
        doOpenLocalFiles();
    });
}

void CMainPanel::refreshAboutVersion()
{
    QString _license = "Licensed under &lt;a onclick=" HTML_QUOTE "window.open('" URL_AGPL "')" HTML_QUOTE
                            " href=" HTML_QUOTE "#" HTML_QUOTE "&gt;GNU AGPL v3&lt;/a&gt;";

    QJsonObject _json_obj;
    _json_obj["version"]    = VER_FILEVERSION_STR;
    _json_obj["edition"]    = "%1";
    _json_obj["appname"]    = WINDOW_NAME;
    _json_obj["rights"]     = "© " ABOUT_COPYRIGHT_STR;
    _json_obj["link"]       = URL_SITE;

    cmdMainPage("app:version", Utils::encodeJson(_json_obj).arg(_license));
}

void CMainPanel::setInputFiles(QStringList * list)
{
    RELEASEOBJECT(m_inFiles)
    m_inFiles = list;
}

void CMainPanel::cmdMainPage(const QString& cmd, const QString& args) const
{
    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(cmd.toStdWString());
    if (args.size())
        pCommand->put_Param(args.toStdWString());

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    ((QCefView *)m_pMainWidget)->GetCefView()->Apply(pEvent);

//    RELEASEOBJECT(pEvent)
//    RELEASEOBJECT(pCommand)
}

void CMainPanel::cmdAppManager(int cmd, void * data)
{
    CAscMenuEvent * pEvent = new CAscMenuEvent(cmd);
    pEvent->m_pData = static_cast<IMenuEventDataBase *>(data);
    m_pManager->Apply(pEvent);
}
