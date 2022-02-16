/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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
#include <QStorageInfo>
#include <regex>
#include <functional>

#include "defines.h"
#include "cprintprogress.h"
#include "cfiledialog.h"
#include "qascprinter.h"
#include "common/Types.h"
#include "utils.h"
#include "version.h"
#include "cmessage.h"
#include "clangater.h"
#include "cascapplicationmanagerwrapper.h"
#include "../Common/OfficeFileFormats.h"

#ifdef _WIN32
#include "win/cprintdialog.h"
#include "shlobj.h"

#else
#define gTopWinId this
#include "linux/cx11decoration.h"
#endif

using namespace NSEditorApi;
using namespace std::placeholders;

#define QCEF_CAST(Obj) qobject_cast<QCefView *>(Obj)
#ifdef _WIN32
#define TOP_NATIVE_WINDOW_HANDLE HWND(parentWidget()->property("handleTopWindow").toInt())
#else
#define TOP_NATIVE_WINDOW_HANDLE this
//#define TOP_NATIVE_WINDOW_HANDLE qobject_cast<QWidget *>(parent())
#endif


struct printdata {
public:
    printdata() : _print_range(QPrintDialog::PrintRange::AllPages) {}
    QPrinterInfo _printer_info;
    QPrintDialog::PrintRange _print_range;
};

CMainPanel::CMainPanel(QWidget *parent, bool isCustomWindow, double dpi_ratio)
    : QWidget(parent),
      CScalingWrapper(dpi_ratio)
      , m_isCustomWindow(isCustomWindow)
      , m_printData(new printdata)
      , m_mainWindowState(Qt::WindowNoState)
      , m_inFiles(NULL)
      , m_saveAction(0)
{
    setObjectName("mainPanel");
    setProperty("uitheme", QString::fromStdWString(AscAppManager::themes().current().id()));
    m_pMainGridLayout = new QGridLayout(this);
    m_pMainGridLayout->setSpacing(0);
    m_pMainGridLayout->setObjectName(QString::fromUtf8("mainGridLayout"));
    m_pMainGridLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pMainGridLayout);

    // Set custom TabBar
    m_pTabBarWrapper = new CTabBarWrapper(this);
    m_pTabBarWrapper->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_pMainGridLayout->addWidget(m_pTabBarWrapper, 0, 1, 1, 1);

//    QSize wide_btn_size(29*g_dpi_ratio, TOOLBTN_HEIGHT*g_dpi_ratio);
    QPalette palette;
#ifdef __linux__
    m_boxTitleBtns = new CX11Caption(this);
#else
    m_boxTitleBtns = new QWidget(this);
#endif
    m_boxTitleBtns->setObjectName("CX11Caption");
    m_boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_pMainGridLayout->addWidget(m_boxTitleBtns, 0, 2, 1, 1);
    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);

#ifdef __DONT_WRITE_IN_APP_TITLE
    QLabel * label = new QLabel(m_boxTitleBtns);
#else
    QLabel * label = new QLabel(APP_TITLE);
#endif
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    layoutBtns->setContentsMargins(0,0,int(4*dpi_ratio),0);
    layoutBtns->setSpacing(int(1*dpi_ratio));
    layoutBtns->addWidget(label);

    // Main
    m_pButtonMain = new CSVGPushButton(this);
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    m_pButtonMain->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_pMainGridLayout->addWidget(m_pButtonMain, 0, 0, 1, 1);
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    if (isCustomWindow) {
//        palette.setColor(QPalette::Background, AscAppManager::themes().color(CThemes::ColorRole::ecrWindowBackground));

        auto _creatToolButton = [](const QString& name, QWidget * parent) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");

            return btn;
        };

        // Minimize
        m_pButtonMinimize = _creatToolButton("toolButtonMinimize", this);
        QObject::connect(m_pButtonMinimize, &QPushButton::clicked, this, &CMainPanel::pushButtonMinimizeClicked);

        // Maximize
        m_pButtonMaximize = _creatToolButton("toolButtonMaximize", this);
        QObject::connect(m_pButtonMaximize, &QPushButton::clicked, this, &CMainPanel::pushButtonMaximizeClicked);

        // Close
        m_pButtonClose = _creatToolButton("toolButtonClose", this);
        QObject::connect(m_pButtonClose, &QPushButton::clicked, this, &CMainPanel::pushButtonCloseClicked);

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

#ifdef __linux__
        m_pMainGridLayout->setMargin( CX11Decoration::customWindowBorderWith() * dpi_ratio );

        connect(m_boxTitleBtns, SIGNAL(mouseDoubleClicked()), this, SLOT(pushButtonMaximizeClicked()));
