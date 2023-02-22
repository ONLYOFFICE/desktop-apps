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
#include <QDir>
#include <QDirIterator>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>
#include "utils.h"
#include "defines.h"
#include "version.h"
#include "clangater.h"
#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"
#ifdef Q_OS_WIN
# include <QProcess>
# include <QCryptographicHash>
# include "platform_win/updatedialog.h"
#endif

#define CHECK_ON_STARTUP_MS 9000
#define CMD_ARGUMENT_CHECK_URL L"--updates-appcast-url"
#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES ""
#endif

using std::vector;


auto currentArch()->QString
{
#ifdef Q_OS_WIN
# ifdef Q_OS_WIN64
    return "_x64";
# else
    return "_x86";
# endif
#else
    return "_x64";
#endif
}

class CUpdateManager::DialogSchedule : public QObject
{
public:
    DialogSchedule(QObject *owner);
    void addToSchedule(const QString &method);

private:
    QTimer *m_timer = nullptr;
    QVector<QString> m_shedule_vec;
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
            QMetaObject::invokeMethod(owner,
                                      m_shedule_vec.first().toLocal8Bit().data(),
                                      Qt::QueuedConnection, Q_ARG(QWidget*, wnd));
            m_shedule_vec.removeFirst();
            if (m_shedule_vec.isEmpty())
                m_timer->stop();
        }
    });
}

void CUpdateManager::DialogSchedule::addToSchedule(const QString &method)
{
    m_shedule_vec.push_back(method);
    if (!m_timer->isActive())
        m_timer->start();
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

CUpdateManager::CUpdateManager(QObject *parent):
    QObject(parent),
    m_checkUrl(L""),
    m_downloadMode(Mode::CHECK_UPDATES),
    m_dialogSchedule(new DialogSchedule(this))
{
    // =========== Set updates URL ============
    auto setUrl = [=] {
        if ( InputArgs::contains(CMD_ARGUMENT_CHECK_URL) ) {
            m_checkUrl = InputArgs::argument_value(CMD_ARGUMENT_CHECK_URL);
        } else m_checkUrl = QString(URL_APPCAST_UPDATES).toStdWString();
    };
#ifdef _WIN32
    GET_REGISTRY_SYSTEM(reg_system)
    if (Utils::getWinVersion() > Utils::WinVer::WinXP && reg_system.value("CheckForUpdates", true).toBool())
        setUrl();
#else
    //setUrl();
#endif

    if ( !m_checkUrl.empty() ) {
        m_pDownloader = new CFileDownloader(m_checkUrl, false);
        m_pDownloader->SetEvent_OnComplete([=](int error) {
            QMetaObject::invokeMethod(this, "onCompleteSlot", Qt::QueuedConnection, Q_ARG(int, error));
        });

#ifdef Q_OS_WIN
        m_pDownloader->SetEvent_OnProgress([=](int percent) {
            QMetaObject::invokeMethod(this, "onProgressSlot", Qt::QueuedConnection, Q_ARG(int, percent));
        });
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
    if (m_dialogSchedule)
        delete m_dialogSchedule, m_dialogSchedule = nullptr;
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
    } else
    if (error == 1) {
        auto wgt = QApplication::activeWindow();
        if (wgt && wgt->objectName() == "MainWindow" && !wgt->isMinimized())
            CMessage::warning(wgt, tr("Server connection error!"));
    } else {
        // Pause or Stop
    }
}

void CUpdateManager::init()
{
    bool checkOnStartup = true;
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
#ifdef _WIN32
    m_savedPackageData.fileName = reg_user.value("Updates/file", QString()).toString();
    m_savedPackageData.hash = reg_user.value("Updates/hash", QByteArray()).toByteArray();
    m_savedPackageData.version = reg_user.value("Updates/version", QString()).toString();
    reg_user.endGroup();
    checkOnStartup = (getUpdateMode() != UpdateMode::DISABLE);
#else
    m_lastCheck = time_t(reg_user.value("Updates/last_check", 0).toLongLong());
    reg_user.endGroup();
    m_currentRate = getUpdateMode();
    checkOnStartup = (m_currentRate != UpdateInterval::NEVER);
#endif
    if (checkOnStartup) {
        m_pCheckOnStartupTimer = new QTimer(this);
        m_pCheckOnStartupTimer->setSingleShot(true);
        m_pCheckOnStartupTimer->setInterval(CHECK_ON_STARTUP_MS);
        connect(m_pCheckOnStartupTimer, &QTimer::timeout, this, &CUpdateManager::updateNeededCheking);
        m_pCheckOnStartupTimer->start();
    }
}

void CUpdateManager::downloadFile(const std::wstring &url, const QString &ext)
{
    if (m_pDownloader) {
        m_pDownloader->Stop();
        m_pDownloader->SetFileUrl(url, false);
        const QUuid uuid = QUuid::createUuid();
        const QRegularExpression branches = QRegularExpression("[{|}]+");
        const QString tmp_file = QDir::tempPath() + "/" + QString(FILE_PREFIX) +
                uuid.toString().replace(branches, "") + currentArch() + ext;
        m_pDownloader->SetFilePath(tmp_file.toStdWString());
        m_pDownloader->Start(0);
    }
}

void CUpdateManager::clearTempFiles(const QString &except)
{
    static bool lock = false;
    if (!lock) { // for one-time cleaning
        lock = true;
        QtConcurrent::run([=]() {
            QStringList filter{"*.json", "*.exe"};
            QDirIterator it(QDir::tempPath(), filter, QDir::Files | QDir::NoSymLinks |
                            QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                const QString tmp = it.next();
                if (tmp.toLower().indexOf(FILE_PREFIX) != -1 && tmp != except)
                    QDir().remove(tmp);
            }
        });
    }
#ifdef _WIN32
    if (except.isEmpty())
        savePackageData();
#endif
}

void CUpdateManager::checkUpdates()
{
    destroyStartupTimer(m_pCheckOnStartupTimer);
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
    if (m_pTimer) {
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
    }
#endif
}

#ifdef Q_OS_WIN
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
    if (!m_savedPackageData.fileName.isEmpty()
            && m_savedPackageData.fileName.indexOf(currentArch()) != -1
            && m_savedPackageData.version == m_newVersion
            && m_savedPackageData.hash == getFileHash(m_savedPackageData.fileName))
    {
        m_packageData.fileName = m_savedPackageData.fileName;
        m_dialogSchedule->addToSchedule("showStartInstallMessage");
    } else
    if (m_packageData.packageUrl != L"") {
        m_downloadMode = Mode::DOWNLOAD_UPDATES;
        downloadFile(m_packageData.packageUrl, ".exe");
    }
}

