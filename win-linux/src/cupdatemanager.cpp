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
#include "cascapplicationmanagerwrapper.h"
#include <QCryptographicHash>
#ifdef _WIN32
# include "platform_win/updatedialog.h"
# define DAEMON_NAME L"/updatesvc.exe"
#else
# include <QProcess>
# include "components/cmessage.h"
# include "platform_linux/updatedialog.h"
# define DAEMON_NAME "/updatesvc"
#endif

#define modeToEnum(mod) ((mod == "silent") ? UpdateMode::SILENT : (mod == "ask") ? UpdateMode::ASK : UpdateMode::DISABLE)
#define WStrToTStr(str) QStrToTStr(QString::fromStdWString(str))
#define DAY_TO_SEC 24*3600
#define MINIMUM_INTERVAL 30
#define CHECK_ON_STARTUP_MS 9000
#define CMD_ARGUMENT_UPDATES_CHANNEL L"--updates-appcast-channel"
#define CMD_ARGUMENT_UPDATES_INTERVAL L"--updates-interval"
#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES ""
#endif
#ifndef URL_APPCAST_DEV_CHANNEL
# define URL_APPCAST_DEV_CHANNEL ""
#endif
#define __GLOBAL_LOCK if (m_lock) {CLogger::log("Blocked in: " + FUNCTION_INFO); return;} m_lock = true; \
                          CLogger::log("Locking and further execution:" + FUNCTION_INFO);
#define __UNLOCK m_lock = false; CLogger::log("Unlocked in:" + FUNCTION_INFO);

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
    QObject *m_owner = nullptr;
    QTimer *m_timer = nullptr;
    QVector<Tag> m_shedule_vec;
};

CUpdateManager::DialogSchedule::DialogSchedule(QObject *owner) :
    QObject(owner),
    m_owner(owner)
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
            if (m_shedule_vec.isEmpty()) {
                m_timer->stop();
                qobject_cast<CUpdateManager*>(owner)->refreshStartPage({"", "", "", "", "false"});
            }
        }
    });
}

