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
#include <QApplication>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <vector>
#include <sstream>
#include "utils.h"
#include "defines.h"
#include "version.h"
#include "clangater.h"
#include "clogger.h"
#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"
#include <QCryptographicHash>
#ifdef _WIN32
# include <Windows.h>
# include "platform_win/updatedialog.h"
# define DAEMON_NAME L"/update-daemon.exe"
#endif

//#define CHECK_DIRECTORY
#define CHECK_ON_STARTUP_MS 9000
#define CMD_ARGUMENT_CHECK_URL L"--updates-appcast-url"
#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES ""
#endif

using std::vector;

class CUpdateManager::DialogSchedule : public QObject
{
    Q_OBJECT
public:
    DialogSchedule(QObject *owner);
public slots:
    void addToSchedule(const QString &method, const QString &text = QString());

private:
    struct Tag {
        QString method,
                text;
    };
    QTimer *m_timer = nullptr;
    QVector<Tag> m_shedule_vec;
};

CUpdateManager::DialogSchedule::DialogSchedule(QObject *owner) :
    QObject(owner)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=] {
        QWidget *wnd = WindowHelper::currentTopWindow();
        if (wnd && !m_shedule_vec.isEmpty()) {
            QByteArray method = m_shedule_vec.first().method.toLocal8Bit();
            QString text = m_shedule_vec.first().text;
            if (text.isEmpty())
                QMetaObject::invokeMethod(owner, method.data(), Qt::QueuedConnection, Q_ARG(QWidget*, wnd));
            else
                QMetaObject::invokeMethod(owner, method.data(), Qt::QueuedConnection, Q_ARG(QWidget*, wnd), Q_ARG(QString, text));
            m_shedule_vec.removeFirst();
            if (m_shedule_vec.isEmpty())
                m_timer->stop();
        }
    });
}

void CUpdateManager::DialogSchedule::addToSchedule(const QString &method, const QString &text)
{
    m_shedule_vec.push_back({method, text});
    if (!m_timer->isActive())
        m_timer->start();
}

auto currentArch()->QString
{
#ifdef _WIN32
# ifdef _WIN64
    return "_x64";
# else
    return "_x86";
# endif
#else
    return "_x64";
#endif
}

auto destroyStartupTimer(QTimer* &timer)->void
{
    if (timer) {
        if (timer->isActive())
            timer->stop();
        timer->deleteLater();
        timer = nullptr;
    }
}

auto getFileHash(const QString &fileName)->QString
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            file.close();
            return QString(hash.result().toHex()).toLower();
        }
        file.close();
    }
    return QString();
}

auto runProcess(const wstring &fileName, const wstring &args, bool runAsAdmin = false)->BOOL
{
    SHELLEXECUTEINFO shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE | SEE_MASK_FLAG_NO_UI;
    shExInfo.hwnd = NULL;
    shExInfo.lpVerb = runAsAdmin ? L"runas" : L"open";
    shExInfo.lpFile = fileName.c_str();
    shExInfo.lpParameters = args.c_str();
    shExInfo.lpDirectory = NULL;
    shExInfo.nShow = SW_HIDE;
    shExInfo.hInstApp = NULL;
    if (ShellExecuteEx(&shExInfo)) {
        //WaitForSingleObject(shExInfo.hProcess, INFINITE);
        CloseHandle(shExInfo.hProcess);
        return TRUE;
    }
    return FALSE;
}

struct CUpdateManager::PackageData {
    QString fileName,
            hash,
            version;
    wstring packageUrl;
    void clear() {
        fileName.clear();
        hash.clear();
        version.clear();
        packageUrl.clear();
    }
};

struct CUpdateManager::SavedPackageData {
    QString fileName,
            version;
};

CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_packageData(new PackageData),
    m_savedPackageData(new SavedPackageData),
    m_checkUrl(L""),
    m_dialogSchedule(new DialogSchedule(this)),
    m_socket(new CSocket(SVC_PORT, APP_PORT))
{
    // =========== Set updates URL ============
    auto setUrl = [=] {
        if ( InputArgs::contains(CMD_ARGUMENT_CHECK_URL) ) {
            m_checkUrl = InputArgs::argument_value(CMD_ARGUMENT_CHECK_URL);
        } else m_checkUrl = TEXT(URL_APPCAST_UPDATES);
    };
#ifdef _WIN32
    if (AppOptions::packageType() == AppOptions::AppPackageType::Portable
            || AppOptions::packageType() == AppOptions::AppPackageType::ISS)
        setUrl();
#else
    if (AppOptions::packageType() == AppOptions::AppPackageType::Portable)
        setUrl();
#endif

    if ( !m_checkUrl.empty()) {
        CLogger::log("Updates is on, URL: " + QString::fromStdWString(m_checkUrl));
//        m_pTimer = new QTimer(this);
//        m_pTimer->setSingleShot(false);
//        connect(m_pTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
        if (AppOptions::packageType() == AppOptions::AppPackageType::Portable)
            runProcess(qApp->applicationDirPath().toStdWString() + DAEMON_NAME, L"--run-as-app");
        init();
    } else
        CLogger::log("Updates is off, URL is empty.");
}

CUpdateManager::~CUpdateManager()
{
    delete m_packageData, m_packageData = nullptr;
    delete m_savedPackageData, m_savedPackageData = nullptr;
    delete m_dialogSchedule, m_dialogSchedule = nullptr;
    delete m_socket, m_socket = nullptr;
    if (AppOptions::packageType() == AppOptions::AppPackageType::Portable) {
        CSocket sock(INSTANCE_SVC_PORT, 0);
        const char msg[] = "stop";
        sock.sendMessage((void*)msg, sizeof(msg));
    }
}

void CUpdateManager::init()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    m_savedPackageData->fileName = reg_user.value("Updates/file", QString()).toString();
    m_savedPackageData->version = reg_user.value("Updates/version", QString()).toString();
//    m_lastCheck = time_t(reg_user.value("Updates/last_check", 0).toLongLong());
    reg_user.endGroup();
    if (getUpdateMode() != UpdateMode::DISABLE) {
        m_pCheckOnStartupTimer = new QTimer(this);
        m_pCheckOnStartupTimer->setSingleShot(true);
        m_pCheckOnStartupTimer->setInterval(CHECK_ON_STARTUP_MS);
        connect(m_pCheckOnStartupTimer, &QTimer::timeout, this, &CUpdateManager::updateNeededCheking);
        m_pCheckOnStartupTimer->start();
    }

    m_socket->onMessageReceived([this](void *data, size_t size) {
        wstring str((const wchar_t*)data), tmp;
        vector<wstring> params;
        std::wstringstream wss(str);
        while (std::getline(wss, tmp, L'|'))
            params.push_back(std::move(tmp));

        if (params.size() == 4) {
            switch (std::stoi(params[0])) {
            case MSG_LoadCheckFinished:
                QMetaObject::invokeMethod(this, "onLoadCheckFinished", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(params[1])));
                break;

            case MSG_LoadUpdateFinished:
                QMetaObject::invokeMethod(this, "onLoadUpdateFinished", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(params[1])));
                break;

            case MSG_ShowStartInstallMessage: {
                QMetaObject::invokeMethod(m_dialogSchedule,
                                          "addToSchedule",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, QString("showStartInstallMessage")));
                break;
            }

            case MSG_Progress:
                QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, std::stoi(params[1])));
                break;

            case MSG_OtherError:
                QMetaObject::invokeMethod(this, "onError", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(params[1])));
                break;

            default:
                break;
            }
        }
    });
}

