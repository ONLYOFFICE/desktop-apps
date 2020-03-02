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
#include "version.h"

#ifdef _WIN32
#include "shlobj.h"
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
#include "utils.h"
#include "chelp.h"
#include "common/File.h"

#include <QTextCodec>
#include <iostream>

QStringList g_cmdArgs;

int main( int argc, char *argv[] )
{
#ifdef _WIN32
    Core_SetProcessDpiAwareness();
    Utils::setAppUserModelId(APP_USER_MODEL_ID);
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
        manager->m_oSettings.user_plugins_path          = (user_data_path + "/sdkjs-plugins").toStdWString();
        manager->m_oSettings.local_editors_path         = app_path + L"/editors/web-apps/apps/api/documents/index.html";
        manager->m_oSettings.additional_fonts_folder.push_back(app_path + L"/fonts");
        manager->m_oSettings.country = Utils::systemLocationCode().toStdString();
    };

    CApplicationCEF::Prepare(argc, argv);

#ifdef _WIN32
    HANDLE hMutex = CreateMutex(NULL, FALSE, (LPCTSTR)QString(APP_MUTEX_NAME).data());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hwnd = FindWindow(WINDOW_CLASS_NAME, NULL);
        if (hwnd != NULL) {
            WCHAR * cm_line = GetCommandLine();

            COPYDATASTRUCT MyCDS = {1}; // 1 - will be used like id
            MyCDS.cbData = sizeof(WCHAR) * (wcslen(cm_line) + 1);
            MyCDS.lpData = cm_line;

            SendMessage(hwnd, WM_COPYDATA, WPARAM(0), LPARAM((LPVOID)&MyCDS));
            return 0;
        }
    }
#endif
    const int ac = argc;
    char ** const av = argv;
    for (int a(1); a < ac; ++a) {
        if ( strcmp(av[a], "--version") == 0 ) {
            qWarning() << VER_PRODUCTNAME_STR << "ver." << VER_FILEVERSION_STR;
            return 0;
        } else
        if ( strcmp(av[a], "--help") == 0 ) {
            CHelp::out();
            return 0;
        }
    }

#ifdef __linux__
    SingleApplication app(argc, argv, APP_MUTEX_NAME ":" + QString::fromStdWString(Utils::systemUserName()));
#else
    QApplication app(argc, argv);
#endif
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_DisableHighDpiScaling);

    g_cmdArgs = QApplication::arguments().mid(1);

    /* the order is important */
    CApplicationCEF* application_cef = new CApplicationCEF();

    setup_paths(&AscAppManager::getInstance());
    application_cef->Init_CEF(&AscAppManager::getInstance(), argc, argv);
    /* ********************** */

//    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    reg_user.setFallbacksEnabled(false);

    /* read lang fom different places
     * cmd argument --lang:en apply the language one time
     * cmd argument --keeplang:en also keep the language for next sessions
    */
    CLangater::init();
    /* applying languages finished */

    AscAppManager::initializeApp();
    AscAppManager::startApp();

    AscAppManager::getInstance().StartSpellChecker();
    AscAppManager::getInstance().StartKeyboardChecker();
    AscAppManager::getInstance().CheckFonts();

    bool bIsOwnMessageLoop = false;
    application_cef->RunMessageLoop(bIsOwnMessageLoop);
    if (!bIsOwnMessageLoop) {
        // Launch
        app.exec();
    }

    // release all subprocesses
    AscAppManager::getInstance().CloseApplication();

    delete application_cef;

#ifdef _WIN32
    if (hMutex != NULL) {
        CloseHandle(hMutex);
    }
#endif
}