#endif
    } else {
//        m_pButtonMain->setProperty("theme", "light");

        QLinearGradient gradient(this->rect().topLeft(), QPoint(this->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));

        palette.setBrush(QPalette::Background, QBrush(gradient));

        label->setFixedHeight(0);
    }

//    m_pTabs->setAutoFillBackground(true);
    // Set TabWidget
    m_pTabs = new CAscTabWidget(this, tabBar());
    m_pTabs->setObjectName(QString::fromUtf8("ascTabWidget"));
    m_pMainGridLayout->addWidget(m_pTabs, 1, 0, 1, 4);
    m_pTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pTabs->activate(false);
    m_pTabs->applyUITheme(AscAppManager::themes().current().id());

    connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(tabBar(), SIGNAL(tabBarClicked(int)), this, SLOT(onTabClicked(int)));
    connect(tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequest(int)));
    connect(m_pTabs, &CAscTabWidget::closeAppRequest, this, &CMainPanel::onAppCloseRequest);
    connect(m_pTabs, &CAscTabWidget::editorInserted, bind(&CMainPanel::onTabsCountChanged, this, _2, _1, 1));
    connect(m_pTabs, &CAscTabWidget::editorRemoved, bind(&CMainPanel::onTabsCountChanged, this, _2, _1, -1));
    m_pTabs->setPalette(palette);
    m_pTabs->setCustomWindowParams(isCustomWindow);
}

void CMainPanel::attachStartPanel(QCefView * const view)
{
    m_pMainWidget = qobject_cast<QWidget *>(view);
#ifdef __linux
    view->setMouseTracking(m_pButtonMain->hasMouseTracking());
#endif

    m_pMainWidget->setParent(this);
    m_pMainGridLayout->addWidget(m_pMainWidget, 1, 0, 1, 4);
    m_pMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    if ( !m_pTabs->isActiveWidget() )
        m_pMainWidget->show();
}

#ifdef __linux
QWidget * CMainPanel::getTitleWidget()
{
    return m_boxTitleBtns;
}

void CMainPanel::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
    findChild<QLabel *>("labelAppTitle")->setMouseTracking(enable);

    m_boxTitleBtns->setMouseTracking(enable);
    m_pTabs->setMouseTracking(enable);
    m_pTabs->tabBar()->setMouseTracking(enable);
    m_pButtonMain->setMouseTracking(enable);
    m_pButtonClose->setMouseTracking(enable);
    m_pButtonMinimize->setMouseTracking(enable);
    m_pButtonMaximize->setMouseTracking(enable);

    if ( m_pMainWidget )
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
    emit mainWindowWantToClose();
}

void CMainPanel::onAppCloseRequest()
{
    onFullScreen(-1, false);
    pushButtonCloseClicked();
}

void CMainPanel::applyMainWindowState(Qt::WindowState s)
{
    m_mainWindowState = s;

    if ( m_isCustomWindow ) {
#ifdef __linux__
        layout()->setMargin(s == Qt::WindowMaximized ? 0 : CX11Decoration::customWindowBorderWith() * scaling());
#endif

        m_pButtonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }
}

void CMainPanel::pushButtonMainClicked()
{
    if ( m_pTabs->isActiveWidget() ) {
        m_pTabs->activate(false);
        m_pMainWidget->setHidden(false);
        m_pTabs->setFocusedView();
        m_pButtonMain->setProperty("class", "active");

        ((QCefView *)m_pMainWidget)->setFocusToCef();
        onTabChanged(m_pTabs->currentIndex());
    }
}

void CMainPanel::toggleButtonMain(bool toggle, bool delay)
{
    auto _toggle = [=] (bool state) {
        if (m_pTabs->isActiveWidget() == state) {
            if ( state ) {
                m_pButtonMain->setProperty("class", "active");
                m_pTabs->activate(false);
                m_pMainWidget->setHidden(false);
//                m_pTabs->setFocusedView();
//                ((QCefView *)m_pMainWidget)->setFocusToCef();
            } else {
                m_pButtonMain->setProperty("class", "normal");
                m_pTabs->activate(true);
                m_pMainWidget->setHidden(true);
                m_pTabs->setFocusedView();
            }

            onTabChanged(m_pTabs->currentIndex());
        }
    };

    if ( delay ) {
        QTimer::singleShot(200, [=]{ _toggle(toggle); });
    } else {
        _toggle(toggle);
    }
}

void CMainPanel::focus() {
    if (m_pTabs->isActiveWidget()) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView *)m_pMainWidget)->setFocusToCef();
    }
}

void CMainPanel::onTabClicked(int index)
{
    Q_UNUSED(index)

    if (!m_pTabs->isActiveWidget()) {
        toggleButtonMain(false);
    }
}

