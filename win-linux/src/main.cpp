/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include <QFile>
#include <QDir>
#include <QTranslator>
#include <QStandardPaths>
#include <QLibraryInfo>

#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "cchooselicensedialog.h"
#include "clicensekeeper.h"

#ifdef _WIN32
#include "win/mainwindow.h"
#include "shlobj.h"
#include "csplash.h"
#else
#include "linux/cmainwindow.h"
#endif

#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QScreen>
#include <QApplication>
#include <QRegularExpression>
#include "cstyletweaks.h"
#include "utils.h"
#include "chelp.h"

BYTE g_dpi_ratio = 1;
BYTE g_lic_type = LICENSE_TYPE_BUSINESS;
QString g_lang;

int main( int argc, char *argv[] )
{
    QString user_data_path = Utils::getUserPath() + APP_DATA_PATH;

    auto setup_paths = [&user_data_path](CAscApplicationManager * manager) {
        std::wstring sAppData(L"");

#ifdef _WIN32
        WCHAR szPath[MAX_PATH];
        if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
            sAppData = std::wstring(szPath);
            std::replace(sAppData.begin(), sAppData.end(), '\\', '/');
            sAppData.append(QString(APP_DATA_PATH).toStdWString());
        }
#else
        sAppData = QString("/var/lib").append(APP_DATA_PATH).toStdWString();
        QFileInfo fi(QString::fromStdWString(sAppData));
        if (fi.isDir() && !fi.isWritable()) {
            // TODO: check directory permissions and warn the user
            qDebug() << "directory permission error";
        }
#endif

        QString app_path = QCoreApplication::applicationDirPath();

        if (sAppData.size() > 0) {
            manager->m_oSettings.SetUserDataPath(sAppData);
            Utils::makepath(user_data_path.append("/data"));
            manager->m_oSettings.cookie_path = (user_data_path + "/cookie").toStdWString();
            manager->m_oSettings.recover_path = (user_data_path + "/recover").toStdWString();
            manager->m_oSettings.fonts_cache_info_path = (user_data_path + "/fonts").toStdWString();

            Utils::makepath(QString().fromStdWString(manager->m_oSettings.fonts_cache_info_path));
        } else {
            manager->m_oSettings.SetUserDataPath(user_data_path.toStdWString());
        }

        manager->m_oSettings.spell_dictionaries_path = (app_path + "/dictionaries").toStdWString();
        manager->m_oSettings.file_converter_path = (app_path + "/converter").toStdWString();
        manager->m_oSettings.local_editors_path = (app_path + "/editors/web-apps/apps/api/documents/index.html").toStdWString();
        manager->m_oSettings.additional_fonts_folder.push_back((app_path + "/fonts").toStdWString());
        manager->m_oSettings.country = Utils::systemLocationCode().toStdString();
    };

#ifdef _WIN32
  #ifdef _AVS
    LPCTSTR mutex_name = (LPCTSTR)QString("AVSMEDIA").data();
  #else
    LPCTSTR mutex_name = (LPCTSTR)QString("TEAMLAB").data();
  #endif

    HANDLE hMutex = CreateMutex(NULL, FALSE, mutex_name);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        hMutex = NULL;
    }
#else
#endif

    if (!CApplicationCEF::IsMainProcess()) { return 0; }

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableHighDpiScaling);

    /* the order is important */
    CApplicationCEF* application_cef = new CApplicationCEF();
    CAscApplicationManager * pApplicationManager = new CAscApplicationManagerWrapper();
    setup_paths(pApplicationManager);
    application_cef->Init_CEF(pApplicationManager, argc, argv);
    /* ********************** */

#ifdef _WIN32
    if (hMutex == NULL) {
        HWND hwnd = FindWindow(L"DocEditorsWindowClass", NULL);
        if (hwnd != NULL) {
            WCHAR * cm_line = GetCommandLine();

            COPYDATASTRUCT MyCDS = {1}; // 1 - will be used like id
            MyCDS.cbData = sizeof(WCHAR) * (wcslen(cm_line) + 1);
            MyCDS.lpData = cm_line;

            SendMessage(hwnd, WM_COPYDATA, WPARAM(0), LPARAM((LPVOID)&MyCDS));

            pApplicationManager->CloseApplication();
            return 0;
        }
    }

    g_dpi_ratio = !(app.primaryScreen()->logicalDotsPerInch() / 96.f < 1.5) ? 2 : 1;