void CUpdateManager::DialogSchedule::addToSchedule(const QString &method, const QString &text)
{
    m_shedule_vec.push_back({method, text});
    if (!m_timer->isActive()) {
        m_timer->start();
        qobject_cast<CUpdateManager*>(m_owner)->refreshStartPage({"", "", "", "", "true"});
    }
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

auto formattedTime(time_t timestamp)->QString
{
    return (timestamp != 0) ? QLocale::system().toString(QDateTime::fromTime_t(timestamp), QLocale::ShortFormat) :
               QString("--.--.---- --:--");
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

auto runProcess(const tstring &fileName, const tstring &args, bool runAsAdmin = false)->bool
{
#ifdef _WIN32
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
        return true;
    }
#else
    Q_UNUSED(runAsAdmin)
    QStringList _args = QString::fromStdString(args).split(" ");
    if (QProcess::startDetached(QString::fromStdString(fileName), _args))
        return true;
#endif
    return false;
}

struct CUpdateManager::PackageData {
    QString fileName,
            fileType,
            hash,
            version;
    wstring packageUrl,
            packageArgs;
    void clear() {
        fileName.clear();
        fileType.clear();
        hash.clear();
        version.clear();
        packageUrl.clear();
        packageArgs.clear();
    }
};

struct CUpdateManager::SavedPackageData {
    QString fileName,
            fileType,
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
    if ( Utils::updatesAllowed() ) {
        if ( InputArgs::contains(CMD_ARGUMENT_UPDATES_CHANNEL) ) {
            std::wstring ch_updates = InputArgs::argument_value(CMD_ARGUMENT_UPDATES_CHANNEL);
            if ( ch_updates == L"dev" ) {
                m_checkUrl = QString(URL_APPCAST_DEV_CHANNEL).toStdWString();
            }
        }
        if ( InputArgs::contains(CMD_ARGUMENT_UPDATES_INTERVAL) ) {
            int interval = QString::fromStdWString(InputArgs::argument_value(CMD_ARGUMENT_UPDATES_INTERVAL)).toInt();
            if (interval >= MINIMUM_INTERVAL) {
                GET_REGISTRY_USER(reg_user)
                reg_user.beginGroup("Updates");
                reg_user.setValue("interval", interval);
                reg_user.endGroup();
            }
        }

        if ( m_checkUrl.empty() )
            m_checkUrl = QString(URL_APPCAST_UPDATES).toStdWString();
    }

    if ( !m_checkUrl.empty()) {
        CLogger::log("Updates is on, URL: " + QString::fromStdWString(m_checkUrl));
        m_pIntervalTimer = new QTimer(this);
        m_pIntervalTimer->setSingleShot(false);
        connect(m_pIntervalTimer, SIGNAL(timeout()), this, SLOT(checkUpdates()));
        m_pIntervalStartTimer = new QTimer(this);
        m_pIntervalStartTimer->setSingleShot(true);
        m_pIntervalStartTimer->setInterval(CHECK_ON_STARTUP_MS);
        connect(m_pIntervalStartTimer, &QTimer::timeout, this, &CUpdateManager::updateNeededCheking);
        if (IsPackage(Portable))
            runProcess(QStrToTStr(qApp->applicationDirPath()) + DAEMON_NAME, _T("--run-as-app"));
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
    if (IsPackage(Portable)) {
        CSocket sock(INSTANCE_SVC_PORT, 0);
        const char msg[] = "stop";
        sock.sendMessage((void*)msg, sizeof(msg));
    }
}

void CUpdateManager::init()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    m_savedPackageData->fileName = reg_user.value("file", QString()).toString();
    m_savedPackageData->fileType = reg_user.value("type", QString()).toString();
    m_savedPackageData->version = reg_user.value("version", QString()).toString();
    m_lastCheck = time_t(reg_user.value("last_check", 0).toLongLong());
    refreshStartPage({"lastcheck", tr("Last check performed ") + formattedTime(m_lastCheck),
                         tr("Check for updates"), "check", "false"});
    m_interval = reg_user.value("interval", DAY_TO_SEC).toInt();
    if (m_interval < MINIMUM_INTERVAL)
        m_interval = MINIMUM_INTERVAL;
    reg_user.endGroup();

    m_socket->onMessageReceived([this](void *data, size_t) {
        vector<tstring> params;
        if (m_socket->parseMessage(data, params) == 3) {
            switch (std::stoi(params[0])) {
            case MSG_LoadCheckFinished:
                QMetaObject::invokeMethod(this, "onLoadCheckFinished", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            case MSG_LoadUpdateFinished:
                QMetaObject::invokeMethod(this, "onLoadUpdateFinished", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            case MSG_ShowStartInstallMessage: {
                refreshStartPage({"success", tr("To finish updating, restart app"), tr("Restart"), "install", "false"});
                QMetaObject::invokeMethod(m_dialogSchedule, "addToSchedule", Qt::QueuedConnection, Q_ARG(QString, QString("showStartInstallMessage")));
                break;
            }

            case MSG_Progress:
                QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, std::stoi(params[1])));
                break;

            case MSG_OtherError:
                QMetaObject::invokeMethod(this, "onError", Qt::QueuedConnection, Q_ARG(QString, TStrToQStr(params[1])));
                break;

            default:
                break;
            }
        }
    });
}

void CUpdateManager::criticalMsg(QWidget *parent, const QString &msg)
{
    if (!m_manualCheck) {
        __UNLOCK
        return;
    }
#ifdef _WIN32
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : NULL;
    wstring lpText = msg.toStdWString();
    MessageBoxW(parent_hwnd, lpText.c_str(), TEXT(APP_TITLE), MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
#else
    CMessage::error(parent, msg);
#endif
    __UNLOCK
}

void CUpdateManager::clearTempFiles(const QString &except)
{
    static bool lock = false;
    if (!lock) { // for one-time cleaning
        lock = true;
        m_socket->sendMessage(MSG_ClearTempFiles, QStrToTStr(QString(FILE_PREFIX)), QStrToTStr(except));
    }
    if (except.isEmpty())
        savePackageData();
}

void CUpdateManager::checkUpdates(bool manualCheck)
{
    m_pIntervalStartTimer->stop();
    m_lastCheck = time(nullptr);
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("last_check", static_cast<qlonglong>(m_lastCheck));
    reg_user.endGroup();
    if (getUpdateMode() != UpdateMode::DISABLE)
        m_pIntervalStartTimer->start();

    __GLOBAL_LOCK

    refreshStartPage({"load", tr("Checking for updates..."), tr("Check for updates"), "check", "true"});
    m_manualCheck = manualCheck;
    m_packageData->clear();

#ifdef CHECK_DIRECTORY
    if (QFileInfo(qApp->applicationDirPath()).baseName() != QString(REG_APP_NAME)) {
        refreshStartPage({"error", tr("Updates are not allowed!")});
        m_dialogSchedule->addToSchedule("criticalMsg", tr("This folder configuration does not allow for "
                       "updates! The folder name should be: ") + QString(REG_APP_NAME));
        return;
    }
#endif

    if (!m_socket->sendMessage(MSG_CheckUpdates, WStrToTStr(m_checkUrl))) {
        refreshStartPage({"error", tr("An error occurred while check updates: Update Service not found!"),
                             tr("Check for updates"), "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while check updates: Update Service not found!"));
    }
}

void CUpdateManager::updateNeededCheking()
{
    if (m_pIntervalTimer) {
        m_pIntervalTimer->stop();
        int elapsed_time = int(time(nullptr) - m_lastCheck);
        if (elapsed_time > m_interval) {
            checkUpdates();
        } else {
            int remaining_time = 1000 * (m_interval - elapsed_time);
            m_pIntervalTimer->setInterval(remaining_time < CHECK_ON_STARTUP_MS + 1000 ? CHECK_ON_STARTUP_MS + 1000 : remaining_time);
            m_pIntervalTimer->start();
        }
    }
}

void CUpdateManager::onProgressSlot(const int percent)
{
    refreshStartPage({"", tr("Downloading new version %1 (%2%)").arg(m_packageData->version, QString::number(percent))});
}

void CUpdateManager::onError(const QString &error)
{
    refreshStartPage({"error", error, tr("Check for updates"), "check", "false"});
    __UNLOCK
//    m_dialogSchedule->addToSchedule("criticalMsg", error);
}

void CUpdateManager::savePackageData(const QString &version, const QString &fileName, const QString &fileType)
{
    m_savedPackageData->fileName = fileName;
    m_savedPackageData->fileType = fileType;
    m_savedPackageData->version = version;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("file", fileName);
    reg_user.setValue("type", fileType);
    reg_user.setValue("version", version);
    reg_user.endGroup();
}

QString CUpdateManager::ignoredVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString ignored_ver = reg_user.value("ignored_ver").toString();
    reg_user.endGroup();
    return ignored_ver;
}

bool CUpdateManager::isSavedPackageValid()
{
    return (m_savedPackageData->fileName.indexOf(currentArch()) != -1
                && m_savedPackageData->fileType == m_packageData->fileType
                && m_savedPackageData->version == m_packageData->version
                && getFileHash(m_savedPackageData->fileName) == m_packageData->hash);
}

bool CUpdateManager::isVersionBHigherThanA(const QString &a, const QString &b)
{
    const QStringList old_ver = a.split('.');
    const QStringList new_ver = b.split('.');
    for (int i = 0; i < std::min(new_ver.size(), old_ver.size()); i++) {
        if (new_ver.at(i).toInt() > old_ver.at(i).toInt()) {
            return true;
        } else
        if (new_ver.at(i).toInt() < old_ver.at(i).toInt())
            break;
    }
    return false;
}

void CUpdateManager::loadUpdates()
{
    __GLOBAL_LOCK

    if (isSavedPackageValid()) {
        m_packageData->fileName = m_savedPackageData->fileName;
        if (m_packageData->fileType == "archive") {
            __UNLOCK
            unzipIfNeeded();
        } else {
            refreshStartPage({"success", tr("To finish updating, restart app"), tr("Restart"), "install", "false"});
            m_dialogSchedule->addToSchedule("showStartInstallMessage");
        }

    } else
    if (!m_packageData->packageUrl.empty()) {
        if (!m_socket->sendMessage(MSG_LoadUpdates, WStrToTStr(m_packageData->packageUrl), QStrToTStr(m_packageData->fileType))) {
            refreshStartPage({"error", tr("An error occurred while loading updates: Update Service not found!"), tr("Check for updates"), "check", "false"});
            __UNLOCK
//            m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while loading updates: Update Service not found!"));
        } else {
            refreshStartPage({"load", tr("Downloading new version %1 (0%)").arg(m_packageData->version), tr("Cancel"), "abort", "false"});
        }
    } else {
        refreshStartPage({"error", tr("An error occurred while loading updates: package Url is empty!"), tr("Check for updates"), "check", "false"});
        __UNLOCK
    }
}

void CUpdateManager::installUpdates()
{
    __GLOBAL_LOCK
    m_dialogSchedule->addToSchedule("showStartInstallMessage");
}

void CUpdateManager::refreshStartPage(const Command &cmd)
{
    static bool lock = true;
    QJsonObject jsn, btn_jsn;
    if (cmd.isEmpty()) {
        if (lock)
            lock = false;
        if (!m_lastCommand.icon.isEmpty())
            jsn["icon"] = m_lastCommand.icon;
        if (!m_lastCommand.text.isEmpty())
            jsn["text"] = m_lastCommand.text;
        if (!m_lastCommand.btn_text.isEmpty())
            btn_jsn["text"] = m_lastCommand.btn_text;
        if (!m_lastCommand.btn_action.isEmpty())
            btn_jsn["action"] = m_lastCommand.btn_action;
        if (!m_lastCommand.btn_lock.isEmpty())
            btn_jsn["lock"] = m_lastCommand.btn_lock;
    } else {
        if (!cmd.icon.isEmpty()) {
            m_lastCommand.icon = cmd.icon;
            jsn["icon"] = cmd.icon;
        }
        if (!cmd.text.isEmpty()) {
            m_lastCommand.text = cmd.text;
            jsn["text"] = cmd.text;
        }
        if (!cmd.btn_text.isEmpty()) {
            m_lastCommand.btn_text = cmd.btn_text;
            btn_jsn["text"] = cmd.btn_text;
        }
        if (!cmd.btn_action.isEmpty()) {
            m_lastCommand.btn_action = cmd.btn_action;
            btn_jsn["action"] = cmd.btn_action;
        }
        if (!cmd.btn_lock.isEmpty()) {
            m_lastCommand.btn_lock = cmd.btn_lock;
            btn_jsn["lock"] = cmd.btn_lock;
        }
    }
    if (lock)
        return;
    if (!btn_jsn.isEmpty())
        jsn["button"] = btn_jsn;
    if (!jsn.isEmpty())
        AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "updates:status", Utils::stringifyJson(jsn));
}

