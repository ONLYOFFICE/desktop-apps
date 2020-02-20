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

#include "csinglewindow.h"
#include "utils.h"
#include "cwindowbase.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "defines_p.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>


CSingleWindow::CSingleWindow(const QRect& geometry, const QString& title, QWidget * view)
    : QMainWindow(nullptr)
    , CX11Decoration(this)
    , m_dpiRatio(Utils::getScreenDpiRatio(QApplication::desktop()->screenNumber(geometry.topLeft())))
{
    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    if (reg_user.value("titlebar") == "custom" ||
            reg_system.value("titlebar") == "custom" )
        CX11Decoration::turnOff();

    // adjust window size
    QRect _window_rect = geometry;

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

    setWindowTitle(title);
    setWindowIcon(Utils::appIcon());
    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
    setGeometry(_window_rect);

    m_pMainPanel = createMainPanel(!CX11Decoration::isDecorated(), title, view);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget(m_boxTitle);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }

    setCentralWidget(m_pMainPanel);
    updateGeometry();
}

bool CSingleWindow::holdView(int id) const
{
    QWidget * mainView = m_pMainPanel->findChild<QWidget *>("mainView");
    return mainView && ((QCefView *)mainView)->GetCefView()->GetId() == id;
}

QWidget * CSingleWindow::createMainPanel(bool custom, const QString& title, QWidget * view)
{
    QWidget * mainPanel = new QWidget;
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));

    QVBoxLayout * _layout = new QVBoxLayout(mainPanel);

    _layout->setAlignment(Qt::AlignTop);
    _layout->setSpacing(0);
    _layout->setMargin(0);

    if ( custom ) {
        m_boxTitle = new QWidget(mainPanel);
        m_boxTitle->setFixedHeight(TOOLBTN_HEIGHT * m_dpiRatio);
        _layout->addWidget(m_boxTitle);

        m_boxTitle->setLayout(new QHBoxLayout);
        m_boxTitle->layout()->setSpacing(0);
        m_boxTitle->layout()->setContentsMargins(40*3,0,0,0);
        m_boxTitle->layout()->setSpacing(1*m_dpiRatio);

        QLabel * label = new QLabel(title);
        label->setObjectName("labelAppTitle");
        label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        label->setMouseTracking(true);
        m_boxTitle->layout()->addWidget(label);

        QSize small_btn_size(40 * m_dpiRatio, TOOLBTN_HEIGHT * m_dpiRatio);

        auto _creatToolButton = [small_btn_size](const QString& name, QWidget * parent = nullptr) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");
            btn->setFixedSize(small_btn_size);
            btn->setMouseTracking(true);

            return btn;
        };

        QPushButton * _btn_minimize = _creatToolButton("toolButtonMinimize");
        connect(_btn_minimize, &QPushButton::clicked, [=]{ setWindowState(Qt::WindowMinimized); });

        m_btnMaximize = _creatToolButton("toolButtonMaximize");
        QObject::connect(m_btnMaximize, &QPushButton::clicked, [=]{
            setWindowState(windowState() == Qt::WindowMaximized ? Qt::WindowNoState : Qt::WindowMaximized);
        });

        QPushButton * _btn_close = _creatToolButton("toolButtonClose");
        QObject::connect(_btn_close, &QPushButton::clicked, bind(&CSingleWindow::pushButtonCloseClicked, this));

        m_boxTitle->layout()->addWidget(_btn_minimize);
        m_boxTitle->layout()->addWidget(m_btnMaximize);
        m_boxTitle->layout()->addWidget(_btn_close);

        _layout->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);

        QPalette _palette(palette());
        _palette.setColor(QPalette::Background, QColor("#f1f1f1"));
        setAutoFillBackground(true);
        setPalette(_palette);

        setStyleSheet("QMainWindow{border:1px solid #888;}");
    }

    if ( !view ) {
        QCefView * _view = AscAppManager::createViewer(mainPanel);
        _view->Create(&AscAppManager::getInstance(), cvwtSimple);
//        pMainWidget->setHidden(false);

        view = _view;
    }

    view->setObjectName("mainView");
    QRect _cef_rect{0,0,width(),height()};
    _cef_rect.setTopLeft(QPoint(_layout->margin() * m_dpiRatio * 2, (TOOLBTN_HEIGHT + _layout->margin() * 2) * m_dpiRatio));
    _cef_rect.translate(-_layout->margin() * m_dpiRatio, -_layout->margin() * m_dpiRatio);
    view->setGeometry(_cef_rect);
    _layout->addWidget(view, 1);

    return mainPanel;
}

void CSingleWindow::pushButtonCloseClicked()
{
    QWidget * mainView = m_pMainPanel->findChild<QWidget *>("mainView");
    if ( mainView ) {
        mainView->setObjectName("destroyed");
        AscAppManager::getInstance().DestroyCefView(
                ((QCefView *)mainView)->GetCefView()->GetId() );
    }

    hide();
}

void CSingleWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void CSingleWindow::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}
void CSingleWindow::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}
void CSingleWindow::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CSingleWindow::mouseDoubleClickEvent(QMouseEvent *)
{
    if ( m_boxTitle->underMouse() ) {
        m_btnMaximize->click();
    }
}

bool CSingleWindow::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;

    if (event->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );

        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);

        if( _e_statechange->oldState() == Qt::WindowNoState && windowState() == Qt::WindowMaximized ) {
            layout()->setMargin(0);

            m_btnMaximize->setProperty("class", "min");
            m_btnMaximize->style()->polish(m_btnMaximize);
        } else
        if (/*_e_statechange->oldState() == Qt::WindowMaximized &*/ this->windowState() == Qt::WindowNoState) {
            layout()->setMargin(CX11Decoration::customWindowBorderWith());

            m_btnMaximize->setProperty("class", "normal");
            m_btnMaximize->style()->polish(m_btnMaximize);
        }
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            uchar dpi_ratio = Utils::getScreenDpiRatioByWidget(this);

            if ( dpi_ratio != m_dpiRatio )
                setScreenScalingFactor(dpi_ratio);
        }

        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if ( !_flg_motion )
            _flg_motion = true;
    }

    return QMainWindow::event(event);
}

void CSingleWindow::setScreenScalingFactor(uchar factor)
{
    QString css(AscAppManager::getWindowStylesheets(factor));

    if ( !css.isEmpty() ) {
        QRect _new_rect = geometry();
        bool increase = factor > m_dpiRatio;
        m_dpiRatio = factor;

        m_pMainPanel->setStyleSheet(css);
        setMinimumSize( WINDOW_MIN_WIDTH*factor, WINDOW_MIN_HEIGHT*factor );

        if ( increase ) {
            _new_rect.setSize(_new_rect.size() * 2);
        } else _new_rect.setSize(_new_rect.size() / 2);

        setGeometry(_new_rect);
    }
}

const QWidget * CSingleWindow::handle() const
{
    return this;
}