#else
    g_dpi_ratio = CX11Decoration::devicePixelRatio();
#endif
    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    reg_user.setFallbacksEnabled(false);

    /* read lang fom different places
     * cmd argument --lang:en apply the language one time
     * cmd argument --keeplang:en also keep the language for next sessions
    */
    int _arg_i;
    if (!(_arg_i = app.arguments().indexOf("--help") < 0)) {
        CHelp::out();

        pApplicationManager->CloseApplication();

        delete application_cef;
        delete pApplicationManager;
        return 0;
    } else
    if (!(_arg_i = app.arguments().indexOf(QRegExp(reCmdLang)) < 0)) {
        g_lang = app.arguments().at(_arg_i).right(2);
    }

    if (!g_lang.size())
        g_lang = reg_user.value("locale").value<QString>();

#ifdef __linux
    if (!g_lang.size()) {
  #ifdef _ONLY_RU
        g_lang = "ru";
  #else
        g_lang = QLocale::system().name().left(2);
        if (!QFile(":/langs/langs/" + g_lang + ".qm").exists()) g_lang = "en";
  #endif
    }
#else
    // read setup language and set application locale
    !g_lang.size() &&
        !((g_lang = reg_system.value("locale").value<QString>()).size()) && (g_lang = "en").size();
#endif

    QTranslator tr;
    if (g_lang.length()) {
        tr.load(g_lang, ":/langs/langs");
        app.installTranslator(&tr);
    }
    /* applying languages finished */

    CLicensekeeper::init(pApplicationManager);

#ifndef _AVS
    if (Utils::firstStart()) {
        CChooseLicenseDialog dlg;
        dlg.setEULAPath(QString("%1/LICENSE.htm").arg(QCoreApplication::applicationDirPath()));
        g_lic_type = dlg.exec();

        Utils::markFirstStart();
        g_lic_type == LICENSE_TYPE_FREE ?
            CLicensekeeper::makeTempLicense() :
            CLicensekeeper::selfActivation(LICENSE_TYPE_TRIAL);
    }

#endif

#ifdef _WIN32
    CSplash::showSplash();
    app.processEvents();
#endif

    /* prevent drawing of focus rectangle on a button */
    app.setStyle(new CStyleTweaks);

    // read installation time and clean cash folders if expired
    if (reg_system.contains("timestamp")) {
        QDateTime time_istall, time_clear;
        time_istall.setMSecsSinceEpoch(reg_system.value("timestamp", 0).toULongLong());

        bool clean = true;
        if (reg_user.contains("timestamp")) {
            time_clear.setMSecsSinceEpoch(reg_user.value("timestamp", 0).toULongLong());

            clean = time_istall > time_clear;
        }

        if (clean) {
            reg_user.setValue("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
            QDir(user_data_path + "/data/fonts").removeRecursively();
        }
    }

    QByteArray css;
    QFile file;
    foreach(const QFileInfo &info, QDir(":styles/res/styles").entryInfoList(QStringList("*.qss"), QDir::Files)) {
        file.setFileName(info.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            css.append(file.readAll());
            file.close();
        }
    }

    if (g_dpi_ratio > 1) {
        file.setFileName(":styles@2x/styles.qss");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            css.append(file.readAll());
            file.close();
        }
    }


    if (css.size()) app.setStyleSheet(css);

    // Font
    QFont mainFont = app.font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    app.setFont( mainFont );

#ifdef _WIN32
    // Background color
    HBRUSH windowBackground = CreateSolidBrush( RGB(49, 52, 55) );

    // Create window
    CMainWindow window(pApplicationManager, windowBackground);
    window.setMinimumSize( 800*g_dpi_ratio, 600*g_dpi_ratio );

#elif defined(Q_OS_LINUX)
    // Create window
    CMainWindow window(pApplicationManager);

    window.show();
    window.setWindowTitle(WINDOW_NAME);
#endif
    pApplicationManager->CheckFonts();

    bool bIsOwnMessageLoop = false;
    application_cef->RunMessageLoop(bIsOwnMessageLoop);
    if (!bIsOwnMessageLoop) {
        // Launch
        app.exec();
    }

    // release all subprocesses
    pApplicationManager->CloseApplication();

    delete application_cef;
    delete pApplicationManager;

#ifdef _WIN32
    if (hMutex != NULL) {
        CloseHandle(hMutex);
    }
#endif
}