void CUpdateManager::criticalMsg(QWidget *parent, const QString &msg)
{
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : NULL;
    wstring lpText = msg.toStdWString();
    MessageBoxW(parent_hwnd, lpText.c_str(), TEXT(APP_TITLE), MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
    m_lock = false;
}

void CUpdateManager::clearTempFiles(const QString &except)
{
    static bool lock = false;
    if (!lock) { // for one-time cleaning
        lock = true;
        sendMessage(MSG_ClearTempFiles, TEXT(FILE_PREFIX), except.toStdWString());
    }
    if (except.isEmpty())
        savePackageData();
}

void CUpdateManager::checkUpdates()
{
    if (m_lock)
        return;
    m_lock = true;

    destroyStartupTimer(m_pCheckOnStartupTimer);
    m_packageData->clear();

#ifdef CHECK_DIRECTORY
    if (QFileInfo(qApp->applicationDirPath()).baseName() != QString(REG_APP_NAME)) {
        m_dialogSchedule->addToSchedule("criticalMsg", tr("This folder configuration does not allow for "
                       "updates! The folder name should be: ") + QString(REG_APP_NAME));
        return;
    }
#endif

//    m_lastCheck = time(nullptr);
//    GET_REGISTRY_USER(reg_user);
//    reg_user.beginGroup("Updates");
//    reg_user.setValue("Updates/last_check", static_cast<qlonglong>(m_lastCheck));
//    reg_user.endGroup();

    if (!sendMessage(MSG_CheckUpdates, m_checkUrl)) {
        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while check updates: Update Service not found!"));
    }
//    QTimer::singleShot(3000, this, [=]() {
//        updateNeededCheking();
//    });
}

void CUpdateManager::updateNeededCheking()
{
    checkUpdates();
//    if (m_pTimer) {
//        m_pTimer->stop();
//        int interval = 0;
//        const time_t DAY_TO_SEC = 24*3600;
//        const time_t curr_time = time(nullptr);
//        const time_t elapsed_time = curr_time - m_lastCheck;
//        if (elapsed_time > DAY_TO_SEC) {
//            checkUpdates();
//        } else {
//            interval = static_cast<int>(DAY_TO_SEC - elapsed_time);
//            m_pTimer->setInterval(interval*1000);
//            m_pTimer->start();
//        }
//    }
}

void CUpdateManager::onProgressSlot(const int percent)
{
    emit progresChanged(percent);
}

void CUpdateManager::onError(const QString &error)
{
    m_dialogSchedule->addToSchedule("criticalMsg", error);
}

void CUpdateManager::savePackageData(const QString &version, const QString &fileName)
{
    m_savedPackageData->fileName = fileName;
    m_savedPackageData->version = version;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/file", fileName);
    reg_user.setValue("Updates/version", version);
    reg_user.endGroup();
}

bool CUpdateManager::sendMessage(int cmd, const wstring &param1, const wstring &param2, const wstring &param3)
{
    wstring str = std::to_wstring(cmd) + L"|" + param1 + L"|" + param2 + L"|" + param3;
    size_t sz = str.size() * sizeof(str.front());
    return m_socket->sendMessage((void*)str.c_str(), sz);
}

void CUpdateManager::loadUpdates()
{
//    if (m_lock)
//        return;

    if (m_savedPackageData->fileName.indexOf(currentArch()) != -1
            && m_savedPackageData->version == m_packageData->version
            && getFileHash(m_savedPackageData->fileName) == m_packageData->hash)
    {
        m_packageData->fileName = m_savedPackageData->fileName;
        AscAppManager::sendCommandTo(0, "updates:download", QString("{\"progress\":\"100\"}"));
        unzipIfNeeded();

    } else
    if (!m_packageData->packageUrl.empty()) {
        if (!sendMessage(MSG_LoadUpdates, m_packageData->packageUrl)) {
            m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while loading updates: Update Service not found!"));
        }
    }
}

void CUpdateManager::installUpdates()
{
    if (m_lock)
        return;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString ignored_ver = reg_user.value("Updates/ignored_ver").toString();
    reg_user.endGroup();
    if (ignored_ver != getVersion())
        m_dialogSchedule->addToSchedule("showStartInstallMessage");
}

QString CUpdateManager::getVersion() const
{
    return m_packageData->version;
}

void CUpdateManager::onLoadUpdateFinished(const QString &filePath)
{
    if (getFileHash(filePath) != m_packageData->hash) {
        AscAppManager::sendCommandTo(0, "updates:checking", QString("{\"version\":\"%1\"}").arg(m_packageData->version));
        m_dialogSchedule->addToSchedule("criticalMsg", "Update package error: md5 sum does not match the original.");
        return;
    }
    m_packageData->fileName = filePath;
    savePackageData(m_packageData->version, filePath);
    unzipIfNeeded();
}

void CUpdateManager::unzipIfNeeded()
{
    if (m_lock)
        return;
    m_lock = true;

    if (!sendMessage(MSG_UnzipIfNeeded, m_packageData->fileName.toStdWString(), m_packageData->version.toStdWString())) {
        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while unzip updates: Update Service not found!"));
    }
}

void CUpdateManager::handleAppClose()
{
    if ( m_restartForUpdate ) {
        if (!sendMessage(MSG_StartReplacingFiles)) {
            criticalMsg(nullptr, QObject::tr("An error occurred while start replacing files: Update Service not found!"));
        }
    } else
        sendMessage(MSG_StopDownload);
}

void CUpdateManager::scheduleRestartForUpdate()
{
    m_restartForUpdate = true;
}

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("autoUpdateMode", _rate);
    int mode = (_rate == "silent") ?
                    UpdateMode::SILENT : (_rate == "ask") ?
                        UpdateMode::ASK : UpdateMode::DISABLE;
    if (mode == UpdateMode::DISABLE)
        destroyStartupTimer(m_pCheckOnStartupTimer);
//    QTimer::singleShot(3000, this, &CUpdateManager::updateNeededCheking);
}

