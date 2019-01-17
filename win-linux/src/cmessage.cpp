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

#include "cmessage.h"
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QDebug>
#include <QTimer>
#include <functional>
#include <QEvent>
#include <QKeyEvent>
#include <QCheckBox>

#include "defines.h"
#include "utils.h"
#include "linux/cx11decoration.h"

#if defined(_WIN32)
# include "win/qwinwidget.h"
#endif

#define MSG_ICON_WIDTH  35
#define MSG_ICON_HEIGHT 35

class CMessageEventsFilter : public QObject {
public:
    CMessageEventsFilter(CMessage * p, QObject * o)
        : QObject(o), m_mess(p)
    {}

protected:
    bool eventFilter(QObject * obj, QEvent * event)
    {
        if (event->type()==QEvent::KeyPress) {
            QKeyEvent * key = static_cast<QKeyEvent*>(event);
            if ( key->key()==Qt::Key_Escape ) {
                m_mess->close();
                return true;
            }
        }

        return QObject::eventFilter(obj, event);
    }

private:
    CMessage * m_mess;
};

#if defined(_WIN32)
CMessage::CMessage(HWND p)
    : CWinWindow(p, QString(APP_TITLE))
    , m_dpiRatio(Utils::getScreenDpiRatioByHWND(int(p)))
#else
CMessage::CMessage(QWidget * p)
    : QDialog(p)
    , m_dpiRatio(Utils::getScreenDpiRatioByWidget(p))
#endif
    , m_message(new QLabel)
    , m_typeIcon(new QLabel)
    , m_modalresult(MODAL_RESULT_CANCEL)
{
#if defined(_WIN32)
    HWND _hwnd = CWinWindow::m_hSelf;
    m_centralWidget = new QWinWidget(_hwnd);
    m_centralWidget->installEventFilter(
                new CMessageEventsFilter(this, m_centralWidget) );
#else
    setWindowTitle(APP_TITLE);
    setLayout(new QVBoxLayout);

    m_centralWidget = new QWidget;
    layout()->addWidget(m_centralWidget);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
#endif

    QVBoxLayout * _c_layout  = new QVBoxLayout;
    QHBoxLayout * _h_layout2 = new QHBoxLayout;
    QHBoxLayout * _h_layout1 = new QHBoxLayout;
    _c_layout->addLayout(_h_layout2, 1);
    _c_layout->addLayout(_h_layout1, 0);

    m_typeIcon->setProperty("class", "msg-icon");
    m_typeIcon->setFixedSize(MSG_ICON_WIDTH*m_dpiRatio, MSG_ICON_HEIGHT*m_dpiRatio);
    _h_layout2->addWidget(m_typeIcon, 0, Qt::AlignTop);

//    m_message->setWordWrap(true);
    m_message->setProperty("class", "msg-report");
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(8*m_dpiRatio));

    QFormLayout * _f_layout = new QFormLayout;
    _f_layout->addWidget(m_message);
    _f_layout->setSpacing(0);
    _f_layout->setContentsMargins(10*m_dpiRatio,0,5*m_dpiRatio,0);
    _h_layout2->addLayout(_f_layout, 1);

    _h_layout2->setContentsMargins(15,10,15,10);
    _f_layout->setContentsMargins(10,0,0,0);

    QPushButton * btn_ok = new QPushButton(QObject::tr("&OK"));
    btn_ok->setAutoDefault(true);
    m_boxButtons = new QWidget;
    m_boxButtons->setLayout(new QHBoxLayout);
    m_boxButtons->layout()->addWidget(btn_ok);
    m_boxButtons->layout()->setContentsMargins(0,8*m_dpiRatio,0,0);
    _h_layout1->addWidget(m_boxButtons, 0, Qt::AlignCenter);

    QObject::connect(btn_ok, &QPushButton::clicked,
        [=] {
            m_modalresult = MODAL_RESULT_YES;
#if defined(_WIN32)
            close();
#else
            close();
#endif
        }
    );

    m_centralWidget->setLayout(_c_layout);
    m_centralWidget->setMinimumWidth(350*m_dpiRatio);
//    m_centralWidget->setWindowTitle(APP_TITLE);
    m_centralWidget->move(0, 0);

    QString _styles(Utils::readStylesheets(":/styles/message.qss"));
    _styles.append(QString("QPushButton{min-width:%1px;}").arg(40*m_dpiRatio));
    m_centralWidget->setStyleSheet( _styles );

    m_centralWidget->setObjectName("messageBody");
    if ( m_dpiRatio > 1 ) {
        m_centralWidget->setProperty("hdpi", true);
    }
}

