/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CMESSAGE_QT_H
#define CMESSAGE_QT_H


#include "components/cmessage.h"
#include <QDialog>
#include <QMetaObject>
#include <vector>
#include "cascapplicationmanagerwrapper.h"
#include "utils.h"
#include <QApplication>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextDocumentFragment>
#include <functional>
#ifdef _WIN32
# include <Windows.h>
#endif

#define MSG_ICON_WIDTH  35
#define MSG_ICON_HEIGHT 35

class MessageQtBackend : public QDialog
{
public:
    explicit MessageQtBackend(QWidget *parent);
    ~MessageQtBackend();

    static int showMessage(QWidget *parent, const QString &msg, CMessage::Icon msgIcon,
                           CMessage::Buttons msgBtns, const CMessage::Config &conf);

    void setButtons(std::initializer_list<QString>);
    void setButtons(CMessage::Buttons);
    void setDefaultButton(CMessage::DefaultButton);
    void setIcon(CMessage::Icon);
    void setText(const QString&);
    void setConfig(const CMessage::Config&);
    int exec();

private:
    void setContent(const QString&);
    void setCheckBox(const QString &checkBoxText, bool checkBoxState);
    bool getCheckStatus() const;
    void addButton(QPushButton*);
    void clearButtons();

    QWidget *m_boxButtons = nullptr,
            *m_centralWidget = nullptr;
    QLabel  *m_message = nullptr,
            *m_content = nullptr,
            *m_typeIcon = nullptr;

    std::vector<QPushButton*> m_buttons;
    QPushButton *m_defaultButton = nullptr;
    QWidget *m_focusWidget = nullptr;
    QMetaObject::Connection m_focusConnection;
    CMessage::Config m_config;
    bool m_configApplied = false;
    double m_dpiRatio = 1;
    bool m_isWindowActive = false;
    int m_modalresult = int(CMessage::Result::Cancel);
};

inline MessageQtBackend::MessageQtBackend(QWidget *parent)
    : QDialog(parent)
    , m_boxButtons(new QWidget)
    , m_message(new QLabel)
    , m_content(new QLabel)
    , m_typeIcon(new QLabel)
    , m_dpiRatio(Utils::getScreenDpiRatioByWidget(parent))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // setWindowTitle(APP_TITLE);
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

    const int _body_margin = int(12 * m_dpiRatio);
    _c_layout->setContentsMargins(_body_margin,_body_margin,_body_margin,_body_margin);

    m_typeIcon->setProperty("class", "msg-icon");
    m_typeIcon->setFixedSize(int(round(MSG_ICON_WIDTH*m_dpiRatio - 0.25)),
                             int(round(MSG_ICON_HEIGHT*m_dpiRatio - 0.25)));
    _h_layout2->addWidget(m_typeIcon, 0, Qt::AlignTop);

    // m_message->setWordWrap(true);
    m_message->setProperty("class", "msg-report");
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(int(8*m_dpiRatio)));
    m_message->setTextFormat(Qt::PlainText);

    m_content->setProperty("class", "msg-report");
    m_content->setStyleSheet(QString("margin-bottom: %1px;").arg(int(8*m_dpiRatio)));
    m_content->setTextFormat(Qt::RichText);
    m_content->setOpenExternalLinks(true);

    QFormLayout * _f_layout = new QFormLayout;
    _f_layout->addWidget(m_message);
    _f_layout->addWidget(m_content);
    _f_layout->setSpacing(0);
    _f_layout->setContentsMargins(int(10*m_dpiRatio),0,int(5*m_dpiRatio),0);
    _h_layout2->addLayout(_f_layout, 1);
    _h_layout2->setContentsMargins(0,0,0,0);

    m_boxButtons->setLayout(new QHBoxLayout);
    m_boxButtons->layout()->setContentsMargins(0,int(10*m_dpiRatio),0,0);
    m_boxButtons->layout()->setSpacing(int(8*m_dpiRatio));
    _h_layout1->addWidget(m_boxButtons, 0, Qt::AlignCenter);

    setButtons(CMessage::Buttons::Ok);

    m_centralWidget->setLayout(_c_layout);
    m_centralWidget->setMinimumWidth(int(350*m_dpiRatio));
    m_centralWidget->move(0, 0);

    QString _styles(Utils::readStylesheets(":/styles/message.qss"));
    _styles.append(QString("QPushButton{min-width:%1px;}").arg(int(40*m_dpiRatio)));
    m_centralWidget->setStyleSheet( _styles );

    QString zoom = QString::number(m_dpiRatio) + "x";
    m_centralWidget->setProperty("scaling", zoom);

    m_focusConnection = QObject::connect(qApp, &QApplication::focusChanged, this,
        [&] (QWidget * from, QWidget *to)
        {
            Q_UNUSED(from)
            if ( m_isWindowActive )
            {
                if ( !to )
                {
                    m_focusWidget
                        ? m_focusWidget->setFocus(Qt::FocusReason::TabFocusReason)
                        : m_buttons.front()->setFocus(Qt::FocusReason::TabFocusReason);
                }
            }
        });
}

