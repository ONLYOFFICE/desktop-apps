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

#include "windows/cmainwindow.h"
#include "ceditortools.h"
#include "defines.h"
#include "utils.h"
#include "components/cfiledialog.h"
#include "common/Types.h"
#include "version.h"
#include "components/cmessage.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <QApplication>
#include "components/cprintdialog.h"
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QPrintEngine>

#ifdef _WIN32
# include "shlobj.h"
# include <platform_win/printdialog.h>
#else
# include <platform_linux/gtkprintdialog.h>
#endif


#ifdef _UPDMODULE
  #include "../version.h"
#endif

using namespace std::placeholders;
using namespace NSEditorApi;

CMainWindow::CMainWindow(const QRect &rect) :
    CWindowPlatform(rect),
    CScalingWrapper(m_dpiRatio),
    m_savePortal(QString())
{
    setObjectName("MainWindow");
    m_pMainPanel = createMainPanel(this);
    setCentralWidget(m_pMainPanel);
#ifdef __linux__
    setAcceptDrops(true);
    if (isCustomWindowStyle()) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
    QMetaObject::connectSlotsByName(this);
#endif
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    updateScalingFactor(m_dpiRatio);
    goStart();
}

CMainWindow::~CMainWindow()
{

}

/** Public **/

QWidget * CMainWindow::editor(int index)
{
    return tabWidget()->panel(index);
}

QRect CMainWindow::windowRect() const
{
    return normalGeometry();
}

QString CMainWindow::documentName(int vid)
{
    int i = tabWidget()->tabIndexByView(vid);
    return (i < 0) ? "" : tabWidget()->panel(i)->data()->title();
}

void CMainWindow::selectView(int viewid)
{
    int _index = tabWidget()->tabIndexByView(viewid);
    if ( !(_index < 0) ) {
        tabWidget()->setCurrentIndex(_index);
        toggleButtonMain(false);
    }
}

void CMainWindow::selectView(const QString& url)
{
    int _index = tabWidget()->tabIndexByUrl(url);
    if ( !(_index < 0) ) {
        tabWidget()->setCurrentIndex(_index);
        toggleButtonMain(false);
    }
}

int CMainWindow::attachEditor(QWidget * panel, int index)
{
    if (!QCefView::IsSupportLayers()) {
        CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
        if (_panel)
            _panel->view()->SetCaptionMaskSize(0);
    }
    int _index = tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        toggleButtonMain(false);
        tabWidget()->setCurrentIndex(_index);
    }
    return _index;
}

int CMainWindow::attachEditor(QWidget * panel, const QPoint& pt)
{
    QPoint _pt_local = tabWidget()->tabBar()->mapFromGlobal(pt);
#ifdef Q_OS_WIN
# if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QPoint _tl = windowRect().topLeft();
    if ( _tl.x() < _pt_local.x() && _tl.y() < _pt_local.y() )
        _pt_local -= windowRect().topLeft();
# endif
#endif
    int _index = tabWidget()->tabBar()->tabIndexAt(_pt_local);
    if ( !(_index < 0) ) {
        QRect _rc_tab = tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }
    return attachEditor(panel, _index);
}

int CMainWindow::editorsCount()
{
    return tabWidget()->count(cvwtEditor);
}

int CMainWindow::editorsCount(const std::wstring& portal)
{
    return tabWidget()->count(portal, true);
}

bool CMainWindow::pointInTabs(const QPoint& pt)
{
    QRect _rc_title(m_pMainPanel->geometry());
    _rc_title.setHeight(tabWidget()->tabBar()->height());
    _rc_title.adjust(m_pButtonMain->width(), 1, -3*int(TOOLBTN_WIDTH*m_dpiRatio), 0);
    return _rc_title.contains(mapFromGlobal(pt));
}

bool CMainWindow::holdView(int id) const
{
    return holdUid(id);
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
    m_pMainPanel->setProperty("uitheme", QString::fromStdWString(GetActualTheme(theme)));
    m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
    for (int i(m_pTabs->count()); !(--i < 0);) {
        CAscTabData& _doc = *m_pTabs->panel(i)->data();
        if ( _doc.isViewType(cvwtEditor) && !_doc.closed() ) {
            AscAppManager::sendCommandTo(m_pTabs->panel(i)->cef(), L"uitheme:changed", theme);
        }
    }
    m_boxTitleBtns->style()->polish(m_boxTitleBtns);
    m_pTabs->tabBar()->style()->polish(m_pTabs->tabBar());
    m_pButtonMain->style()->polish(m_pButtonMain);
    if (m_pTopButtons[BtnType::Btn_Minimize]) {
        foreach (auto btn, m_pTopButtons)
            btn->style()->polish(btn);
    }
    m_pTabs->applyUITheme(GetActualTheme(theme));
    m_pMainPanel->style()->polish(m_pMainPanel);
    m_pMainPanel->update();

    m_pButtonMain->setIcon(MAIN_ICON_PATH, GetCurrentTheme().isDark() ? "logo-light" : "logo-dark");
    m_pButtonMain->setIconSize(MAIN_ICON_SIZE * m_dpiRatio);
    if (m_pWidgetDownload && m_pWidgetDownload->toolButton()) {
        m_pWidgetDownload->applyTheme(QString::fromStdWString(GetActualTheme(theme)));
        m_pWidgetDownload->toolButton()->style()->polish(m_pWidgetDownload->toolButton());
    }
}

