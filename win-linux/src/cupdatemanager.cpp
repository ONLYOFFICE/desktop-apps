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
#include <QProcess>

#define URL_APPCAST_UPDATES  "http://nct.onlyoffice.com/sh/XHh" // Temporary URL

CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_currentRate(UpdateInterval::DAY),
    m_downloadMode(Mode::CHECK_UPDATES),
    m_newVersion(""),
    m_lastCheck(0)
{
#ifdef Q_OS_WIN
    m_packageUrl = L"";
#endif
    // =========== Set updates URL ============
    bool cmd_flag = false;  // updates URL set from command line argument
    const QStringList args = QCoreApplication::arguments();
    foreach (const QString &arg, args) {
        const QStringList params = arg.split('=');
        if (params.size() == 2) {
            if (params.at(0) == QString("--updates-appcast-url")) {
                m_checkUrl = params.at(1).toStdWString();
                cmd_flag = true;
                break;
            }
        }
    }
    if (!cmd_flag) m_checkUrl = QString(URL_APPCAST_UPDATES).toStdWString();
    qDebug() << "URL_APPCAST_UPDATES: " << QString::fromStdWString(m_checkUrl);
    // ========================================

    m_pDownloader = new Downloader(m_checkUrl, false);
    m_pDownloader->SetEvent_OnComplete(std::bind(&CUpdateManager::onComplete, this, std::placeholders::_1));
    m_pDownloader->SetEvent_OnProgress(std::bind(&CUpdateManager::onProgress, this, std::placeholders::_1));
    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(false);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    readUpdateSettings();
}

CUpdateManager::~CUpdateManager()
{
    delete m_pDownloader;
}

void CUpdateManager::onComplete(const int error)
{
    QMetaObject::invokeMethod(this, "onCompleteSlot", Qt::QueuedConnection, Q_ARG(int, error));
}

void CUpdateManager::onProgress(const int percent)
{
    QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, percent));
}

void CUpdateManager::onCompleteSlot(const int error)
{
    if (error == 0) {
        qDebug() << "Download complete... Mode: " << m_downloadMode;
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
        qDebug() << "Download error: " << error;
    }
}

void CUpdateManager::onProgressSlot(const int percent)
{
    Q_UNUSED(percent);
#ifdef Q_OS_WIN
    if (m_downloadMode == Mode::DOWNLOAD_UPDATES) {
        emit progresChanged(percent);
    }
#endif
    //qDebug() << "Percent... " << percent;
}

void CUpdateManager::checkUpdates()
{
    qDebug() << "Check updates...";
#ifdef Q_OS_WIN
    m_packageUrl = L"";
    m_packageArgs = L"";
#endif
    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(m_lastCheck));
    reg_user.endGroup();

    // =========== Download JSON ============
    m_pDownloader->Stop();
    m_downloadMode = Mode::CHECK_UPDATES;
    m_pDownloader->SetFileUrl(m_checkUrl);
    const QUuid uuid = QUuid::createUuid();
    QRegularExpression _ignoreBranches = QRegularExpression("[{|}]+");
    const QString tmp_name = uuid.toString().replace(_ignoreBranches, "") + QString(".json");
    const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
    m_pDownloader->SetFilePath(tmp_file.toStdWString());
    m_pDownloader->Start(0);
    // ======================================
#ifndef Q_OS_WIN
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
#endif
}

void CUpdateManager::readUpdateSettings()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    m_lastCheck = time_t(reg_user.value("Updates/last_check").toLongLong());
    reg_user.endGroup();

#ifdef Q_OS_WIN
    reg_user.beginGroup("Updates"); // Removing the update package at the start of the program
    const QString tmp_file = reg_user.value("Updates/temp_file").toString();
    reg_user.endGroup();
    if (!tmp_file.isEmpty()) {
        if (QDir().exists(tmp_file)) QDir().remove(tmp_file);
        reg_user.beginGroup("Updates");
        reg_user.setValue("Updates/temp_file", QString(""));
        reg_user.endGroup();
    }
