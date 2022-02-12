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


CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    current_rate(UpdateInterval::DAY),
    downloadMode(Mode::CHECK_UPDATES),
    locale("en-EN"),
    new_version(""),
    last_check(0)
{
#ifdef Q_OS_WIN
    package_url = L"";
#endif
    // =========== Set updates URL ============
    bool cmd_flag = false;  // updates URL set from command line argument
    const QStringList args = QCoreApplication::arguments();
    foreach (const QString &arg, args) {
        const QStringList params = arg.split('=');
        if (params.size() == 2) {
            if (params.at(0) == QString("--updates-appcast-url")) {
                check_url = params.at(1).toStdWString();
                cmd_flag = true;
                break;
            }
        }
    }
    if (!cmd_flag) check_url = QString(URL_APPCAST_UPDATES).toStdWString();
    qDebug() << "URL_APPCAST_UPDATES: " << QString::fromStdWString(check_url);
    // ========================================

    downloader = new Downloader(check_url, false);
    downloader->SetEvent_OnComplete(std::bind(&CUpdateManager::onComplete, this, std::placeholders::_1));
    downloader->SetEvent_OnProgress(std::bind(&CUpdateManager::onProgress, this, std::placeholders::_1));
    /*downloader->SetEvent_OnComplete([=](int error) {
        QMetaObject::invokeMethod(this, "onCompleteSlot", Qt::QueuedConnection, Q_ARG(int, error));
    });
    downloader->SetEvent_OnProgress([=](int percent) {
        if (downloadMode == Mode::DOWNLOAD_UPDATES) {
            QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, percent));
        }
    });*/
    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
    readUpdateSettings();
}

CUpdateManager::~CUpdateManager()
{
    delete downloader;
}

void CUpdateManager::onComplete(const int &error)
{
    QMetaObject::invokeMethod(this, "onCompleteSlot", Qt::QueuedConnection, Q_ARG(int, error));
}

void CUpdateManager::onProgress(const int &percent)
{
    QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, percent));
}

