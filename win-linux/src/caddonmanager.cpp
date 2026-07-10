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

#include "caddonmanager.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"
#include "Network/FileTransporter/include/FileTransporter.h"
#ifdef _WIN32
# include <tchar.h>
# include <wininet.h>
#endif

#include <QFile>
#include "../../../core/DesktopEditor/common/Directory.h"

using NSNetwork::NSFileTransport::CFileDownloader;


class CAddonManager::CAddonManagerPrivate : public QObject
{
    Q_OBJECT

    CFileDownloader *pDownloader = nullptr;
    QString sTmpPath;

public:
    CAddonManagerPrivate(CAddonManager *parent)
        : QObject(parent)
        , q_ptr(parent)
    {
    }

    ~CAddonManagerPrivate()
    {
        resetDownloader();
    }

    void install()
    {
        std::wstring path = NSDirectory::GetTempPath() + L"/onlyoffice_addon.zip";
        sTmpPath = QString::fromStdWString(path);

        resetDownloader();

        bInstalling = true;
        emit q_ptr->progressChanged(0);

        // CFileDownloader writes directly to the target path and reports through its
        // own worker thread, so the callbacks are marshaled back to the GUI thread.
#ifdef _WIN32
        DeleteUrlCacheEntryW(_T(URL_ADDON_ENTERPRISE));
#endif
        pDownloader = new CFileDownloader(QString(URL_ADDON_ENTERPRISE).toStdWString(), false);
        pDownloader->SetFilePath(sTmpPath.toStdWString());
        pDownloader->SetEvent_OnProgress(
            [this](int percent)
            {
                QMetaObject::invokeMethod(this, "onProgress", Qt::QueuedConnection, Q_ARG(int, percent));
            });
        pDownloader->SetEvent_OnComplete(
            [this](int error)
            {
                QMetaObject::invokeMethod(this, "onComplete", Qt::QueuedConnection, Q_ARG(int, error));
            });
        pDownloader->Start(0);
    }

    void uninstall()
    {
        AscAppManager::getInstance().RemoveMainAddon();

        // Like install, removal takes effect only after an application restart.
        bRestartRequired = true;

        emit q_ptr->uninstallFinished();
    }

    void stop()
    {
        resetDownloader();
    }

    void finishInstall(bool success)
    {
        bInstalling = false;

        if (success)
        {
            int res = AscAppManager::getInstance().InstallMainAddon(sTmpPath.toStdWString());
            success = (res == 0);
            bRestartRequired = success;
        }

        QFile::remove(sTmpPath);
        sTmpPath.clear();

        emit q_ptr->installFinished(success);
    }

    void resetDownloader()
    {
        if (pDownloader)
        {
            pDownloader->Stop();
            delete pDownloader;
            pDownloader = nullptr;
        }
    }

    void writeMarker()
    {
    }
    
    // Set after an install/remove in this session: the addon takes effect only
    // after an application restart (plugins are read at startup).
    bool  bRestartRequired = false;
    bool  bInstalling = false;
    
    CAddonManager *q_ptr;

private slots:
    // Invoked (queued) from the downloader's worker thread onto the GUI thread.
    void onProgress(int percent)
    {
        if (percent < 0)
            percent = 0;
        else if (percent > 100)
            percent = 100;

        emit q_ptr->progressChanged(percent);
    }

    void onComplete(int error)
    {
        // The downloader is not deleted here (it is still unwinding its worker thread);
        // it is torn down on the next install() call or in the destructor.
        finishInstall(error == 0);
    }
};


CAddonManager::CAddonManager(QObject *parent)
    : QObject(parent)
    , pimpl(new CAddonManagerPrivate(this))
{
}

CAddonManager::~CAddonManager()
{
    delete pimpl;
    pimpl = nullptr;
}

bool CAddonManager::isInstalled() const
{
	return CAscApplicationManagerWrapper::getInstance().CheckMainAddon();
}

bool CAddonManager::restartRequired() const
{
    return pimpl->bRestartRequired;
}

void CAddonManager::doAction(Action action)
{
    if (pimpl->bInstalling)
        return;

    if (action == Action::Install) {
        if (!isInstalled())
            pimpl->install();
    } else if (action == Action::Uninstall) {
        if (isInstalled())
            pimpl->uninstall();
    } else if (action == Action::Stop) {
        pimpl->stop();
    }
}

#include "caddonmanager.moc"