void CMainPanel::onTabsCountChanged(int count, int i, int d)
{
    Q_UNUSED(i)
    Q_UNUSED(d)

    if ( count == 0 ) {
        toggleButtonMain(true);
    }

    if ( d < 0 ) {
        //RecalculatePlaces();
    } else
    QTimer::singleShot(200, [=]{
        //RecalculatePlaces();
    });
}

void CMainPanel::onEditorAllowedClose(int uid)
{
    if ( ((QCefView *)m_pMainWidget)->GetCefView()->GetId() == uid ) {
//        if ( m_pTabs->count() ) {
//            m_pMainWidget->setProperty("removed", true);
//        }
    } else {
        int _index = m_pTabs->tabIndexByView(uid);
        if ( !(_index < 0) ) {
            QWidget * _view = m_pTabs->widget(_index);
            _view->deleteLater();

            m_pTabs->removeTab(_index);
            //m_pTabs->adjustTabsSize();
            if ( !m_pTabs->count() ) {
                m_pTabs->setProperty("empty", true);
                m_pTabs->style()->polish(m_pTabs);

                toggleButtonMain(true);
            }

            onTabChanged(m_pTabs->currentIndex());

            CInAppEventBase _event{CInAppEventBase::CEventType::etEditorClosed};
            AscAppManager::getInstance().commonEvents().signal(&_event);
        }
    }
}

void CMainPanel::onTabChanged(int index)
{
    Q_UNUSED(index)

#ifndef __DONT_WRITE_IN_APP_TITLE
    QLabel * title = (QLabel *)m_boxTitleBtns->layout()->itemAt(0)->widget();

    if (m_pTabs->isActive() && !(index < 0) && index < m_pTabs->count()) {
        QString docName = m_pTabs->titleByIndex(index, false);
        if (!docName.length())
            docName = m_pTabs->tabBar()->tabText(index);

        title->setText(QString(APP_TITLE) + " - " + docName);
    } else {
        title->setText(APP_TITLE);
    }
#endif
}

void CMainPanel::onTabCloseRequest(int index)
{
    onFullScreen(-1, false);
    if ( m_pTabs->isProcessed(index) ) {
        return;
    } else {
        if ( trySaveDocument(index) == MODAL_RESULT_NO ) {
            m_pTabs->editorCloseRequest(index);
            onDocumentSave(m_pTabs->panel(index)->cef()->GetId());
        }
    }
}

int CMainPanel::tabCloseRequest(int index)
{
    if ( m_pTabs->count() ) {
        if ( index == -1 ) {
            if ( !m_pTabs->closedByIndex(m_pTabs->currentIndex()) )
                index = m_pTabs->currentIndex();
            else {
                for (int i(0); i < m_pTabs->count(); ++i) {
                    if ( !m_pTabs->closedByIndex(i) ) {
                        index = i;
                        break;
                    }
                }
            }
        }
    }

    if ( !(index < 0) && index < m_pTabs->count() ) {
        onFullScreen(-1, false);
        if ( !m_pTabs->isProcessed(index) ) {
            int _result = trySaveDocument(index);
            if ( _result == MODAL_RESULT_NO ) {
                m_pTabs->editorCloseRequest(index);
                onDocumentSave(m_pTabs->panel(index)->cef()->GetId());
            }

            return _result;
        }
    }

    return MODAL_RESULT_CUSTOM;
}

int CMainPanel::trySaveDocument(int index)
{
    if (m_pTabs->closedByIndex(index)) return MODAL_RESULT_YES;

    int modal_res = MODAL_RESULT_NO;
    if ( m_pTabs->modifiedByIndex(index) ) {
        toggleButtonMain(false);
        m_pTabs->setCurrentIndex(index);

        CMessage mess(TOP_NATIVE_WINDOW_HANDLE, CMessageOpts::moButtons::mbYesDefNoCancel);
        modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));

        switch (modal_res) {
        case MODAL_RESULT_CANCEL: break;
        case MODAL_RESULT_CUSTOM + 1: modal_res = MODAL_RESULT_NO; break;
        case MODAL_RESULT_CUSTOM + 2: modal_res = MODAL_RESULT_CANCEL; break;
        case MODAL_RESULT_CUSTOM + 0:
        default:{
            m_pTabs->editorCloseRequest(index);
            m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

            modal_res = MODAL_RESULT_YES;
            break;}
        }
    }

    return modal_res;
}