/** Private **/

void CMainWindow::focus()
{
    if ( m_pTabs->isActiveWidget() ) {
        m_pTabs->setFocusedView();
    } else {
        ((QCefView *)m_pMainWidget)->setFocusToCef();
    }
}

void CMainWindow::onCloseEvent()
{
    close();
}

void CMainWindow::closeEvent(QCloseEvent * e)
{
    AscAppManager::getInstance().closeQueue().enter(sWinTag{CLOSE_QUEUE_WIN_TYPE_MAIN, size_t(this)});
    e->ignore();
}

void CMainWindow::close()
{
    CWindowBase::saveWindowState();
    m_isCloseAll = true;

    if ( m_pTabs->count() == 0 ) {
        emit aboutToClose();
    } else {
        onFullScreen(-1, false);

#ifdef _WIN32
        if (isSessionInProgress() && m_pTabs->count() > 1) {
#else
        if (m_pTabs->count() > 1) {
#endif
            GET_REGISTRY_USER(reg_user);
            if (!reg_user.value("ignoreMsgAboutOpenTabs", false).toBool()) {
                for (int i = 0; i < m_pTabs->count(); i++) {
                    if (!m_pTabs->modifiedByIndex(i)) {
                        bool dontAskAgain = false;
                        int res = CMessage::showMessage(this, tr("More than one document is open.<br>Close the window anyway?"),
                                                           MsgType::MSG_WARN, MsgBtns::mbYesDefNo, &dontAskAgain,
                                                           tr("Don't ask again."));
                        if (dontAskAgain)
                            reg_user.setValue("ignoreMsgAboutOpenTabs", true);
                        if (res != MODAL_RESULT_YES) {
                            AscAppManager::cancelClose();
                            return;
                        }
                        break;
                    }
                }
            }
        }

        for (int i(m_pTabs->count()); i-- > 0;) {
            if ( !m_pTabs->closedByIndex(i) ) {
                if ( !m_pTabs->isProcessed(i) ) {
                    int _result = trySaveDocument(i);
                    if ( _result == MODAL_RESULT_NO ) {
                        m_pTabs->editorCloseRequest(i);
                        onDocumentSave(m_pTabs->panel(i)->cef()->GetId());
                    } else
                    if ( _result == MODAL_RESULT_CANCEL ) {
                        AscAppManager::cancelClose();
                        return;
                    }
                } else {
                    m_pTabs->editorCloseRequest(i);
                }
            }

            qApp->processEvents();
        }
    }
}

void CMainWindow::captureMouse(int tabindex)
{
    if (tabindex >= 0 && tabindex < tabWidget()->count()) {
        QPoint spt = tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        QTimer::singleShot(0, this, [=] {
            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent((QWidget *)tabWidget()->tabBar(), &event);
            tabWidget()->tabBar()->grabMouse();
        });
    }
}

#ifdef __linux__
void CMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QSet<QString> _exts;
    _exts << "docx" << "doc" << "odt" << "rtf" << "txt" << "doct" << "dotx" << "ott";
    _exts << "html" << "mht" << "epub";
    _exts << "pptx" << "ppt" << "odp" << "ppsx" << "pptt" << "potx" << "otp";
    _exts << "xlsx" << "xls" << "ods" << "csv" << "xlst" << "xltx" << "ots";
    _exts << "pdf" << "djvu" << "xps";
    _exts << "plugin";

    QFileInfo oInfo(urls[0].toString());

    if (_exts.contains(oInfo.suffix()))
        event->acceptProposedAction();
}

void CMainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QString _path = urls[0].path();

    Utils::keepLastPath(LOCAL_PATH_OPEN, _path);
    COpenOptions opts = {"", etLocalFile, _path};
    opts.wurl = _path.toStdWString();

    std::wstring::size_type nPosPluginExt = opts.wurl.rfind(L".plugin");
    std::wstring::size_type nUrlLen = opts.wurl.length();
    if ((nPosPluginExt != std::wstring::npos) && ((nPosPluginExt + 7) == nUrlLen))
    {
        // register plugin
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent();
        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_ADD_PLUGIN;
        NSEditorApi::CAscAddPlugin* pData = new NSEditorApi::CAscAddPlugin();
        pData->put_Path(opts.wurl);
        pEvent->m_pData = pData;

        AscAppManager::getInstance().Apply(pEvent);
    }
    else
    {
        doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}
#endif

/** MainPanel **/


