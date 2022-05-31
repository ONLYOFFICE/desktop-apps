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

#include <QObject>
#include <QTimer>
#include <ctime>
#include "Network/FileTransporter/include/FileTransporter.h"

using NSNetwork::NSFileTransport::CFileDownloader;
using std::wstring;



class CUpdateManager: public QObject
{
    Q_OBJECT
public:
    explicit CUpdateManager(QObject *parent = nullptr);
    ~CUpdateManager();

    void setNewUpdateSetting(const QString& _rate);
    void cancelLoading();
#ifdef Q_OS_WIN
    QStringList getInstallArguments() const;
    QString getInstallPackagePath() const;
    QString getVersion() const;
    void scheduleRestartForUpdate();
    void handleAppClose();
    void loadUpdates();
#endif

private:
    void init();
    void clearTempFiles(const QString &except = QString());
    void updateNeededCheking();
    void onLoadCheckFinished();
    void onComplete(const int error);
    void onProgress(const int percent);
    void downloadFile(const wstring &url, const QString &ext);
#ifdef Q_OS_WIN
    void onLoadUpdateFinished();
    void savePackageData(const QByteArray &hash = QByteArray(),
                         const QString &version = QString(),
                         const QString &fileName = QString());
    QByteArray getFileHash(const QString &fileName);

    struct PackageData {
        QString     fileName = "";
        wstring     packageUrl = L"",
                    packageArgs = L"";
    };
    struct SavedPackageData {
        QByteArray hash = QByteArray();
        QString    version = "",
                   fileName = "";
    };
    PackageData      m_packageData;
    SavedPackageData m_savedPackageData;

    bool        m_restartForUpdate = false;
#else
    QTimer      *m_pTimer;
    time_t      m_lastCheck;
    int         m_currentRate;

    enum UpdateInterval {
        NEVER=0, DAY=1, WEEK=2
    };
#endif
    wstring     m_checkUrl;
    int         m_downloadMode;
    QString     m_newVersion;
    CFileDownloader  * m_pDownloader = nullptr;

    enum Mode {
        CHECK_UPDATES=0, DOWNLOAD_CHANGELOG=1, DOWNLOAD_UPDATES=2
    };

public slots:
    void checkUpdates();

signals:
    void checkFinished(const bool error, const bool updateExist, const QString &version, const QString &changelog);
#ifdef Q_OS_WIN
    void progresChanged(const int percent);
    void updateLoaded();
#endif

private slots:
    void onCompleteSlot(const int error);
#ifdef Q_OS_WIN
    void onProgressSlot(const int percent);
#endif
};


#endif // CUPDATEMANAGER_H