void CUpdateManager::installUpdates()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    const QString ignored_ver = reg_user.value("Updates/ignored_ver").toString();
    reg_user.endGroup();
    if (ignored_ver != getVersion())
        m_dialogSchedule->addToSchedule("showStartInstallMessage");
}

QString CUpdateManager::getVersion() const
{
    return m_newVersion;
}

void CUpdateManager::onLoadUpdateFinished()
{
    m_packageData.fileName = QString::fromStdWString(m_pDownloader->GetFilePath());
    savePackageData(getFileHash(m_packageData.fileName), m_newVersion, m_packageData.fileName);
    m_dialogSchedule->addToSchedule("showStartInstallMessage");
}

void CUpdateManager::handleAppClose()
{
    if ( m_restartForUpdate ) {
        GET_REGISTRY_SYSTEM(reg_system)
        QString prev_inst_lang = reg_system.value("locale", "en").toString();

        QStringList args{"/LANG=" + prev_inst_lang};
        if ( !m_packageData.packageArgs.empty() )
            args << QString::fromStdWString(m_packageData.packageArgs).split(" ");
        if (!QProcess::startDetached(m_packageData.fileName, args)) {
            //qDebug() << "Install command not found!" << m_packageData.fileName << args;
        }
    } else
        cancelLoading();
}

void CUpdateManager::scheduleRestartForUpdate()
{
    m_restartForUpdate = true;
}
#endif

