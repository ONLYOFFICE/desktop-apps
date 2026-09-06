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
#include "utils.h"
#include "defines.h"
#include "clangater.h"
#include <QDir>
#include "caddonmanager.h"
#include "version.h"
#include <qtcomp/qdesktopwidget.h>

const QSize UpgradeDialog::IntroPageSize(480, 700);
const QSize UpgradeDialog::InstallPageSize(480, 304);

namespace
{
    QString getPlatformPackageName()
    {
#ifdef _WIN32
# if defined(_M_ARM64)
        return QStringLiteral("win_arm64");
# elif defined(_M_X64)
        return QStringLiteral("win_64");
# elif defined(_M_IX86)
        return QStringLiteral("win_32");
# else
#  error Unsupported architecture
# endif
#else
# if defined(__aarch64__)
        return QStringLiteral("linux_aarch64");
# elif defined(__x86_64__)
        return QStringLiteral("linux_x86_64");
# else
#  error Unsupported architecture
# endif
#endif
    }
}

UpgradeDialog::UpgradeDialog(QWidget *parent, const QRect &rc, int page)
    : CAscDialog(parent, rc)
    , m_bDialogActivated(false)
{
    setFocusPolicy(Qt::StrongFocus);
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

    GET_REGISTRY_USER(reg_user);
    QString html_path;
#if defined(QT_DEBUG)
    html_path = reg_user.value("addonpage").value<QString>();
#endif
    if ( html_path.isEmpty() )
        html_path = qApp->applicationDirPath() + QStringLiteral("/addon.html");

    QString url_additional = QString("?lang=%1&page=%2").arg(
        CLangater::getCurrentLangCode(), pageToString(page));

    if (page == Page::Install)
    {
        QString install_params = QString("&appcast=%1&version=%2&package=%3").arg(
            AscAppManager::addonManager()->appcastUrl(), VER_FILEVERSION_STR,
            getPlatformPackageName());
        url_additional.append(install_params);
    }

    const QString addonUrl = HTML_SCHEME + QString("%1%2").arg(html_path, url_additional);

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
    QCefView *contentCefView = qobject_cast<QCefView*>(m_pMainView);

    QJsonObject jsn;
    jsn["progress"] = QString::number(percent);
    AscAppManager::sendCommandTo(contentCefView, "addon:status", Utils::stringifyJson(jsn));
}

void UpgradeDialog::showStartPage()
{
    QCefView *contentCefView = qobject_cast<QCefView*>(m_pMainView);

    QJsonObject jsn;
    jsn["page"] = pageToString(Page::Install);
    jsn["appcast"] = AscAppManager::addonManager()->appcastUrl();
    jsn["version"] = VER_FILEVERSION_STR;
    jsn["package"] = getPlatformPackageName();

    AscAppManager::sendCommandTo(contentCefView, "addon:status", Utils::stringifyJson(jsn));

    applyGeometry(InstallPageSize);
}

void UpgradeDialog::showFinalPage()
{
    QCefView *contentCefView = qobject_cast<QCefView*>(m_pMainView);

    QJsonObject jsn;
    jsn["page"] = pageToString(Page::Complete);
    AscAppManager::sendCommandTo(contentCefView, "addon:status", Utils::stringifyJson(jsn));
}

void UpgradeDialog::showErrorPage(const QString &message)
{
    QCefView *contentCefView = qobject_cast<QCefView*>(m_pMainView);

    QJsonObject jsn;
    jsn["page"] = pageToString(Page::Error);
    jsn["message"] = message;
    AscAppManager::sendCommandTo(contentCefView, "addon:status", Utils::stringifyJson(jsn));
}

/** Protected **/

void UpgradeDialog::showEvent(QShowEvent *ev)
{
    CAscDialog::showEvent(ev);
    if ( !m_bDialogActivated )
    {
        m_bDialogActivated = true;

#ifdef _WIN32
        // HACK: CEF randomly steals window activation after loading.
        // Without this delay the dialog disappears behind other windows.
        QTimer::singleShot(100, this,
            [this]()
            {
                WindowHelper::bringToTop((HWND)winId());
            });
#endif
    }
}

/** Private **/

void UpgradeDialog::applyGeometry(const QSize &baseSize)
{
    // HACK: Avoid visible position change when updating the geometry of an already shown dialog.
    setWindowOpacity(0.0);

    const bool wasResizable = isResizable();
    if (!wasResizable)
        setResizable(true);

    QRect rc(QPoint(0, 0), baseSize * m_dpiRatio);
    QWidget *parent = parentWidget();
    if (parent)
        rc.moveCenter(parent->geometry().center());

    const QRect scrRc = QtComp::DesktopWidget::availableGeometry(parent ? parent : this);
    if (rc.bottom() > scrRc.bottom())
        rc.moveBottom(scrRc.bottom());
    if (rc.top() < scrRc.top())
        rc.moveTop(scrRc.top());
    if (rc.right() > scrRc.right())
        rc.moveRight(scrRc.right());
    if (rc.left() < scrRc.left())
        rc.moveLeft(scrRc.left());

    setGeometry(adjustWindowRect(rc, m_dpiRatio));

    if (!wasResizable)
        setResizable(false);

    // Restore visibility after applying the new dialog geometry.
    QTimer::singleShot(100, this,
        [this]()
        {
            setWindowOpacity(1.0);
        });
}

QString UpgradeDialog::pageToString(int page)
{
    switch (page)
    {
    case Page::Install:  return QStringLiteral("install");
    case Page::Complete: return QStringLiteral("complete");
    case Page::Error:    return QStringLiteral("error");
    case Page::Intro:
    default:             return QStringLiteral("intro");
    }
}
