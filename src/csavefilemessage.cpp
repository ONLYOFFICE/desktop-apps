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

#include "csavefilemessage.h"
#include "asctabwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#ifdef Q_WS_WIN32
//#define WINVER 0x0500
#include <windows.h>
#endif // Q_WS_WIN32

#include <QDebug>
extern BYTE g_dpi_ratio;

#if defined(_WIN32)
CSaveFileMessage::CSaveFileMessage(HWND hParentWnd) : QWinWidget(hParentWnd),
#else
CSaveFileMessage::CSaveFileMessage(QWidget * parent) : QObject(parent),
#endif
    m_pDlg(parent), m_result(0), m_fLayout(new QFormLayout), m_mapFiles(NULL)
{
    m_pDlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

//        HWND hwnd = (HWND)winId();
//        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
//        style &= ~WS_SYSMENU; // unset the system menu flag
//        SetWindowLongPtr(hwnd, GWL_STYLE, style);
//        // force Windows to refresh some cached window styles
//        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);


    QVBoxLayout * layout = new QVBoxLayout;
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    layout->addLayout(h_layout2, 1);
    layout->addLayout(h_layout1, 0);

//    QVBoxLayout * v_layout1 = new QVBoxLayout;
    QLabel * icon = new QLabel;
    icon->setProperty("class","msg-icon");
    icon->setProperty("type","msg-question");
    icon->setFixedSize(35*g_dpi_ratio, 35*g_dpi_ratio);

    QLabel * question = new QLabel(tr("Do you want to save modified files?"));
    question->setStyleSheet(QString("margin-bottom: %1px;").arg(8*g_dpi_ratio));
    m_fLayout->addWidget(question);
    h_layout2->addWidget(icon, 0, Qt::AlignTop);
    h_layout2->addLayout(m_fLayout, 1);

    QPushButton * btn_yes       = new QPushButton("&Yes");
    QPushButton * btn_no        = new QPushButton("&No");
    QPushButton * btn_cancel    = new QPushButton("&Cancel");
    QWidget * box = new QWidget;
    box->setLayout(new QHBoxLayout);
    box->layout()->addWidget(btn_yes);
    box->layout()->addWidget(btn_no);
    box->layout()->addWidget(btn_cancel);
    box->layout()->setContentsMargins(0,8*g_dpi_ratio,0,0);
    h_layout1->addWidget(box, 0, Qt::AlignCenter);

    m_pDlg.setLayout(layout);
    m_pDlg.setMinimumWidth(400*g_dpi_ratio);

    QObject::connect(btn_yes, SIGNAL(clicked()), this, SLOT(onYesClicked()));
    connect(btn_no, SIGNAL(clicked()), this, SLOT(onNoClicked()));
    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

CSaveFileMessage::~CSaveFileMessage()
{
}

int CSaveFileMessage::showModal()
{
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

void CSaveFileMessage::setFiles(QMap<int, QString> * f)
{
    m_mapFiles = f;
    int count = m_mapFiles->size();

    QWidget * chb;
//    QMapIterator<int,QString> i(*m_mapFiles);
//    while (i.hasNext()) {
//        i.next();

//        if (count > 1) {
//            chb = new QCheckBox(i.value());
//            qobject_cast<QCheckBox*>(chb)->setCheckState(Qt::Checked);
//            chb->setProperty("view_id", i.key());
//        } else {
//            chb = new QLabel(i.value());
//        }

//        m_fLayout->addWidget(chb);
//    }

    QString str;
    for (auto k : m_mapFiles->keys()) {
        str = m_mapFiles->value(k);

        if (count > 1) {
            chb = new QCheckBox(str);
            qobject_cast<QCheckBox*>(chb)->setCheckState(Qt::Checked);
            chb->setProperty("view_id", k);
        } else {
            chb = new QLabel(str);
        }

        m_fLayout->addWidget(chb);
    }
}

void CSaveFileMessage::setFiles(const QString& file)
{
    m_fLayout->addWidget(new QLabel(file));
}

void CSaveFileMessage::onYesClicked()
{
    if (m_mapFiles && m_mapFiles->size() > 1) {
        QCheckBox * chb;
        int count = m_fLayout->count();
        for (int i = count; i-- > 1; ) {
            chb = (QCheckBox *)m_fLayout->itemAt(i, QFormLayout::FieldRole)->widget();

            if (chb->checkState() == Qt::Unchecked) {
                m_mapFiles->remove(chb->property("view_id").toInt());
            }
        }
    }

    m_result = 1;
    m_pDlg.accept();
}

void CSaveFileMessage::onNoClicked()
{
    m_result = -1;
    m_pDlg.reject();
}

void CSaveFileMessage::onCancelClicked()
{
    m_result = 0;
    m_pDlg.reject();
}
