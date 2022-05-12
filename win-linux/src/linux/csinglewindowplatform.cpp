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

#include "csinglewindowplatform.h"
#include "cwindowbase.h"
#include "utils.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"

#define EDITOR_WINDOW_MIN_WIDTH     920

class CSingleWindowPlatform::impl {
    CSingleWindowPlatform * m_owner = nullptr;
    WindowHelper::CParentDisable * m_disabler = nullptr;
public:
    impl(CSingleWindowPlatform * owner)
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

CSingleWindowPlatform::CSingleWindowPlatform(const QRect& rect, const QString& title, QWidget * panel)
    : CSingleWindowBase(const_cast<QRect&>(rect))
    , QMainWindow()
    , CX11Decoration(this)
    , pimpl{new impl(this)}
{
    if ( isCustomWindowStyle() )
        CX11Decoration::turnOff();

    setWindowTitle(title);
    setWindowIcon(Utils::appIcon());
    setGeometry(rect);
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * m_dpiRatio, MAIN_WINDOW_MIN_HEIGHT * m_dpiRatio}));

    connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, this, &CSingleWindowPlatform::slot_modalDialog);
}

CSingleWindowPlatform::~CSingleWindowPlatform()
{

}

void CSingleWindowPlatform::resizeEvent(QResizeEvent *)
{
    onSizeEvent(0);
}

void CSingleWindowPlatform::onMinimizeEvent()
{
    CSingleWindowBase::onMinimizeEvent();
    setWindowState(Qt::WindowMinimized);
}

void CSingleWindowPlatform::onMaximizeEvent()
{
    CSingleWindowBase::onMaximizeEvent();
    setWindowState(windowState().testFlag(Qt::WindowMaximized) ? Qt::WindowNoState : Qt::WindowMaximized);
}

void CSingleWindowPlatform::onSizeEvent(int type)
{
    CSingleWindowBase::onSizeEvent(type);

    if ( type == Qt::WindowMinimized ) {
//        m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
//        m_buttonMaximize->style()->polish(m_buttonMaximize);
    }
}

QWidget * CSingleWindowPlatform::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CSingleWindowPlatform *>(this));
}

void CSingleWindowPlatform::show(bool maximized)
{
    QMainWindow::show();

    if ( maximized )
        QMainWindow::setWindowState(Qt::WindowMaximized);
}

void CSingleWindowPlatform::bringToTop()
{
//    QMainWindow::show();
    QMainWindow::raise();
    QMainWindow::activateWindow();
//    QApplication::setActiveWindow(this);
}

bool CSingleWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if ( isCustomWindowStyle() ) {
            QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );

            CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);

            if( _e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized ) {
                layout()->setMargin(0);

                m_buttonMaximize->setProperty("class", "min");
                m_buttonMaximize->style()->polish(m_buttonMaximize);
            } else
            if (/*_e_statechange->oldState() == Qt::WindowMaximized &*/ this->windowState() == Qt::WindowNoState) {
                layout()->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);

                m_buttonMaximize->setProperty("class", "normal");
                m_buttonMaximize->style()->polish(m_buttonMaximize);
            }
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        flag_mouse_button_left = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( flag_mouse_button_left && flag_mouse_motion ) {
            onExitSizeMove();
        }

        flag_mouse_button_left = flag_mouse_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if ( !flag_mouse_motion )
            flag_mouse_motion = true;

        QMoveEvent * _e = static_cast<QMoveEvent *>(event);
        onMoveEvent(QRect(_e->pos(), QSize(1,1)));
    } else
    if ( event->type() == QEvent::Close ) {
        if ( !AscAppManager::mainWindow() || !AscAppManager::mainWindow()->isVisible() ) {
            GET_REGISTRY_USER(reg_user);
            reg_user.setValue("position", normalGeometry());
        }

        onCloseEvent();
        event->ignore();
        return false;
    }

    return QMainWindow::event(event);
}

void CSingleWindowPlatform::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}
void CSingleWindowPlatform::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}
void CSingleWindowPlatform::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CSingleWindowPlatform::mouseDoubleClickEvent(QMouseEvent *)
{
    if ( m_boxTitleBtns->underMouse() ) {
        onMaximizeEvent();
    }
}

void CSingleWindowPlatform::onScreenScalingFactor(double f)
{
    setMinimumSize(QSize(0,0));

    double change_factor = f / m_dpiRatio;
    m_dpiRatio = f;

    QRect _src_rect = geometry();
    int dest_width_change = int(_src_rect.width() * (1 - change_factor));
    QRect _dest_rect = QRect{_src_rect.translated(dest_width_change/2,0).topLeft(), _src_rect.size() * change_factor};

    setGeometry(_dest_rect);
//    setMinimumSize(WindowHelper::correctWindowMinimumSize(geometry(), {EDITOR_WINDOW_MIN_WIDTH * f, MAIN_WINDOW_MIN_HEIGHT * f}));
}

void CSingleWindowPlatform::onExitSizeMove()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);

    if ( dpi_ratio != m_dpiRatio )
        setScreenScalingFactor(dpi_ratio);
}

void CSingleWindowPlatform::setWindowTitle(const QString& t)
{
    CSingleWindowBase::setWindowTitle(t);
    QMainWindow::setWindowTitle(t);
}

void CSingleWindowPlatform::captureMouse()
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

void CSingleWindowPlatform::slot_modalDialog(bool status, WId)
{
    status ? pimpl->lockParentUI() : pimpl->unlockParentUI();
}