inline MessageQtBackend::~MessageQtBackend()
{
    QObject::disconnect(m_focusConnection);
}

inline void MessageQtBackend::addButton(QPushButton *btn)
{
    if ( !m_defaultButton )
        m_defaultButton = btn;

    m_buttons.push_back(btn);
}

inline void MessageQtBackend::clearButtons()
{
    m_defaultButton = nullptr;
    m_buttons.clear();
}

inline void MessageQtBackend::setButtons(std::initializer_list<QString> btns)
{
    QLayoutItem * item;
    QWidget * widget;
    while ( (item = m_boxButtons->layout()->takeAt(0)) )
    {
        if ( (widget = item->widget()) )
        {
            delete widget;
        }

        delete item;
    }
    clearButtons();

    auto _fn_click =
        [=](int res)
        {
            m_modalresult = res;
            close();
        };

    auto _fn_result =
        [](const QString &name, int index)
        {
            using BtnText = CMessage::ButtonText;
            if ( name == BtnText::ok() )     return int(CMessage::Result::Ok);
            if ( name == BtnText::yes() )    return int(CMessage::Result::Yes);
            if ( name == BtnText::no() )     return int(CMessage::Result::No);
            if ( name == BtnText::cancel() ) return int(CMessage::Result::Cancel);
            return int(CMessage::Result::FirstButton) + index;
        };

    QPushButton * _btn;
    int _btn_num(0);
    for (auto &btn: btns)
    {
        _btn = new QPushButton(btn);

        m_boxButtons->layout()->addWidget(_btn);
        int res = _fn_result(btn, _btn_num);
        QObject::connect(_btn, &QPushButton::clicked, std::bind(_fn_click, res));

        addButton(_btn);
        _btn_num++;
    }

    if (_btn_num > 2)
        m_centralWidget->setMinimumWidth(int(400*m_dpiRatio));
}

inline void MessageQtBackend::setButtons(CMessage::Buttons btns)
{
    using Buttons = CMessage::Buttons;
    using BtnText = CMessage::ButtonText;
    using DefaultButton = CMessage::DefaultButton;
    switch (btns)
    {
    case Buttons::Ok:
        setButtons({BtnText::ok()});
        setDefaultButton(DefaultButton::FirstButton);
        break;
    case Buttons::OkCancel:
        setButtons({BtnText::ok(), BtnText::cancel()});
        setDefaultButton(DefaultButton::SecondButton);
        break;
    case Buttons::OkDefCancel:
        setButtons({BtnText::ok(), BtnText::cancel()});
        setDefaultButton(DefaultButton::FirstButton);
        break;
    case Buttons::YesNo:
        setButtons({BtnText::yes(), BtnText::no()});
        setDefaultButton(DefaultButton::SecondButton);
        break;
    case Buttons::YesDefNo:
        setButtons({BtnText::yes(), BtnText::no()});
        setDefaultButton(DefaultButton::FirstButton);
        break;
    case Buttons::YesNoCancel:
        setButtons({BtnText::yes(), BtnText::no(), BtnText::cancel()});
        setDefaultButton(DefaultButton::ThirdButton);
        break;
    case Buttons::YesDefNoCancel:
        setButtons({BtnText::yes(), BtnText::no(), BtnText::cancel()});
        setDefaultButton(DefaultButton::FirstButton);
        break;
    default: break;
    }
}