void CMainPanel::onPortalLogout(std::wstring wjson)
{
    if ( m_pTabs->count() ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            QString _portal = objRoot["portal"].toString(),
                    _action;

            if ( objRoot.contains("onsuccess") )
                _action = objRoot["onsuccess"].toString();

            for (int i(m_pTabs->count()); !(--i < 0);) {
                int _answer = MODAL_RESULT_NO;

                CAscTabData& _doc = *m_pTabs->panel(i)->data();
                if ( _doc.isViewType(cvwtEditor) && !_doc.closed() &&
                        QString::fromStdWString(_doc.url()).startsWith(_portal) )
                {
                    if ( _doc.hasChanges() ) {
                        _answer = trySaveDocument(i);
                        if ( _answer == MODAL_RESULT_CANCEL) {
                            AscAppManager::cancelClose();
                            return;
                        }
                    }

                    if ( _answer != MODAL_RESULT_YES ) {
                        m_pTabs->editorCloseRequest(i);
                        onDocumentSave(m_pTabs->panel(i)->cef()->GetId());
                    }
                }
            }
        }
    }
}

void CMainPanel::onCloudDocumentOpen(std::wstring url, int id, bool select)
{
    COpenOptions opts = {url};
    opts.id = id;

    int _index = m_pTabs->openCloudDocument(opts, select, true);
    if ( !(_index < 0) ) {
        if ( select )
            toggleButtonMain(false, true);

        CAscTabData& _panel = *(m_pTabs->panel(_index)->data());
        QRegularExpression re("ascdesktop:\\/\\/compare");
        QRegularExpressionMatch match = re.match(QString::fromStdWString(_panel.url()));

        if (match.hasMatch()) {
             _panel.setIsLocal(true);
             _panel.setUrl("");
        }
    }
}

void CMainPanel::doOpenLocalFile(COpenOptions& opts)
{
    QFileInfo info(opts.url);
    if (!info.exists()) { return; }
    if (!info.isFile()) { return; }

    int result = m_pTabs->openLocalDocument(opts, true);
    if ( !(result < 0) ) {
        toggleButtonMain(false, true);
    } else
    if (result == -255) {
        QTimer::singleShot(0, [=]{
            CMessage::error(TOP_NATIVE_WINDOW_HANDLE, tr("File format not supported."));
        });
    }
}

void CMainPanel::onLocalFileRecent(void * d)
{
    CAscLocalOpenFileRecent_Recover * pData = static_cast<CAscLocalOpenFileRecent_Recover *>(d);

    COpenOptions opts = { pData->get_Path(),
          pData->get_IsRecover() ? etRecoveryFile : etRecentFile, pData->get_Id() };

    RELEASEINTERFACE(pData);

    onLocalFileRecent(opts);
}

void CMainPanel::onLocalFileRecent(const COpenOptions& opts)
{
    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(opts.url);

    bool forcenew = false;
    if ( !match.hasMatch() ) {
        QFileInfo _info(opts.url);
        if ( opts.srctype != etRecoveryFile && !_info.exists() ) {
            CMessage mess(TOP_NATIVE_WINDOW_HANDLE, CMessageOpts::moButtons::mbYesDefNo);
            int modal_res = mess.warning(
                        tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName()));

            if (modal_res == MODAL_RESULT_CUSTOM) {
                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "file:skip", QString::number(opts.id));
            }

            return;
        }
    } else forcenew = true;

//    openLocalFile(opts);
    int result = m_pTabs->openLocalDocument(opts, true, forcenew);
    if ( !(result < 0) ) {
        toggleButtonMain(false);
    } else
    if (result == -255) {
        CMessage::error(TOP_NATIVE_WINDOW_HANDLE, tr("File format not supported."));
    }
}

void CMainPanel::createLocalFile(const QString& name, int format)
{
    COpenOptions opts{name, etNewFile};
    opts.format = format;

    int tabIndex = m_pTabs->addEditor(opts);

    if ( !(tabIndex < 0) ) {
        m_pTabs->updateIcons();
        m_pTabs->setCurrentIndex(tabIndex);

        toggleButtonMain(false, true);
    }
}

void CMainPanel::onLocalFilesOpen(void * data)
{
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    std::vector<std::wstring> vctFiles = pData->get_Files();

    doOpenLocalFiles(&vctFiles);

    RELEASEINTERFACE(pData);
}

void CMainPanel::onLocalFileLocation(QString path)
{
    Utils::openFileLocation(path);
}

