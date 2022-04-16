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

#include "linux/cmainwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QDesktopWidget>
#include <QTimer>
#include <QMimeData>


#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "cdialogopenssl.h"
#endif

class CMainWindow::impl {
    CMainWindow * m_owner = nullptr;
    WindowHelper::CParentDisable * m_disabler = nullptr;
public:
    impl(CMainWindow * owner)
        : m_owner{owner}
        , m_disabler{new WindowHelper::CParentDisable}
    {}

    ~impl()
    {
        delete m_disabler,
        m_disabler = nullptr;
    }

    void lockParentUI(){
        m_disabler->disable(m_owner);
    }

    void unlockParentUI() {
        m_disabler->enable();
    }
};

auto adjustRect(QRect& window_rect, const double dpiRatio)->void
{
    if (window_rect.isEmpty())
        window_rect = QRect(100, 100, 1324 * dpiRatio, 800 * dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(window_rect.topLeft());
    if (_screen_size.width() < window_rect.width() + 120 ||
            _screen_size.height() < window_rect.height() + 120 )
    {
        window_rect.setLeft(_screen_size.left()),
        window_rect.setTop(_screen_size.top());
        if (_screen_size.width() < window_rect.width()) window_rect.setWidth(_screen_size.width());
        if (_screen_size.height() < window_rect.height()) window_rect.setHeight(_screen_size.height());
    }
}

CMainWindow::CMainWindow(const QRect &rect) :
    CMainWindow(rect, WindowType::MAIN, QString(), nullptr)
{}

CMainWindow::CMainWindow(const QRect &rect, const QString &title, QWidget *panel) :
    CMainWindow(rect, WindowType::SINGLE, title, panel)
{}

CMainWindow::CMainWindow(const QRect &rect, const QString &title, QCefView *view) :
    CMainWindow(rect, WindowType::REPORTER, title, static_cast<QWidget*>(view))
{}

CMainWindow::CMainWindow(const QRect &rect, const WindowType winType, const QString &title, QWidget *widget) :
    QMainWindow(nullptr),
    CX11Decoration(this),
    CMainWindowBase(const_cast<QRect&>(rect)),
    m_winType(winType)
{
    setWindowIcon(Utils::appIcon());
    if (m_winType == WindowType::MAIN) {
        setAcceptDrops(true);
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
        QRect _window_rect = rect;
        m_dpiRatio = Utils::getScreenDpiRatio( QApplication::desktop()->screenNumber(_window_rect.topLeft()) );
        adjustRect(_window_rect, m_dpiRatio);
        // TODO: skip window min size for usability test
        setGeometry(_window_rect);

        _m_pMainPanel = new CMainPanelImpl(this, !CX11Decoration::isDecorated(), m_dpiRatio);
        setCentralWidget(_m_pMainPanel);

        if ( !CX11Decoration::isDecorated() ) {
            CX11Decoration::setTitleWidget((_m_pMainPanel)->getTitleWidget());
            (_m_pMainPanel)->setMouseTracking(true);
            setMouseTracking(true);

            QPalette _palette(palette());
            _palette.setColor(QPalette::Background, AscAppManager::themes().current()
                              .color(CTheme::ColorRole::ecrWindowBackground));
            setStyleSheet(QString("QMainWindow{border:1px solid %1;}")
                          .arg(QString::fromStdWString(AscAppManager::themes().current().
                                                       value(CTheme::ColorRole::ecrWindowBorder))));
            setAutoFillBackground(true);
            setPalette(_palette);
        }

        QMetaObject::connectSlotsByName(this);

        connect(_m_pMainPanel, &CMainPanel::mainWindowChangeState, this, &CMainWindow::slot_windowChangeState);
        connect(_m_pMainPanel, &CMainPanel::mainWindowWantToClose, this, &CMainWindow::slot_windowClose);
        connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, this, &CMainWindow::slot_modalDialog);

        _m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
        _m_pMainPanel->updateScaling(m_dpiRatio);
        _m_pMainPanel->goStart();
    } else
    if (m_winType == WindowType::SINGLE) {
        pimpl = std::unique_ptr<impl>(new impl(this));
        CMainWindowBase::setWindowTitle(title);
        if (isCustomWindowStyle())
            CX11Decoration::turnOff();
        setGeometry(rect);
    //    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio}));
        connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, this, &CMainWindow::slot_modalDialog);
    } else
    if (m_winType == WindowType::REPORTER) {
        CMainWindowBase::setWindowTitle(title);
        GET_REGISTRY_SYSTEM(reg_system)
        GET_REGISTRY_USER(reg_user)
        if (reg_user.value("titlebar") == "custom" ||
                reg_system.value("titlebar") == "custom" )
            CX11Decoration::turnOff();
        // adjust window size
        QRect _window_rect = rect;
        m_dpiRatio = Utils::getScreenDpiRatio(QApplication::desktop()->screenNumber(rect.topLeft()));
        adjustRect(_window_rect, m_dpiRatio);
    //    setMinimumSize(WindowHelper::correctWindowMinimumSize(_window_rect, {WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio}));
        setGeometry(_window_rect);

        m_pMainPanel = createMainPanel(this, title, !CX11Decoration::isDecorated(), widget);
        if ( !CX11Decoration::isDecorated() ) {
            CX11Decoration::setTitleWidget(m_boxTitleBtns);
            m_pMainPanel->setMouseTracking(true);
            setMouseTracking(true);
        }
        setCentralWidget(m_pMainPanel);
        updateGeometry();
    }
}