void CUpdateManager::onCompleteSlot(const int &error)
{
    if (error == 0) {
        qDebug() << "Download complete... Mode: " << downloadMode;
        switch (downloadMode) {
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

void CUpdateManager::onProgressSlot(const int &percent)
{
#ifdef Q_OS_WIN
    if (downloadMode == Mode::DOWNLOAD_UPDATES) {
        emit progresChanged(percent);
    }
#endif
    //qDebug() << "Percent... " << percent;
}

void CUpdateManager::checkUpdates()
{
    qDebug() << "Check updates...";
#ifdef Q_OS_WIN
    package_url = L"";
    package_args = L"";
#endif
    last_check = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    locale = reg_user.value("locale").toString();
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(last_check));
    reg_user.endGroup();
    qDebug() << "Locale: " << locale;

    // =========== Download JSON ============
    downloader->Stop();
    downloadMode = Mode::CHECK_UPDATES;
    downloader->SetFileUrl(check_url);
    const QUuid uuid = QUuid::createUuid();
    const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".json");
    const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
    downloader->SetFilePath(tmp_file.toStdWString());
    downloader->Start(0);
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
    locale = reg_user.value("locale").toString();
    reg_user.beginGroup("Updates");
    last_check = time_t(reg_user.value("Updates/last_check").toLongLong());
    reg_user.endGroup();
    qDebug() << "Locale: " << locale;
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
    current_rate = (interval == "disabled") ? UpdateInterval::NEVER : (interval == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#endif

    QTimer::singleShot(6000, this, [=]() {
        updateNeededCheking();
    });
}

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    current_rate = (_rate == "never") ? UpdateInterval::NEVER : (_rate == "day") ?  UpdateInterval::DAY : UpdateInterval::WEEK;
#ifndef Q_OS_WIN
    QTimer::singleShot(3000, this, [=]() {
        updateNeededCheking();
    });
#endif
    qDebug() << "Set new updates mode: " << current_rate;
}

void CUpdateManager::updateNeededCheking() {    
#ifdef Q_OS_WIN
    checkUpdates();
#else
    timer->stop();
    int interval = 0;
    const time_t DAY_TO_SEC = 24*3600;
    const time_t WEEK_TO_SEC = 7*24*3600;
    const time_t curr_time = time(nullptr);
    const time_t elapsed_time = curr_time - last_check;
    switch (current_rate) {
    case UpdateInterval::DAY:
        if (elapsed_time > DAY_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(DAY_TO_SEC - elapsed_time);
            timer->setInterval(interval*1000);
            timer->start();
        }
        break;
    case UpdateInterval::WEEK:
        if (elapsed_time > WEEK_TO_SEC) {
            checkUpdates();
        } else {
            interval = static_cast<int>(WEEK_TO_SEC - elapsed_time);
            timer->setInterval(interval*1000);
            timer->start();
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
    downloader->Stop();
    if (package_url != L"") {
        downloadMode = Mode::DOWNLOAD_UPDATES;
        downloader->SetFileUrl(package_url);
        const QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".exe");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        downloader->SetFilePath(tmp_file.toStdWString());
        downloader->Start(0);
    }
}

QString CUpdateManager::getVersion() const
{
    if (!new_version.isEmpty()) {
        return new_version;
    }
    return QString("");
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
        arguments << QString::fromStdWString(package_args).split(" ");
        emit updateLoaded(path, arguments);
    }
}

void CUpdateManager::onLoadUpdateFinished()
{
    qDebug() << "Load updates finished...";
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/temp_file", path);
    reg_user.endGroup();

    // ========== Start installation signal ============
    QStringList arguments;
    arguments << QString::fromStdWString(package_args).split(" ");
    emit updateLoaded(path, arguments);
}
#endif

void CUpdateManager::cancelLoading()
{
    qDebug() << "Loading cancel...";
    downloadMode = Mode::CHECK_UPDATES;
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    downloader->Stop();
    if (QDir().exists(path)) QDir().remove(path);
}

void CUpdateManager::onLoadCheckFinished()
{
    qDebug() << "Check updates finished...";
    const QString path = QString::fromStdWString(downloader->GetFilePath());
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
        package_url = url_win.toString().toStdWString();
        package_args = arguments.toString().toStdWString();
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
            new_version = version.toString();
            emit checkFinished(false, true, new_version, changelog.toString());
            //loadChangelog(changelog_url);
        } else {
            emit checkFinished(false, false, QString(""), QString(""));
        }
    } else {
        emit checkFinished(true, false, QString(""), QString("Error receiving updates..."));
    }
    if (QDir().exists(path)) QDir().remove(path);
}

/*void CUpdateManager::loadChangelog(const WString &changelog_url)
{
    qDebug() << "Load changelog... " << QString::fromStdWString(changelog_url);
    downloader->Stop();
    if (changelog_url != L"") {
        downloadMode = Mode::DOWNLOAD_CHANGELOG;
        downloader->SetFileUrl(changelog_url);
        const QUuid uuid = QUuid::createUuid();
        const QString tmp_name = uuid.toString().replace(QRegularExpression("[{|}]+"), "") + QString(".html");
        const QString tmp_file = QDir::tempPath() + QDir::separator() + tmp_name;
        downloader->SetFilePath(tmp_file.toStdWString());
        downloader->Start(0);
    }
}

void CUpdateManager::onLoadChangelogFinished()
{
    qDebug() << "Load changelog finished... ";
    const QString path = QString::fromStdWString(downloader->GetFilePath());
    QFile htmlFile(path);
    if (htmlFile.open(QIODevice::ReadOnly)) {
        const QString html = QString(htmlFile.readAll());
        htmlFile.close();
        emit checkFinished(false, true, new_version, html);
    } else {
        emit checkFinished(false, true, new_version, QString("No available description..."));
    }
    if (QDir().exists(path)) QDir().remove(path);
}*/