QWidget* CMainWindow::createMainPanel(QWidget *parent)
{
    QWidget *mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");
    QGridLayout *_pMainGridLayout = new QGridLayout(mainPanel);
    _pMainGridLayout->setSpacing(0);
    _pMainGridLayout->setObjectName(QString::fromUtf8("mainGridLayout"));
    _pMainGridLayout->setContentsMargins(0, 0, 0, 0);
    mainPanel->setLayout(_pMainGridLayout);

    // Set custom TabBar
    CTabBar *pTabBar = new CTabBar(mainPanel);
    _pMainGridLayout->addWidget(pTabBar, 0, 1, 1, 1);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(1);
    pTabBar->setSizePolicy(sizePolicy);

    m_boxTitleBtns = createTopPanel(mainPanel);
    m_boxTitleBtns->setObjectName("CX11Caption");
    _pMainGridLayout->addWidget(m_boxTitleBtns, 0, 2, 1, 1);
    m_boxTitleBtns->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

#ifdef __DONT_WRITE_IN_APP_TITLE
    QLabel * label = new QLabel(m_boxTitleBtns);
#else
    QLabel * label = new QLabel(APP_TITLE, m_boxTitleBtns);
#endif
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->insertWidget(1,label);

    // Main
    m_pButtonMain = new CSVGPushButton(mainPanel);
    m_pButtonMain->setObjectName( "toolButtonMain" );
    m_pButtonMain->setProperty("class", "active");
    m_pButtonMain->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    _pMainGridLayout->addWidget(m_pButtonMain, 0, 0, 1, 1);
    QObject::connect(m_pButtonMain, SIGNAL(clicked()), this, SLOT(pushButtonMainClicked()));

    QPalette palette;
    if (!isCustomWindowStyle()) {
//        m_pButtonMain->setProperty("theme", "light");
        QLinearGradient gradient(mainPanel->rect().topLeft(), QPoint(mainPanel->rect().left(), 29));
        gradient.setColorAt(0, QColor(0xeee));
        gradient.setColorAt(1, QColor(0xe4e4e4));
        palette.setBrush(QPalette::Background, QBrush(gradient));
        label->setFixedHeight(0);
    }

    // Set TabWidget
    m_pTabs = new CAscTabWidget(mainPanel, pTabBar);
    m_pTabs->setObjectName(QString::fromUtf8("ascTabWidget"));
    _pMainGridLayout->addWidget(m_pTabs, 1, 0, 1, 3);
    m_pTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pTabs->activate(false);
    m_pTabs->applyUITheme(GetCurrentTheme().id());

    connect(m_pTabs, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(pTabBar, SIGNAL(tabBarClicked(int)), this, SLOT(onTabClicked(int)));
    connect(pTabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequest(int)));
    connect(m_pTabs, &CAscTabWidget::editorInserted, bind(&CMainWindow::onTabsCountChanged, this, _2, _1, 1));
    connect(m_pTabs, &CAscTabWidget::editorRemoved, bind(&CMainWindow::onTabsCountChanged, this, _2, _1, -1));
    m_pTabs->setPalette(palette);
    m_pTabs->setCustomWindowParams(isCustomWindowStyle());
    return mainPanel;
}

void CMainWindow::attachStartPanel(QCefView * const view)
{
    m_pMainWidget = qobject_cast<QWidget *>(view);
#ifdef __linux
    view->setMouseTracking(m_pButtonMain->hasMouseTracking());
#endif
    m_pMainWidget->setParent(m_pMainPanel);
    QGridLayout *_pMainGridLayout = dynamic_cast<QGridLayout*>(m_pMainPanel->layout());
    Q_ASSERT(_pMainGridLayout != nullptr);
    if (_pMainGridLayout)
        _pMainGridLayout->addWidget(m_pMainWidget, 1, 0, 1, 3);
    m_pMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (!m_pTabs->isActiveWidget())
        m_pMainWidget->show();
}

#ifdef __linux
void CMainWindow::setMouseTracking(bool enable)
{
    QWidget::setMouseTracking(enable);
    m_pMainPanel->findChild<QLabel *>("labelAppTitle")->setMouseTracking(enable);

    m_boxTitleBtns->setMouseTracking(enable);
    m_pTabs->setMouseTracking(enable);
    m_pTabs->tabBar()->setMouseTracking(enable);
    m_pButtonMain->setMouseTracking(enable);
    if (m_pTopButtons[BtnType::Btn_Minimize]) {
        foreach (auto btn, m_pTopButtons)
            btn->setMouseTracking(enable);
    }
    if ( m_pMainWidget )
        m_pMainWidget->setMouseTracking(enable);
}
#endif

void CMainWindow::pushButtonMainClicked()
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

void CMainWindow::toggleButtonMain(bool toggle, bool delay)
{
    auto _toggle = [=] (bool state) {
        if (m_pTabs->isActiveWidget() == state) {
            m_pButtonMain->setProperty("class", state ? "active" : "normal");
            m_pTabs->activate(!state);
            m_pMainWidget->setHidden(!state);
            if (!state)
                m_pTabs->setFocusedView();
            onTabChanged(m_pTabs->currentIndex());
        }
    };

    if ( delay ) {
        QTimer::singleShot(200, [=]{ _toggle(toggle); });
    } else {
        _toggle(toggle);
    }
}

void CMainWindow::onTabClicked(int index)
{
    Q_UNUSED(index)
    if (!m_pTabs->isActiveWidget()) {
        toggleButtonMain(false);
    }
}

