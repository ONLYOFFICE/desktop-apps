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

#include "cwindowbase.h"
//#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include "ccefeventsgate.h"
//#include "defines.h"
#include "clangater.h"
//#include <QLayout>
#include <QVariant>
#include <QSettings>


class CWindowBase::CWindowBasePrivate {
    bool is_custom_window_ = false;

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


CWindowBase::CWindowBase()
    : QMainWindow(nullptr)
    , m_boxTitleBtns(nullptr)
    , m_pMainPanel(nullptr)
    , m_pMainView(nullptr)
    , m_buttonMinimize(nullptr)
    , m_buttonMaximize(nullptr)
    , m_buttonClose(nullptr)
    , m_labelTitle(nullptr)
    , pimpl{new CWindowBasePrivate}
{
    setWindowIcon(Utils::appIcon());
    /*m_dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
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
    }*/

    if ( pimpl->is_custom_window() ) {
        initTopButtons(this);
    }
}

CWindowBase::~CWindowBase()
{

}

/** Protected **/

QPushButton * CWindowBase::createToolButton(QWidget * parent, const QString& name) const
{
    QPushButton * btn = new QPushButton(parent);
    btn->setObjectName(name);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));

    return btn;
}

void CWindowBase::initTopButtons(QWidget *parent)
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

QWidget * CWindowBase::titleWidget(WindowTitleWidget id) const
{
    switch (id) {
    case WindowTitleWidget::twButtonClose: return m_buttonClose;
    case WindowTitleWidget::twButtonMax: return m_buttonMaximize;
    case WindowTitleWidget::twButtonMin: return m_buttonMinimize;
    case WindowTitleWidget::twLabelCaption: return m_labelTitle;
    default: return nullptr;
    }
}

bool CWindowBase::isCustomWindowStyle()
{
    return pimpl->is_custom_window();
}

void CWindowBase::applyWindowState(Qt::WindowState s)
{
    m_buttonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
    m_buttonMaximize->style()->polish(m_buttonMaximize);
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