CMainWindow::~CMainWindow()
{

}

/** Public **/

QWidget * CMainWindow::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CMainWindow *>(this));
}

void CMainWindow::show(bool maximized)
{
    QMainWindow::show();
    if (maximized) {
        if (m_winType == WindowType::MAIN) {
            slot_windowChangeState(Qt::WindowMaximized);
        } else if (m_winType == WindowType::SINGLE) {
            QMainWindow::setWindowState(Qt::WindowMaximized);
        }
    }
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

bool CMainWindow::isMaximized() const
{
    return windowState() == Qt::WindowMaximized;
}

CMainPanel * CMainWindow::mainPanel() const
{
    return _m_pMainPanel;
}

QRect CMainWindow::windowRect() const
{
    return normalGeometry();
}

void CMainWindow::bringToTop()
{
    QMainWindow::raise();
    QMainWindow::activateWindow();
}

void CMainWindow::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio )
        setScreenScalingFactor(dpi_ratio);
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    if (m_winType == WindowType::MAIN) {
        CMainWindowBase::applyTheme(theme);
        if ( !CX11Decoration::isDecorated() ) {
            QPalette _palette(palette());
            _palette.setColor(QPalette::Background, AscAppManager::themes()
                          .current().color(CTheme::ColorRole::ecrWindowBackground));
            setStyleSheet(QString("QMainWindow{border:1px solid %1;}")
                          .arg(QString::fromStdWString(AscAppManager::themes().current()
                          .value(CTheme::ColorRole::ecrWindowBorder))));
            setAutoFillBackground(true);
            setPalette(_palette);
        }
    } else
    if (m_winType == WindowType::REPORTER) {
        m_pMainPanel->setProperty("uitheme", QString::fromStdWString(theme));
        if ( m_boxTitleBtns ) {
            m_labelTitle->style()->polish(m_labelTitle);
            m_buttonMinimize->style()->polish(m_buttonMinimize);
            m_buttonMaximize->style()->polish(m_buttonMaximize);
            m_buttonClose->style()->polish(m_buttonClose);
            m_boxTitleBtns->style()->polish(m_boxTitleBtns);
        }
        m_pMainPanel->style()->polish(m_pMainPanel);
        update();
    }
}

