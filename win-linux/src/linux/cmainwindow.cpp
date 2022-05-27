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

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "cdialogopenssl.h"
#endif

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
    setWindowIcon(Utils::appIcon());
    setObjectName("MainWindow");

    GET_REGISTRY_USER(reg_user)

    if ( InputArgs::contains(L"--system-title-bar") )
        reg_user.setValue("titlebar", "system");
    else
    if ( InputArgs::contains(L"--custom-title-bar") )
        reg_user.setValue("titlebar", "custom");

    if ( !reg_user.contains("titlebar") )
        reg_user.setValue("titlebar", "custom");

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

    // TODO: skip window min size for usability test
//    QSize _window_min_size{MAIN_WINDOW_MIN_WIDTH * m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio};
//    if ( _window_rect.width() < _window_min_size.width() )
//        _window_rect.setWidth(_window_min_size.width());

//    if ( _window_rect.height() < _window_min_size.height() )
//        _window_rect.setHeight(_window_min_size.height());

    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if ( _screen_size.width() < _window_rect.width() + 120 ||
            _screen_size.height() < _window_rect.height() + 120 )
    {
        _window_rect.setLeft(_screen_size.left()),
        _window_rect.setTop(_screen_size.top());

        if ( _screen_size.width() < _window_rect.width() ) _window_rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < _window_rect.height() ) _window_rect.setHeight(_screen_size.height());
    }

    // TODO: skip window min size for usability test
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(_window_rect, _window_min_size));
    setGeometry(_window_rect);

    m_pMainPanel = new CMainPanelImpl(this, !CX11Decoration::isDecorated(), m_dpiRatio);
    setCentralWidget(m_pMainPanel);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget((m_pMainPanel)->getTitleWidget());
        (m_pMainPanel)->setMouseTracking(true);
        setMouseTracking(true);

        QPalette _palette(palette());
        _palette.setColor(QPalette::Background, AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground));
        setStyleSheet(QString("QMainWindow{border:1px solid %1;}").arg(QString::fromStdWString(AscAppManager::themes().current().value(CTheme::ColorRole::ecrWindowBorder))));
        setAutoFillBackground(true);
        setPalette(_palette);
    }

//    restoreGeometry(reg_user.value("position").toByteArray());
//    restoreState(reg_user.value("windowstate").toByteArray());

    QMetaObject::connectSlotsByName(this);

    connect(m_pMainPanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow::slot_windowChangeState);
    connect(m_pMainPanel, &CMainPanel::mainWindowWantToClose, this, &CMainWindow::slot_windowClose);
    connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, this, &CMainWindow::slot_modalDialog);

    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    m_pMainPanel->updateScaling(m_dpiRatio);
    m_pMainPanel->goStart();
}

CMainWindow::~CMainWindow()
{
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
    static bool _flg_motion = false;
    static bool _flg_left_button = false;

    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
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
        } else
        if (this->windowState() == Qt::WindowMinimized) {
            ((CMainPanel *)m_pMainPanel)->applyMainWindowState(Qt::WindowMinimized);
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            updateScaling();
        }

        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if ( !_flg_motion )
            _flg_motion = true;
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
        ((CMainPanel *)m_pMainPanel)->doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}

void CMainWindow::slot_windowChangeState(Qt::WindowState s)
{
    if (s == Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", normalGeometry());
        reg_user.setValue("maximized", windowState().testFlag(Qt::WindowMaximized));
//        reg_user.setValue("windowstate", saveState());

//        showFullScreen();
    } else {
        if ( s == Qt::WindowMinimized && windowState().testFlag(Qt::WindowMaximized) ) {
            CX11Decoration::setMinimized();
        } else setWindowState(s);

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
    if (windowState() != Qt::WindowFullScreen && isVisible()) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", normalGeometry());
        reg_user.setValue("maximized", windowState().testFlag(Qt::WindowMaximized));
//        reg_user.setValue("windowstate", saveState());
    }

    AscAppManager::closeMainWindow();
}

