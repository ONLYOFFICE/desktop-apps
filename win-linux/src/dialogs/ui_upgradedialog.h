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

#ifndef UI_UPGRADEDIALOG_H
#define UI_UPGRADEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "defines.h"

#ifdef _WIN32
# include "windows/platform_win/cwindowplatform.h"
#else
# include "windows/platform_linux/cwindowplatform.h"
#endif

QT_BEGIN_NAMESPACE

class Ui_UpgradeDialog
{
public:
    QWidget *centralwidget;
    QVBoxLayout *centralVLayout;
    QLabel *labelTitle;
    QLabel *labelIcon;
    QStackedWidget *stackedWidget;

    QVBoxLayout *page1VLayout;
    QLabel *labelDescription;
    QLabel *labelLicenseUrl;
    QPushButton *btnDeclineInstall;
    QSpacerItem *page1SpacerHCenter;
    QPushButton *btnAcceptInstall;

    QVBoxLayout *page2VLayout;
    QProgressBar *progressBar;
    QSpacerItem *page2SpacerVTop;
    QLabel *labelInstalling;

    QVBoxLayout *page3VLayout;
    QLabel *labelStatus;
    QLabel *labelStatusInfo;
    QSpacerItem *page3SpacerVBottom;
    QPushButton *btnRestartLater;
    QSpacerItem *page3SpacerHCenter;
    QPushButton *btnRestartNow;

    QVBoxLayout *page4VLayout;
    QLabel *labelError;
    QLabel *labelErrorInfo;
    QSpacerItem *page4SpacerVBottom;
    QPushButton *btnCancel;
    QSpacerItem *page4SpacerHCenter;
    QPushButton *btnTryAgain;

    void setupUi(QMainWindow *upgradeDialog, double dpi)
    {
        upgradeDialog->setObjectName(QString::fromUtf8("UpgradeDialog"));        
        centralwidget = new AscMainPanel(upgradeDialog);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralVLayout = new QVBoxLayout(centralwidget);
        centralVLayout->setSpacing(9 * dpi);
        centralVLayout->setContentsMargins(18 * dpi, 14 * dpi, 18 * dpi, 16 * dpi);

        labelTitle = new QLabel(centralwidget);
        labelTitle->setObjectName(QString::fromUtf8("labelTitle"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelTitle->sizePolicy().hasHeightForWidth());
        labelTitle->setSizePolicy(sizePolicy);
        labelTitle->setAlignment(Qt::AlignCenter);
        labelTitle->setWordWrap(true);
        centralVLayout->addWidget(labelTitle);

        labelIcon = new QLabel(centralwidget);
        labelIcon->setObjectName(QString::fromUtf8("labelIcon"));
        sizePolicy.setHeightForWidth(labelIcon->sizePolicy().hasHeightForWidth());
        labelIcon->setSizePolicy(sizePolicy);
        labelIcon->setMinimumSize(QSize(0, 60 * dpi));
        QIcon icon(QString::fromUtf8(":/addon/icons/pro-tools.svg"));
        labelIcon->setPixmap(icon.pixmap(60 * dpi,60 * dpi));
        labelIcon->setAlignment(Qt::AlignCenter);
        centralVLayout->addWidget(labelIcon);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));

        /** Page 1 **/
        QWidget *page1 = new QWidget();
        page1->setObjectName(QString::fromUtf8("page1"));
        page1VLayout = new QVBoxLayout(page1);
        page1VLayout->setSpacing(12 * dpi);

        labelDescription = new QLabel(page1);
        labelDescription->setObjectName(QString::fromUtf8("labelDescription"));
        labelDescription->setTextFormat(Qt::RichText);
        labelDescription->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        labelDescription->setWordWrap(true);
        page1VLayout->addWidget(labelDescription);

        QSpacerItem *page1SpacerVCenter = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
        page1VLayout->addItem(page1SpacerVCenter);

        labelLicenseUrl = new QLabel(page1);
        labelLicenseUrl->setObjectName(QString::fromUtf8("labelLicenseUrl"));
        sizePolicy.setHeightForWidth(labelLicenseUrl->sizePolicy().hasHeightForWidth());
        labelLicenseUrl->setSizePolicy(sizePolicy);
        labelLicenseUrl->setTextFormat(Qt::RichText);
        labelLicenseUrl->setScaledContents(false);
        labelLicenseUrl->setAlignment(Qt::AlignCenter);
        labelLicenseUrl->setWordWrap(true);
        labelLicenseUrl->setOpenExternalLinks(true);
        page1VLayout->addWidget(labelLicenseUrl);

        QHBoxLayout *page1HLayout = new QHBoxLayout();
        QSpacerItem *page1SpacerHLeft = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page1HLayout->addItem(page1SpacerHLeft);

        btnDeclineInstall = new QPushButton(page1);
        btnDeclineInstall->setObjectName(QString::fromUtf8("btnDeclineInstall"));
        btnDeclineInstall->setFixedHeight(28 * dpi);
        page1HLayout->addWidget(btnDeclineInstall);

