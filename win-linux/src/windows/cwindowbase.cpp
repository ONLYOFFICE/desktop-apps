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

#include "windows/cwindowbase.h"
#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include "ccefeventsgate.h"
#include "clangater.h"
#ifdef __linux__
# include "defines.h"
#else
# include "windows/platform_win/caption.h"
#endif
#include <QDesktopWidget>
#include <QVariant>
#include <QSettings>
#include <QHBoxLayout>
#include <QScreen>
#include <functional>


class CWindowBase::CWindowBasePrivate
{
public:
    CWindowBasePrivate() {
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

    bool is_custom_window_ = false;
};


CWindowBase::CWindowBase(const QRect& rect)
    : QMainWindow(nullptr)
    , m_pTopButtons(3, nullptr)
    , pimpl{new CWindowBasePrivate}
    , m_windowActivated(false)
{
    setWindowIcon(Utils::appIcon());
    if ( !rect.isEmpty() )
        m_dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
    else {
        QScreen * _screen = QApplication::primaryScreen();
        m_dpiRatio = Utils::getScreenDpiRatio(_screen->geometry().topLeft());
    }
    m_window_rect = rect;
    if (m_window_rect.isEmpty())
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, MAIN_WINDOW_DEFAULT_SIZE * m_dpiRatio);
    QRect _screen_size = Utils::getScreenGeometry(m_window_rect.topLeft());
    if (_screen_size.intersects(m_window_rect)) {
        if (_screen_size.width() < m_window_rect.width() || _screen_size.height() < m_window_rect.height()) {
            m_window_rect.setLeft(_screen_size.left()),
            m_window_rect.setTop(_screen_size.top());
            if (_screen_size.width() < m_window_rect.width()) m_window_rect.setWidth(_screen_size.width());
            if (_screen_size.height() < m_window_rect.height()) m_window_rect.setHeight(_screen_size.height());
        }
    } else {
        m_window_rect = QRect(QPoint(100, 100)*m_dpiRatio, QSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT)*m_dpiRatio);
    }
}

CWindowBase::~CWindowBase()
{

}

/** Public **/

QWidget * CWindowBase::handle() const
{
    return qobject_cast<QWidget *>(const_cast<CWindowBase*>(this));
}

bool CWindowBase::isCustomWindowStyle()
{
    return pimpl->is_custom_window_;
}

void CWindowBase::updateScaling()
{
    double dpi_ratio = Utils::getScreenDpiRatioByWidget(this);
    if ( dpi_ratio != m_dpiRatio ) {
        setScreenScalingFactor(dpi_ratio);
        adjustGeometry();
    }
}

void CWindowBase::setWindowColors(const QColor& background, const QColor& border)
{
    Q_UNUSED(border)
    QPalette pal = palette();
    pal.setColor(QPalette::Window, background);
    /*setStyleSheet(QString("QMainWindow{border:1px solid %1; border-top:2px solid %1;}").
                  arg(border.name()));*/
    setAutoFillBackground(true);
    setPalette(pal);
}

void CWindowBase::applyTheme(const std::wstring& theme)
{
    Q_UNUSED(theme)
    QColor background = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground);
    QColor border = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBorder);
    setWindowColors(background, border);
}

/** Protected **/

QPushButton* CWindowBase::createToolButton(QWidget * parent, const QString& name)
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

QWidget* CWindowBase::createTopPanel(QWidget *parent)
{
    QWidget *_boxTitleBtns;
#ifdef __linux__
    _boxTitleBtns = new QWidget(parent);
#else
    _boxTitleBtns = static_cast<QWidget*>(new Caption(parent));
#endif
    _boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *layoutBtns = new QHBoxLayout(_boxTitleBtns);
    layoutBtns->setContentsMargins(0, 0, int(1*m_dpiRatio), 0);
    layoutBtns->setSpacing(int(1*m_dpiRatio));
    layoutBtns->addStretch();
    _boxTitleBtns->setLayout(layoutBtns);
    if (isCustomWindowStyle()) {
        const QString names[3] = {"toolButtonMinimize", "toolButtonMaximize", "toolButtonClose"};
        std::function<void(void)> btn_methods[3] = {
            [=]{onMinimizeEvent();}, [=]{onMaximizeEvent();}, [=]{onCloseEvent();}};
        m_pTopButtons.clear();
        for (int i = 0; i < 3; i++) {
            QPushButton *btn = createToolButton(_boxTitleBtns, names[i]);
            QObject::connect(btn, &QPushButton::clicked, btn_methods[i]);
            m_pTopButtons.push_back(btn);
            layoutBtns->addWidget(btn);
        }
    }

    return _boxTitleBtns;
}

void CWindowBase::setIsCustomWindowStyle(bool custom)
{
    pimpl->is_custom_window_ = custom;
}

void CWindowBase::setScreenScalingFactor(double factor)
{
    setMinimumSize(0,0);
    if (!isMaximized()) {
        double change_factor = factor / m_dpiRatio;
        QRect _src_rect = geometry();
        double dest_width_change = _src_rect.width() * (1 - change_factor);
        QRect _dest_rect = QRect{_src_rect.translated(int(dest_width_change/2), 0).topLeft(), _src_rect.size() * change_factor};
        setGeometry(_dest_rect);
    }
    m_dpiRatio = factor;
    if (m_boxTitleBtns) {
        QLayout *pLayoutBtns = m_boxTitleBtns->layout();
        pLayoutBtns->setSpacing(int(1 * m_dpiRatio));
        if (isCustomWindowStyle()) {
            pLayoutBtns->setContentsMargins(0, 0, 0, 0);
            QSize small_btn_size(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
            foreach (auto pBtn, m_pTopButtons)
                pBtn->setFixedSize(small_btn_size);
        }
    }
}

void CWindowBase::applyWindowState(Qt::WindowState s)
{
    if (isCustomWindowStyle() && m_pTopButtons[BtnType::Btn_Maximize]) {
        m_pTopButtons[BtnType::Btn_Maximize]->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pTopButtons[BtnType::Btn_Maximize]->style()->polish(m_pTopButtons[BtnType::Btn_Maximize]);
    }
}

void CWindowBase::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle(title);
    if (m_labelTitle)
        m_labelTitle->setText(title);
}

void CWindowBase::onMinimizeEvent()
{
    QMainWindow::showMinimized();
}

void CWindowBase::onMaximizeEvent()
{
    isMaximized() ? QMainWindow::showNormal() : QMainWindow::showMaximized();
}

void CWindowBase::onCloseEvent()
{
    close();
}

void CWindowBase::focus()
{
    setFocus();
}

/** Private **/

void CWindowBase::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_windowActivated) {
        m_windowActivated = true;
        setGeometry(m_window_rect);
        adjustGeometry();
        applyTheme(AscAppManager::themes().current().id());
    }
}
