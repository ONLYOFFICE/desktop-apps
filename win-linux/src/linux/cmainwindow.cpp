/*
 * (c) Copyright Ascensio System SIA 2010-2017
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

#include "cmainwindow.h"
#include "../defines.h"
#include "../prop/cmainpanelimpl.h"
#include "../cascapplicationmanagerwrapper.h"
#include <QProxyStyle>
#include <QApplication>
#include <QFileInfo>
#include <QDesktopWidget>
#include "../utils.h"
#include <QTimer>
#include <QMimeData>
#include "singleapplication.h"

extern QStringList g_cmdArgs;

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , CX11Decoration(this)
{
//    resize(1200, 700);
    setAcceptDrops(true);
}

CMainWindow::CMainWindow(const QRect& geometry)
    : CMainWindow(nullptr)
{
    parseInputArgs(g_cmdArgs);

    setWindowIcon(Utils::appIcon());
    setObjectName("MainWindow");

    GET_REGISTRY_USER(reg_user)

    QString _title_style = reg_user.value("titlebar").toString();
    if ( _title_style.isEmpty() ) {
        GET_REGISTRY_SYSTEM(reg_system);
        _title_style = reg_system.value("titlebar").toString();
    }

    if ( _title_style == "custom" )
        CX11Decoration::turnOff();

    // adjust window size
    QRect _window_rect = geometry;
    m_dpiRatio = Utils::getScreenDpiRatio( QApplication::desktop()->screenNumber(_window_rect.topLeft()) );

    if ( _window_rect.isEmpty() )
        _window_rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);

    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if ( _screen_size.width() < _window_rect.width() + 120 ||
            _screen_size.height() < _window_rect.height() + 120 )
    {
        _window_rect.setLeft(_screen_size.left()),
        _window_rect.setTop(_screen_size.top());

        if ( _screen_size.width() < _window_rect.width() ) _window_rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < _window_rect.height() ) _window_rect.setHeight(_screen_size.height());
    }

    resize(_window_rect.width(), _window_rect.height());

    m_pMainPanel = new CMainPanelImpl(this, !CX11Decoration::isDecorated(), m_dpiRatio);
    setCentralWidget(m_pMainPanel);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget((m_pMainPanel)->getTitleWidget());
        (m_pMainPanel)->setMouseTracking(true);
        setMouseTracking(true);
    }

    restoreGeometry(reg_user.value("position").toByteArray());
    restoreState(reg_user.value("windowstate").toByteArray());

    QMetaObject::connectSlotsByName(this);

    connect(m_pMainPanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow::slot_windowChangeState);
    connect(m_pMainPanel, &CMainPanel::mainWindowClose, this, &CMainWindow::slot_windowClose);

    SingleApplication * app = static_cast<SingleApplication *>(QCoreApplication::instance());
    m_pMainPanel->setInputFiles(Utils::getInputFiles(g_cmdArgs));
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling();
    m_pMainPanel->goStart();

    connect(app, &SingleApplication::showUp, [=](QString args){
        QStringList * _list = Utils::getInputFiles(args.split(";"));

        // remove app's self name from start arguments
        if ( !_list->isEmpty() ) _list->removeFirst();

        if ( !_list->isEmpty() ) {
            m_pMainPanel->doOpenLocalFiles(*_list);
        }

        delete _list, _list = NULL;

        CX11Decoration::raiseWindow();
    });
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::parseInputArgs(const QStringList& inlist)
{
    GET_REGISTRY_USER(reg_user)

    if ( !inlist.isEmpty() ) {
        QString _arg;
        QStringListIterator i(inlist); i.next();
        while (i.hasNext()) {
            _arg = i.next();

            if (_arg.contains("--system-title-bar")) {
                reg_user.setValue("titlebar", "system");
            } else
            if (_arg.contains("--custom-title-bar")) {
                reg_user.setValue("titlebar", "custom");
            }
        }
    }

    if ( !reg_user.contains("titlebar") )
        reg_user.setValue("titlebar", "custom");
}

void CMainWindow::closeEvent(QCloseEvent * e)
{
    ((CMainPanel *)m_pMainPanel)->pushButtonCloseClicked();
    e->ignore();
}

void CMainWindow::showEvent(QShowEvent * e)
{
    Q_UNUSED(e)

//    qDebug() << "SHOW EVENT: " << e->type();
}

bool CMainWindow::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );

        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);

//        if( _e_statechange->oldState() & Qt::WindowMinimized ) {
//            qDebug() << "Window restored (to normal or maximized state)!";
//        } else
        if( _e_statechange->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized ) {
            ((CMainPanel *)m_pMainPanel)->applyMainWindowState(Qt::WindowMaximized);
        } else
        if (/*_e_statechange->oldState() == Qt::WindowMaximized &*/ this->windowState() == Qt::WindowNoState) {
            ((CMainPanel *)m_pMainPanel)->applyMainWindowState(Qt::WindowNoState);
        }
    }

    return QMainWindow::event(event);
}

void CMainWindow::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}
void CMainWindow::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}
void CMainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.length() != 1)
        return;

    QSet<QString> _exts;
    _exts << "docx" << "doc" << "odt" << "rtf" << "txt" << "doct";
    _exts << "html" << "mht" << "epub";
    _exts << "pptx" << "ppt" << "odp" << "ppsx" << "pptt";
    _exts << "xlsx" << "xls" << "ods" << "csv" << "xlst";
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
        ((CMainPanel *)m_pMainPanel)->doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}

void CMainWindow::slot_windowChangeState(Qt::WindowState s)
{
    if (s == Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", saveGeometry());
        reg_user.setValue("windowstate", saveState());

        showFullScreen();
    } else {
        setWindowState(s);

        switch (s) {
        case Qt::WindowMaximized:
        case Qt::WindowMinimized:
            break;
        default:
        case Qt::WindowNoState:
            activateWindow();
            break;
        }
    }
}

void CMainWindow::slot_windowClose()
{
    if (windowState() != Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", saveGeometry());
        reg_user.setValue("windowstate", saveState());
    }

    AscAppManager::closeMainWindow( (size_t)this );
}

CMainPanel * CMainWindow::mainPanel() const
{
    return m_pMainPanel;
}

bool CMainWindow::holdView(uint id) const
{
    return m_pMainPanel->holdUid(id);
}

QRect CMainWindow::windowRect() const
{
    return geometry();
}

bool CMainWindow::isMaximized() const
{
    return windowState() == Qt::WindowMaximized;
}