bool CMainWindow::holdView(int id) const
{
    if (m_winType == WindowType::REPORTER) {
        QWidget * mainView = m_pMainPanel->findChild<QWidget *>("mainView");
        return mainView && ((QCefView *)mainView)->GetCefView()->GetId() == id;
    } else {
        return mainPanel()->holdUid(id);
    }
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
    Q_UNUSED(h)
    if (m_winType == WindowType::MAIN) {
        static WindowHelper::CParentDisable * const _disabler = new WindowHelper::CParentDisable;
        if ( status ) {
            _disabler->disable(this);
        } else _disabler->enable();
    } else
    if (m_winType == WindowType::SINGLE) {
        status ? pimpl->lockParentUI() : pimpl->unlockParentUI();
    }
}

/** Protected **/

void CMainWindow::captureMouse()
{
    QMouseEvent _event(QEvent::MouseButtonRelease, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(AscAppManager::mainWindow(), &_event);
    setGeometry(QRect(QCursor::pos() - QPoint(300, 15), size()));
    QPoint pt_in_title = (m_boxTitleBtns->geometry().topLeft() + QPoint(300,15));
    _event = {QEvent::MouseButtonPress, pt_in_title, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
//    QApplication::sendEvent(this, &_event1);
    CX11Decoration::dispatchMouseDown(&_event);
    _event = {QEvent::MouseMove, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
//    QApplication::sendEvent(this, &_event);
    CX11Decoration::dispatchMouseMove(&_event);
}

void CMainWindow::setScreenScalingFactor(double factor)
{
    if (m_winType == WindowType::MAIN) {
        CX11Decoration::onDpiChanged(factor);
        QString css(AscAppManager::getWindowStylesheets(factor));
        if (!css.isEmpty()) {
            onScreenScalingFactor(factor);
            _m_pMainPanel->setStyleSheet(css);
            _m_pMainPanel->setScreenScalingFactor(factor);
            // TODO: skip window min size for usability test
    //        setMinimumSize(WindowHelper::correctWindowMinimumSize(_src_rect, {MAIN_WINDOW_MIN_WIDTH * factor, MAIN_WINDOW_MIN_HEIGHT * factor}));
        }
    } else
    if (m_winType == WindowType::SINGLE) {
        CMainWindowBase::setScreenScalingFactor(factor);
    } else
    if (m_winType == WindowType::REPORTER) {
        QString css(AscAppManager::getWindowStylesheets(factor));
        if (!css.isEmpty()) {
            onScreenScalingFactor(factor);
            m_pMainPanel->setStyleSheet(css);
    //        setMinimumSize(WindowHelper::correctWindowMinimumSize(_dest_rect, {WINDOW_MIN_WIDTH*factor, WINDOW_MIN_HEIGHT*factor}));
        }
    }
}

void CMainWindow::onMinimizeEvent()
{
    //CMainWindowBase::onMinimizeEvent();
    setWindowState(Qt::WindowMinimized);
}

void CMainWindow::onMaximizeEvent()
{
    //CMainWindowBase::onMaximizeEvent();
    setWindowState(windowState().testFlag(Qt::WindowMaximized) ? Qt::WindowNoState : Qt::WindowMaximized);
}

void CMainWindow::onSizeEvent(int type)
{
    CMainWindowBase::onSizeEvent(type);
    if ( type == Qt::WindowMinimized ) {
//        m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
//        m_buttonMaximize->style()->polish(m_buttonMaximize);
    }
}

void CMainWindow::onExitSizeMove()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio )
        setScreenScalingFactor(dpi_ratio);
}

void CMainWindow::setWindowTitle(const QString& t)
{
    CMainWindowBase::setWindowTitle(t);
    QMainWindow::setWindowTitle(t);
}

/** Private **/

void CMainWindow::onScreenScalingFactor(double factor)
{
    setMinimumSize(QSize(0,0));
    double change_factor = factor / m_dpiRatio;
    m_dpiRatio = factor;
    if (isMaximized() && m_winType == WindowType::MAIN) return;
    QRect _src_rect = geometry();
    int dest_width_change = int(_src_rect.width() * (1 - change_factor));
    QRect _dest_rect = QRect{_src_rect.translated(dest_width_change/2,0).topLeft(), _src_rect.size() * change_factor};
    setGeometry(_dest_rect);
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * f, MAIN_WINDOW_MIN_HEIGHT * f}));
}

