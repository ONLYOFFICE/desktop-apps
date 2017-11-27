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

    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
    resize(_window_rect.width(), _window_rect.height());

    m_pMainView = createMainPanel(!CX11Decoration::isDecorated(), title, view);

    recalculatePlaces();
}

bool CSingleWindow::holdView(int id) const
{
    return ((QCefView *)m_pMainView)->GetCefView()->GetId() == id;
}

QWidget * CSingleWindow::createMainPanel(bool custom, const QString& title, QWidget * view)
{
    QWidget * mainPanel = new QWidget(this);
//    mainpanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing(0);
    mainGridLayout->setMargin(0);
    mainPanel->setLayout(mainGridLayout);
    m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));

    // Central widget
    QWidget * centralWidget = new QWidget(mainPanel);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QSize small_btn_size(28 * m_dpiRatio, TOOLBTN_HEIGHT * m_dpiRatio);

    m_boxTitleBtns = new CX11Caption(centralWidget);

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(title);
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    layoutBtns->setContentsMargins(0, 0, 4*m_dpiRatio, 0);
    layoutBtns->setSpacing(1*m_dpiRatio);
    layoutBtns->addWidget(label);

    if ( custom ) {
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
        connect(m_pButtonMinimize, &QPushButton::clicked, bind(&CSingleWindow::pushButtonMinimizeClicked, this));

        // Maximize
        m_pButtonMaximize = _creatToolButton("toolButtonMaximize", centralWidget);
        QObject::connect(m_pButtonMaximize, &QPushButton::clicked, bind(&CSingleWindow::pushButtonMaximizeClicked, this));

        // Close
        m_pButtonClose = _creatToolButton("toolButtonClose", centralWidget);
        QObject::connect(m_pButtonClose, &QPushButton::clicked, bind(&CSingleWindow::pushButtonCloseClicked, this));

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

#ifdef __linux
        mainGridLayout->setMargin( CX11Decoration::customWindowBorderWith() );

        QPalette _palette(palette());
        _palette.setColor(QPalette::Background, QColor(0x31, 0x34, 0x37));
        setAutoFillBackground(true);
        setPalette(_palette);

        connect(m_boxTitleBtns, SIGNAL(mouseDoubleClicked()), this, SLOT(pushButtonMaximizeClicked()));
#endif

        m_boxTitleBtns->setFixedSize(282*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);
    } else {
        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));

        label->setFixedHeight(0);
        m_boxTitleBtns->setFixedSize(342*m_dpiRatio, 16*m_dpiRatio);
    }

    if ( !view ) {
        QCefView * pMainWidget = new QCefView(centralWidget);
        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
        pMainWidget->setObjectName( "mainPanel" );
        pMainWidget->setHidden(false);

        m_pMainView = (QWidget *)pMainWidget;
    } else {
        m_pMainView = view;
        m_pMainView->setParent(centralWidget);
        m_pMainView->show();
    }

//    m_pMainWidget->setVisible(false);

    mainGridLayout->addWidget( centralWidget );
    return mainPanel;
}

void CSingleWindow::pushButtonCloseClicked()
{
    if ( m_pMainView ) {
        AscAppManager::getInstance().DestroyCefView(
                ((QCefView *)m_pMainView)->GetCefView()->GetId() );

        m_pMainView = nullptr;
    }

    AscAppManager::closeEditorWindow( size_t(this) );
}

void CSingleWindow::pushButtonMinimizeClicked()
{
    setWindowState(Qt::WindowMinimized);
}

void CSingleWindow::pushButtonMaximizeClicked()
{
    bool _is_maximized = windowState() == Qt::WindowMaximized;
    if ( !CX11Decoration::isDecorated() ) {
#ifdef __linux
        layout()->setMargin(!_is_maximized ? 0 : CX11Decoration::customWindowBorderWith());
#endif

        m_pButtonMaximize->setProperty("class", _is_maximized ? "min" : "normal");
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }

    setWindowState(_is_maximized ? Qt::WindowNoState : Qt::WindowMaximized);
}

void CSingleWindow::recalculatePlaces()
{
    int cbw = 0;

    QWidget * cw = findChild<QWidget *>("centralWidget");
    int windowW = cw->width(),
        windowH = cw->height(),
        captionH = TITLE_HEIGHT * m_dpiRatio;

    int contentH = windowH - captionH;
    if ( contentH < 1 ) contentH = 1;

    m_boxTitleBtns->setFixedSize(windowW, TOOLBTN_HEIGHT * m_dpiRatio);
    m_boxTitleBtns->move(windowW - m_boxTitleBtns->width() + cbw, cbw);
    m_pMainView->setGeometry(cbw, captionH + cbw, windowW, contentH);
}