void CMainPanel::onFileLocation(int uid, QString param)
{
    if ( param == "offline" ) {
        QString path = m_pTabs->urlByView(uid);
        if ( !path.isEmpty() ) {
//            if ( Utils::isFileLocal(path) )
                onLocalFileLocation(path);
//            else {
//            }
        } else {
            CMessage::info(TOP_NATIVE_WINDOW_HANDLE, tr("Document must be saved firstly."));
        }
    } else {
        QRegularExpression _re("^((?:https?:\\/{2})?[^\\s\\/]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re.match(param);

        if ( _re_match.hasMatch() ) {
            QString _domain = _re_match.captured(1);
            QString _folder = param;

            if ( !_folder.contains("desktop=true") ) {
                if ( _folder.contains("?") )
                    _folder.append("&desktop=true");
                else {
                    int pos = _folder.indexOf(QRegularExpression("#\\d+"));
                    !(pos < 0) ? _folder.insert(pos, "?desktop=true&") : _folder.append("?desktop=true");
                }
            }

            int _tab_index = m_pTabs->tabIndexByTitle(Utils::getPortalName(_domain), etPortal);
            if ( !(_tab_index < 0)) {
                ((CAscTabWidget *)m_pTabs)->updatePortal(_tab_index, _folder);
            } else {
                _tab_index = m_pTabs->addPortal(_folder, "", "");
            }

            if ( !(_tab_index < 0) ) {
                if (m_mainWindowState == Qt::WindowMinimized)
                    emit mainWindowChangeState(Qt::WindowNoState);

                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(_tab_index);
            }
        }
    }
}

void CMainPanel::doOpenLocalFiles(const std::vector<std::wstring> * vec)
{
    if (qApp->activeModalWidget()) return;

    for (const auto& wstr : (*vec)) {
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
        QString n = i.next();
        if ( n.startsWith("--new:") ) {
        } else {
            COpenOptions opts = {n.toStdWString(), etLocalFile};
            doOpenLocalFile(opts);
        }
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
    QString path = !pData->get_Url().empty() ? QString() : QString::fromStdWString(pData->get_Path());

    m_pTabs->applyDocumentChanging(pData->get_Id(), name, path);
    onTabChanged(m_pTabs->currentIndex());

    RELEASEINTERFACE(pData);
}

void CMainPanel::onEditorConfig(int, std::wstring cfg)
{
}

void CMainPanel::onWebAppsFeatures(int id, std::wstring opts)
{
    m_pTabs->setEditorOptions(id, opts);
}

void CMainPanel::onDocumentReady(int uid)
{
    if ( uid < 0 ) {
        QTimer::singleShot(20, this, [=]{
            refreshAboutVersion();
            emit mainPageReady();

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"app:ready");
            focus(); // TODO: move to app manager
        });
    } else {
        m_pTabs->applyDocumentChanging(uid, DOCUMENT_CHANGED_LOADING_FINISH);
    }
}

void CMainPanel::onDocumentLoadFinished(int uid)
{
    m_pTabs->applyDocumentChanging(uid, DOCUMENT_CHANGED_PAGE_LOAD_FINISH);
}

void CMainPanel::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void CMainPanel::onDocumentSave(int id, bool cancel)
{
    int _i = m_pTabs->tabIndexByView(id);
    if ( !(_i < 0) ) {
        if ( !cancel ) {
            if ( m_pTabs->closedByIndex(_i) &&
                    !m_pTabs->isProcessed(_i) &&
                        !m_pTabs->isFragmented(_i) )
                {
                    m_pTabs->closeEditorByIndex(_i);
                }
        } else {
            m_pTabs->cancelDocumentSaving(_i);

//            AscAppManager::cancelClose();
        }
    }
}

void CMainPanel::onDocumentSaveInnerRequest(int id)
{
    CMessage mess(TOP_NATIVE_WINDOW_HANDLE, CMessageOpts::moButtons::mbYesDefNo);
    int modal_res = mess.confirm(tr("Document must be saved to continue.<br>Save the document?"));

    CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
    pData->put_Value((modal_res == MODAL_RESULT_CUSTOM + 0) ? true : false);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
    pEvent->m_pData = pData;

    AscAppManager::getInstance().GetViewById(id)->Apply(pEvent);
}

void CMainPanel::onDocumentDownload(void * info)
{
    if ( !m_pWidgetDownload ) {
        m_pWidgetDownload = new CDownloadWidget(this);

        QHBoxLayout * layoutBtns = qobject_cast<QHBoxLayout *>(m_boxTitleBtns->layout());
        layoutBtns->insertWidget(1, m_pWidgetDownload->toolButton());
    }

    m_pWidgetDownload->downloadProcess(info);

//    CAscDownloadFileInfo * pData = reinterpret_cast<CAscDownloadFileInfo *>(info);
//    RELEASEINTERFACE(pData);
}

void CMainPanel::onDocumentFragmented(int id, bool isfragmented)
{
    int index = m_pTabs->tabIndexByView(id);
    if ( !(index < 0) ) {
            int _answer = MODAL_RESULT_NO;
            if ( isfragmented ) {
                static const bool _skip_user_warning = !InputArgs::contains(L"--warning-doc-fragmented");
                if ( _skip_user_warning ) {
                    m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
                    return;
                } else {
//                    CMessage mess(TOP_NATIVE_WINDOW_HANDLE);
//                    mess.setButtons(CMessageOpts::cmButtons::cmbYesDefNoCancel);
//                    _answer = mess.warning(tr("%1 must be built. Continue?").arg(m_pTabs->titleByIndex(index)));
//                    switch (_answer) {
//                    case MODAL_RESULT_CUSTOM + 0:
//                        m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
//                        return;
//                    case MODAL_RESULT_CUSTOM + 1: _answer == MODAL_RESULT_NO; break;
//                    case MODAL_RESULT_CUSTOM + 2:
//                    default:                      _answer = MODAL_RESULT_CANCEL; break;
//                    }
                }
            } else {
                onDocumentFragmentedBuild(id, 0);
            }

            if ( _answer == MODAL_RESULT_CANCEL ) {
                AscAppManager::cancelClose();
            }
    }
}

void CMainPanel::onDocumentFragmentedBuild(int vid, int error)
{
    int index = m_pTabs->tabIndexByView(vid);
    if ( error == 0 ) {
        m_pTabs->closeEditorByIndex(index, false);
    } else {
        m_pTabs->cancelDocumentSaving(index);
        AscAppManager::cancelClose();
    }
}

void CMainPanel::onEditorActionRequest(int vid, const QString& args)
{
    int index = m_pTabs->tabIndexByView(vid);
    if ( !(index < 0) ) {
        if ( args.contains(QRegExp("action\\\":\\\"file:close")) ) {
            bool _is_local = m_pTabs->isLocalByIndex(index);
            onTabCloseRequest(index);

            if (  !_is_local  ) {
                QJsonParseError jerror;
                QJsonDocument jdoc = QJsonDocument::fromJson(args.toLatin1(), &jerror);

                if( jerror.error == QJsonParseError::NoError ) {
                    QJsonObject objRoot = jdoc.object();

                    QString _url = objRoot["url"].toString();
                    if ( !_url.isEmpty() )
                        onFileLocation(vid, _url);
                    else _is_local = true;
                }
            }

            if (  _is_local  ) toggleButtonMain(true);
        }
    }
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

#ifdef Q_OS_LINUX
    WindowHelper::CParentDisable disabler(qobject_cast<QWidget*>(parent()));
#endif

    CAscPrintEnd * pData = (CAscPrintEnd *)opts;
    CCefView * pView = AscAppManager::getInstance().GetViewById(pData->get_Id());

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
        CPrintDialogWinWrapper wrapper(printer, TOP_NATIVE_WINDOW_HANDLE);
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
#ifdef _WIN32
        int res = wrapper.showModal();
#else
        int res = dialog->exec();
#endif
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

                if ( pContext->BeginPaint() ) {
#if defined(_WIN32)
                    CPrintProgress progressDlg((HWND)parentWidget()->winId());
#else
                    CPrintProgress progressDlg(qobject_cast<QWidget *>(parent()));
#endif
                    progressDlg.startProgress();

                    CAscPrintPage * pData;
                    uint count = finish - start;
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

void CMainPanel::onLocalFileSaveAs(void * d)
{
}

void CMainPanel::onFullScreen(int id, bool apply)
{
    if ( apply ) {
        if ( m_mainWindowState != Qt::WindowFullScreen ) {
            m_isMaximized = m_mainWindowState == Qt::WindowMaximized;
            m_mainWindowState = Qt::WindowFullScreen;

            m_pTabs->setFullScreen(apply, id);
//            emit mainWindowChangeState(Qt::WindowFullScreen);

            QTimer::singleShot(0, [=]{
                CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER);
                AscAppManager::getInstance().GetViewById(id)->Apply(pEvent);
            });
        }
    } else
    if ( m_mainWindowState == Qt::WindowFullScreen ) {
        m_mainWindowState = m_isMaximized ? Qt::WindowMaximized : Qt::WindowNoState;

//        emit mainWindowChangeState(m_mainWindowState);
        m_pTabs->setFullScreen(apply);
        toggleButtonMain(false);
    }
}

void CMainPanel::onKeyDown(void * eventData)
{
    NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)eventData;

    int key = pData->get_KeyCode();
    bool _is_ctrl = pData->get_IsCtrl();
    bool _is_shift = pData->get_IsShift();

    RELEASEINTERFACE(pData)

    switch (key) {
    case 'W':
    case VK_F4:
        if ( _is_ctrl && m_pTabs->isActiveWidget() ) {
            onTabCloseRequest(m_pTabs->currentIndex());
        }
        break;
    case VK_TAB:
        if (m_pTabs->count()) {
            if ( _is_ctrl ) {
                int _new_index = 0;

                if ( _is_shift ) {
                    if ( m_pTabs->isActiveWidget() )
                        _new_index = m_pTabs->currentIndex() - 1; else
                        _new_index = m_pTabs->count() - 1;
                } else {
                    if ( m_pTabs->isActiveWidget() )
                        _new_index =  m_pTabs->currentIndex() + 1;
                }

                if ( _new_index < 0 || !(_new_index < m_pTabs->count()) )
                    toggleButtonMain(true);
                else {
                    toggleButtonMain(false);
                    m_pTabs->setCurrentIndex( _new_index );
                }
            }
        }
        break;
    }
}

void CMainPanel::onPortalOpen(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();

        QString _portal = objRoot["portal"].toString(),
                _entry = objRoot["entrypage"].toString();
        if ( !_portal.isEmpty() ) {
            int res = m_pTabs->openPortal( _portal, objRoot["provider"].toString("onlyoffice"), _entry);
            if ( !(res < 0) ) {
                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(res);
            }
        }
    }
}

