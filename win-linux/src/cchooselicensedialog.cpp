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

#include "cchooselicensedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVariant>
#include <QDesktopServices>
#include <QUrl>
#include "defines.h"

extern uchar g_dpi_ratio;
extern QString g_lang;

CChooseLicenseDialog::CChooseLicenseDialog(QWidget *parent) : QDialog(parent)
  , m_license(LICENSE_TYPE_BUSINESS)
{
    QVBoxLayout * _layout = new QVBoxLayout;
    QLabel * _labelWelcome = new QLabel;
    QLabel * _labelChoose = new QLabel(tr("Choose type of the license"));
    QLabel * _labelLicense = new QLabel;

    QString _str_welcome = tr("Welcome to %1!");
#ifdef _IVOLGA_PRO
    _labelWelcome->setText(_str_welcome.arg(APP_TITLE));
    setWindowIcon(QIcon(":/ivolga/app.ico"));
#else
    _labelWelcome->setText(_str_welcome.arg("OnlyOffice"));
    setWindowIcon(QIcon(":/res/icons/desktopeditors.ico"));
#endif

    _labelWelcome->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _labelChoose->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _labelLicense->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    _labelWelcome->setObjectName("labelWelcome");
    _labelChoose->setObjectName("labelChoose");
    _labelLicense->setObjectName("labelGoLicense");

    _layout->addWidget(_labelWelcome, 0, Qt::AlignHCenter);
    _layout->addWidget(_labelChoose, 0, Qt::AlignHCenter);

    _layout->setContentsMargins(30,20,30,30);

    QHBoxLayout * _hbox = new QHBoxLayout;
    QToolButton * _btn1 = new QToolButton;
    QToolButton * _btn2 = new QToolButton;
    _btn1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _btn1->setIconSize(QSize(128,128));
    _btn1->setIcon(QIcon(":/res/home.png"));
    _btn1->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _btn1->setText(tr("HOME"));
    _btn1->setCursor(QCursor(Qt::PointingHandCursor));

    _btn2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _btn2->setIconSize(QSize(128,128));
    _btn2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _btn2->setIcon(QIcon(":/res/busines.png"));
    _btn2->setText(tr("COMMERCIAL"));
    _btn2->setCursor(QCursor(Qt::PointingHandCursor));

    g_dpi_ratio > 1 ?
        _btn1->setIcon(QIcon(":/license/home@2x.png")), _btn2->setIcon(QIcon(":/license/busines@2x.png")) :
                _btn1->setIcon(QIcon(":/license/home.png")), _btn2->setIcon(QIcon(":/license/busines.png"));

    connect(_btn1, &QToolButton::clicked, [=](){
        done((m_license = LICENSE_TYPE_FREE));
    });

    connect(_btn2, &QToolButton::clicked, [=](){
        done(m_license);
    });

    QFrame * _vline = new QFrame;
    _vline->setFixedWidth(1);

    _hbox->addWidget(_btn1);
    _hbox->addWidget(_vline);
    _hbox->addWidget(_btn2);
    _hbox->setSpacing(0);
    _layout->addLayout(_hbox);

    _labelLicense->setText("<a href=\"#\">"+tr("License")+"</a>");
    _labelLicense->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(_labelLicense, &QLabel::linkActivated, [=](){
        if (m_eulaPath.size()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_eulaPath));
        }
    });

    _layout->addWidget(_labelLicense, 0, Qt::AlignHCenter);

    QString _css = "QToolButton{padding:20px; border:0 none; font-weight:bold;}";
    _css.append("QToolButton{border-radius:5px;}");
    _css.append("QToolButton:hover{background:#e5e5e5;}");
    _css.append("#labelWelcome{font-size:20px; color:#3d4a6b; font-weight:100; margin-bottom:0px}");
    _css.append("#labelChoose{margin-bottom:10px}");
    _css.append("#labelGoLicense{margin-top:20px}");
    _css.append(".QFrame{border:5px solid #fff; background-color:#f0f0f0;}");
    _css.append(".QFrame{border-left:0 none; border-right:0 none;}");
    _css.append("QDialog{background:#fff;}");
    _css.append("*{font-family:'Open Sans',sans-serif; font-size:15px; color:#333;}");

    setStyleSheet(_css);
    setLayout(_layout);

    setFixedSize(500, 385);
    setWindowTitle(APP_TITLE);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::MSWindowsFixedSizeDialogHint);
}

int CChooseLicenseDialog::license() const
{
    return m_license;
}

void CChooseLicenseDialog::setEULAPath(const QString& path)
{
    m_eulaPath = path;
}

void CChooseLicenseDialog::reject()
{

}