void CUpdateManager::launchIntervalStartTimer()
{
    if (m_pIntervalStartTimer && getUpdateMode() != UpdateMode::DISABLE)
        m_pIntervalStartTimer->start();
}

QString CUpdateManager::getVersion() const
{
    return m_packageData->version;
}

void CUpdateManager::onLoadUpdateFinished(const QString &filePath)
{
    if (getFileHash(filePath) != m_packageData->hash) {
        refreshStartPage({"error", tr("Update package error: md5 sum does not match the original."),
                             tr("Check for updates"), "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", "Update package error: md5 sum does not match the original.");
        return;
    }
    m_packageData->fileName = filePath;
    savePackageData(m_packageData->version, filePath, m_packageData->fileType);
    if (m_packageData->fileType == "archive") {
        __UNLOCK
        unzipIfNeeded();
    } else {
        refreshStartPage({"success", tr("To finish updating, restart app"), tr("Restart"), "install", "false"});
        m_dialogSchedule->addToSchedule("showStartInstallMessage");
    }
}

void CUpdateManager::unzipIfNeeded()
{
    __GLOBAL_LOCK

    refreshStartPage({"load", tr("Preparing update..."), tr("Cancel"), "abort", "true"});
    if (!m_socket->sendMessage(MSG_UnzipIfNeeded, QStrToTStr(m_packageData->fileName), QStrToTStr(m_packageData->version))) {
        refreshStartPage({"error", tr("An error occurred while unzip updates: Update Service not found!"), tr("Check for updates"), "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", QObject::tr("An error occurred while unzip updates: Update Service not found!"));
    }
}

void CUpdateManager::handleAppClose()
{
    if ( m_startUpdateOnClose ) {
#ifdef _WIN32
        if (m_packageData->fileType != "archive") {
            wstring args = m_packageData->packageArgs;
            if (m_packageData->fileType == "iss") {
                GET_REGISTRY_SYSTEM(reg_system)
                QString prev_inst_lang = " /LANG=" + reg_system.value("locale", "en").toString();
                args += prev_inst_lang.toStdWString();
            }
            if (!runProcess(m_packageData->fileName.toStdWString(), args)) {
                criticalMsg(nullptr, QObject::tr("An error occurred while start install updates!"));
            }
        } else {
            if (!Utils::isSessionInProgress()) {
                CLogger::log("Update skipped: session is being terminated.");
                return;
            }
#endif
            if (!m_socket->sendMessage(MSG_StartReplacingFiles, IsPackage(ISS) ? _T("iss") : IsPackage(MSI) ? _T("msi") :
                   IsPackage(Portable) ? _T("portable") : _T("other"), m_restartAfterUpdate ? _T("true") : _T("false"))) {
                criticalMsg(nullptr, QObject::tr("An error occurred while start replacing files: Update Service not found!"));
            }
#ifdef _WIN32
        }
#endif
    } else
        m_socket->sendMessage(MSG_StopDownload);
}

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("autoUpdateMode", _rate);
    if (modeToEnum(_rate) == UpdateMode::DISABLE) {
        m_pIntervalStartTimer->stop();
        m_pIntervalTimer->stop();
    } else {
        m_pIntervalStartTimer->start();
    }
}

void CUpdateManager::cancelLoading()
{
    refreshStartPage({"lastcheck", tr("Last check performed ") + formattedTime(m_lastCheck),
                      tr("Check for updates"), "check", "false"});
    m_socket->sendMessage(MSG_StopDownload);
    __UNLOCK
}

void CUpdateManager::skipVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("ignored_ver", m_packageData->version);
    reg_user.endGroup();
}