void CMainWindow::onTabsCountChanged(int count, int i, int d)
{
    Q_UNUSED(i)
    Q_UNUSED(d)
    if ( count == 0 ) {
        toggleButtonMain(true);
    }
}

void CMainWindow::onEditorAllowedClose(int uid)
{
    if ( ((QCefView *)m_pMainWidget)->GetCefView()->GetId() == uid ) {
//        if ( m_pTabs->count() ) {
//            m_pMainWidget->setProperty("removed", true);
//        }
    } else {
        int _index = m_pTabs->tabIndexByView(uid);
        if ( !(_index < 0) ) {
            QWidget * _view = m_pTabs->widget(_index);
            m_pTabs->removeWidget(_view);
            _view->deleteLater();

            m_pTabs->tabBar()->removeTab(_index);
            //m_pTabs->adjustTabsSize();

            onTabChanged(m_pTabs->currentIndex());
            CInAppEventBase _event{CInAppEventBase::CEventType::etEditorClosed};
            AscAppManager::getInstance().commonEvents().signal(&_event);

            if ( !m_pTabs->count() ) {
                m_pTabs->setProperty("empty", true);
                m_pTabs->style()->polish(m_pTabs);
                toggleButtonMain(true);

                if ( m_isCloseAll ) {
                    emit aboutToClose();
                }
            }
        }
    }
}

void CMainWindow::onTabChanged(int index)
{
    QString title("");
    if (index > -1) {
        auto _panel = m_pTabs->panel(index);
        if (_panel)
            title = _panel->data()->title();
    } else {
        ((QCefView *)m_pMainWidget)->setFocusToCef();
    }

    if (title != windowTitle()) {
        QTimer::singleShot(100, this, [=]() {
            setWindowTitle(title);
        });
    }

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

void CMainWindow::onTabCloseRequest(int index)
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

int CMainWindow::tabCloseRequest(int index)
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
        } else {
            m_pTabs->editorCloseRequest(index);
        }
    }

    return MODAL_RESULT_YES;
}

int CMainWindow::trySaveDocument(int index)
{
    if (m_pTabs->closedByIndex(index)) return MODAL_RESULT_YES;

    int modal_res = MODAL_RESULT_NO;
    if ( m_pTabs->modifiedByIndex(index) ) {
        toggleButtonMain(false);
        m_pTabs->setCurrentIndex(index);

        modal_res = CMessage::showMessage(this, getSaveMessage().arg(m_pTabs->titleByIndex(index)),
                                          MsgType::MSG_WARN, MsgBtns::mbYesDefNoCancel);
        switch (modal_res) {
        case MODAL_RESULT_NO: break;
        case MODAL_RESULT_CANCEL: modal_res = MODAL_RESULT_CANCEL; break;
        case MODAL_RESULT_YES:
        default:{
            m_pTabs->editorCloseRequest(index);
            m_pTabs->panel(index)->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

            modal_res = MODAL_RESULT_YES;
            break;}
        }
    }

    return modal_res;
}

void CMainWindow::onPortalLogout(std::wstring wjson)
{
    if ( m_pTabs->count() ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            QString _portal = objRoot["domain"].toString(),
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

void CMainWindow::onPortalLogin(int viewid, const std::wstring &json)
{
    if ( !(json.find(L"uiTheme") == std::wstring::npos) ) {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toLatin1(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            QString _ui_theme = objRoot["uiTheme"].toString();
            if ( !_ui_theme.isEmpty() ) {
//                onFileLocation(vid, _url);

                if ( _ui_theme == "default-dark" )
                    m_pTabs->setTabThemeType(m_pTabs->tabIndexByView(viewid), "dark");
            }
        }
    }
}

void CMainWindow::doOpenLocalFile(COpenOptions& opts)
{
    QFileInfo info(opts.url);
    if (!info.exists()) { return; }
    if (!info.isFile()) { return; }
    if (!info.isReadable()) {
        QTimer::singleShot(0, this, [=] {
            CMessage::error(this, QObject::tr("Access to file '%1' is denied!").arg(opts.url));
        });
        return;
    }

    int result = m_pTabs->openLocalDocument(opts, true);
    if ( !(result < 0) ) {
        toggleButtonMain(false, true);
    } else
    if (result == -255) {
        QTimer::singleShot(0, this, [=] {
            CMessage::error(this, tr("File format not supported."));
        });
    }
    bringToTop();
}

void CMainWindow::onLocalFileRecent(void * d)
{
    CAscLocalOpenFileRecent_Recover * pData = static_cast<CAscLocalOpenFileRecent_Recover *>(d);
    COpenOptions opts = { pData->get_Path(),
          pData->get_IsRecover() ? etRecoveryFile : etRecentFile, pData->get_Id() };
    RELEASEINTERFACE(pData);
    onLocalFileRecent(opts);
}

void CMainWindow::onLocalFileRecent(const COpenOptions& opts)
{
    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(opts.url);

    bool forcenew = false;
    if ( !match.hasMatch() ) {
        QFileInfo _info(opts.url);
        if ( opts.srctype != etRecoveryFile && !_info.exists() ) {
            int modal_res = CMessage::showMessage(this, tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName()),
                                                  MsgType::MSG_WARN, MsgBtns::mbYesDefNo);
            if (modal_res == MODAL_RESULT_YES) {
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
        CMessage::error(this, tr("File format not supported."));
    }
}

void CMainWindow::createLocalFile(const QString& name, int format)
{
    COpenOptions opts{name, etNewFile};
    opts.format = format;

    int tabIndex = m_pTabs->addEditor(opts);

    if ( !(tabIndex < 0) ) {
        m_pTabs->setCurrentIndex(tabIndex);

        toggleButtonMain(false, true);
    }
}

void CMainWindow::onLocalFilesOpen(void * data)
{
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    std::vector<std::wstring> vctFiles = pData->get_Files();

    doOpenLocalFiles(&vctFiles);

    RELEASEINTERFACE(pData);
}

void CMainWindow::onLocalFileLocation(QString path)
{
    Utils::openFileLocation(path);
}

void CMainWindow::onFileLocation(int uid, QString param)
{
    if ( param == "offline" ) {
        QString path = m_pTabs->urlByView(uid);
        if ( !path.isEmpty() ) {
//            if ( Utils::isFileLocal(path) )
                onLocalFileLocation(path);
//            else {
//            }
        } else {
            CMessage::info(this, tr("Document must be saved firstly."));
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
                if (windowState().testFlag(Qt::WindowMinimized))
                    QMainWindow::setWindowState(Qt::WindowNoState);

                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(_tab_index);
            }
        }
    }
}

void CMainWindow::doOpenLocalFiles(const std::vector<std::wstring> * vec)
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

void CMainWindow::doOpenLocalFiles(const QStringList& list)
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

void CMainWindow::onDocumentType(int id, int type)
{
    m_pTabs->applyDocumentChanging(id, type);
}

void CMainWindow::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);
    QString name = QString::fromStdWString(pData->get_Name());
    QString path = !pData->get_Url().empty() ? QString() : QString::fromStdWString(pData->get_Path());
    m_pTabs->applyDocumentChanging(pData->get_Id(), name, path);
    onTabChanged(m_pTabs->currentIndex());
    RELEASEINTERFACE(pData);
}