void CMainWindow::closeEvent(QCloseEvent * e)
{
    ((CMainPanel *)_m_pMainPanel)->pushButtonCloseClicked();
    e->ignore();
}

void CMainWindow::showEvent(QShowEvent * e)
{
    QMainWindow::showEvent(e);
}

bool CMainWindow::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;

    if (event->type() == QEvent::WindowStateChange && this->isVisible()) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );
        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);
        if (m_winType == WindowType::MAIN) {
            if( _e_statechange->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized ) {
                ((CMainPanel *)_m_pMainPanel)->applyMainWindowState(Qt::WindowMaximized);
            } else
            if (this->windowState() == Qt::WindowNoState) {
                ((CMainPanel *)_m_pMainPanel)->applyMainWindowState(Qt::WindowNoState);
            } else
            if (this->windowState() == Qt::WindowMinimized) {
                ((CMainPanel *)_m_pMainPanel)->applyMainWindowState(Qt::WindowMinimized);
            }
        } else
        if (m_winType == WindowType::SINGLE || m_winType == WindowType::REPORTER) {
            if (!isCustomWindowStyle() && m_winType == WindowType::SINGLE) return QMainWindow::event(event);
            if(_e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized) {
                layout()->setMargin(0);
                applyWindowState(Qt::WindowMaximized);
            } else
            if (this->windowState() == Qt::WindowNoState) {
                layout()->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
                applyWindowState(Qt::WindowNoState);
            }
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            if (m_winType == WindowType::MAIN) {
                updateScaling();
            } else
            if (m_winType == WindowType::SINGLE) {
                onExitSizeMove();
            } else
            if (m_winType == WindowType::REPORTER) {
                double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
                if (dpi_ratio != m_dpiRatio)
                    setScreenScalingFactor(dpi_ratio);
            }
        }
        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if (!_flg_motion)
            _flg_motion = true;
        if (m_winType == WindowType::SINGLE) {
            QMoveEvent * _e = static_cast<QMoveEvent *>(event);
            onMoveEvent(QRect(_e->pos(), QSize(1,1)));
        }
    } else
    if (event->type() == QEvent::Close && m_winType == WindowType::SINGLE) {
        if (!AscAppManager::mainWindow() || !AscAppManager::mainWindow()->isVisible()) {
            GET_REGISTRY_USER(reg_user);
            reg_user.setValue("position", normalGeometry());
        }
        onCloseEvent();
        event->ignore();
        return false;
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

void CMainWindow::mouseDoubleClickEvent(QMouseEvent *)
{
    if (m_winType == WindowType::SINGLE) {
        if (m_boxTitleBtns->underMouse())
            onMaximizeEvent();
    } else {
        if (m_boxTitleBtns->underMouse())
            m_buttonMaximize->click();
    }
}

void CMainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_winType == WindowType::SINGLE) {
        onSizeEvent(0);
    } else {
        QMainWindow::resizeEvent(event);
    }
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
        ((CMainPanel *)_m_pMainPanel)->doOpenLocalFile(opts);
    }
    event->acceptProposedAction();
}

void CMainWindow::captureMouse(int tabindex)
{
    CMainWindowBase::captureMouse(tabindex);
    if (tabindex >= 0 && tabindex < mainPanel()->tabWidget()->count()) {
        QPoint spt = mainPanel()->tabWidget()->tabBar()->tabRect(tabindex).topLeft() + QPoint(30, 10);
        //QPoint gpt = mainPanel()->tabWidget()->tabBar()->mapToGlobal(spt);
//        CX11Decoration::setCursorPos(100, 100);
//        QCursor::setPos(0, 0);
        QTimer::singleShot(0, this, [=] {
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

void CMainWindow::onCloseEvent()
{
    QWidget * mainView = m_pMainPanel->findChild<QWidget *>("mainView");
    if ( mainView ) {
        mainView->setObjectName("destroyed");
        AscAppManager::getInstance().DestroyCefView(
                ((QCefView *)mainView)->GetCefView()->GetId() );
    }
    hide();
}
