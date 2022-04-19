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

#include "cmainwindowbase.h"
#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include "ccefeventsgate.h"
#include "defines.h"
#include "clangater.h"
#ifdef _WIN32
# include "win/caption.h"
#endif

#include <QLayout>
#include <QVariant>
#include <QSettings>


class CMainWindowBase::impl {
    bool is_custom_window_ = false;

public:
    impl() {
#ifdef Q_OS_LINUX
        GET_REGISTRY_SYSTEM(reg_system)
        GET_REGISTRY_USER(reg_user)
        if ( reg_user.value("titlebar") == "custom" ||
                reg_system.value("titlebar") == "custom" )
        {
            is_custom_window_ = true;
        }
#else
        is_custom_window_ = true;
#endif
    }

    auto is_custom_window() -> bool {
        return is_custom_window_;
    }
};

auto ellipsis_text_(const QWidget * widget, const QString& str, Qt::TextElideMode mode = Qt::ElideRight) -> QString {
    QMargins _margins = widget->contentsMargins();
    int _padding = _margins.left() + _margins.right();
    int _width = widget->maximumWidth() != QWIDGETSIZE_MAX ? widget->maximumWidth() : widget->width();
    QFontMetrics _metrics(widget->font());

    return _metrics.elidedText(str, mode, _width - _padding - 1);
}

CElipsisLabel::CElipsisLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{}

CElipsisLabel::CElipsisLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
    , orig_text(text)
{
//    QString elt = elipsis_text(this, text, Qt::ElideMiddle);
//    setText(elt);
}

void CElipsisLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);

    if ( event->size().width() != event->oldSize().width() ) {
        QString elt = ellipsis_text_(this, orig_text, elide_mode);
        QLabel::setText(elt);
    }
}

auto CElipsisLabel::setText(const QString& text) -> void
{
    orig_text = text;

    QString elt = ellipsis_text_(this, text, elide_mode);
    QLabel::setText(elt);
}

auto CElipsisLabel::setEllipsisMode(Qt::TextElideMode mode) -> void
{
    elide_mode = mode;
}

auto CElipsisLabel::updateText() -> void
{
    QString elt = ellipsis_text_(this, orig_text, elide_mode);
    if ( elt != text() ) {
        QLabel::setText(elt);
    }
}


CMainWindowBase::CMainWindowBase(QRect& rect)
    : m_pubMainImpl(nullptr)
    , m_boxTitleBtns(nullptr)
    , m_pMainPanel(nullptr)
    , m_pMainView(nullptr)
    , m_buttonMinimize(nullptr)
    , m_buttonMaximize(nullptr)
    , m_buttonClose(nullptr)
    , m_labelTitle(nullptr)
    , pimpl{new impl}
{
    m_dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
    if ( rect.isEmpty() )
        rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);

    QRect _screen_size = Utils::getScreenGeometry(rect.topLeft());
    if ( _screen_size.width() < rect.width() + 120 ||
            _screen_size.height() < rect.height() + 120 )
    {
        rect.setLeft(_screen_size.left()),
        rect.setTop(_screen_size.top());

        if ( _screen_size.width() < rect.width() ) rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < rect.height() ) rect.setHeight(_screen_size.height());
    }
}

CMainWindowBase::~CMainWindowBase()
{

}

/** Public **/

QWidget * CMainWindowBase::editor(int index)
{
    return mainPanel()->tabWidget()->panel(index);
}

QString CMainWindowBase::documentName(int vid)
{
    int i = mainPanel()->tabWidget()->tabIndexByView(vid);
    if ( !(i < 0) ) {
        return mainPanel()->tabWidget()->panel(i)->data()->title();
    }

    return "";
}

double CMainWindowBase::scaling() const
{
    return m_dpiRatio;
}

int CMainWindowBase::editorsCount() const
{
    return mainPanel()->tabWidget()->count(cvwtEditor);
}

int CMainWindowBase::editorsCount(const std::wstring& portal) const
{
    return mainPanel()->tabWidget()->count(portal, true);
}

