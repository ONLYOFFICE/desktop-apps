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
#ifdef Q_OS_WIN
    #include <QProcess>
    #include <QCryptographicHash>
#endif

#define URL_APPCAST_UPDATES  "http://nct.onlyoffice.com/sh/XHh" // Temporary URL

using std::vector;


CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_downloadMode(Mode::CHECK_UPDATES)
{
    // =========== Set updates URL ============
    bool cmd_flag = false;  // updates URL set from command line argument
    vector<wstring> args = InputArgs::arguments();
    foreach (const wstring &arg, args) {
        const QStringList params = QString::fromStdWString(arg).split('=');
        if (params.size() == 2) {
            if (params.at(0) == QString("--updates-appcast-url")) {
                m_checkUrl = params.at(1).toStdWString();
                cmd_flag = true;
                break;
            }
        }
    }
    if (!cmd_flag) {
#if defined (URL_APPCAST_UPDATES)
    m_checkUrl = QString(URL_APPCAST_UPDATES).toStdWString();
#endif
    }
    //qDebug() << "URL_APPCAST_UPDATES: " << QString::fromStdWString(m_checkUrl);
    // ========================================

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

CUpdateManager::~CUpdateManager()
{
    delete m_pDownloader;
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
        /*case Mode::DOWNLOAD_CHANGELOG:
            onLoadChangelogFinished();
            break;*/
#ifdef Q_OS_WIN
        case Mode::DOWNLOAD_UPDATES:
            onLoadUpdateFinished();
            break;
#endif
        default:
            break;
        }
    }
    else {
        //qDebug() << "Download error: " << error;
    }
}

void CUpdateManager::init()
{
    QStringList filter;
    filter << QString("*.json");
    m_newVersion = QString("");
    GET_REGISTRY_USER(reg_user);
#ifdef Q_OS_WIN
    reg_user.beginGroup("Updates");
    const bool _wasInstalledFlag = reg_user.value("Updates/was_installed", false).toBool();
    if (_wasInstalledFlag) {
        filter << QString("*.exe");
    }
    m_savedPackageFileName = reg_user.value("Updates/file", QString()).toString();
    m_savedHash = reg_user.value("Updates/hash", QByteArray()).toByteArray();
    m_savedVersion = reg_user.value("Updates/version", QString()).toString();
    reg_user.endGroup();
    m_packageUrl = L"";
    m_packageArgs = L"";
    m_packageFileName = QString("");
    m_restartForUpdate = false;
#else
    reg_user.beginGroup("Updates");
    m_lastCheck = time_t(reg_user.value("Updates/last_check", 0).toLongLong());
    reg_user.endGroup();
    const QString interval = reg_user.value("checkUpdatesInterval","day").toString();
    m_currentRate = (interval == "disabled") ? UpdateInterval::NEVER : (interval == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#endif
    // ====== Clear temp files =========
    QDirIterator it(QDir::tempPath(), filter, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString tmp = it.next();
        if (tmp.indexOf("onlyoffice_") != -1 || tmp.indexOf("ONLYOFFICE_") != -1) QDir().remove(tmp);
    }
    // =================================

    QTimer::singleShot(6000, this, [=]() {
        updateNeededCheking();
    });
}

void CUpdateManager::downloadFile(const std::wstring &url, const QString &ext)
{
    m_pDownloader->Stop();
    m_pDownloader->SetFileUrl(url, false);
    const QUuid uuid = QUuid::createUuid();
    const QRegularExpression _ignoreBranches = QRegularExpression("[{|}]+");
    const QString tmp_name = QString("onlyoffice_") + uuid.toString().replace(_ignoreBranches, "") + ext;
    const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
    m_pDownloader->SetFilePath(tmp_file.toStdWString());
    m_pDownloader->Start(0);
}

void CUpdateManager::checkUpdates()
{
    //qDebug() << "Check updates...";
    m_newVersion = QString("");
#ifdef Q_OS_WIN
    m_packageUrl = L"";
    m_packageArgs = L"";
    m_packageFileName = QString("");
#else
    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(m_lastCheck));
    reg_user.endGroup();
#endif

    // =========== Download JSON ============
    m_downloadMode = Mode::CHECK_UPDATES;
    downloadFile(m_checkUrl, QString(".json"));
    // ======================================
#ifndef Q_OS_WIN
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
#endif
}

void CUpdateManager::updateNeededCheking() {
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
    if (m_downloadMode == Mode::DOWNLOAD_UPDATES) {
        emit progresChanged(percent);
    }
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
    //qDebug() << "Load updates...";
    if (!m_savedPackageFileName.isEmpty() && m_savedVersion == m_newVersion
            && m_savedHash == getFileHash(m_savedPackageFileName)) {
        //qDebug() << "Saved package is valid...";
        m_packageFileName = m_savedPackageFileName;
        emit updateLoaded();
    } else
    if (m_packageUrl != L"") {
        m_downloadMode = Mode::DOWNLOAD_UPDATES;
        downloadFile(m_packageUrl, QString(".exe"));
    }
}