void CMainWindow::onEditorConfig(int, std::wstring cfg)
{
    Q_UNUSED(cfg)
}

void CMainWindow::onWebAppsFeatures(int id, std::wstring opts)
{
    m_pTabs->setEditorOptions(id, opts);
}

void CMainWindow::onDocumentReady(int uid)
{
    if ( uid < 0 ) {
        QTimer::singleShot(20, this, [=]{
            refreshAboutVersion();
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"app:ready");
            focus(); // TODO: move to app manager
        });
    } else {
        m_pTabs->applyDocumentChanging(uid, DOCUMENT_CHANGED_LOADING_FINISH);
    }
    AscAppManager::getInstance().onDocumentReady(uid);
}

void CMainWindow::onDocumentLoadFinished(int uid)
{
    m_pTabs->applyDocumentChanging(uid, DOCUMENT_CHANGED_PAGE_LOAD_FINISH);
}

void CMainWindow::onDocumentChanged(int id, bool changed)
{
    m_pTabs->applyDocumentChanging(id, changed);
    onTabChanged(m_pTabs->currentIndex());
}

void CMainWindow::onDocumentSave(int id, bool cancel)
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

void CMainWindow::onDocumentSaveInnerRequest(int id)
{
    int modal_res = CMessage::showMessage(this, tr("Document must be saved to continue.<br>Save the document?"),
                                          MsgType::MSG_CONFIRM, MsgBtns::mbYesDefNo);
    CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
    pData->put_Value(modal_res == MODAL_RESULT_YES);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
    pEvent->m_pData = pData;

    AscAppManager::getInstance().GetViewById(id)->Apply(pEvent);
}

void CMainWindow::onDocumentDownload(void * info)
{
    CAscDownloadFileInfo *pData = reinterpret_cast<CAscDownloadFileInfo*>(info);
    if (!pData->get_IsCanceled()) {
        if ( !m_pWidgetDownload ) {
            m_pWidgetDownload = new CDownloadWidget(this);
            connect(m_pWidgetDownload, &QWidget::destroyed, this, [=]() {
                m_pWidgetDownload = nullptr;
            });
            QHBoxLayout * layoutBtns = qobject_cast<QHBoxLayout *>(m_boxTitleBtns->layout());
            layoutBtns->insertWidget(1, m_pWidgetDownload->toolButton());
            m_pWidgetDownload->show();
            std::vector<std::string> files{":/styles/download.qss"};
            m_pWidgetDownload->setStyleSheet(Utils::readStylesheets(&files));
            m_pWidgetDownload->applyTheme(m_pMainPanel->property("uitheme").toString());
            m_pWidgetDownload->updateScalingFactor(m_dpiRatio);
            m_pWidgetDownload->move(geometry().bottomRight() - m_pWidgetDownload->rect().bottomRight());
        }
        m_pWidgetDownload->downloadProcess(info);
    }
}

void CMainWindow::onDocumentFragmented(int id, bool isfragmented)
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

