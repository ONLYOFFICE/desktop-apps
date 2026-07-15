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
#include "cdownloader.h"
#include <QFile>
#include <QJsonObject>
#ifdef _WIN32
# include <tchar.h>
# define QStrToTStr(str) str.toStdWString()
#else
# define _T(str) str
# define QStrToTStr(str) str.toStdString()
#endif

#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES ""
#endif
#ifndef URL_APPCAST_DEV_CHANNEL
# define URL_APPCAST_DEV_CHANNEL ""
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
# define SKIP_EMPTY Qt::SkipEmptyParts
#else
# define SKIP_EMPTY QString::SkipEmptyParts
#endif


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

    CDownloader *pDownloader = nullptr;
    QString sTmpPath;
    tstring sMetadataUrl;
    int nDownloadMode = DownloadMetadata;

public:
    CAddonManagerPrivate(CAddonManager *parent)
        : QObject(parent)
        , q_ptr(parent)
    {
        sMetadataUrl = InputArgs::contains(L"--appcast-dev-channel")
                           ? _T(URL_APPCAST_DEV_CHANNEL)
                           : _T(URL_APPCAST_UPDATES);
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

        startDownload(sMetadataUrl, QStrToTStr(sTmpPath));
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

    void startDownload(const tstring& url, const tstring& filePath)
    {
        resetDownloader();

        // CDownloader writes directly to the target path and reports through its
        // own worker thread, so the callbacks are marshaled back to the GUI thread.
        pDownloader = new CDownloader;

        pDownloader->onProgress(
            [this](int percent)
            {
                QMetaObject::invokeMethod(this, "onProgress", Qt::QueuedConnection, Q_ARG(int, percent));
            });

        pDownloader->onComplete(
            [this](int error, const tstring&)
            {
                QMetaObject::invokeMethod(this, "onComplete", Qt::QueuedConnection, Q_ARG(int, error));
            });

        pDownloader->downloadFile(url, filePath);
    }

    void resetDownloader()
    {
        if (pDownloader)
        {
            pDownloader->stop();
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
        if (nDownloadMode == DownloadMetadata)
            return;

        emit q_ptr->progressChanged(percent);
    }

    void onComplete(int error)
    {
        // The downloader is not deleted here (it is still unwinding its worker thread);
        // it is torn down on the next install() call or in the destructor.
        if (error == CDownloader::Success)
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
                goto metadataFailed;

            QString currVersion = QString::fromLatin1(VER_FILEVERSION_STR);
            QString version = addon.value("version").toString();
            if (version.isEmpty())
                goto metadataFailed;

            QJsonObject package = addon.value("package").toObject();
            if (package.isEmpty())
                goto metadataFailed;
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
                goto metadataFailed;

            if (isNewerVersion(currVersion, version))
            {
                bInstalling = false;
                emit q_ptr->compatibilityFailed(
                    tr("To install the add-on, update the application to the latest version."));
            }
            else if (isNewerVersion(version, currVersion))
            {
                bInstalling = false;
                emit q_ptr->compatibilityFailed(
                    tr("No compatible add-on version is available for this version of the application."));
            }
            else
            {
                QString url = platform.value("url").toString();
                QString url2 = platform.value("url2").toString();
                if (url.isEmpty() && url2.isEmpty())
                    goto metadataFailed;

                CLogger::log("Primary package URL: " + url + "\nSecondary package URL: " + url2);
                bool primary_bad = (url.isEmpty() || !pDownloader->isUrlAccessible(QStrToTStr(url)));
                bool prefer_secondary = InputArgs::contains(L"--prefer-alternate-url");
                QString packageUrl = (!url2.isEmpty() && (primary_bad || prefer_secondary)) ? url2 : url;
                CLogger::log("Download from URL:\n" + url);

                QString hash = platform.value("md5").toString();

                packageData.version = version;
                packageData.hash = hash.toLower();

                QFile::remove(sTmpPath);
                nDownloadMode = DownloadPackage;
                sTmpPath = Utils::generateTmpFileName(".zip");
                startDownload(QStrToTStr(packageUrl), QStrToTStr(sTmpPath));
            }

            return;
        }

metadataFailed:
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
