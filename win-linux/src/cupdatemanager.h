/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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

#ifndef CUPDATEMANAGER_H
#define CUPDATEMANAGER_H

#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDebug>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>
#include "utils.h"
#include "defines.h"
#include "version.h"
#include "Network/FileTransporter/include/FileTransporter.h"

using NSNetwork::NSFileTransport::CFileDownloader;
using std::vector;
using std::wstring;

class CUpdateManager: public QObject
{
    Q_OBJECT

public:

    explicit CUpdateManager(QObject *parent = nullptr);
    ~CUpdateManager();

    void setNewUpdateSetting(const QString& _rate);
    QStringList getInstallArguments() const;
    QString getInstallPackagePath() const;
    void scheduleRestartForUpdate();
    void handleAppClose();
    void cancelLoading();
#ifdef Q_OS_WIN
    void loadUpdates();
    void getInstallParams();
    QString getVersion() const;
#endif

private:
    void init();
    void updateNeededCheking();
    //void loadChangelog(const wstring &changelog_url);
    void onLoadCheckFinished();
    //void onLoadChangelogFinished();
    void onComplete(const int error);
    void onProgress(const int percent);
    void downloadFile(const wstring &url, const QString &ext);
    QByteArray getFileHash(const QString &fileName);

#if defined (Q_OS_WIN)
    void onLoadUpdateFinished();

    wstring     m_packageUrl,
                m_packageArgs;
#endif

    wstring     m_checkUrl;

    int         m_currentRate,
                m_downloadMode;

    time_t      m_lastCheck;
    QString     m_newVersion,
                m_savedVersion,
                m_packageFileName,
                m_savedPackageFileName;
    QTimer      *m_pTimer;
    CFileDownloader  *m_pDownloader;
    bool        m_restartForUpdate;
    QByteArray  m_newHash,
                m_savedHash;

    enum Mode {
        CHECK_UPDATES=0, DOWNLOAD_CHANGELOG=1, DOWNLOAD_UPDATES=2
    };

    enum UpdateInterval {
        NEVER=0, DAY=1, WEEK=2
    };

public slots:
    void checkUpdates();
#ifdef Q_OS_WIN

#endif

       signals:
    void checkFinished(const bool error, const bool updateExist,
                       const QString &version, const QString &changelog);
    void progresChanged(const int percent);
    void updateLoaded();

private slots:
    void onCompleteSlot(const int error);
    void onProgressSlot(const int percent);
};


#endif // CUPDATEMANAGER_H
