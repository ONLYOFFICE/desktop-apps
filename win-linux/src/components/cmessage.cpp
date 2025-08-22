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

#include "components/cmessage.h"
#include <QTextDocumentFragment>
#include <QDialog>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QVariant>
#include <QTimer>
#include <functional>
#include <QEvent>
//#include <QKeyEvent>
#include <QCheckBox>
#include <QApplication>
#include "defines.h"
#include "utils.h"
#include "cascapplicationmanagerwrapper.h"
#include <initializer_list>
#include <memory.h>


#ifdef __linux__
# include "platform_linux/gtkmessage.h"
#else
# include "platform_win/message.h"
#endif

#define MSG_ICON_WIDTH  35
#define MSG_ICON_HEIGHT 35

#define DEFAULT_BUTTON(label) label + ":default"

//    class CMessageEventsFilter : public QObject {
//    public:
//        CMessageEventsFilter(CMessage * p, QObject * o)
//            : QObject(o), m_mess(p)
//        {}
//
//    protected:
//        bool eventFilter(QObject * obj, QEvent * event)
//        {
//            if (event->type()==QEvent::KeyPress) {
//                QKeyEvent * key = static_cast<QKeyEvent*>(event);
//                if ( key->key()==Qt::Key_Escape ) {
//                    m_mess->close();
//                    return true;
//                }
//            }
//
//            return QObject::eventFilter(obj, event);
//        }
//
//    private:
//        CMessage * m_mess;
//    };

class QtMsg : public QDialog
{
public:
    explicit QtMsg(QWidget *);
    ~QtMsg();

    static int showMessage(QWidget *parent,
                           const QString &msg,
                           MsgType msgType,
                           MsgBtns msgBtns = MsgBtns::mbOk,
                           bool   *checkBoxState = nullptr,
                           const QString &chekBoxText = QString());
private:
    void setButtons(std::initializer_list<QString>);
    void setButtons(MsgBtns);
    void setIcon(MsgType);
    void setText(const QString&);
    void setCheckBox(const QString &chekBoxText, bool checkBoxState);
    bool getCheckStatus();

    QWidget *m_boxButtons = nullptr,
            *m_centralWidget = nullptr;
    QLabel  *m_message = nullptr,
            *m_typeIcon = nullptr;

    static int m_modalresult;

    class QtMsgPrivateIntf;
    std::unique_ptr<QtMsgPrivateIntf> m_priv;
};

class QtMsg::QtMsgPrivateIntf {
public:
    explicit QtMsgPrivateIntf(QtMsg * parent)
        : m_mess(parent)
        , dpiRatio(Utils::getScreenDpiRatioByWidget(parent))
    {}

    auto addButton(QPushButton * b) -> void {
        if ( !defaultButton )
            defaultButton = b;

        buttons.push_back(b);
    }

    auto firstButton() -> QPushButton * {
        return buttons.front();
    }

    auto lastButton() -> QPushButton * {
        return buttons.back();
    }

    auto clearButtons() -> void {
        defaultButton = nullptr;
        buttons.clear();
    }

    QtMsg * m_mess = nullptr;
    std::vector<QPushButton *> buttons;
    QPushButton * defaultButton = nullptr;
    QWidget * focusWidget = nullptr;
    double dpiRatio = 1;
    QMetaObject::Connection focusConnection;
    bool isWindowActive = false;
};

int QtMsg::m_modalresult(MODAL_RESULT_CANCEL);