/*int CMainWindowBase::attachEditor(QWidget * panel, int index)
{
    CMainPanel * _pMainPanel = mainPanel();

    if (!QCefView::IsSupportLayers())
    {
        CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
        if (_panel)
            _panel->view()->SetCaptionMaskSize(0);
    }

    int _index = _pMainPanel->tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        _pMainPanel->toggleButtonMain(false);

        _pMainPanel->tabWidget()->setCurrentIndex(_index);
    }
    return _index;
}

int CMainWindowBase::attachEditor(QWidget * panel, const QPoint& pt)
{
    CMainPanel * _pMainPanel = mainPanel();
    QPoint _pt_local = _pMainPanel->tabWidget()->tabBar()->mapFromGlobal(pt);
#ifdef Q_OS_WIN
# if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QPoint _tl = windowRect().topLeft();
    if ( _tl.x() < _pt_local.x() && _tl.y() < _pt_local.y() )
        _pt_local -= windowRect().topLeft();
# endif
#endif
    int _index = _pMainPanel->tabWidget()->tabBar()->tabAt(_pt_local);

    if ( !(_index < 0) ) {
        QRect _rc_tab = _pMainPanel->tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }

    return attachEditor(panel, _index);
}*/

void CMainWindowBase::selectView(int viewid) const
{
    int _index = mainPanel()->tabWidget()->tabIndexByView(viewid);
    if ( !(_index < 0) ) {
        mainPanel()->tabWidget()->setCurrentIndex(_index);
        mainPanel()->toggleButtonMain(false);
    }
}

void CMainWindowBase::selectView(const QString& url) const
{
    int _index = mainPanel()->tabWidget()->tabIndexByUrl(url);
    if ( !(_index < 0) ) {
        mainPanel()->tabWidget()->setCurrentIndex(_index);
        mainPanel()->toggleButtonMain(false);
    }
}

bool CMainWindowBase::pointInTabs(const QPoint& pt) const
{
    QRect _rc_title(mainPanel()->geometry());
    _rc_title.setHeight(mainPanel()->tabWidget()->tabBar()->height());

#ifdef Q_OS_LINUX
    _rc_title.moveTop(1);
#endif

    return _rc_title.contains(mainPanel()->mapFromGlobal(pt));
}

/** Protected **/

QWidget * CMainWindowBase::createTopPanel(QWidget * parent, const QString& title)
{
    if (pimpl->is_custom_window()) {
#ifdef __linux__
        m_boxTitleBtns = new QWidget(parent);
#else
        m_boxTitleBtns = static_cast<QWidget*>(new Caption(parent));
#endif
        m_boxTitleBtns->setObjectName("box-title-tools");
        m_boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT * m_dpiRatio);

        QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
        layoutBtns->setContentsMargins(0,0,0,0);
        layoutBtns->setSpacing(1 * m_dpiRatio);

        m_labelTitle = new CElipsisLabel(title, m_boxTitleBtns);
        m_labelTitle->setObjectName("labelTitle");
        m_labelTitle->setMouseTracking(true);
        m_labelTitle->setEllipsisMode(Qt::ElideMiddle);
        m_labelTitle->setMaximumWidth(100);

        layoutBtns->addStretch();
        layoutBtns->addWidget(m_labelTitle, 0);
        layoutBtns->addStretch();

        initTopButtons(m_boxTitleBtns);
    }

    return nullptr;
}

QPushButton * CMainWindowBase::createToolButton(QWidget * parent, const QString& name)
{
    QPushButton * btn = new QPushButton(parent);
    btn->setObjectName(name);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
#ifdef __linux__
    btn->setMouseTracking(true);
#endif
    return btn;
}

void CMainWindowBase::updateTitleCaption()
{
    if ( m_labelTitle ) {
        int _width = calcTitleCaptionWidth();
        if ( !(_width < 0) ) {
            m_labelTitle->setMaximumWidth(_width);
            m_labelTitle->updateText();
        }
    }
}

void CMainWindowBase::applyWindowState(Qt::WindowState s)
{
    m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
    m_buttonMaximize->style()->polish(m_buttonMaximize);
}

bool CMainWindowBase::isCustomWindowStyle()
{
    return pimpl->is_custom_window();
}

QWidget * CMainWindowBase::createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * view)
{
    QWidget * mainPanel = new QWidget(parent);
#ifndef __linux__
    parent->layout()->addWidget(mainPanel);
#endif
    mainPanel->setObjectName("mainPanel");
    mainPanel->setProperty("uitheme", QString::fromStdWString(AscAppManager::themes().current().id()));
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));

    QGridLayout * mainGridLayout = new QGridLayout(mainPanel);
    mainGridLayout->setSpacing(0);
    mainGridLayout->setMargin(0);
    mainPanel->setLayout(mainGridLayout);

#ifdef __linux__
    m_boxTitleBtns = new QWidget(mainPanel);
#else
    m_boxTitleBtns = static_cast<QWidget*>(new Caption(mainPanel));
