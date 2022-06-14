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

#include "cupdatemanager.h"
#include <QSettings>
#include <QDir>
#include <QDirIterator>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>
#include "utils.h"
#include "defines.h"
#include "version.h"
#include "clangater.h"
#ifdef Q_OS_WIN
# include <QProcess>
# include <QCryptographicHash>
#endif

#define FILE_PREFIX QString("onlyoffice_")
#define CMD_ARGUMENT_CHECK_URL L"--updates-appcast-url"
#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES L""
#endif

using std::vector;


CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_downloadMode(Mode::CHECK_UPDATES)
{
    // =========== Set updates URL ============
    if ( InputArgs::contains(CMD_ARGUMENT_CHECK_URL) ) {
        m_checkUrl = InputArgs::argument_value(CMD_ARGUMENT_CHECK_URL);
    } else m_checkUrl = QString(URL_APPCAST_UPDATES).toStdWString();

    if ( !m_checkUrl.empty() ) {
        m_pDownloader = new CFileDownloader(m_checkUrl, false);
        m_pDownloader->SetEvent_OnComplete(std::bind(&CUpdateManager::onComplete, this, std::placeholders::_1));

#ifdef Q_OS_WIN
        m_pDownloader->SetEvent_OnProgress(std::bind(&CUpdateManager::onProgress, this, std::placeholders::_1));
#else
        m_pTimer = new QTimer(this);
        m_pTimer->setSingleShot(false);
        connect(m_pTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
#endif
        init();
    }
}

CUpdateManager::~CUpdateManager()
{
    if ( m_pDownloader )
        delete m_pDownloader, m_pDownloader = nullptr;
}

void CUpdateManager::onComplete(const int error)
{
    QMetaObject::invokeMethod(this, "onCompleteSlot", Qt::QueuedConnection, Q_ARG(int, error));
}

void CUpdateManager::onCompleteSlot(const int error)
{
    if (error == 0) {
        //qDebug() << "Download complete... Mode: " << m_downloadMode;
        switch (m_downloadMode) {
        case Mode::CHECK_UPDATES:
            onLoadCheckFinished();
            break;
#ifdef Q_OS_WIN
        case Mode::DOWNLOAD_UPDATES:
            onLoadUpdateFinished();
            break;
#endif
        default: break;
        }
    }
    else {
        //qDebug() << "Download error: " << error;
    }
}

void CUpdateManager::init()
{
    GET_REGISTRY_USER(reg_user);
#ifdef Q_OS_WIN
    reg_user.beginGroup("Updates");
    m_savedPackageData.fileName = reg_user.value("Updates/file", QString()).toString();
    m_savedPackageData.hash = reg_user.value("Updates/hash", QByteArray()).toByteArray();
    m_savedPackageData.version = reg_user.value("Updates/version", QString()).toString();
    reg_user.endGroup();
#else
    reg_user.beginGroup("Updates");
    m_lastCheck = time_t(reg_user.value("Updates/last_check", 0).toLongLong());
    reg_user.endGroup();
    const QString interval = reg_user.value("checkUpdatesInterval","day").toString();
    m_currentRate = (interval == "disabled") ? UpdateInterval::NEVER : (interval == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#endif

    QTimer::singleShot(6000, this, [=]() {
        updateNeededCheking();
    });
}

void CUpdateManager::downloadFile(const std::wstring &url, const QString &ext)
{
    if (m_pDownloader) {
        m_pDownloader->Stop();
        m_pDownloader->SetFileUrl(url, false);
        const QUuid uuid = QUuid::createUuid();
        const QRegularExpression branches = QRegularExpression("[{|}]+");
        const QString tmp_file = QDir::tempPath() + "/" + QString(FILE_PREFIX) +
                uuid.toString().replace(branches, "") + ext;
        m_pDownloader->SetFilePath(tmp_file.toStdWString());
        m_pDownloader->Start(0);
    }
}

void CUpdateManager::clearTempFiles(const QString &except)
{
    static bool lock = false;
    if (!lock) { // for one-time cleaning
        lock = true;
        QStringList filter{"*.json", "*.exe"};
        QDirIterator it(QDir::tempPath(), filter, QDir::Files | QDir::NoSymLinks |
                        QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString tmp = it.next();
            if (tmp.toLower().indexOf(FILE_PREFIX) != -1 && tmp != except)
                QDir().remove(tmp);
        }
    }
#ifdef _WIN32
    if (except.isEmpty())
        savePackageData();
#endif
}

void CUpdateManager::checkUpdates()
{
    m_newVersion = "";
#ifdef Q_OS_WIN
    m_packageData.packageUrl = L"";
    m_packageData.packageArgs = L"";
    m_packageData.fileName = "";
#else
    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(m_lastCheck));
    reg_user.endGroup();
#endif

    m_downloadMode = Mode::CHECK_UPDATES;
    downloadFile(m_checkUrl, ".json");
#ifndef Q_OS_WIN
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
#endif
}

void CUpdateManager::updateNeededCheking()
{
#ifdef Q_OS_WIN
    checkUpdates();
#else
    m_pTimer->stop();
    int interval = 0;
    const time_t DAY_TO_SEC = 24*3600;
    const time_t WEEK_TO_SEC = 7*24*3600;
    const time_t curr_time = time(nullptr);
    const time_t elapsed_time = curr_time - m_lastCheck;
    switch (m_currentRate) {
    case UpdateInterval::DAY:
        if (elapsed_time > DAY_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(DAY_TO_SEC - elapsed_time);
            m_pTimer->setInterval(interval*1000);
            m_pTimer->start();
        }
        break;
    case UpdateInterval::WEEK:
        if (elapsed_time > WEEK_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(WEEK_TO_SEC - elapsed_time);
            m_pTimer->setInterval(interval*1000);
            m_pTimer->start();
        }
        break;
    case UpdateInterval::NEVER:
    default:
        break;
    }
#endif
}

#ifdef Q_OS_WIN
void CUpdateManager::onProgress(const int percent)
{
    QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, percent));
}