QString CUpdateManager::getVersion() const
{
    if (!m_newVersion.isEmpty()) {
        return m_newVersion;
    }
    return QString("");
}

QStringList CUpdateManager::getInstallArguments() const
{
    if ( !m_packageArgs.empty() ) {
        QStringList arguments;
        arguments << QString::fromStdWString(m_packageArgs).split(" ");

        return arguments;
    } else return QStringList();
}

QString CUpdateManager::getInstallPackagePath() const
{
    if (!m_packageFileName.isEmpty()) return m_packageFileName;
    return QString();
}

void CUpdateManager::getInstallParams()
{
    //qDebug() << "Get install params...";
    if (!m_packageFileName.isEmpty()) {
        emit updateLoaded();
    }
}

void CUpdateManager::onLoadUpdateFinished()
{
    //qDebug() << "Load updates finished...";
    m_packageFileName = QString::fromStdWString(m_pDownloader->GetFilePath());
    const QByteArray _newHash = getFileHash(m_packageFileName);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/file", m_packageFileName);
    reg_user.setValue("Updates/hash", _newHash);
    reg_user.setValue("Updates/version", m_newVersion);
    reg_user.setValue("Updates/was_installed", false);
    reg_user.endGroup();

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
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/was_installed", true);
    reg_user.endGroup();
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
    //qDebug() << "Loading cancel...";
    m_downloadMode = Mode::CHECK_UPDATES;
    //const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    m_pDownloader->Stop();
    //if (QDir().exists(path)) QDir().remove(path);
}

void CUpdateManager::onLoadCheckFinished()
{
    //qDebug() << "Check updates finished...";
    const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    QFile jsonFile(path);
    if (jsonFile.open(QIODevice::ReadOnly)) {
        QByteArray ReplyText = jsonFile.readAll();
        jsonFile.close();
        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject obj = doc.object();

        // parse version
        QJsonValue version = obj.value(QString("version"));
        QJsonValue date = obj.value(QString("date"));

        // parse release notes
        QJsonValue release_notes = obj.value(QString("releaseNotes"));
        QJsonObject obj_1 = release_notes.toObject();
        GET_REGISTRY_USER(reg_user);
        const QString locale = reg_user.value("locale").toString();
        //qDebug() << "Locale: " << locale;
        const QString page = (locale == "ru-RU") ? "ru-RU" : "en-EN";
        QJsonValue changelog = obj_1.value(page);
        //const wstring changelog_url = changelog.toString().toStdWString();

        // parse package
#ifdef Q_OS_WIN
        QJsonValue package = obj.value(QString("package"));
        QJsonObject obj_2 = package.toObject();
    #if defined (Q_OS_WIN64)
        QJsonValue win = obj_2.value(QString("win_64"));
    #elif defined (Q_OS_WIN32)
        QJsonValue win = obj_2.value(QString("win_32"));
    #endif
        QJsonObject obj_3 = win.toObject();
        QJsonValue url_win = obj_3.value(QString("url"));
        QJsonValue arguments = obj_3.value(QString("installArguments"));
        m_packageUrl = url_win.toString().toStdWString();
        m_packageArgs = arguments.toString().toStdWString();
        //qDebug() << url_win.toString() << "\n" << arguments.toString();
#endif
        //qDebug() << "Version: " << version.toString();
        //qDebug() << "Date: " << date.toString();
        //qDebug() << "Changelog: " << changelog.toString();

        bool updateExist = false;
        int curr_ver[4] = {VER_NUM_MAJOR, VER_NUM_MINOR, VER_NUM_BUILD, VER_NUM_REVISION};
        QStringList ver = version.toString().split('.');
        for (int i = 0; i < std::min(ver.size(), 4); i++) {
            if (ver.at(i).toInt() > curr_ver[i]) {
                updateExist = true;
                break;
            }
        }
        if (updateExist) {
            m_newVersion = version.toString();
            emit checkFinished(false, true, m_newVersion, changelog.toString());
            //loadChangelog(changelog_url);
        } else {
            emit checkFinished(false, false, QString(""), QString(""));
        }
    } else {
        emit checkFinished(true, false, QString(""), QString("Error receiving updates..."));
    }
    if (QDir().exists(path)) QDir().remove(path);
}

/*void CUpdateManager::loadChangelog(const wstring &changelog_url)
{
    //qDebug() << "Load changelog... " << QString::fromStdWString(changelog_url);
    if (changelog_url != L"") {
        m_downloadMode = Mode::DOWNLOAD_CHANGELOG;
        downloadFile(changelog_url, QString(".html"));
    }
}

void CUpdateManager::onLoadChangelogFinished()
{
    //qDebug() << "Load changelog finished... ";
    const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    QFile htmlFile(path);
    if (htmlFile.open(QIODevice::ReadOnly)) {
        const QString html = QString(htmlFile.readAll());
        htmlFile.close();
        emit checkFinished(false, true, m_newVersion, html);
    } else {
        emit checkFinished(false, true, m_newVersion, QString("No available description..."));
    }
    if (QDir().exists(path)) QDir().remove(path);
}*/
