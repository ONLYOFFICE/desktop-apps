#include "application.h"
#include "mainwindow.h"
#include <locale>
#include "resource.h"
#include "utils.h"
#include "translator.h"
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#ifndef URL_INSTALL_X64
# define URL_INSTALL_X64 ""
#endif
#ifndef URL_INSTALL_X86
# define URL_INSTALL_X86 ""
#endif
#ifndef URL_INSTALL_X64_MSI
# define URL_INSTALL_X64_MSI ""
#endif
#ifndef URL_INSTALL_X86_MSI
# define URL_INSTALL_X86_MSI ""
#endif
#define _TR(str) Translator::tr(str).c_str()
#define WINDOW_SIZE Size(768, 480)


int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
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
    std::locale::global(std::locale(""));
    LCID lcid = MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT);
    Translator lang(lcid, IDT_TRANSLATIONS);
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T(VER_PRODUCTNAME_STR));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NS_Utils::ShowMessage(_TR(MSG_ERR_ALREADY_RUNNING));
        return 0;
    }

    if (HWND hWnd = FindWindow(WINDOW_CLASS_NAME, NULL)) {
        wstring msg(_TR(MSG_ERR_CLOSE_APP));
        NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
        NS_Utils::ShowMessage(msg);
        return 0;
    }

    wstring url_or_path, arch;
    bool app_installed = NS_Utils::IsAppInstalled(url_or_path, &arch);
    if (!app_installed) {
        url_or_path = NS_Utils::IsWin64() ? _T(URL_INSTALL_X64) : _T(URL_INSTALL_X86);
    }

    Application app(hInst, lpCmdLine, nCmdShow);
    if (NS_Utils::IsRtlLanguage(lcid))
        app.setLayoutDirection(LayoutDirection::RightToLeft);
    int scrWidth = GetSystemMetrics(SM_CXSCREEN);
    int scrHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (scrWidth - WINDOW_SIZE.width) / 2;
    int y = (scrHeight - WINDOW_SIZE.height) / 2;
    MainWindow w(nullptr, Rect(x, y, WINDOW_SIZE.width, WINDOW_SIZE.height));
    w.onAboutToDestroy([&app]() {
        app.exit(0);
    });
    if (!app_installed)
        w.initInstallationMode(url_or_path);
    else
        w.initControlMode(arch);
    w.showAll();
    int exit_code = app.exec();
    CloseHandle(hMutex);
    return exit_code;
}
