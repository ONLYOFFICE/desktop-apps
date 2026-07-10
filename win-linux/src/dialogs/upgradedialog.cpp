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
#include "ui_upgradedialog.h"
#include "cascapplicationmanagerwrapper.h"
#include "clangater.h"
#include "utils.h"

namespace
{
    QSize minimumWindowSize(double dpi)
    {
        QSize minSize(334, 400);
        minSize *= dpi;
    #ifdef __linux__
        if ( WindowHelper::shouldUseThinFrame() ) {
            int d = 2 * SHADOW_WIDTH * dpi;
            minSize += QSize(d, d);
        }
    #endif
        return minSize;
    }
}

UpgradeDialog::UpgradeDialog(QWidget *parent, const QRect &rc)
    : CWindowPlatform(parent, rc)
    , ui(new Ui::UpgradeDialog)
    , m_loop(new QEventLoop)
    , m_result(DialogCode::Rejected)
{    
    setWindowFlags(windowFlags() | Qt::Dialog);
    ui->setupUi(this, m_dpiRatio);
    setMinimumSize(minimumWindowSize(m_dpiRatio));
    setMaximumSize(minimumWindowSize(m_dpiRatio));

    ui->labelTitle->setVisible(false);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);

    ui->centralwidget->setProperty("rtl", AscAppManager::isRtlEnabled());
    ui->centralwidget->setProperty("rtl-font", CLangater::isRtlLanguage(CLangater::getCurrentLangCode()));
    ui->centralwidget->setProperty("zoom", QString::number(m_dpiRatio) + "x");
    ui->centralwidget->setProperty("uitheme", QString::fromStdWString(GetCurrentTheme().id()));

    QString css = Utils::readStylesheets(":/styles/upgradedialog.qss");
    ui->centralwidget->setStyleSheet(css.arg(GetColorQValueByRole(ecrDownloadWidgetBackground),
                                             GetColorQValueByRole(ecrDownloadWidgetBorder),
                                             GetColorQValueByRole(ecrTextNormal),
                                             GetColorQValueByRole(ecrButtonHoverBackground),
                                             GetColorQValueByRole(ecrButtonPressedBackground),
                                             GetColorQValueByRole(ecrBorderControlFocus),
                                             GetColorQValueByRole(ecrTextPretty),
                                             GetColorQValueByRole(ecrDownloadProgressBarChunk),
                                             GetColorQValueByRole(ecrDownloadProgressBarBackground)));

    connect(ui->btnDeclineInstall, &QPushButton::clicked, this, &UpgradeDialog::reject);
    connect(ui->btnAcceptInstall, &QPushButton::clicked, this,
        [this]()
        {
            ui->labelTitle->setVisible(true);
            ui->stackedWidget->setCurrentIndex(1);
            emit installClicked();
        });
    connect(ui->btnRestartNow, &QPushButton::clicked, this, &UpgradeDialog::acceptRestartNow);
    connect(ui->btnRestartLater, &QPushButton::clicked, this, &UpgradeDialog::acceptRestartLater);
}

UpgradeDialog::~UpgradeDialog()
{
    delete m_loop;
    m_loop = nullptr;
}

/** Public **/

void UpgradeDialog::setModal()
{
    setWindowModality(Qt::ApplicationModal);
}

void UpgradeDialog::setProgress(int percent)
{
    ui->progressBar->setValue(percent);
}

void UpgradeDialog::showStartPage()
{
    ui->labelTitle->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->btnAcceptInstall->setFocus();
}

void UpgradeDialog::showFinalPage()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->btnRestartNow->setFocus();
}

int UpgradeDialog::exec()
{
    show(false);
    m_loop->exec();
    hide();
    return m_result;
}

void UpgradeDialog::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
}

/** Protected **/

bool UpgradeDialog::event(QEvent *ev)
{
    switch (ev->type())
    {
    case QEvent::Close:
    {
        if (m_loop->isRunning())
        {
            reject();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return CWindowPlatform::event(ev);
}

void UpgradeDialog::onLayoutDirectionChanged()
{}

/** Private **/

void UpgradeDialog::setScreenScalingFactor(double factor, bool resize)
{
    CWindowPlatform::setScreenScalingFactor(factor, resize);
    setMinimumSize(minimumWindowSize(m_dpiRatio));
    setMaximumSize(minimumWindowSize(m_dpiRatio));
}

void UpgradeDialog::reject()
{
    m_result = DialogCode::Rejected;
    m_loop->exit();
}

void UpgradeDialog::acceptRestartNow()
{
    m_result = DialogCode::RestartNow;
    m_loop->exit();
}

void UpgradeDialog::acceptRestartLater()
{
    m_result = DialogCode::RestartLater;
    m_loop->exit();
}
