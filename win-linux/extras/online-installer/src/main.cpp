#include "application.h"
#include "mainwindow.h"
#include <locale>
#include "resource.h"
#include "utils.h"
#include "baseutils.h"
#include "translator.h"
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#define WINDOW_SIZE Size(768, 480)


int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    int num_args = 0;
    if (LPTSTR *args = CommandLineToArgvW(lpCmdLine, &num_args)) {
        NS_Utils::parseCmdArgs(num_args, args);
        LocalFree(args);
    }
    if (NS_Utils::cmdArgContains(_T("--log")))
        NS_Logger::AllowWriteLog();

    std::locale::global(std::locale(""));
    LCID lcid = MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT);
    Translator lang(lcid, IDT_TRANSLATIONS);
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T(VER_PRODUCTNAME_STR));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NS_Utils::ShowMessage(_TR(MSG_ERR_ALREADY_RUNNING));
        return 0;
    }

    if (Utils::getWinVersion() < Utils::Win7) {
        wstring msg(_TR(MSG_ERR_SYSTEM));
        NS_Utils::Replace(msg, L"%1", _TR(CAPTION));
        NS_Utils::ShowMessage(msg);
        CloseHandle(hMutex);
        return 0;
    }

    if (!NS_Utils::checkAndWaitForAppClosure()) {
        CloseHandle(hMutex);
        return 0;
    }

    wstring url_or_path, arch;
    bool app_installed = NS_Utils::IsAppInstalled(url_or_path, &arch);
    if (!app_installed) {
        url_or_path = NS_Utils::cmdArgContains(_T("--appcast-dev-channel")) ? _T(URL_INSTALL_DEV) : _T(URL_INSTALL);
        wstring url_filename = L"DesktopEditors_";
        url_filename += NS_Utils::IsWin64() ? _T("x64") : _T("x86");
        url_filename += _T(".exe");
        NS_Utils::Replace(url_or_path, _T("<file>"), url_filename);
    }

    Application app(hInst, lpCmdLine, nCmdShow);
    app.setFont(L"Segoe UI");
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