void CUpdateManager::onProgressSlot(const int percent)
{
    if (m_downloadMode == Mode::DOWNLOAD_UPDATES)
        emit progresChanged(percent);
}

void CUpdateManager::savePackageData(const QByteArray &hash, const QString &version, const QString &fileName)
{
    m_savedPackageData.fileName = fileName;
    m_savedPackageData.hash = hash;
    m_savedPackageData.version = version;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/file", fileName);
    reg_user.setValue("Updates/hash", hash);
    reg_user.setValue("Updates/version", version);
    reg_user.endGroup();
}

QByteArray CUpdateManager::getFileHash(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            file.close();
            return hash.result();
        }
        file.close();
    }
    return QByteArray();
}

void CUpdateManager::loadUpdates()
{
    if (!m_savedPackageData.fileName.isEmpty() && m_savedPackageData.version == m_newVersion
            && m_savedPackageData.hash == getFileHash(m_savedPackageData.fileName))
    {
        m_packageData.fileName = m_savedPackageData.fileName;
        emit updateLoaded();
    } else
    if (m_packageData.packageUrl != L"") {
        m_downloadMode = Mode::DOWNLOAD_UPDATES;
        downloadFile(m_packageData.packageUrl, ".exe");
    }
}

QString CUpdateManager::getVersion() const
{
    return m_newVersion;
}

QStringList CUpdateManager::getInstallArguments() const
{
    QStringList arguments;
    if ( !m_packageData.packageArgs.empty() )
        arguments << QString::fromStdWString(m_packageData.packageArgs).split(" ");
    return arguments;
}

QString CUpdateManager::getInstallPackagePath() const
{
    return m_packageData.fileName;
}

void CUpdateManager::onLoadUpdateFinished()
{
    m_packageData.fileName = QString::fromStdWString(m_pDownloader->GetFilePath());
    savePackageData(getFileHash(m_packageData.fileName), m_newVersion, m_packageData.fileName);
    emit updateLoaded();
}

void CUpdateManager::handleAppClose()
{
    if ( m_restartForUpdate ) {
        if ( QProcess::startDetached(getInstallPackagePath(), getInstallArguments())) {
            //qDebug() << "Start installation...";
        } else {
            //qDebug() << "Install command not found!";
        }
    } else {
        cancelLoading();
    }
}

void CUpdateManager::scheduleRestartForUpdate()
{
    m_restartForUpdate = true;
}

#else
void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    m_currentRate = (_rate == "never") ? UpdateInterval::NEVER : (_rate == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
    //qDebug() << "Set new updates mode: " << m_currentRate;
}
#endif

void CUpdateManager::cancelLoading()
{
    m_downloadMode = Mode::CHECK_UPDATES;
    if (m_pDownloader)
        m_pDownloader->Stop();
}

void CUpdateManager::onLoadCheckFinished()
{
    const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    QFile jsonFile(path);
    if ( jsonFile.open(QIODevice::ReadOnly) ) {
        QByteArray ReplyText = jsonFile.readAll();
        jsonFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject root = doc.object();

        bool updateExist = false;
        QString version = root.value("version").toString();
        const QStringList curr_ver = QString::fromLatin1(VER_FILEVERSION_STR).split('.');
        const QStringList ver = version.split('.');
        for (int i = 0; i < std::min(ver.size(), curr_ver.size()); i++) {
            if (ver.at(i).toInt() > curr_ver.at(i).toInt()) {
                updateExist = true;
                break;
            }
        }

        if ( updateExist ) {
        // parse package
#ifdef Q_OS_WIN
            QJsonObject package = root.value("package").toObject();
# if defined (Q_OS_WIN64)
            QJsonValue win = package.value("win_64");
# else
            QJsonValue win = package.value("win_32");
# endif
            QJsonObject win_params = win.toObject();
            m_packageData.packageUrl = win_params.value("url").toString().toStdWString();
            m_packageData.packageArgs = win_params.value("installArguments").toString().toStdWString();
#endif

            // parse release notes
            QJsonObject release_notes = root.value("releaseNotes").toObject();
            const QString lang = CLangater::getCurrentLangCode() == "ru-RU" ? "ru-RU" : "en-EN";
            QJsonValue changelog = release_notes.value(lang);

            m_newVersion = version;
#ifdef Q_OS_WIN
            if (m_newVersion == m_savedPackageData.version)
                clearTempFiles(m_savedPackageData.fileName);
            else
#endif
                clearTempFiles();
            emit checkFinished(false, true, m_newVersion, changelog.toString());
        } else {
            clearTempFiles();
            emit checkFinished(false, false, "", "");
        }
    } else {
        emit checkFinished(true, false, "", "Error receiving updates...");
    }
}