void CMainPanel::onPortalNew(QString in)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(in.toLatin1(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();

        QString _domain = objRoot["domain"].toString();
        QString _name = Utils::getPortalName(_domain);

        int _tab_index = m_pTabs->tabIndexByEditorType(etNewPortal);
        if ( !(_tab_index < 0)) {
            int _uid = m_pTabs->viewByIndex(_tab_index);
            m_pTabs->applyDocumentChanging(_uid, _name, _domain);
            m_pTabs->applyDocumentChanging(_uid, etPortal);

            onTabChanged(m_pTabs->currentIndex());
        }
    }
}

void CMainPanel::onPortalCreate()
{
    QString _url = URL_SIGNUP;

    GET_REGISTRY_SYSTEM(reg_system)
    if ( reg_system.contains("Store") )
            _url += "&store=" + reg_system.value("Store").toString();

    int res = m_pTabs->newPortal(_url, tr("Sign Up"));
    if ( !(res < 0) ) {
        toggleButtonMain(false, true);
        m_pTabs->setCurrentIndex(res);
    }
}

void CMainPanel::onOutsideAuth(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject objRoot = jdoc.object();

        QString _domain = objRoot["portal"].toString();
        int _tab_index = m_pTabs->tabIndexByTitle(Utils::getPortalName(_domain), etPortal);
        if ( _tab_index < 0 ) {
            _tab_index = m_pTabs->addOAuthPortal(_domain,
                                objRoot["type"].toString(), objRoot["provider"].toString(), objRoot["entrypage"].toString());
        }

        if ( !(_tab_index < 0) ) {
            m_pTabs->setCurrentIndex(_tab_index);
            toggleButtonMain(false, true);
        }
    }
}

