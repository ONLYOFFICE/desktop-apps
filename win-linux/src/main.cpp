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

#include "cascapplicationmanagerwrapper.h"
#ifdef _WIN32
# include "platform_win/singleapplication.h"
#else
# include "platform_linux/singleapplication.h"
# include "components/cmessage.h"
# include <unistd.h>
#endif
#include "defines.h"
#include "clangater.h"
#include "version.h"
#include "utils.h"
#include "chelp.h"
#include "common/File.h"
#include <QStyleFactory>


int main( int argc, char *argv[] )
{
#ifdef _WIN32
    if (argc > 1 && strcmp(argv[1], "--add-to-recent") == 0)
        return 0;
    Core_SetProcessDpiAwareness();
    Utils::setAppUserModelId(APP_USER_MODEL_ID);
    WCHAR * cm_line = GetCommandLine();
    InputArgs::init(cm_line);
#else
    //qputenv("LC_ALL", "en_US.UTF8");
    qputenv("QT_QPA_PLATFORM", "xcb");
    qputenv("GDK_BACKEND", "x11");
    InputArgs::init(argc, argv);
    if (geteuid() == 0) {
        CMessage::warning(nullptr, WARNING_LAUNCH_WITH_ADMIN_RIGHTS);
        return 0;
    }
#endif
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);
    QCoreApplication::setApplicationName(QString::fromUtf8(WINDOW_NAME));
    QApplication::setApplicationDisplayName(QString::fromUtf8(WINDOW_NAME));

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
        std::wstring app_path = NSFile::GetProcessDirectory();
        manager->m_oSettings.spell_dictionaries_path    = app_path + L"/dictionaries";
        manager->m_oSettings.file_converter_path        = app_path + L"/converter";
        manager->m_oSettings.recover_path               = (user_data_path + "/recover").toStdWString();
        manager->m_oSettings.user_plugins_path          = (user_data_path + "/sdkjs-plugins").toStdWString();
        manager->m_oSettings.local_editors_path         = app_path + L"/editors/web-apps/apps/api/documents/index.html";
        manager->m_oSettings.additional_fonts_folder.push_back(app_path + L"/fonts");
        manager->m_oSettings.country = Utils::systemLocationCode().toStdString();
    };

    if ( InputArgs::contains(L"--version") ) {
        qWarning() << VER_PRODUCTNAME_STR << "ver." << VER_FILEVERSION_STR;
        return 0;
    } else
    if ( InputArgs::contains(L"--help") ) {
        CHelp::out();
        return 0;
    }
    if ( InputArgs::contains(L"--updates-reset") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.beginGroup("Updates");
        reg_user.remove("");
        reg_user.endGroup();
        reg_user.remove("autoUpdateMode");
    }
    if ( InputArgs::contains(L"--geometry=default") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.remove("maximized");
        reg_user.remove("position");
    }

    SingleApplication app(argc, argv, QString(APP_MUTEX_NAME));

    if (!app.isPrimary() && !InputArgs::contains(L"--single-window-app")) {
        QString _out_args;
        auto _args = InputArgs::arguments();
        if (_args.size() > 0) {
            foreach (auto w_arg, _args) {
                const QString arg = QString::fromStdWString(w_arg);
                if ( arg.startsWith("--new:") )
                    _out_args.append(arg).append(";");
                else
                if ( arg.mid(0,2) != "--" )
                    _out_args.append(arg + ";");
            }
        }
        app.sendMessage(_out_args.toUtf8());
        return 0;
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setStyle(QStyleFactory::create("Fusion"));

    /* the order is important */
    CApplicationCEF::Prepare(argc, argv);
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
    AscAppManager::initializeApp();
    AscAppManager::startApp();
    AscAppManager::getInstance().StartSpellChecker();
    AscAppManager::getInstance().StartKeyboardChecker();
    AscAppManager::getInstance().CheckFonts();

    bool bIsOwnMessageLoop = false;
    int exit_code = application_cef->RunMessageLoop(bIsOwnMessageLoop);
    if (!bIsOwnMessageLoop)
        exit_code = app.exec();

    AscAppManager::getInstance().CloseApplication();
    delete application_cef;
    return exit_code;
}