#endif
    m_boxTitleBtns->setObjectName("box-title-tools");
    m_boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT * m_dpiRatio);

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    layoutBtns->setContentsMargins(0,0,0,0);
    layoutBtns->setSpacing(1*m_dpiRatio);
    m_boxTitleBtns->setLayout(layoutBtns);

    m_labelTitle = new CElipsisLabel(title, m_boxTitleBtns);
    m_labelTitle->setObjectName("labelAppTitle");
    m_labelTitle->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layoutBtns->addWidget(m_labelTitle);

    if (custom) {
        initTopButtons(m_boxTitleBtns);
        layoutBtns->addWidget(m_buttonMinimize);
        layoutBtns->addWidget(m_buttonMaximize);
        layoutBtns->addWidget(m_buttonClose);
#ifdef __linux__
        m_labelTitle->setMouseTracking(true);
        mainGridLayout->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        /*QPalette _palette(palette());
        _palette.setColor(QPalette::Background, QColor("#f1f1f1"));
        setAutoFillBackground(true);
        setPalette(_palette);
        setStyleSheet("QMainWindow{border:1px solid #888;}");*/
#else
        QSize small_btn_size(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
        QWidget * _lb = new QWidget(m_boxTitleBtns);
        _lb->setFixedWidth( (small_btn_size.width() + layoutBtns->spacing()) * 3 );
        layoutBtns->insertWidget(0, _lb);
#endif
    } else {
        QLinearGradient gradient(mainPanel->rect().topLeft(), QPoint(mainPanel->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));
        m_labelTitle->setFixedHeight(0);
        m_boxTitleBtns->setFixedHeight(16*m_dpiRatio);
    }
    mainGridLayout->addWidget(m_boxTitleBtns, 0, 0, Qt::AlignTop);
    if (!view) {
        QCefView * pMainWidget = AscAppManager::createViewer(mainPanel);
        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
        pMainWidget->setObjectName("mainPanel");
        pMainWidget->setHidden(false);
        m_pMainView = (QWidget*)pMainWidget;
    } else {
        m_pMainView = view;
        m_pMainView->setParent(mainPanel);
        m_pMainView->show();
    }
    m_pMainView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainGridLayout->addWidget(m_pMainView, 1, 0);

    return mainPanel;
}

void CMainWindowBase::setScreenScalingFactor(double f)
{
    if ( m_dpiRatio != f ) {
        if ( isCustomWindowStyle() ) {
            QSize small_btn_size(int(TOOLBTN_WIDTH * f), int(TOOLBTN_HEIGHT*f));

            m_buttonMinimize->setFixedSize(small_btn_size);
            m_buttonMaximize->setFixedSize(small_btn_size);
            m_buttonClose->setFixedSize(small_btn_size);

            m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * f));
            m_boxTitleBtns->layout()->setSpacing(int(1 * f));
        }
        m_dpiRatio = f;
    }
}

void CMainWindowBase::setWindowTitle(const QString& title)
{
    if ( m_labelTitle ) {
        m_labelTitle->setText(title);
    }
}

void CMainWindowBase::updateScaling()
{
    onExitSizeMove();
}

void CMainWindowBase::applyTheme(const std::wstring& name)
{
    mainPanel()->applyTheme(name);
}

void CMainWindowBase::captureMouse(int)
{
#ifdef Q_OS_WIN
    ReleaseCapture();
#endif
}

void CMainWindowBase::onSizeEvent(int)
{
    updateTitleCaption();
}

bool CMainWindowBase::holdView(int id) const
{
    return mainPanel()->holdUid(id);
}

int CMainWindowBase::calcTitleCaptionWidth()
{
    if ( pimpl->is_custom_window() ) {
        return m_boxTitleBtns->width() - (m_buttonMaximize->width() * 3);
    }

    return 0;
}

/** Private **/

void CMainWindowBase::initTopButtons(QWidget *parent)
{
    // Minimize
    m_buttonMinimize = createToolButton(parent, "toolButtonMinimize");
    QObject::connect(m_buttonMinimize, &QPushButton::clicked, [=]{onMinimizeEvent();});
    // Maximize
    m_buttonMaximize = createToolButton(parent, "toolButtonMaximize");
    QObject::connect(m_buttonMaximize, &QPushButton::clicked, [=]{onMaximizeEvent();});
    // Close
    m_buttonClose = createToolButton(parent, "toolButtonClose");
    QObject::connect(m_buttonClose, &QPushButton::clicked, [=]{onCloseEvent();});
}
