#include "application.h"
//#include "mainwindow.h"
#include <cstring>
#include <locale>
#include "resource.h"
//#include "utils.h"
//#include "baseutils.h"
//#include "translator.h"
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#ifndef URL_INSTALL_X64
# define URL_INSTALL_X64 ""
#endif
#ifndef URL_INSTALL_X86
# define URL_INSTALL_X86 ""
#endif
#ifndef URL_INSTALL_X64_XP
# define URL_INSTALL_X64_XP ""
#endif
#ifndef URL_INSTALL_X86_XP
# define URL_INSTALL_X86_XP ""
#endif
#ifndef URL_INSTALL_X64_MSI
# define URL_INSTALL_X64_MSI ""
#endif
#ifndef URL_INSTALL_X86_MSI
# define URL_INSTALL_X86_MSI ""
#endif
#define _TR(str) Translator::tr(str).c_str()
#define WINDOW_SIZE Size(768, 480)


#ifdef _WIN32
int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _WIN32
    UNREFERENCED_PARAMETER(hPrevInstance);
    int num_args = 0;
    if (LPTSTR *args = CommandLineToArgvW(lpCmdLine, &num_args)) {
        for (int i = 0; i < num_args; i++) {
            if (lstrcmpi(args[i], _T("--log")) == 0) {
                NS_Logger::AllowWriteLog();
                break;
            }
        }
        LocalFree(args);
    }
#else
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--log") == 0) {
            // NS_Logger::AllowWriteLog();
            break;
        }
    }
#endif

    std::locale::global(std::locale(""));
    // LCID lcid = MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT);
    // Translator lang(lcid, IDT_TRANSLATIONS);
#ifdef _WIN32
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T(VER_PRODUCTNAME_STR));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NS_Utils::ShowMessage(_TR(MSG_ERR_ALREADY_RUNNING));
        return 0;
    }
#else
    GtkApplication *gtk_app = gtk_application_new("com.onlyoffice.OnlineInstaller", G_APPLICATION_FLAGS_NONE);
    if (!g_application_register(G_APPLICATION(gtk_app), NULL, NULL)) {
        g_object_unref(gtk_app);
        return 1;
    }
    if (g_application_get_is_remote(G_APPLICATION(gtk_app))) {
        g_object_unref(gtk_app);
        return 0;
    }
#endif

    // if (HWND hWnd = FindWindow(WINDOW_CLASS_NAME, NULL)) {
    //     wstring msg(_TR(MSG_ERR_CLOSE_APP));
    //     NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
    //     NS_Utils::ShowMessage(msg);
    //     return 0;
    // }

    // wstring url_or_path, arch;
    // bool app_installed = NS_Utils::IsAppInstalled(url_or_path, &arch);
    // if (!app_installed) {
    //     if (NS_Utils::IsWin64()) {
    //         url_or_path = (Utils::getWinVersion() <= Utils::WinVer::WinVista) ? _T(URL_INSTALL_X64_XP) : _T(URL_INSTALL_X64);
    //     } else {
    //         url_or_path = (Utils::getWinVersion() <= Utils::WinVer::WinVista) ? _T(URL_INSTALL_X86_XP) : _T(URL_INSTALL_X86);
    //     }
    // }

#ifdef _WIN32
    Application app(hInst, lpCmdLine, nCmdShow);
#else
    Application app(argc, argv);
#endif
    // if (NS_Utils::IsRtlLanguage(lcid))
    //     app.setLayoutDirection(LayoutDirection::RightToLeft);
    // int scrWidth = GetSystemMetrics(SM_CXSCREEN);
    // int scrHeight = GetSystemMetrics(SM_CYSCREEN);
    // int x = (scrWidth - WINDOW_SIZE.width) / 2;
    // int y = (scrHeight - WINDOW_SIZE.height) / 2;
    // MainWindow w(nullptr, Rect(x, y, WINDOW_SIZE.width, WINDOW_SIZE.height));
    w.onAboutToDestroy([&app]() {
        app.exit(0);
    });
    // if (!app_installed)
    //     w.initInstallationMode(url_or_path);
    // else
    //     w.initControlMode(arch);
    w.showAll();
    int exit_code = app.exec();
#ifdef _WIN32
    CloseHandle(hMutex);
#else
    g_object_unref(gtk_app);
#endif
    return exit_code;
}