int CUpdateManager::getUpdateMode()
{
    GET_REGISTRY_USER(reg_user);
    return modeToEnum(reg_user.value("autoUpdateMode", "ask").toString());
}

void CUpdateManager::onLoadCheckFinished(const QString &filePath)
{
    m_manualCheck = true;
    QFile jsonFile(filePath);
    if ( jsonFile.open(QIODevice::ReadOnly) ) {
        QByteArray ReplyText = jsonFile.readAll();
        jsonFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(ReplyText);
        QJsonObject root = doc.object();

        QString version = root.value("version").toString();
        QString curr_version = QString::fromLatin1(VER_FILEVERSION_STR);

        if (isVersionBHigherThanA(curr_version, version) && (version != ignoredVersion())) {
            m_packageData->version = version;
            // parse package
            QJsonObject package = root.value("package").toObject();
#ifdef _WIN32
# ifdef _WIN64
            QJsonObject win = package.value("win_64").toObject();
# else
            QJsonObject win = package.value("win_32").toObject();
# endif
            QJsonObject package_type = win.value("archive").toObject();
            m_packageData->fileType = "archive";
            if (!IsPackage(Portable)) {
                const QString install_key = IsPackage(MSI) ? "msi" : "iss";
                if (win.contains(install_key)) {
                    QJsonObject install_type = win.value(install_key).toObject();
                    if (install_type.contains("maxVersion")) {
                        QString maxVersion = install_type.value("maxVersion").toString();
                        if (!isVersionBHigherThanA(maxVersion, curr_version)) {
                            package_type = install_type;
                            m_packageData->fileType = install_key;
                            m_packageData->packageArgs = package_type.value("arguments").toString().toStdWString();
                        }
                    }
                }
            }
#else
            QJsonObject win = package.value("linux_64").toObject();
            QJsonObject package_type = win.value("archive").toObject();
            m_packageData->fileType = "archive";
#endif
            m_packageData->packageUrl = package_type.value("url").toString().toStdWString();
            m_packageData->hash = package_type.value("md5").toString().toLower();

            // parse release notes
            QJsonObject release_notes = root.value("releaseNotes").toObject();
            const QString lang = CLangater::getCurrentLangCode() == "ru-RU" ? "ru-RU" : "en-EN";
            QJsonValue changelog = release_notes.value(lang);

            clearTempFiles(isSavedPackageValid() ? m_savedPackageData->fileName : "");
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
            switch (getUpdateMode()) {
            case UpdateMode::SILENT:
                __UNLOCK
                loadUpdates();
                break;
            case UpdateMode::ASK:
            case UpdateMode::DISABLE:
                if (isSavedPackageValid()) {
                    __UNLOCK
                    loadUpdates();
                } else {
                    refreshStartPage({"lastcheck", tr("Update is available (version %1)").arg(version),
                                         tr("Download update"), "download", "false"});
                    m_dialogSchedule->addToSchedule("showUpdateMessage");
                }
                break;
            }
        } else {
            refreshStartPage({"success", tr("Current version is up to date"), tr("Check for updates"), "check", "false"});
            __UNLOCK;
        }
    } else {
        refreshStartPage({"error", changelog, tr("Check for updates"), "check", "false"});
        __UNLOCK
//        m_dialogSchedule->addToSchedule("criticalMsg", changelog);
    }
}