QtMsg::QtMsg(QWidget * p)
    : QDialog(p)
    , m_boxButtons(new QWidget)
    , m_message(new QLabel)
    , m_typeIcon(new QLabel)
    , m_priv(new QtMsgPrivateIntf(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //setWindowTitle(APP_TITLE);
    setLayout(new QVBoxLayout);
    layout()->setContentsMargins(0, 0, 0, 0);

    m_centralWidget = new QWidget(this);
    layout()->addWidget(m_centralWidget);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    m_centralWidget->setObjectName("messageBody");
    m_centralWidget->setProperty("uitheme", QString::fromStdWString(GetCurrentTheme().originalId()));

    QVBoxLayout * _c_layout  = new QVBoxLayout;
    QHBoxLayout * _h_layout2 = new QHBoxLayout;
    QHBoxLayout * _h_layout1 = new QHBoxLayout;
    _c_layout->addLayout(_h_layout2, 1);
    _c_layout->addLayout(_h_layout1, 0);

    const int _body_margin = int(12 * m_priv->dpiRatio);
    _c_layout->setContentsMargins(_body_margin,_body_margin,_body_margin,_body_margin);

    m_typeIcon->setProperty("class", "msg-icon");
    m_typeIcon->setFixedSize(int(round(MSG_ICON_WIDTH*m_priv->dpiRatio - 0.25)),
                             int(round(MSG_ICON_HEIGHT*m_priv->dpiRatio - 0.25)));
    _h_layout2->addWidget(m_typeIcon, 0, Qt::AlignTop);

//    m_message->setWordWrap(true);
    m_message->setProperty("class", "msg-report");
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(int(8*m_priv->dpiRatio)));
    m_message->setTextFormat(Qt::PlainText);

    QFormLayout * _f_layout = new QFormLayout;
    _f_layout->addWidget(m_message);
    _f_layout->setSpacing(0);
    _f_layout->setContentsMargins(int(10*m_priv->dpiRatio),0,int(5*m_priv->dpiRatio),0);
    _h_layout2->addLayout(_f_layout, 1);
    _h_layout2->setContentsMargins(0,0,0,0);

    QPushButton * btn_ok = new QPushButton("&" + QObject::tr("OK"));
    btn_ok->setAutoDefault(true);
    m_boxButtons->setLayout(new QHBoxLayout);
    m_boxButtons->layout()->addWidget(btn_ok);
    m_boxButtons->layout()->setContentsMargins(0,int(10*m_priv->dpiRatio),0,0);
    m_boxButtons->layout()->setSpacing(int(8*m_priv->dpiRatio));
    _h_layout1->addWidget(m_boxButtons, 0, Qt::AlignCenter);

    m_priv->addButton(btn_ok);

    QObject::connect(btn_ok, &QPushButton::clicked, this,
        [=] {
            m_modalresult = MODAL_RESULT_OK;
            close();
        }
    );

    m_centralWidget->setLayout(_c_layout);
    m_centralWidget->setMinimumWidth(int(350*m_priv->dpiRatio));
    m_centralWidget->move(0, 0);

    QString _styles(Utils::readStylesheets(":/styles/message.qss"));
    _styles.append(QString("QPushButton{min-width:%1px;}").arg(int(40*m_priv->dpiRatio)));
    m_centralWidget->setStyleSheet( _styles );

    QString zoom = QString::number(m_priv->dpiRatio) + "x";
    m_centralWidget->setProperty("scaling", zoom);

    m_priv->focusConnection = QObject::connect(qApp, &QApplication::focusChanged, this,
                                               [&] (QWidget * from, QWidget *to){
        Q_UNUSED(from)
        if ( m_priv->isWindowActive ) {
            if ( !to ) {
                m_priv->focusWidget ?
                    m_priv->focusWidget->setFocus(Qt::FocusReason::TabFocusReason) :
                    m_priv->firstButton()->setFocus(Qt::FocusReason::TabFocusReason);
            }
        } else {
        }
    });
}

QtMsg::~QtMsg()
{
    QObject::disconnect(m_priv->focusConnection);
}

void QtMsg::setButtons(std::initializer_list<QString> btns)
{
    QLayoutItem * item;
    QWidget * widget;
    while ( (item = m_boxButtons->layout()->takeAt(0)) ) {
        if ( (widget = item->widget()) ) {
            delete widget;
        }

        delete item;
    }
    m_priv->clearButtons();

    auto _fn_click = [=](MsgRes msgRes) {
        m_modalresult = msgRes;
        close();
    };

    QRegularExpression reFocus("([^:]+)\\:?(default)?$");
    QRegularExpressionMatch match;

    QPushButton * _btn;
    int _btn_num(0);
    for (auto &btn: btns) {
        match = reFocus.match(btn);

        _btn = new QPushButton(match.captured(1));
        if ( !match.captured(2).isEmpty() ) {
            _btn->setDefault(true);
        }

        QMap<MsgRes, QString> btnNames = {
            {MODAL_RESULT_CANCEL, BTN_TEXT_CANCEL},
            {MODAL_RESULT_YES,    BTN_TEXT_YES},
            {MODAL_RESULT_NO,     BTN_TEXT_NO},
            {MODAL_RESULT_OK,     BTN_TEXT_OK},
            {MODAL_RESULT_SKIP,   BTN_TEXT_SKIP},
            {MODAL_RESULT_BUY,    BTN_TEXT_BUY},
            {MODAL_RESULT_ACTIVATE,    BTN_TEXT_ACTIVATE},
            {MODAL_RESULT_CONTINUE,    BTN_TEXT_CONTINUE}
        };

        m_boxButtons->layout()->addWidget(_btn);
        MsgRes msgRes = btnNames.key(match.captured(1), MODAL_RESULT_CANCEL);
        QObject::connect(_btn, &QPushButton::clicked, std::bind(_fn_click, msgRes));

        m_priv->addButton(_btn);
        _btn_num++;
    }

    if (_btn_num > 2)
        m_centralWidget->setMinimumWidth(int(400*m_priv->dpiRatio));
}

