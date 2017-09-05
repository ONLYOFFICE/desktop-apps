/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
#include <QDesktopWidget>
#include <memory>

#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "clangater.h"

#ifdef _WIN32
#include "win/mainwindow.h"
#include "shlobj.h"
#include "csplash.h"
#else
#include "linux/cmainwindow.h"
#include "linux/singleapplication.h"
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
#include "common/File.h"

QStringList g_cmdArgs;

int main( int argc, char *argv[] )
{
#ifdef _WIN32
    Core_SetProcessDpiAwareness();
#endif

    QString user_data_path = Utils::getUserPath() + APP_DATA_PATH;

    auto setup_paths = [&user_data_path](CAscApplicationManager * manager) {

#ifdef _WIN32
        QString common_data_path = Utils::getAppCommonPath();

        if ( !common_data_path.isEmpty() ) {
            manager->m_oSettings.SetUserDataPath(common_data_path.toStdWString());

            Utils::makepath(user_data_path.append("/data"));
            manager->m_oSettings.cookie_path = (user_data_path + "/cookie").toStdWString();
            manager->m_oSettings.recover_path = (user_data_path + "/recover").toStdWString();
            manager->m_oSettings.fonts_cache_info_path = (user_data_path + "/fonts").toStdWString();

            Utils::makepath(QString().fromStdWString(manager->m_oSettings.fonts_cache_info_path));
        } else
#else
#endif
        {
            manager->m_oSettings.SetUserDataPath(user_data_path.toStdWString());
        }

        wstring app_path = NSFile::GetProcessDirectory();
        manager->m_oSettings.spell_dictionaries_path    = app_path + L"/dictionaries";
        manager->m_oSettings.file_converter_path        = app_path + L"/converter";
        manager->m_oSettings.recover_path               = (user_data_path + "/recover").toStdWString();
        manager->m_oSettings.local_editors_path         = app_path + L"/editors/web-apps/apps/api/documents/index.html";
        manager->m_oSettings.additional_fonts_folder.push_back(app_path + L"/fonts");
        manager->m_oSettings.country = Utils::systemLocationCode().toStdString();
    };

    if (!CApplicationCEF::IsMainProcess(argc, argv))
    {
        CApplicationCEF* application_cef = new CApplicationCEF();
        std::unique_ptr<CAscApplicationManager> appmananger(new CAscApplicationManagerWrapper);

        setup_paths(appmananger.get());
        int nReturnCode = application_cef->Init_CEF(appmananger.get(), argc, argv);

        delete application_cef;
        return nReturnCode;
    }

#ifdef _WIN32
    HANDLE hMutex = CreateMutex(NULL, FALSE, (LPCTSTR)QString(APP_MUTEX_NAME).data());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        hMutex = NULL;
    }
#endif

#ifdef __linux__
    SingleApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableHighDpiScaling);

    /* the order is important */
    CApplicationCEF* application_cef = new CApplicationCEF();
    CAscApplicationManager * pApplicationManager = new CAscApplicationManagerWrapper();
    setup_paths(pApplicationManager);
    application_cef->Init_CEF(pApplicationManager, argc, argv);
    /* ********************** */

    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)

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
#endif

    reg_user.setFallbacksEnabled(false);
    g_cmdArgs = QCoreApplication::arguments();

    /* read lang fom different places
     * cmd argument --lang:en apply the language one time
     * cmd argument --keeplang:en also keep the language for next sessions
    */
    int _arg_i;
    if (!(_arg_i = g_cmdArgs.indexOf("--help") < 0)) {
        CHelp::out();

        pApplicationManager->CloseApplication();

        delete application_cef;
        delete pApplicationManager;
        return 0;
    }

    CLangater::init();

    /* applying languages finished */

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
            QDir(user_data_path + "/fonts").removeRecursively();
        }
    }

#ifdef _WIN32
    int _scr_num = QApplication::desktop()->primaryScreen();
    if (reg_user.contains("position")) {
        _scr_num = QApplication::desktop()->screenNumber(
                            reg_user.value("position").toRect().topLeft() );
    }

    byte dpi_ratio = Utils::getScreenDpiRatio(_scr_num);
#else
    byte dpi_ratio = CX11Decoration::devicePixelRatio();
#endif

    QByteArray css(Utils::getAppStylesheets(dpi_ratio));
    if ( !css.isEmpty() ) app.setStyleSheet(css);

    // Font
    QFont mainFont = app.font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    app.setFont( mainFont );

#ifdef _WIN32
    // Create window
    CMainWindow window(pApplicationManager);

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