void CMainWindow::slot_modalDialog(bool status, WId h)
{
    static WindowHelper::CParentDisable * const _disabler = new WindowHelper::CParentDisable;

    if ( status ) {
        _disabler->disable(this);
    } else _disabler->enable();
}

void CMainWindow::setScreenScalingFactor(double factor)
{
    CX11Decoration::onDpiChanged(factor);
    QString css(AscAppManager::getWindowStylesheets(factor));

    if ( !css.isEmpty() ) {
        QRect _src_rect = geometry();

        setMinimumSize({0, 0});

        m_pMainPanel->setStyleSheet(css);
        m_pMainPanel->setScreenScalingFactor(factor);

        double change_factor = factor / m_dpiRatio;
        m_dpiRatio = factor;

        if ( !isMaximized() ) {
            int dest_width_change = int(_src_rect.width() * (1 - change_factor));
            QRect dest_rect = QRect{_src_rect.translated(dest_width_change/2,0).topLeft(), _src_rect.size() * change_factor};

            setGeometry(dest_rect);
        }

        // TODO: skip window min size for usability test
//        setMinimumSize(WindowHelper::correctWindowMinimumSize(_src_rect, {MAIN_WINDOW_MIN_WIDTH * factor, MAIN_WINDOW_MIN_HEIGHT * factor}));
    }
}

CMainPanel * CMainWindow::mainPanel() const
{
    return m_pMainPanel;
}

QRect CMainWindow::windowRect() const
{
    return normalGeometry();
}

bool CMainWindow::isMaximized() const
{
    return windowState() == Qt::WindowMaximized;
}

void CMainWindow::sendSertificate(int viewid)
{
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    CDialogOpenSsl _dialog(this);

    NSEditorApi::CAscOpenSslData * pData = new NSEditorApi::CAscOpenSslData;
    if ( _dialog.exec() == QDialog::Accepted ) {
        _dialog.getResult(*pData);
    }

    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE);
    pEvent->m_pData = pData;
    AscAppManager::getInstance().GetViewById(viewid)->Apply(pEvent);
#endif
}

QWidget * CMainWindow::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CMainWindow *>(this));
}

void CMainWindow::captureMouse(int tabindex)
{
    CMainWindowBase::captureMouse(tabindex);

    if ( !(tabindex < 0) &&
            tabindex < mainPanel()->tabWidget()->count() )
    {
        QPoint spt = mainPanel()->tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        QPoint gpt = mainPanel()->tabWidget()->tabBar()->mapToGlobal(spt);

//        CX11Decoration::setCursorPos(100, 100);

//        QCursor::setPos(0, 0);
        QTimer::singleShot(0,[=] {
            QMouseEvent event(QEvent::MouseButtonPress, spt, Qt::LeftButton, Qt::MouseButton::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent((QWidget *)mainPanel()->tabWidget()->tabBar(), &event);
            mainPanel()->tabWidget()->tabBar()->grabMouse();
//            mainPanel()->tabWidget()->grabMouse();
        });
    }
}

void CMainWindow::bringToTop() const
{
    QApplication::setActiveWindow(const_cast<CMainWindow *>(this));
}

void CMainWindow::show(bool maximized)
{
    QMainWindow::show();

    if ( maximized )
        slot_windowChangeState(Qt::WindowMaximized);
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    CMainWindowBase::applyTheme(theme);

    if ( !CX11Decoration::isDecorated() ) {
        QPalette _palette(palette());
        _palette.setColor(QPalette::Background, AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground));
        setStyleSheet(QString("QMainWindow{border:1px solid %1;}").arg(QString::fromStdWString(AscAppManager::themes().current().value(CTheme::ColorRole::ecrWindowBorder))));
        setAutoFillBackground(true);
        setPalette(_palette);
    }
}

void CMainWindow::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);

    if ( dpi_ratio != m_dpiRatio )
        setScreenScalingFactor(dpi_ratio);
}
