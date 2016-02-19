/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
#include "defines.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <QLabel>
#include "qcefview.h"

extern BYTE g_dpi_ratio;
extern QString g_lang;

#ifdef _WIN32
CMessage::CMessage(HWND hParentWnd) : QWinWidget(hParentWnd)
  , m_pDlg(this)
#else
CMessage::CMessage(QWidget * parent) : QObject(parent)
  , m_pDlg(parent)
#endif
    , m_result(MODAL_RESULT_CANCEL)
    , m_fLayout(new QFormLayout)
{
    m_pDlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    QVBoxLayout * layout = new QVBoxLayout;
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    layout->addLayout(h_layout2, 1);
    layout->addLayout(h_layout1, 0);

    m_typeIcon = new QLabel;
    m_typeIcon->setProperty("class","msg-icon");
    m_typeIcon->setFixedSize(35*g_dpi_ratio, 35*g_dpi_ratio);

    m_message = new QLabel;
//    m_message->setWordWrap(true);
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(8*g_dpi_ratio));
//    question->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_fLayout->addWidget(m_message);
    m_fLayout->setSpacing(0);
    m_fLayout->setContentsMargins(10,0,5,0);
    h_layout2->addWidget(m_typeIcon, 0, Qt::AlignTop);
    h_layout2->addLayout(m_fLayout, 1);

    QPushButton * btn_yes       = new QPushButton(tr("&OK"));
//    QPushButton * btn_no        = new QPushButton("&No");
//    QPushButton * btn_cancel    = new QPushButton("&Cancel");
    m_boxButtons = new QWidget;
    m_boxButtons->setLayout(new QHBoxLayout);
    m_boxButtons->layout()->addWidget(btn_yes);
//    box->layout()->addWidget(btn_no);
//    box->layout()->addWidget(btn_cancel);
    m_boxButtons->layout()->setContentsMargins(0,8*g_dpi_ratio,0,0);
    h_layout1->addWidget(m_boxButtons, 0, Qt::AlignCenter);

    m_pDlg.setLayout(layout);
    m_pDlg.setMinimumWidth(350*g_dpi_ratio);
    m_pDlg.setWindowTitle(APP_TITLE);

    connect(btn_yes, &QPushButton::clicked, this, &CMessage::onYesClicked);
//    connect(btn_no, SIGNAL(clicked()), this, SLOT(onNoClicked()));
//    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));

    setStyleSheet("QPushButton:focus{border-color:#3a83db;}");
}

void CMessage::error(const QString& title, const QString& text)
{
    QMessageBox msgBox((QWidget *)this);
    msgBox.setWindowTitle( title );
    msgBox.setText( text );
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setIcon(QMessageBox::Critical);

    msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    msgBox.exec();
}

int CMessage::showModal(const QString& mess, QMessageBox::Icon icon)
{
    m_message->setText(mess);
    if (icon == QMessageBox::Critical) {
        m_typeIcon->setProperty("type","msg-error");
    } else
    if (icon == QMessageBox::Information) {
        m_typeIcon->setProperty("type","msg-info");
    }

    m_pDlg.adjustSize();

#if defined(_WIN32)
    RECT rc;
    ::GetWindowRect(parentWindow(), &rc);

    int x = rc.left + (rc.right - rc.left - m_pDlg.width())/2;
    int y = (rc.bottom - rc.top - m_pDlg.height())/2;

    m_pDlg.move(x, y);
#endif

    m_pDlg.exec();

    return m_result;
}

void CMessage::onYesClicked()
{
    m_result = MODAL_RESULT_YES;
    m_pDlg.accept();
}

void CMessage::setButtons(const QString& cbtn1, const QString& cbtn2)
{
    foreach (QWidget * w, m_boxButtons->findChildren<QWidget*>()) {
        delete w;
    }

    QPushButton * _btn = new QPushButton(cbtn1);
    m_boxButtons->layout()->addWidget(_btn);
    connect(_btn, &QPushButton::clicked, [=](){
        m_result = 201;
        m_pDlg.accept();
    });

    if (cbtn2.size()) {
        _btn = new QPushButton(cbtn2);
        m_boxButtons->layout()->addWidget(_btn);
        connect(_btn, &QPushButton::clicked, [=](){
            m_result = 202;
            m_pDlg.accept();
        });
    }
}
