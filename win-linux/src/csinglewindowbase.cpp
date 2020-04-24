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

#include "csinglewindowbase.h"
#include "utils.h"
#include "cwindowbase.h"
#include "ccefeventsgate.h"

#include <QLayout>
#include <QVariant>
#include <QDebug>

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


CSingleWindowBase::CSingleWindowBase()
{

}

CSingleWindowBase::~CSingleWindowBase()
{
//    if ( m_pButtonClose ) {
//        m_pButtonClose->deleteLater();
//        m_pButtonClose = nullptr;
//    }
//    if ( m_pButtonMinimize ) {
//        m_pButtonMinimize->deleteLater();
//        m_pButtonMinimize = nullptr;
//    }
//    if ( m_pButtonMaximize ) {
//        m_pButtonMaximize->deleteLater();
//        m_pButtonMaximize = nullptr;
//    }
//    qDebug() << "destroy base single window";
}

CSingleWindowBase::CSingleWindowBase(QRect& rect)
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

void CSingleWindowBase::setScreenScalingFactor(int f)
{
    if ( m_dpiRatio != f ) {
        QSize small_btn_size(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f);

        m_buttonMinimize->setFixedSize(small_btn_size);
        m_buttonMaximize->setFixedSize(small_btn_size);
        m_buttonClose->setFixedSize(small_btn_size);

        m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT * f);
        m_boxTitleBtns->layout()->setSpacing(1 * f);

//        onScreenScalingFactor(f);

        m_dpiRatio = f;
    }
}

void CSingleWindowBase::setWindowTitle(const QString& title)
{
    if ( m_labelTitle ) {
        m_labelTitle->setText(title);
    }
}

//#include <QSvgRenderer>
//#include <QPainter>
QWidget * CSingleWindowBase::createMainPanel(QWidget * parent, const QString& title, bool custom)
{
    m_boxTitleBtns = new QWidget;
    m_boxTitleBtns->setObjectName("box-title-tools");
    m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT * m_dpiRatio);

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    layoutBtns->setContentsMargins(0,0,0,0);
    layoutBtns->setSpacing(1 * m_dpiRatio);

    if ( custom ) {
        m_labelTitle = new CElipsisLabel(title);
        m_labelTitle->setObjectName("labelTitle");
        m_labelTitle->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        m_labelTitle->setMouseTracking(true);
        m_labelTitle->setEllipsisMode(Qt::ElideMiddle);

//        layoutBtns->addStretch();
        layoutBtns->addWidget(m_labelTitle, 1);
//        layoutBtns->addStretch();

        QSize small_btn_size(TOOLBTN_WIDTH*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

        auto _creatToolButton = [&small_btn_size](const QString& name, QWidget * parent) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");
            btn->setFixedSize(small_btn_size);
            btn->setMouseTracking(true);

            return btn;
        };

        // Minimize
        m_buttonMinimize = _creatToolButton("toolButtonMinimize", parent);
        QObject::connect(m_buttonMinimize, &QPushButton::clicked, [=]{onMinimizeEvent();});

        // Maximize
        m_buttonMaximize = _creatToolButton("toolButtonMaximize", parent);
        QObject::connect(m_buttonMaximize, &QPushButton::clicked, [=]{onMaximizeEvent();});

        // Close
        m_buttonClose = _creatToolButton("toolButtonClose", parent);
        QObject::connect(m_buttonClose, &QPushButton::clicked, [=]{onCloseEvent();});


//        m_pButtonMaximize = new QPushButton(parent);
//        m_pButtonMaximize->setFixedSize(small_btn_size);
//        m_pButtonMaximize->setIconSize(QSize(16,16));

//        QSvgRenderer _svg;
//        _svg.load(QString(":/tools.svg"));
//qDebug() << "def size: " << _svg.defaultSize();
//        QPixmap image(_svg.defaultSize());
//        image.fill(Qt::transparent);
//        QPainter painter( &image );
//        _svg.render(&painter, "svg-g-max");

//        m_pButtonMaximize->setIcon(QIcon(image));
    }

    return nullptr;
}

void CSingleWindowBase::onCloseEvent()
{
}

void CSingleWindowBase::onMinimizeEvent()
{

}

void CSingleWindowBase::onMaximizeEvent()
{

}

void CSingleWindowBase::onExitSizeMove()
{

}

void CSingleWindowBase::onDpiChanged(int, int)
{
}

QPushButton * CSingleWindowBase::createToolButton(QWidget * parent)
{
    QPushButton * btn = new QPushButton(parent);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(QSize(TOOLBTN_WIDTH*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio));

    return btn;
}

void CSingleWindowBase::adjustGeometry()
{

}