void CMainWindow::onDocumentFragmentedBuild(int vid, int error)
{
    int index = m_pTabs->tabIndexByView(vid);
    if ( error == 0 ) {
        m_pTabs->closeEditorByIndex(index, false);
    } else {
        m_pTabs->cancelDocumentSaving(index);
        AscAppManager::cancelClose();
    }
}

void CMainWindow::onEditorActionRequest(int vid, const QString& args)
{
    int index = m_pTabs->tabIndexByView(vid);
    if (!(index < 0)) {
        if (args.contains(QRegExp("action\\\":\\\"file:close"))) {
            bool _is_local = m_pTabs->isLocalByIndex(index);
            onTabCloseRequest(index);
            if (!_is_local) {
                QJsonParseError jerror;
                QJsonDocument jdoc = QJsonDocument::fromJson(args.toLatin1(), &jerror);
                if(jerror.error == QJsonParseError::NoError) {
                    QJsonObject objRoot = jdoc.object();
                    QString _url = objRoot["url"].toString();
                    if (!_url.isEmpty())
                        onFileLocation(vid, _url);
                    else _is_local = true;
                }
            }
        }
    }
}

void CMainWindow::goStart()
{
//    loadStartPage();
    toggleButtonMain(true);
}

void CMainWindow::onDocumentPrint(void * opts)
{
    Q_UNUSED(opts)
#ifdef __OS_WIN_XP
    if (QPrinterInfo::availablePrinterNames().size() == 0) {
        CMessage::info(this, tr("There are no printers available"));
        return;
    }
#endif

    static bool printInProcess = false;
    if (!printInProcess)
        printInProcess = true; else
        return;

#ifdef Q_OS_LINUX
    WindowHelper::CParentDisable disabler(qobject_cast<QWidget*>(this));
#endif

    CCefView * pView = AscAppManager::getInstance().GetViewById(AscAppManager::printData().viewId());

    int pagesCount = AscAppManager::printData().pagesCount(),
        currentPage = AscAppManager::printData().pageCurrent();


    if (pView && !(pagesCount < 1)) {
        NSEditorApi::CAscMenuEvent * pEvent;
        QAscPrinterContext * pContext = new QAscPrinterContext(AscAppManager::printData().printerInfo());
        QString documentName = m_pTabs->titleByIndex(m_pTabs->tabIndexByView(AscAppManager::printData().viewId()), true);

        QPrinter * printer = pContext->getPrinter();
        printer->setFromTo(1, pagesCount);
        printer->printEngine()->setProperty(QPrintEngine::PPK_DocumentName, documentName);
        printer->setDuplex(AscAppManager::printData().duplexMode());
        if ( printer->supportsMultipleCopies() ) {
            printer->setCopyCount(AscAppManager::printData().copiesCount());
        }

        if ( !AscAppManager::printData().isQuickPrint() ) {
            printer->setPageOrientation(AscAppManager::printData().pageOrientation());
            printer->setPageSize(AscAppManager::printData().pageSize());
            printer->setFullPage(true);
        }

#ifdef _WIN32
        printer->setOutputFileName("");
        PrintDialog * dialog =  new PrintDialog(printer, this);
#else
        QFileInfo info(documentName);
        QString pdfName = Utils::lastPath(LOCAL_PATH_SAVE) + "/" + info.baseName() + ".pdf";
        QString outputName = AscAppManager::printData().isQuickPrint() ? Utils::uniqFileName(pdfName) : pdfName;
        if ( AscAppManager::printData().printerInfo().printerName().isEmpty() ) {
            printer->setOutputFileName(outputName);
        } else {
            printer->printEngine()->setProperty(QPrintEngine::PPK_OutputFileName, outputName);
        }

# ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
        CPrintDialog * dialog =  new CPrintDialog(printer, this);
# else
        GtkPrintDialog * dialog = new GtkPrintDialog(printer, this);
# endif
#endif // _WIN32

        dialog->setWindowTitle(tr("Print Document"));
        dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintToFile);
        if (!(currentPage < 0)) {
            currentPage++;
            dialog->setEnabledOptions(dialog->enabledOptions() | QPrintDialog::PrintCurrentPage);
            dialog->setOptions(dialog->options() | QPrintDialog::PrintCurrentPage);
        }
        dialog->setPrintRange(AscAppManager::printData().printRange());
        if ( dialog->printRange() == QPrintDialog::PageRange )
            dialog->setFromTo(AscAppManager::printData().pageFrom(), AscAppManager::printData().pageTo());

        int modal_res = QDialog::Accepted;
        if ( AscAppManager::printData().isQuickPrint() ) {
            dialog->accept();
        } else modal_res = dialog->exec();
        qApp->processEvents();

        if ( modal_res == QDialog::Accepted ) {
            if ( !AscAppManager::printData().isQuickPrint() )
                AscAppManager::printData().setPrinterInfo(*printer);

            QVector<PageRanges> page_ranges;

#ifdef Q_OS_LINUX
            if ( printer->outputFormat() == QPrinter::PdfFormat ) {
                if ( !AscAppManager::printData().isQuickPrint() ) {
                    info.setFile(printer->outputFileName());
                    Utils::keepLastPath(LOCAL_PATH_SAVE, info.absolutePath());
                }
            } else {
                if ( AscAppManager::printData().isQuickPrint() && !printer->outputFileName().isEmpty() ) {
                    info.setFile(printer->outputFileName());
                    if ( info.suffix() == "pdf" )
                        printer->setOutputFileName("");
                }
            }
#endif

            switch(dialog->printRange()) {
            case QPrintDialog::AllPages:
                page_ranges.append(PageRanges(1, pagesCount));
                break;
            case QPrintDialog::PageRange:
                page_ranges = dialog->getPageRanges();
                break;
            case QPrintDialog::Selection:
                page_ranges.append(PageRanges(-1, -1));
                break;
            case QPrintDialog::CurrentPage:
                page_ranges.append(PageRanges(currentPage, currentPage));
                break;
            }

            CEditorTools::print({pView, pContext, &page_ranges, this});
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
//    RELEASEINTERFACE(pData)
}

void CMainWindow::onFullScreen(int id, bool apply)
{
    if (apply) {
        if (isVisible()) {
            m_isMaximized = windowState().testFlag(Qt::WindowMaximized);
            m_pTabs->setFullScreen(apply, id);
            QTimer::singleShot(0, this, [=] {
                CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER);
                AscAppManager::getInstance().GetViewById(id)->Apply(pEvent);
            });
        }
    } else
    if (isHidden()) {
        m_pTabs->setFullScreen(apply);
        toggleButtonMain(false);
        QCoreApplication::processEvents();
        focus();
    }
}