void CMainPanel::applyTheme(const std::wstring& theme)
{
    this->setProperty("uitheme", QString::fromStdWString(theme));

    for (int i(m_pTabs->count()); !(--i < 0);) {
        CAscTabData& _doc = *m_pTabs->panel(i)->data();
        if ( _doc.isViewType(cvwtEditor) && !_doc.closed() ) {
            AscAppManager::sendCommandTo(m_pTabs->panel(i)->cef(), L"uitheme:changed", theme);
        }
    }

//    m_pTabs->style()->polish(m_pTabs);
    m_boxTitleBtns->style()->polish(m_boxTitleBtns);
    m_pTabBarWrapper->style()->polish(m_pTabBarWrapper);
    m_pButtonMain->style()->polish(m_pButtonMain);
    if ( m_pButtonMinimize ) {
        m_pButtonMinimize->style()->polish(m_pButtonMinimize);
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
        m_pButtonClose->style()->polish(m_pButtonClose);
    }

    m_pTabs->applyUITheme(theme);

    //QWidget * centralwidget = layout()->itemAt(0)->widget();
    //centralwidget->style()->polish(centralwidget);
    style()->polish(this);

    update();
}

void CMainPanel::setInputFiles(QStringList * list)
{
    RELEASEOBJECT(m_inFiles)
    m_inFiles = list;
}

QString CMainPanel::getSaveMessage() const
{
    return tr("%1 is modified.<br>Do you want to keep changes?");
}