#else
    const QString interval = reg_user.value("checkUpdatesInterval","day").toString();
    m_currentRate = (interval == "disabled") ? UpdateInterval::NEVER : (interval == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#endif

    QTimer::singleShot(6000, this, [=]() {
        updateNeededCheking();
    });
}

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    m_currentRate = (_rate == "never") ? UpdateInterval::NEVER : (_rate == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#ifndef Q_OS_WIN
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
#endif
    qDebug() << "Set new updates mode: " << m_currentRate;
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
void CUpdateManager::loadUpdates()
{
    qDebug() << "Load updates...";
    m_pDownloader->Stop();
    if (m_packageUrl != L"") {
        m_downloadMode = Mode::DOWNLOAD_UPDATES;
        m_pDownloader->SetFileUrl(m_packageUrl);
        const QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".exe");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        m_pDownloader->SetFilePath(tmp_file.toStdWString());
        m_pDownloader->Start(0);
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
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString path = reg_user.value("Updates/temp_file").toString();
    reg_user.endGroup();

    return path;
}

void CUpdateManager::getInstallParams()
{
    qDebug() << "Get install params...";
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString path = reg_user.value("Updates/temp_file").toString();
    reg_user.endGroup();
    if (!path.isEmpty()) {

        // ========== Start installation signal ============
        QStringList arguments;
        arguments << QString::fromStdWString(m_packageArgs).split(" ");
        emit updateLoaded();
    }
}

void CUpdateManager::onLoadUpdateFinished()
{
    qDebug() << "Load updates finished...";
    const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/temp_file", path);
    reg_user.endGroup();

    // ========== Start installation signal ============
    QStringList arguments;
    arguments << QString::fromStdWString(m_packageArgs).split(" ");
    emit updateLoaded();
}

void CUpdateManager::handleAppClose()
{
    if ( m_restartForUpdate ) {
        if ( QProcess::startDetached(getInstallPackagePath(), getInstallArguments())) {
            qDebug() << "Start installation...";
        } else {
            qDebug() << "Install command not found!";
        }
    } else {
        cancelLoading();
    }
}
#endif

void CUpdateManager::cancelLoading()
{
    qDebug() << "Loading cancel...";
    m_downloadMode = Mode::CHECK_UPDATES;
    const QString path = QString::fromStdWString(m_pDownloader->GetFilePath());
    m_pDownloader->Stop();
    if (QDir().exists(path)) QDir().remove(path);
}

void CUpdateManager::onLoadCheckFinished()
{
    qDebug() << "Check updates finished...";
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
        qDebug() << "Locale: " << locale;
        const QString page = (locale == "ru-RU") ? "ru-RU" : "en-EN";
        QJsonValue changelog = obj_1.value(page);
        //const WString changelog_url = changelog.toString().toStdWString();

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
        qDebug() << url_win.toString() << "\n" << arguments.toString();
#endif
        qDebug() << "Version: " << version.toString();
        qDebug() << "Date: " << date.toString();
        qDebug() << "Changelog: " << changelog.toString();

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

void CUpdateManager::scheduleRestartForUpdate()
{
    m_restartForUpdate = true;
}

/*void CUpdateManager::loadChangelog(const WString &changelog_url)
{
    qDebug() << "Load changelog... " << QString::fromStdWString(changelog_url);
    m_pDownloader->Stop();
    if (changelog_url != L"") {
        m_downloadMode = Mode::DOWNLOAD_CHANGELOG;
        m_pDownloader->SetFileUrl(changelog_url);
        const QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".html");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        m_pDownloader->SetFilePath(tmp_file.toStdWString());
        m_pDownloader->Start(0);
    }
}

void CUpdateManager::onLoadChangelogFinished()
{
    qDebug() << "Load changelog finished... ";
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