void CUpdateManager::showUpdateMessage(QWidget *parent) {
    int result = WinDlg::showDialog(parent, tr("Update is available"),
                        QString("%1\n%2: %3\n%4: %5\n%6").arg(QString(WINDOW_NAME), tr("Current version"),
                        QString(VER_FILEVERSION_STR), tr("Update version"), getVersion(),
                        tr("Would you like to download update now?")),
                        WinDlg::DlgBtns::mbSkipRemindDownload);
    __UNLOCK
    switch (result) {
    case WinDlg::DLG_RESULT_DOWNLOAD:
        loadUpdates();
        break;
    case WinDlg::DLG_RESULT_SKIP: {
        skipVersion();
        refreshStartPage({"success", tr("Current version is up to date"), tr("Check for updates"), "check", "false"});
        break;
    }
    default:
        break;
    }
}

void CUpdateManager::showStartInstallMessage(QWidget *parent)
{
    int result = WinDlg::showDialog(parent, tr("Update is ready to install"),
                        QString("%1\n%2: %3\n%4: %5\n%6").arg(QString(WINDOW_NAME), tr("Current version"),
                        QString(VER_FILEVERSION_STR), tr("Update version"), getVersion(),
                        tr("Would you like to restart app now?")),
                        WinDlg::DlgBtns::mbInslaterRestart);
    __UNLOCK
    switch (result) {
    case WinDlg::DLG_RESULT_RESTART: {
        m_startUpdateOnClose = true;
        m_restartAfterUpdate = true;
        AscAppManager::closeAppWindows();
        break;
    }
    case WinDlg::DLG_RESULT_INSLATER: {
#ifdef _WIN32
        if (m_packageData->fileType == "archive") {
            m_startUpdateOnClose = true;
            m_restartAfterUpdate = false;
        } else {
#endif
            m_startUpdateOnClose = false;
            m_restartAfterUpdate = false;
#ifdef _WIN32
        }
#endif
        break;
    }
//    case WinDlg::DLG_RESULT_SKIP: {
//        skipVersion();
//        AscAppManager::sendCommandTo(0, "updates:link", "lock");
//        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
//        break;
//    }
    default:
        m_startUpdateOnClose = false;
        m_restartAfterUpdate = false;
        break;
    }
}

#include "cupdatemanager.moc"