void CMainWindow::onKeyDown(void * eventData)
{
    NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)eventData;
    int key = pData->get_KeyCode();
    bool _is_ctrl = pData->get_IsCtrl();
    bool _is_shift = pData->get_IsShift();

    RELEASEINTERFACE(pData)
    switch (key) {
    case 'W':
    case VK_F4:
        if (_is_ctrl && m_pTabs->isActiveWidget()) {
            onTabCloseRequest(m_pTabs->currentIndex());
        }
        break;
    case VK_TAB:
        if (m_pTabs->count()) {
            if (_is_ctrl) {
                int _new_index = 0;
                if (_is_shift) {
                    if ( m_pTabs->isActiveWidget() )
                        _new_index = m_pTabs->currentIndex() - 1; else
                        _new_index = m_pTabs->count() - 1;
                } else {
                    if (m_pTabs->isActiveWidget())
                        _new_index = m_pTabs->currentIndex() + 1;
                }

                if (_new_index < 0 || !(_new_index < m_pTabs->count()))
                    toggleButtonMain(true);
                else {
                    toggleButtonMain(false);
                    m_pTabs->setCurrentIndex(_new_index);
                }
            }
        }
        break;
    case VK_F1:
        if ( _is_ctrl && _is_shift ) {
            GET_REGISTRY_USER(reg_user)

            CFileDialogWrapper _dlg(this);
            QString _dir = _dlg.selectFolder(reg_user.contains("helpUrl") ?
                        reg_user.value("helpUrl").toString() : Utils::lastPath(LOCAL_PATH_OPEN));

            if ( !_dir.isEmpty() ) {
                QString _path = _dir + "/apps/documenteditor/main/resources/help/en/Contents.json";
                if( QFileInfo::exists(_path) ) {
                    reg_user.setValue("helpUrl", _dir + "/apps");
                    EditorJSVariables::setVariable("helpUrl", _dir + "/apps");
                    EditorJSVariables::apply();

                    CMessage::error(this, "Successfully");
                } else {
                    CMessage::error(this, "Failed");
                }
            }
        }
        break;
    }
}

void CMainWindow::onPortalOpen(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);
    if (jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();
        QString _portal = objRoot["portal"].toString(),
                _entry = objRoot["entrypage"].toString();
        if (!_portal.isEmpty()) {
            int res = m_pTabs->openPortal(_portal, objRoot["provider"].toString("onlyoffice"), _entry);
            if (!(res < 0)) {
                toggleButtonMain(false, true);
                m_pTabs->setCurrentIndex(res);
            }

            QString _title = objRoot["title"].toString();
            if ( !_title.isEmpty() ) {
                m_pTabs->applyDocumentChanging(m_pTabs->viewByIndex(res), _title, "");
            }
        }
    }
}

void CMainWindow::onPortalNew(QString in)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(in.toLatin1(), &jerror);
    if (jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();
        QString _domain = objRoot["domain"].toString();
        QString _name = Utils::getPortalName(_domain);
        int _tab_index = m_pTabs->tabIndexByEditorType(etNewPortal);
        if (!(_tab_index < 0)) {
            int _uid = m_pTabs->viewByIndex(_tab_index);
            m_pTabs->applyDocumentChanging(_uid, _name, _domain);
            m_pTabs->applyDocumentChanging(_uid, etPortal);
            onTabChanged(m_pTabs->currentIndex());
        }
    }
}

void CMainWindow::onPortalCreate()
{
    QString _url = URL_SIGNUP;
    GET_REGISTRY_SYSTEM(reg_system)
    if (reg_system.contains("Store"))
            _url += "&store=" + reg_system.value("Store").toString();
    int res = m_pTabs->newPortal(_url, tr("Sign Up"));
    if (!(res < 0)) {
        toggleButtonMain(false, true);
        m_pTabs->setCurrentIndex(res);
    }
}

