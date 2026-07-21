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


#include "upgradedialog.h"
#include "qcefview.h"
#include "cascapplicationmanagerwrapper.h"
#include <QVBoxLayout>


UpgradeDialog::UpgradeDialog(QWidget *parent, const QRect &rc)
    : CAscDialog(parent, rc)
{
    m_pMainPanel = new AscMainPanel(this);
    m_pMainPanel->setObjectName(QString::fromUtf8("mainPanel"));

    QVBoxLayout *centralVLayout = new QVBoxLayout(m_pMainPanel);
    centralVLayout->setSpacing(0);
    centralVLayout->setContentsMargins(0, 0, 0, 0);

    QCefView *contentCefView = AscAppManager::createViewer(m_pMainPanel, rc.size());
    contentCefView->setObjectName("contentCefView");
    contentCefView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centralVLayout->addWidget(contentCefView);
    setCentralWidget(m_pMainPanel);

    m_pMainView = contentCefView;

    AscAppManager &app = AscAppManager::getInstance();
    contentCefView->Create(&app, cvwtSimple);

    const QColor color = GetCurrentTheme().color(CTheme::ColorRole::ecrWindowBackground);
    contentCefView->SetBackgroundCefColor(uchar(color.red()), uchar(color.green()), uchar(color.blue()));

    const QString addonUrl = QStringLiteral("file:///") + qApp->applicationDirPath() +
                             QStringLiteral("/addon.html");
    contentCefView->GetCefView()->load(addonUrl.toStdWString());
}

UpgradeDialog::~UpgradeDialog()
{
    AscAppManager &app = AscAppManager::getInstance();
    QCefView *contentCefView = qobject_cast<QCefView*>(m_pMainView);
    app.DestroyCefView(contentCefView->GetCefView()->GetId());
    m_pMainView = nullptr;
}

/** Public **/

void UpgradeDialog::setProgress(int percent)
{

}

void UpgradeDialog::showStartPage()
{

}

void UpgradeDialog::showFinalPage()
{

}

void UpgradeDialog::showErrorPage(const QString &message)
{

}

/** Protected **/

/** Private **/

void UpgradeDialog::acceptRestartNow()
{
    accept(DialogCode::RestartNow);
}

void UpgradeDialog::acceptRestartLater()
{
    accept(DialogCode::RestartLater);
}