void CUpdateManager::setNewUpdateSetting(const QString& _rate)
{
    GET_REGISTRY_USER(reg_user);
#ifdef _WIN32
    reg_user.setValue("autoUpdateMode", _rate);
    int mode = (_rate == "silent") ?
                    UpdateMode::SILENT : (_rate == "ask") ?
                        UpdateMode::ASK : UpdateMode::DISABLE;
    if (mode == UpdateMode::DISABLE)
        destroyStartupTimer(m_pCheckOnStartupTimer);
#else
    reg_user.setValue("checkUpdatesInterval", _rate);
    m_currentRate = (_rate == "never") ?
                UpdateInterval::NEVER : (_rate == "day") ?
                    UpdateInterval::DAY : UpdateInterval::WEEK;
    if (m_currentRate == UpdateInterval::NEVER)
        destroyStartupTimer(m_pCheckOnStartupTimer);
    QTimer::singleShot(3000, this, &CUpdateManager::updateNeededCheking);
#endif
}

void CUpdateManager::cancelLoading()
{
    m_downloadMode = Mode::CHECK_UPDATES;
    if (m_pDownloader)
        m_pDownloader->Stop();
}

void CUpdateManager::skipVersion()
{
    GET_REGISTRY_USER(reg_user);
    reg_user.beginGroup("Updates");
    reg_user.setValue("Updates/ignored_ver", m_newVersion);
    reg_user.endGroup();
}

int CUpdateManager::getUpdateMode()
{
    GET_REGISTRY_USER(reg_user);
#ifdef _WIN32
    const QString mode = reg_user.value("autoUpdateMode", "ask").toString();
    return (mode == "silent") ?
                UpdateMode::SILENT : (mode == "ask") ?
                    UpdateMode::ASK : UpdateMode::DISABLE;
#else
    const QString interval = reg_user.value("checkUpdatesInterval", "day").toString();
    return (interval == "never") ?
                UpdateInterval::NEVER : (interval == "day") ?
                    UpdateInterval::DAY : UpdateInterval::WEEK;
#endif
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
        // parse package
#ifdef Q_OS_WIN
            QJsonObject package = root.value("package").toObject();
# ifdef Q_OS_WIN64
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
            if (m_newVersion == m_savedPackageData.version
                    && m_savedPackageData.fileName.indexOf(currentArch()) != -1)
                clearTempFiles(m_savedPackageData.fileName);
            else
#endif
                clearTempFiles();
            onCheckFinished(false, true, m_newVersion, changelog.toString());
        } else {
            clearTempFiles();
            onCheckFinished(false, false, "", "");
        }
    } else {
        onCheckFinished(true, false, "", "Error receiving updates...");
    }
}

void CUpdateManager::onCheckFinished(bool error, bool updateExist, const QString &version, const QString &changelog)
{
    Q_UNUSED(changelog);
    if (!error && updateExist) {
        AscAppManager::sendCommandTo(0, "updates:checking", QString("{\"version\":\"%1\"}").arg(version));
#ifdef Q_OS_WIN
        switch (getUpdateMode()) {
        case UpdateMode::SILENT:
            loadUpdates();
            break;
        case UpdateMode::ASK:
            m_dialogSchedule->addToSchedule("showUpdateMessage");
            break;
        }
#else
        m_dialogSchedule->addToSchedule("showUpdateMessage");
#endif
    } else
    if (!error && !updateExist) {
        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
    } else
    if (error) {
        //qDebug() << "Error while loading check file...";
    }
}

void CUpdateManager::showUpdateMessage(QWidget *parent) {
# ifdef _WIN32
    int result = WinDlg::showDialog(parent,
                        tr("A new version of %1 is available!").arg(QString(WINDOW_NAME)),
                        tr("%1 %2 is now available (you have %3). "
                           "Would you like to download it now?").arg(QString(WINDOW_NAME),
                                                                    getVersion(),
                                                                    QString(VER_FILEVERSION_STR)),
                        WinDlg::DlgBtns::mbSkipRemindDownload);

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
# else
    CMessage mbox(mainWindow()->handle(), CMessageOpts::moButtons::mbYesDefSkipNo);
    switch (mbox.info(tr("Do you want to install a new version %1 of the program?").arg(version))) {
    case MODAL_RESULT_CUSTOM + 0:
        QDesktopServices::openUrl(QUrl(DOWNLOAD_PAGE, QUrl::TolerantMode));
        break;
    case MODAL_RESULT_CUSTOM + 1: {
        skipVersion();
        AscAppManager::sendCommandTo(0, "updates:checking", "{\"version\":\"no\"}");
        break;
    }
    default:
        break;
    }
# endif
}

#ifdef Q_OS_WIN
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
#endif