void CUpdateManager::cancelLoading()
{
    if (m_lock)
        return;
    AscAppManager::sendCommandTo(0, "updates:checking", QString("{\"version\":\"%1\"}").arg(m_packageData->version));
    sendMessage(MSG_StopDownload);
}

void CUpdateManager::skipVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/ignored_ver", m_packageData->version);
    reg_user.endGroup();
}

int CUpdateManager::getUpdateMode()
{
    GET_REGISTRY_USER(reg_user);
    const QString mode = reg_user.value("autoUpdateMode", "ask").toString();
    return (mode == "silent") ?
                UpdateMode::SILENT : (mode == "ask") ?
                    UpdateMode::ASK : UpdateMode::DISABLE;
}

void CUpdateManager::onLoadCheckFinished(const QString &filePath)
{
//    if (m_lock)
//        return;
    QFile jsonFile(filePath);
    if ( jsonFile.open(QIODevice::ReadOnly) ) {
        QByteArray ReplyText = jsonFile.readAll();
        jsonFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject root = doc.object();

        bool updateExist = false;
        QString version = root.value("version").toString();

        GET_REGISTRY_USER(reg_user);
        reg_user.beginGroup("Updates");
        const QString ignored_ver = reg_user.value("Updates/ignored_ver").toString();
        reg_user.endGroup();

        const QStringList curr_ver = QString::fromLatin1(VER_FILEVERSION_STR).split('.');
        const QStringList ver = version.split('.');
        for (int i = 0; i < std::min(ver.size(), curr_ver.size()); i++) {
            if (ver.at(i).toInt() > curr_ver.at(i).toInt()) {
                updateExist = (version != ignored_ver);
                break;
            } else
            if (ver.at(i).toInt() < curr_ver.at(i).toInt())
                break;
        }

        if ( updateExist ) {
            m_packageData->version = version;

            // parse package
            QJsonObject package = root.value("package").toObject();
#ifdef _WIN32
# ifdef _WIN64
            QJsonObject win = package.value("win_64").toObject();
# else
            QJsonObject win = package.value("win_32").toObject();
# endif
#else
            // TO_DO: linux package parsing
#endif
            QJsonObject archive = win.value("archive").toObject();
            m_packageData->packageUrl = archive.value("url").toString().toStdWString();
            m_packageData->hash = archive.value("md5").toString().toLower();

            // parse release notes
            QJsonObject release_notes = root.value("releaseNotes").toObject();
            const QString lang = CLangater::getCurrentLangCode() == "ru-RU" ? "ru-RU" : "en-EN";
            QJsonValue changelog = release_notes.value(lang);

            if (m_savedPackageData->version == version
                    && m_savedPackageData->fileName.indexOf(currentArch()) != -1
                    && getFileHash(m_savedPackageData->fileName) == m_packageData->hash)
                clearTempFiles(m_savedPackageData->fileName);
            else
                clearTempFiles();
            onCheckFinished(false, true, m_packageData->version, changelog.toString());
        } else {
            clearTempFiles();
            onCheckFinished(false, false, "", "");
        }
    } else {
        onCheckFinished(true, false, "", "Error opening JSON file.");
    }
}