void CMessage::setButtons(std::initializer_list<QString> btns)
{
    QLayoutItem * item;
    QWidget * widget;
    while ( (item = m_boxButtons->layout()->takeAt(0)) ) {
        if ( (widget = item->widget()) ) {
            delete widget;
        }

        delete item;
    }

    auto _fn_click = [=](int num) {
        m_modalresult = MODAL_RESULT_CUSTOM + num;
        close();
    };

    QRegExp reFocus("([^:]+)\\:?(default)?$");

    QPushButton * _btn;
    int _btn_num(0);
    for (auto btn: btns) {
        reFocus.indexIn(btn);

        _btn = new QPushButton(reFocus.cap(1));
        if ( !reFocus.cap(2).isEmpty() ) {
            _btn->setAutoDefault(true);
        }

        m_boxButtons->layout()->addWidget(_btn);
        QObject::connect(_btn, &QPushButton::clicked, std::bind(_fn_click, _btn_num++));
    }

    if (_btn_num > 2)
        m_centralWidget->setMinimumWidth(400*m_dpiRatio);
}

int CMessage::info(const QString& mess)
{
    m_message->setText(mess);
    m_typeIcon->setProperty("type","msg-info");

    modal();

    return m_modalresult;
}

int CMessage::warning(const QString& mess)
{
    m_message->setText(mess);
    m_typeIcon->setProperty("type","msg-warn");

    modal();

    return m_modalresult;
}

int CMessage::error(const QString& mess)
{
    m_message->setText(mess);
    m_typeIcon->setProperty("type","msg-error");

    modal();

    return m_modalresult;
}

int CMessage::confirm(const QString& mess)
{
    m_message->setText(mess);
    m_typeIcon->setProperty("type","msg-confirm");

    modal();

    return m_modalresult;
}

#if defined(_WIN32)
int CMessage::confirm(HWND p, const QString& m)
#else
int CMessage::confirm(QWidget * p, const QString& m)
#endif
{
    CMessage mess(p);
    return mess.confirm(m);
}

#if defined(_WIN32)
int CMessage::info(HWND p, const QString& m)
#else
int CMessage::info(QWidget * p, const QString& m)
#endif
{
    CMessage mess(p);
    return mess.info(m);
}

#if defined(_WIN32)
int CMessage::warning(HWND p, const QString& m)
#else
int CMessage::warning(QWidget * p, const QString& m)
#endif
{
    CMessage mess(p);
    return mess.warning(m);
}

#if defined(_WIN32)
int CMessage::error(HWND p, const QString& m)
#else
int CMessage::error(QWidget * p, const QString& m)
#endif
{
    CMessage mess(p);
    return mess.error(m);
}

void CMessage::modal()
{
#if defined(_WIN32)
    m_centralWidget->adjustSize();
    m_centralWidget->show();

    QList<QPushButton *> l = m_boxButtons->findChildren<QPushButton *>();
    foreach (QPushButton * b, l) {
        if (l.size() == 1 || b->autoDefault() || b->isDefault()) {
            QTimer::singleShot(200, m_centralWidget, [b]{
                b->setFocus();
            });

            break;
        }
    }

    CWinWindow::setSize(m_centralWidget->width(), m_centralWidget->height());
    CWinWindow::center();
    CWinWindow::modal();
#else
    exec();
#endif
}

void CMessage::setIcon(int it)
{
    switch (it) {
    case MESSAGE_TYPE_WARN:     m_typeIcon->setProperty("type","msg-warn"); break;
    case MESSAGE_TYPE_INFO:     m_typeIcon->setProperty("type","msg-info"); break;
    case MESSAGE_TYPE_CONFIRM:  m_typeIcon->setProperty("type","msg-conf"); break;
    case MESSAGE_TYPE_ERROR:    m_typeIcon->setProperty("type","msg-error"); break;
    default:
        break;
    }
}

void CMessage::setText( const QString& t)
{
    m_message->setText(t);
}

void CMessage::applyForAll(const QString& str, bool checked)
{
    QBoxLayout * layout = qobject_cast<QBoxLayout *>(m_centralWidget->layout());
    QCheckBox * chbox = new QCheckBox(str);
    chbox->setObjectName("check-apply-for-all");
    chbox->setStyleSheet(QString("margin-left: %1px").arg(15 + m_typeIcon->width() * m_dpiRatio + 15 * m_dpiRatio));
    chbox->setChecked(checked);
    layout->insertWidget(1, chbox, 0);
}

bool CMessage::isForAll()
{
    QCheckBox * chbox = m_centralWidget->findChild<QCheckBox *>("check-apply-for-all");
    return chbox && chbox->checkState() == Qt::Checked;
}

void CMessage::onScreenScaling()
{
#if defined(_WIN32)
    uchar f = Utils::getScreenDpiRatioByHWND( int(CWinWindow::handle()) );
    if ( m_dpiRatio != f ) {
        /* change scaling factor for elements */
    }
#endif
}