        page1SpacerHCenter = new QSpacerItem(6 * dpi, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
        page1HLayout->addItem(page1SpacerHCenter);

        btnAcceptInstall = new QPushButton(page1);
        btnAcceptInstall->setObjectName(QString::fromUtf8("btnAcceptInstall"));
        btnAcceptInstall->setFixedHeight(28 * dpi);
        btnAcceptInstall->setDefault(true);
        btnAcceptInstall->setAutoDefault(true);
        page1HLayout->addWidget(btnAcceptInstall);

        QSpacerItem *page1SpacerHRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page1HLayout->addItem(page1SpacerHRight);

        page1VLayout->addLayout(page1HLayout);
        stackedWidget->addWidget(page1);

        /** Page 2 **/
        QWidget *page2 = new QWidget();
        page2->setObjectName(QString::fromUtf8("page2"));
        page2VLayout = new QVBoxLayout(page2);
        page2VLayout->setSpacing(12 * dpi);

        progressBar = new QProgressBar(page2);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        progressBar->setFixedHeight(4 * dpi);
        progressBar->setTextVisible(false);
        page2VLayout->addWidget(progressBar);

        page2SpacerVTop = new QSpacerItem(20, 10 * dpi, QSizePolicy::Minimum, QSizePolicy::Fixed);
        page2VLayout->addItem(page2SpacerVTop);

        labelInstalling = new QLabel(page2);
        labelInstalling->setObjectName(QString::fromUtf8("labelInstalling"));
        labelInstalling->setAlignment(Qt::AlignCenter);
        page2VLayout->addWidget(labelInstalling);

        QSpacerItem *page2SpacerVBottom = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        page2VLayout->addItem(page2SpacerVBottom);

        stackedWidget->addWidget(page2);

        /** Page 3 **/
        QWidget *page3 = new QWidget();
        page3->setObjectName(QString::fromUtf8("page3"));
        page3VLayout = new QVBoxLayout(page3);
        page3VLayout->setSpacing(12 * dpi);

        labelStatus = new QLabel(page3);
        labelStatus->setObjectName(QString::fromUtf8("labelStatus"));
        labelStatus->setAlignment(Qt::AlignCenter);
        page3VLayout->addWidget(labelStatus);

        QSpacerItem *page3SpacerVTop = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        page3VLayout->addItem(page3SpacerVTop);

        labelStatusInfo = new QLabel(page3);
        labelStatusInfo->setObjectName(QString::fromUtf8("labelStatusInfo"));
        labelStatusInfo->setAlignment(Qt::AlignCenter);
        page3VLayout->addWidget(labelStatusInfo);

        page3SpacerVBottom = new QSpacerItem(20, 6 * dpi, QSizePolicy::Minimum, QSizePolicy::Fixed);
        page3VLayout->addItem(page3SpacerVBottom);

        QHBoxLayout *page3HLayout = new QHBoxLayout();
        QSpacerItem *page3SpacerHLeft = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page3HLayout->addItem(page3SpacerHLeft);

        btnRestartLater = new QPushButton(page3);
        btnRestartLater->setObjectName(QString::fromUtf8("btnRestartLater"));
        btnRestartLater->setFixedHeight(28 * dpi);
        page3HLayout->addWidget(btnRestartLater);

        page3SpacerHCenter = new QSpacerItem(6 * dpi, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
        page3HLayout->addItem(page3SpacerHCenter);

        btnRestartNow = new QPushButton(page3);
        btnRestartNow->setObjectName(QString::fromUtf8("btnRestartNow"));
        btnRestartNow->setFixedHeight(28 * dpi);
        btnRestartNow->setDefault(true);
        btnRestartNow->setAutoDefault(true);
        page3HLayout->addWidget(btnRestartNow);

        QSpacerItem *page3SpacerHRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page3HLayout->addItem(page3SpacerHRight);

        page3VLayout->addLayout(page3HLayout);
        stackedWidget->addWidget(page3);

        /** Page 4 **/
        QWidget *page4 = new QWidget();
        page4->setObjectName(QString::fromUtf8("page4"));
        page4VLayout = new QVBoxLayout(page4);
        page4VLayout->setSpacing(12 * dpi);

        labelError = new QLabel(page4);
        labelError->setObjectName(QString::fromUtf8("labelError"));
        labelError->setAlignment(Qt::AlignCenter);
        page4VLayout->addWidget(labelError);

        QSpacerItem *page4SpacerVTop = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        page4VLayout->addItem(page4SpacerVTop);

        labelErrorInfo = new QLabel(page4);
        labelErrorInfo->setObjectName(QString::fromUtf8("labelErrorInfo"));
        labelErrorInfo->setAlignment(Qt::AlignCenter);
        labelErrorInfo->setWordWrap(true);
        page4VLayout->addWidget(labelErrorInfo);

        page4SpacerVBottom = new QSpacerItem(20, 6 * dpi, QSizePolicy::Minimum, QSizePolicy::Fixed);
        page4VLayout->addItem(page4SpacerVBottom);

        QHBoxLayout *page4HLayout = new QHBoxLayout();
        QSpacerItem *page4SpacerHLeft = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page4HLayout->addItem(page4SpacerHLeft);

        btnCancel = new QPushButton(page4);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setFixedHeight(28 * dpi);
        page4HLayout->addWidget(btnCancel);

        page4SpacerHCenter = new QSpacerItem(6 * dpi, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
        page4HLayout->addItem(page4SpacerHCenter);

        btnTryAgain = new QPushButton(page4);
        btnTryAgain->setObjectName(QString::fromUtf8("btnTryAgain"));
        btnTryAgain->setFixedHeight(28 * dpi);
        btnTryAgain->setDefault(true);
        btnTryAgain->setAutoDefault(true);
        page4HLayout->addWidget(btnTryAgain);

        QSpacerItem *page4SpacerHRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        page4HLayout->addItem(page4SpacerHRight);

        page4VLayout->addLayout(page4HLayout);
        stackedWidget->addWidget(page4);

        /** Central widget **/
        centralVLayout->addWidget(stackedWidget);
        upgradeDialog->setCentralWidget(centralwidget);

        retranslateUi(upgradeDialog);

        stackedWidget->setCurrentIndex(0);

        QMetaObject::connectSlotsByName(upgradeDialog);
    } // setupUi

    void retranslateUi(QMainWindow *upgradeDialog)
    {
        upgradeDialog->setWindowTitle(QString::fromUtf8("Upgrade Dialog"));
        labelTitle->setText(QCoreApplication::translate("UpgradeDialog",
            "ONLYOFFICE Pro tools", nullptr));
        labelIcon->setText(QString());

        // Description
        QString upgradeTitle = QCoreApplication::translate(
            "UpgradeDialog",
            "Get ONLYOFFICE Pro tools for next-level productivity:",
            nullptr);

        QStringList features =
            {
                QCoreApplication::translate("UpgradeDialog",
                    "AI in documents, spreadsheets, presentations, and PDFs", nullptr),
                QCoreApplication::translate("UpgradeDialog",
                    "Smart form creation &amp; auto-filled fields", nullptr),
                QCoreApplication::translate("UpgradeDialog",
                    "Custom AI providers + web search + MCP support", nullptr),
                QCoreApplication::translate("UpgradeDialog",
                    "Advanced PDF editing with table recognition", nullptr)
            };

        QString featuresHtml;
        for (const QString& feature : features)
        {
            featuresHtml += QString(
                "<p><img src=\":/addon/icons/check.svg\" width=\"14\" height=\"14\"/> %1</p>")
                    .arg(feature);
        }

        QString descriptionHtml = QString(
            "<html><head/><body>"
            "<div style=\"font-size:120%; font-weight:600; margin-bottom:2em;\">%1</div>"
            "<div>%2</div>"
            "</body></html>")
                .arg(upgradeTitle, featuresHtml);

        labelDescription->setText(descriptionHtml);

        // License Url
        QString licenseNotice = QCoreApplication::translate("UpgradeDialog",
            "By continuing, you understand and agree to our", nullptr);
        QString licenseAgreement = QCoreApplication::translate("UpgradeDialog",
            "License Agreement", nullptr);
        QString licenseNoticeHtml = QString(
            "<html><head/><body><p>%1 "
            "<a href=\"%2\">"
            "<span style=\"text-decoration: underline; color:#0000ff;\">%3</span>"
            "</a>"
            "</p></body></html>").arg(licenseNotice, URL_BSL_LICENSE, licenseAgreement);
        labelLicenseUrl->setText(licenseNoticeHtml);

        btnDeclineInstall->setText(QCoreApplication::translate("UpgradeDialog",
            "May be later", nullptr));
        btnAcceptInstall->setText(QCoreApplication::translate("UpgradeDialog",
            "Accept && Install", nullptr));
        labelInstalling->setText(QCoreApplication::translate("UpgradeDialog",
            "Installing...", nullptr));
        labelStatus->setText(QCoreApplication::translate("UpgradeDialog",
            "Successfully installed", nullptr));
        labelStatusInfo->setText(QCoreApplication::translate("UpgradeDialog",
            "Application need to restart", nullptr));
        btnRestartLater->setText(QCoreApplication::translate("UpgradeDialog",
            "I'll do it later", nullptr));
        btnRestartNow->setText(QCoreApplication::translate("UpgradeDialog",
            "Restart now", nullptr));
        labelError->setText(QCoreApplication::translate("UpgradeDialog",
            "Installation failed", nullptr));
        btnCancel->setText(QCoreApplication::translate("UpgradeDialog",
            "Cancel", nullptr));
        btnTryAgain->setText(QCoreApplication::translate("UpgradeDialog",
            "Try again", nullptr));
    } // retranslateUi

private:

};

namespace Ui {
    class UpgradeDialog: public Ui_UpgradeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UPGRADEDIALOG_H