inline void MessageQtBackend::setDefaultButton(CMessage::DefaultButton btn)
{
    const size_t index = size_t(btn);
    if (index < m_buttons.size())
    {
        for (size_t i = 0; i < m_buttons.size(); i++)
            m_buttons[i]->setDefault(i == index);
        m_defaultButton = m_buttons[index];
    }
}

inline void MessageQtBackend::setIcon(CMessage::Icon msgIcon)
{
    using Type = CMessage::Icon;
    switch (msgIcon)
    {
    case Type::Warning: m_typeIcon->setProperty("type", "msg-warn"); break;
    case Type::Info:    m_typeIcon->setProperty("type", "msg-info"); break;
    case Type::Confirm: m_typeIcon->setProperty("type", "msg-conf"); break;
    case Type::Error:   m_typeIcon->setProperty("type", "msg-error"); break;
    case Type::Brand:   m_typeIcon->setProperty("type", "msg-info"); break;
    default: break;
    }
}

inline void MessageQtBackend::setText(const QString &text)
{
    m_message->setText(text);
}

inline void MessageQtBackend::setContent(const QString &content)
{
    m_content->setText(content);
}

inline void MessageQtBackend::setConfig(const CMessage::Config &conf)
{
    m_config = conf;
    m_configApplied = false;
}

inline void MessageQtBackend::setCheckBox(const QString &checkBoxText, bool checkBoxState)
{
    QBoxLayout * layout = qobject_cast<QBoxLayout *>(m_centralWidget->layout());
    QCheckBox * chbox = new QCheckBox(checkBoxText);
    chbox->setObjectName("check-apply-for-all");
    chbox->setStyleSheet(QString("margin-left: %1px")
                             .arg(15 + int(m_typeIcon->width() * m_dpiRatio) + int(15 * m_dpiRatio)));
    chbox->setChecked(checkBoxState);
    layout->insertWidget(1, chbox, 0);
}

inline bool MessageQtBackend::getCheckStatus() const
{
    QCheckBox * chbox = m_centralWidget->findChild<QCheckBox *>("check-apply-for-all");
    return chbox && chbox->checkState() == Qt::Checked;
}

inline int MessageQtBackend::exec()
{
    if (!m_configApplied)
    {
        m_configApplied = true;
        QString content = m_config.contentText;
        if (!content.isEmpty() && !m_config.linkText.isEmpty())
        {
            content.append("\n");
            content.replace("\n", "<br>");
        }
        content.append(m_config.linkText);
        if (!content.isEmpty())
            setContent(content);
        if (m_config.checkBoxState != nullptr)
            setCheckBox(m_config.checkBoxText, *m_config.checkBoxState);
    }

    QDialog::exec();
    if (m_config.checkBoxState != nullptr)
        *m_config.checkBoxState = getCheckStatus();
    return m_modalresult;
}

inline int MessageQtBackend::showMessage(QWidget *parent, const QString &msg, CMessage::Icon msgIcon,
                                         CMessage::Buttons msgBtns, const CMessage::Config &conf)
{
#ifdef __linux__
    WindowHelper::CParentDisable oDisabler(parent);
#endif
    MessageQtBackend dlg(parent);
    if (AscAppManager::isRtlEnabled())
    {
#ifdef _WIN32
        LONG exstyle = GetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE);
        SetWindowLong((HWND)dlg.winId(), GWL_EXSTYLE, exstyle | WS_EX_LAYOUTRTL);
#else
        dlg.setLayoutDirection(Qt::RightToLeft);
#endif
    }
    dlg.setText(QTextDocumentFragment::fromHtml(msg).toPlainText());
    dlg.setConfig(conf);
    dlg.setIcon(msgIcon);
    if (msgBtns != CMessage::Buttons::Ok)
        dlg.setButtons(msgBtns);
    return dlg.exec();
}

#endif // CMESSAGE_QT_H