void CUpdateManager::onCheckFinished(bool error, bool updateExist, const QString &version, const QString &changelog)
{
    if ( !error) {
        if ( updateExist ) {
            QString args = QString("{\"version\":\"%1\"}").arg(version);
            AscAppManager::sendCommandTo(0, "updates:checking", args);
            AscAppManager::sendCommandToAllEditors(L"updates:checking", args.toStdWString());
            switch (getUpdateMode()) {
            case UpdateMode::SILENT:
                m_lock = false;
                loadUpdates();
                break;
            case UpdateMode::ASK:
                m_dialogSchedule->addToSchedule("showUpdateMessage");
                break;
            }
        } else {
            AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
            m_lock = false;
        }
    } else {
        m_dialogSchedule->addToSchedule("criticalMsg", changelog);
    }
}

void CUpdateManager::showUpdateMessage(QWidget *parent) {
    int result = WinDlg::showDialog(parent,
                        tr("A new version of %1 is available!").arg(QString(WINDOW_NAME)),
                        tr("%1 %2 is now available (you have %3). "
                           "Would you like to download it now?").arg(QString(WINDOW_NAME),
                                                                    getVersion(),
                                                                    QString(VER_FILEVERSION_STR)),
                        WinDlg::DlgBtns::mbSkipRemindDownload);
    m_lock = false;
    switch (result) {
    case WinDlg::DLG_RESULT_DOWNLOAD:
        loadUpdates();
        break;
    case WinDlg::DLG_RESULT_SKIP: {
        skipVersion();
        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
        break;
    }
    default:
        break;
    }
}

void CUpdateManager::showStartInstallMessage(QWidget *parent)
{
    AscAppManager::sendCommandTo(0, "updates:download", "{\"progress\":\"done\"}");
    int result = WinDlg::showDialog(parent,
                                    tr("A new version of %1 is available!").arg(QString(WINDOW_NAME)),
                                    tr("%1 %2 is now downloaded (you have %3). "
                                       "Would you like to install it now?").arg(QString(WINDOW_NAME),
                                                                                getVersion(),
                                                                                QString(VER_FILEVERSION_STR)),
                                    WinDlg::DlgBtns::mbSkipRemindSaveandinstall);
    m_lock = false;
    switch (result) {
    case WinDlg::DLG_RESULT_INSTALL: {
        scheduleRestartForUpdate();
        AscAppManager::closeAppWindows();
        break;
    }
    case WinDlg::DLG_RESULT_SKIP: {
        skipVersion();
        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
        break;
    }
    default:
        break;
    }
}

#include "cupdatemanager.moc"
