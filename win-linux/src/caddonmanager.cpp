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
#include "version.h"
#include "utils.h"
#include "clogger.h"
#include "cascapplicationmanagerwrapper.h"
#include "Network/FileTransporter/include/FileTransporter.h"
#include <QFile>
#include <QJsonObject>
#ifdef _WIN32
# include <tchar.h>
# include <wininet.h>
#endif

#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES L""
#endif
#ifndef URL_APPCAST_DEV_CHANNEL
# define URL_APPCAST_DEV_CHANNEL L""
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
# define SKIP_EMPTY Qt::SkipEmptyParts
#else
# define SKIP_EMPTY QString::SkipEmptyParts
#endif

using NSNetwork::NSFileTransport::CFileDownloader;


class CAddonManager::CAddonManagerPrivate : public QObject
{
    Q_OBJECT

    enum DownloadMode
    {
        DownloadMetadata,
        DownloadPackage
    };

    struct PackageData
    {
        QString hash;
        QString version;

        void clear()
        {
            hash.clear();
            version.clear();
        }

    } packageData;

    CFileDownloader *pDownloader = nullptr;
    QString sTmpPath;
    std::wstring sMetadataUrl;
    int nDownloadMode = DownloadMetadata;

public:
    CAddonManagerPrivate(CAddonManager *parent)
        : QObject(parent)
        , q_ptr(parent)
    {
        sMetadataUrl = InputArgs::contains(L"--appcast-dev-channel")
                           ? URL_APPCAST_DEV_CHANNEL
                           : URL_APPCAST_UPDATES;
    }

    ~CAddonManagerPrivate()
    {
        resetDownloader();
    }

    void install()
    {
        nDownloadMode = DownloadMetadata;
        packageData.clear();
        sTmpPath = Utils::generateTmpFileName(".json");

        bInstalling = true;
        emit q_ptr->progressChanged(0);

        startDownload(sMetadataUrl, sTmpPath.toStdWString());
    }

    void uninstall()
    {
        AscAppManager::getInstance().RemoveMainAddon();

        // Like install, removal takes effect only after an application restart.
        bRestartRequired = true;

        emit q_ptr->uninstallFinished();
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

    void startDownload(const std::wstring& url, const std::wstring& filePath)
    {
        nLastProgress = -1;
        resetDownloader();
#ifdef _WIN32
        // Clear cache to force downloading the latest version.
        DeleteUrlCacheEntryW(url.c_str());
#endif
        // CFileDownloader writes directly to the target path and reports through its
        // own worker thread, so the callbacks are marshaled back to the GUI thread.
        pDownloader = new CFileDownloader(url, false);
        pDownloader->SetFilePath(filePath);

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
    int   nLastProgress = -1;
    
    CAddonManager *q_ptr;

private slots:
    // Invoked (queued) from the downloader's worker thread onto the GUI thread.
    void onProgress(int percent)
    {
        if (nDownloadMode == DownloadMetadata)
            return;

        if (percent < 0)
            percent = 0;
        else if (percent > 100)
            percent = 100;

        if (percent > nLastProgress)
        {
            nLastProgress = percent;
            emit q_ptr->progressChanged(percent);
        }
    }

    void onComplete(int error)
    {
        // The downloader is not deleted here (it is still unwinding its worker thread);
        // it is torn down on the next install() call or in the destructor.
        if (error == 0)
        {
            switch (nDownloadMode)
            {
            case DownloadMetadata:
            {
                onLoadMetadataFinished();
                break;
            }
            case DownloadPackage:
            {
                if (!sTmpPath.isEmpty() && Utils::getFileHash(sTmpPath) == packageData.hash)
                {
                    finishInstall(true);
                }
                else
                {
                    finishInstall(false);
                }
                break;
            }
            default:
                break;
            }
        }
        else
        {
            finishInstall(false);
        }
    }

private:
    bool isNewerVersion(const QString &currentVer, const QString &targetVer)
    {
        const QStringList oldVer = currentVer.split('.', SKIP_EMPTY);
        const QStringList newVer = targetVer.split('.', SKIP_EMPTY);
        int size = std::max(newVer.size(), oldVer.size());

        for (int i = 0; i < size; i++)
        {
            const int oldPart = i < oldVer.size() ? oldVer.at(i).toInt() : 0;
            const int newPart = i < newVer.size() ? newVer.at(i).toInt() : 0;

            if (newPart > oldPart)
                return true;

            if (newPart < oldPart)
                return false;
        }

        return false;
    }

    void onLoadMetadataFinished()
    {
        const QJsonObject root = Utils::parseJsonFile(sTmpPath);
        if (!root.isEmpty())
        {
            QJsonObject addon = root.value("addon").toObject();
            if (addon.isEmpty())
                goto installFailed;

            QString currVersion = QString::fromLatin1(VER_FILEVERSION_STR);
            QString version = addon.value("version").toString();
            if (version.isEmpty())
                goto installFailed;

            QJsonObject package = addon.value("package").toObject();
            if (package.isEmpty())
                goto installFailed;
#ifdef _WIN32
# if defined(_M_ARM64)
            QJsonObject platform = package.value("win_arm64").toObject();
# elif defined(_M_X64)
            QJsonObject platform = package.value("win_64").toObject();
# elif defined(_M_IX86)
            QJsonObject platform = package.value("win_32").toObject();
# else
#  error Unsupported architecture
# endif
#else
# if defined(__aarch64__)
            QJsonObject platform = package.value("linux_aarch64").toObject();
# elif defined(__x86_64__)
            QJsonObject platform = package.value("linux_x86_64").toObject();
# else
#  error Unsupported architecture
# endif
#endif
            if (platform.isEmpty())
                goto installFailed;

            if (isNewerVersion(currVersion, version))
            {
                // TODO: show a message about the need to update the application
            }
            else if (isNewerVersion(version, currVersion))
            {
                // TODE: show message about missing update
            }
            else
            {
                QString url = platform.value("url").toString();
                QString url2 = platform.value("url2").toString();
                if (url.isEmpty() && url2.isEmpty())
                    goto installFailed;

                CLogger::log("Primary package URL: " + url + "\nSecondary package URL: " + url2);
                bool primary_bad = (url.isEmpty() /*|| !m_pDownloader->isUrlAccessible(url)*/);
                bool prefer_secondary = InputArgs::contains(L"--prefer-alternate-url");
                QString packageUrl = (!url2.isEmpty() && (primary_bad || prefer_secondary)) ? url2 : url;

                QString hash = platform.value("md5").toString();

                packageData.version = version;
                packageData.hash = hash.toLower();

                nDownloadMode = DownloadPackage;
                sTmpPath = Utils::generateTmpFileName(".zip");
                startDownload(packageUrl.toStdWString(), sTmpPath.toStdWString());
            }

            return;
        }

installFailed:
        finishInstall(false);
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
        pimpl->resetDownloader();
    }
}

#include "caddonmanager.moc"