void CMainWindow::onOutsideAuth(QString json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);
    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject objRoot = jdoc.object();
        QString _domain = objRoot["portal"].toString();
        int _tab_index = m_pTabs->tabIndexByTitle(Utils::getPortalName(_domain), etPortal);
        if (_tab_index < 0) {
            _tab_index = m_pTabs->addOAuthPortal(_domain,
                    objRoot["type"].toString(), objRoot["provider"].toString(),
                    objRoot["entrypage"].toString());
        }
        if (!(_tab_index < 0)) {
            m_pTabs->setCurrentIndex(_tab_index);
            toggleButtonMain(false, true);
        }
    }
}

void CMainWindow::onReporterMode(int id, bool status)
{
    if ( m_pTabs->fullScreenWidget() ) {
        CTabPanel * _widget = qobject_cast<CTabPanel *>(m_pTabs->fullScreenWidget());
        if (_widget && _widget->cef()->GetId() == id) {
            _widget->setReporterMode(status);
            return;
        }
    }

    int _i{m_pTabs->tabIndexByView(id)};
    if ( !(_i < 0) ) {
        m_pTabs->panel(_i)->setReporterMode(status);
    }
}

void CMainWindow::updateScalingFactor(double dpiratio)
{
    CScalingWrapper::updateScalingFactor(dpiratio);
    /*QLayout * layoutBtns = m_boxTitleBtns->layout();
    layoutBtns->setSpacing(int(1 * dpiratio));
    if (isCustomWindowStyle()) {
        layoutBtns->setContentsMargins(0,0,0,0);
        QSize small_btn_size(int(TOOLBTN_WIDTH*dpiratio), int(TOOLBTN_HEIGHT*dpiratio));
        foreach (auto btn, m_pTopButtons)
            btn->setFixedSize(small_btn_size);
    }*/
    m_pButtonMain->setFixedSize(int(BUTTON_MAIN_WIDTH * dpiratio), int(TITLE_HEIGHT * dpiratio));
    m_pMainPanel->setProperty("zoom", QString::number(dpiratio) + "x");
    std::vector<std::string> _files{":/styles/tabbar.qss"};
    QString _style = Utils::readStylesheets(&_files);
    m_pTabs->tabBar()->setStyleSheet(_style);
    m_pTabs->setStyleSheet(_style);
//    m_pTabs->updateScalingFactor(dpiratio);
    m_pTabs->reloadTabIcons();
    m_pButtonMain->setIcon(MAIN_ICON_PATH, GetCurrentTheme().isDark() ? "logo-light" : "logo-dark");
    m_pButtonMain->setIconSize(MAIN_ICON_SIZE * dpiratio);
    if (m_pWidgetDownload && m_pWidgetDownload->toolButton()) {
        m_pWidgetDownload->updateScalingFactor(dpiratio);
        m_pWidgetDownload->toolButton()->style()->polish(m_pWidgetDownload->toolButton());
    }
}

void CMainWindow::setScreenScalingFactor(double factor, bool resize)
{
    CWindowPlatform::setScreenScalingFactor(factor, resize);
    QString css(AscAppManager::getWindowStylesheets(factor));
    if (!css.isEmpty()) {
        m_pMainPanel->setStyleSheet(css);
    }
    updateScalingFactor(factor);
    CScalingWrapper::updateChildScaling(m_pMainPanel, factor);
}

QString CMainWindow::getSaveMessage() const
{
    return tr("%1 is modified.<br>Do you want to keep changes?");
}

bool CMainWindow::holdUid(int uid) const
{
    CCefView * _view = (qobject_cast<QCefView *>(m_pMainWidget))->GetCefView();
    bool _res_out = _view->GetId() == uid;
    if (!_res_out) {
        CTabPanel * _widget = qobject_cast<CTabPanel *>(m_pTabs->fullScreenWidget());
        if (_widget) {
            _res_out = _widget->cef()->GetId() == uid;
        }
    }
    return _res_out ? true : !(m_pTabs->tabIndexByView(uid) < 0);
}

bool CMainWindow::holdUrl(const QString& url, AscEditorType type) const
{
    if (type == etPortal) {
        return !(m_pTabs->tabIndexByTitle(Utils::getPortalName(url), etPortal) < 0);
    } else
    if (type == etLocalFile) {
        return !(m_pTabs->tabIndexByUrl(url) < 0);
    }
    return false;
}

int CMainWindow::startPanelId()
{
    return ((QCefView *)m_pMainWidget)->GetCefView()->GetId();
}

CAscTabWidget * CMainWindow::tabWidget()
{
    return m_pTabs;
}

void CMainWindow::showEvent(QShowEvent * e)
{
    CWindowPlatform::showEvent(e);
    cancelClose();
}

bool CMainWindow::isAboutToClose() const
{
    return m_isCloseAll;
}

void CMainWindow::cancelClose()
{
    m_isCloseAll && (m_isCloseAll = false);
}