void QtMsg::setButtons(MsgBtns btns)
{
    switch (btns) {
    case MsgBtns::mbYesNo:          setButtons({BTN_TEXT_YES, DEFAULT_BUTTON(BTN_TEXT_NO)}); break;
    case MsgBtns::mbYesDefNo:       setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_NO}); break;
    case MsgBtns::mbYesNoCancel:    setButtons({BTN_TEXT_YES, BTN_TEXT_NO, DEFAULT_BUTTON(BTN_TEXT_CANCEL)}); break;
    case MsgBtns::mbYesDefNoCancel: setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_NO, BTN_TEXT_CANCEL}); break;
    case MsgBtns::mbOkCancel:       setButtons({BTN_TEXT_OK, DEFAULT_BUTTON(BTN_TEXT_CANCEL)}); break;
    case MsgBtns::mbOkDefCancel:    setButtons({DEFAULT_BUTTON(BTN_TEXT_OK), BTN_TEXT_CANCEL}); break;
    case MsgBtns::mbYesDefSkipNo:   setButtons({DEFAULT_BUTTON(BTN_TEXT_YES), BTN_TEXT_SKIP, BTN_TEXT_NO}); break;
    case MsgBtns::mbBuy:            setButtons({DEFAULT_BUTTON(BTN_TEXT_BUY)}); break;
    case MsgBtns::mbActivateDefContinue:   setButtons({DEFAULT_BUTTON(BTN_TEXT_ACTIVATE), BTN_TEXT_CONTINUE}); break;
    case MsgBtns::mbContinue:       setButtons({DEFAULT_BUTTON(BTN_TEXT_CONTINUE)}); break;
    default: break;
    }
}

void QtMsg::setCheckBox(const QString &chekBoxText, bool checkBoxState)
{
    QBoxLayout * layout = qobject_cast<QBoxLayout *>(m_centralWidget->layout());
    QCheckBox * chbox = new QCheckBox(chekBoxText);
    chbox->setObjectName("check-apply-for-all");
    chbox->setStyleSheet(QString("margin-left: %1px").arg(15 + int(m_typeIcon->width() * m_priv->dpiRatio) + int(15 * m_priv->dpiRatio)));
    chbox->setChecked(checkBoxState);
    layout->insertWidget(1, chbox, 0);
}

bool QtMsg::getCheckStatus()
{
    QCheckBox * chbox = m_centralWidget->findChild<QCheckBox *>("check-apply-for-all");
    return chbox && chbox->checkState() == Qt::Checked;
}

int QtMsg::showMessage(QWidget *parent,
                          const QString &msg,
                          MsgType msgType,
                          MsgBtns msgBtns,
                          bool   *checkBoxState,
                          const QString &chekBoxText)
{
#ifdef __linux__
    WindowHelper::CParentDisable oDisabler(parent);
#endif
    QtMsg dlg(parent);
    if (AscAppManager::isRtlEnabled()) {
#ifdef _WIN32
        LONG exstyle = GetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE);
        SetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE, exstyle | WS_EX_LAYOUTRTL);
#else
        dlg.setLayoutDirection(Qt::RightToLeft);
#endif
    }
    dlg.setText(QTextDocumentFragment::fromHtml(msg).toPlainText());
    dlg.setIcon(msgType);
    if (msgBtns != MsgBtns::mbOk)
        dlg.setButtons(msgBtns);
    if (checkBoxState != nullptr)
        dlg.setCheckBox(chekBoxText, *checkBoxState);
    dlg.exec();
    if (checkBoxState != nullptr)
        *checkBoxState = dlg.getCheckStatus();
    return m_modalresult;
}

void QtMsg::setIcon(MsgType msgType)
{
    switch (msgType) {
    case MsgType::MSG_WARN:    m_typeIcon->setProperty("type", "msg-warn"); break;
    case MsgType::MSG_INFO:    m_typeIcon->setProperty("type", "msg-info"); break;
    case MsgType::MSG_CONFIRM: m_typeIcon->setProperty("type", "msg-conf"); break;
    case MsgType::MSG_ERROR:   m_typeIcon->setProperty("type", "msg-error"); break;
    default: break;
    }
}

void QtMsg::setText( const QString& t)
{
    m_message->setText(t);
}

// -------------------- CMessage ------------------

int CMessage::showMessage(QWidget *parent,
                          const QString &msg,
                          MsgType msgType,
                          MsgBtns msgBtns,
                          bool   *checkBoxState,
                          const QString &chekBoxText)
{
    if (WindowHelper::useNativeDialog()) {
#ifdef _WIN32
# ifndef __OS_WIN_XP
        return WinMsg::showMessage(parent, msg, msgType, msgBtns, checkBoxState, chekBoxText);
# endif
#else
        WindowHelper::CParentDisable oDisabler(parent);
        return GtkMsg::showMessage(parent, msg, msgType, msgBtns, checkBoxState, chekBoxText);
#endif
    }
    return QtMsg::showMessage(parent, msg, msgType, msgBtns, checkBoxState, chekBoxText);
}

void CMessage::confirm(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_CONFIRM);
}

void CMessage::info(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_INFO);
}

void CMessage::warning(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_WARN);
}

void CMessage::error(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, MsgType::MSG_ERROR);
}