void CMainPanel::updateScaling(double dpiratio)
{
    CScalingWrapper::updateScaling(dpiratio);

    QLayout * layoutBtns = m_boxTitleBtns->layout();
    layoutBtns->setSpacing(int(1 * dpiratio));

    if ( m_isCustomWindow ) {
        layoutBtns->setContentsMargins(0,0,0,0);

        QSize small_btn_size(int(40*dpiratio), int(TOOLBTN_HEIGHT*dpiratio));
        m_pButtonMinimize->setFixedSize(small_btn_size);
        m_pButtonMaximize->setFixedSize(small_btn_size);
        m_pButtonClose->setFixedSize(small_btn_size);
    }

    //m_pButtonMain->setGeometry(0, 0, int(BUTTON_MAIN_WIDTH * dpiratio), int(TITLE_HEIGHT * dpiratio));
    m_pButtonMain->setFixedSize(int(BUTTON_MAIN_WIDTH * dpiratio), int(TITLE_HEIGHT * dpiratio));

    QString _tabs_stylesheets = dpiratio > 1.75 ? ":/sep-styles/tabbar@2x" :
                                    dpiratio > 1.5 ? ":/sep-styles/tabbar@1.75x" :
                                    dpiratio > 1.25 ? ":/sep-styles/tabbar@1.5x" :
                                    dpiratio > 1 ? ":/sep-styles/tabbar@1.25x" : ":/sep-styles/tabbar";
    if ( m_isCustomWindow ) {
        _tabs_stylesheets += ".qss";
    } else {
#ifdef __linux__
        _tabs_stylesheets += ".nix.qss";
#endif
    }

    QFile styleFile(_tabs_stylesheets);
    styleFile.open( QFile::ReadOnly );
    const QString _style = QString(styleFile.readAll());
    m_pTabBarWrapper->applyTheme(_style);
    m_pTabs->setStyleSheet(_style);
    m_pTabs->updateScaling(dpiratio);
    styleFile.close();

//    std::map<int, std::pair<QString, QString> > icons;
//    if ( dpiratio > 1 ) {
//        icons.insert({
//            {etUndefined, std::make_pair(":/tabbar/icons/newdoc@2x.png", ":/tabbar/icons/newdoc@2x.png")},
//            {etDocument, std::make_pair(":/tabbar/icons/de@2x.png", ":/tabbar/icons/de@2x.png")},
//            {etPresentation, std::make_pair(":/tabbar/icons/pe@2x.png", ":/tabbar/icons/pe@2x.png")},
//            {etSpreadsheet, std::make_pair(":/tabbar/icons/se@2x.png", ":/tabbar/icons/se@2x.png")},
//            {etPortal, std::make_pair(":/tabbar/icons/portal@2x.png", ":/tabbar/icons/portal@2x.png")},
//            {etNewPortal, std::make_pair(":/tabbar/icons/portal@2x.png", ":/tabbar/icons/portal@2x.png")}
//        });
//    } else {
//        icons.insert({
//            {etUndefined, std::make_pair(":/tabbar/icons/newdoc.png", ":/tabbar/icons/newdoc.png")},
//            {etDocument, std::make_pair(":/tabbar/icons/de.png", ":/tabbar/icons/de.png")},
//            {etPresentation, std::make_pair(":/tabbar/icons/pe.png", ":/tabbar/icons/pe.png")},
//            {etSpreadsheet, std::make_pair(":/tabbar/icons/se.png", ":/tabbar/icons/se.png")},
//            {etPortal, std::make_pair(":/tabbar/icons/portal_light.png", ":/tabbar/icons/portal.png")},
//            {etNewPortal, std::make_pair(":/tabbar/icons/portal.png", ":/tabbar/icons/portal.png")}
//        });
//    }

//    m_pTabs->setTabIcons(icons);
    m_pTabs->reloadTabIcons();

    //if ( m_mainWindowState == Qt::WindowMaximized )
        //RecalculatePlaces();
}

void CMainPanel::setScreenScalingFactor(double s)
{
    updateScaling(s);
    CScalingWrapper::updateChildScaling(this, s);
}

bool CMainPanel::holdUid(int uid) const
{
    CCefView * _view = (qobject_cast<QCefView *>(m_pMainWidget))->GetCefView();
    bool _res_out = _view->GetId() == uid;

    if ( !_res_out ) {
        CTabPanel *  _widget = qobject_cast<CTabPanel *>(m_pTabs->fullScreenWidget());

        if ( _widget ) {
            _res_out = _widget->cef()->GetId() == uid;
        }
    }

    return _res_out ?
        true : !(m_pTabs->tabIndexByView(uid) < 0);
}

bool CMainPanel::holdUrl(const QString& url, AscEditorType type) const
{
    if ( type == etPortal ) {
        return !(m_pTabs->tabIndexByTitle(Utils::getPortalName(url), etPortal) < 0);
    } else
    if ( type == etLocalFile ) {
        return !(m_pTabs->tabIndexByUrl(url) < 0);
    } else {

    }

    return false;
}

CAscTabWidget * CMainPanel::tabWidget(){return m_pTabs;}

CTabBar *CMainPanel::tabBar()
{
    return m_pTabBarWrapper->tabBar();
}
