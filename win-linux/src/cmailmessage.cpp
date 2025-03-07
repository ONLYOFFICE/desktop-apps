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

#include "cmailmessage.h"
#include <fstream>
#include <ctime>
#include <stack>
#ifdef WIN32_USING_MAPI
# include <Windows.h>
# include <shlwapi.h>
# include <commctrl.h>
# include <mapi.h>
# include "cascapplicationmanagerwrapper.h"
# include "defines.h"
# define REG_MAIL_CLIENTS "SOFTWARE\\Clients\\Mail"
#else
# include <iomanip>
# include <sstream>
# ifdef __APPLE__

# else
#  include "utils.h"
# endif
#endif

#ifdef WIN32_USING_MAPI
static void regValue(HKEY rootKey, std::wstring &value)
{
    HKEY hKey;
    if (RegOpenKeyEx(rootKey, TEXT(REG_MAIL_CLIENTS), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_SZ, cbData = 0;
        if (SHGetValue(hKey, L"", L"", &type, NULL, &cbData) == ERROR_SUCCESS) {
            wchar_t *pvData = (wchar_t*)malloc(cbData);
            if (SHGetValue(hKey, L"", L"", &type, (void*)pvData, &cbData) == ERROR_SUCCESS)
                value.assign(pvData);
            free(pvData);
        }
        RegCloseKey(hKey);
    }
}

static bool isMAPIClientAssigned()
{
    std::wstring client;
    regValue(HKEY_CURRENT_USER, client);
    if (!client.empty())
        return true;
    regValue(HKEY_LOCAL_MACHINE, client);
    return !client.empty();
}

static bool assignMAPIClient(const std::wstring &client)
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT(REG_MAIL_CLIENTS), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        LSTATUS result = SHSetValue(hKey, L"", NULL, REG_SZ, (const BYTE*)client.c_str(), (DWORD)(client.length() + 1) * sizeof(WCHAR));
        RegCloseKey(hKey);
        return result == ERROR_SUCCESS;
    }
    return false;
}

static void getMailClients(std::vector<std::wstring> &clients)
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(REG_MAIL_CLIENTS), 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS) {
        DWORD cSubKeys = 0, cbMaxSubKeyLen = 0;
        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, &cbMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            if (cSubKeys > 0 && cbMaxSubKeyLen > 0) {
                WCHAR *lpSubKeyName = new WCHAR[cbMaxSubKeyLen + 1];
                for (DWORD i = 0; i < cSubKeys; i++) {
                    if (RegEnumKey(hKey, i, lpSubKeyName, cbMaxSubKeyLen + 1) == ERROR_SUCCESS) {
                        if (lpSubKeyName[0] != '\0' && wcscmp(lpSubKeyName, L"Hotmail") != 0) {
                            clients.emplace_back(lpSubKeyName);
                        }
                    }
                }
                delete[] lpSubKeyName;
            }
        }
        RegCloseKey(hKey);
    }
}

static std::wstring selectClient(std::vector<std::wstring> &clients)
{
    std::wstring caption = QString("  %1").arg(WINDOW_TITLE).toStdWString();
    std::wstring mainText = QObject::tr("Select default email client", "CMailMessage").toStdWString();
    std::wstring okBtnText = BTN_TEXT_OK.toStdWString();
    std::wstring cancelBtnText = BTN_TEXT_CANCEL.toStdWString();

    constexpr uint cButtons = 2;
    TASKDIALOG_BUTTON pButtons[cButtons];
    pButtons[0] = {IDOK, okBtnText.c_str()};
    pButtons[1] = {IDCANCEL, cancelBtnText.c_str()};

    TASKDIALOG_BUTTON *pRadioBtns = new TASKDIALOG_BUTTON[clients.size()];
    constexpr int dfltRadioId = 0;
    for (int i = 0; i < clients.size(); i++) {
        pRadioBtns[i].nButtonID = dfltRadioId + i;
        pRadioBtns[i].pszButtonText = clients[i].c_str();
    }

    TASKDIALOGCONFIG cfg = { 0 };
    cfg.cbSize              = sizeof(cfg);
    cfg.dwFlags             = TDF_POSITION_RELATIVE_TO_WINDOW |
                              TDF_ALLOW_DIALOG_CANCELLATION |
                              TDF_SIZE_TO_CONTENT;
    if (AscAppManager::isRtlEnabled())
        cfg.dwFlags |= TDF_RTL_LAYOUT;
    cfg.hwndParent          = GetForegroundWindow();
    cfg.hInstance           = GetModuleHandle(NULL);
    cfg.pButtons            = pButtons;
    cfg.cButtons            = cButtons;
    cfg.nDefaultButton      = IDOK;
    cfg.pszMainIcon         = TD_SHIELD_ICON;
    cfg.pszWindowTitle      = caption.c_str();
    cfg.pszMainInstruction  = mainText.c_str();
    cfg.pszContent          = NULL;
    cfg.pRadioButtons       = pRadioBtns;
    cfg.cRadioButtons       = clients.size();
    cfg.nDefaultRadioButton = dfltRadioId;

    int result = 0, selRadio = 0;
    TaskDialogIndirect(&cfg, &result, &selRadio, nullptr);
    delete[] pRadioBtns;
    if (result == IDOK) {
        try {
            return clients.at(selRadio);
        } catch (...) {}
    }
    return L"";
}
#else
static std::string getFormattedDate()
{
    std::time_t now = std::time(nullptr);
    std::tm local_tm, utc_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &now);
    gmtime_s(&utc_tm, &now);
#else
    localtime_r(&now, &local_tm);
    gmtime_r(&now, &utc_tm);
#endif
    int offsetSec = static_cast<int>(std::difftime(std::mktime(&local_tm), std::mktime(&utc_tm)));
    int hrs = offsetSec / 3600;
    int min = std::abs(offsetSec % 3600) / 60;
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%a, %d %b %Y %H:%M:%S")
        << " " << (hrs >= 0 ? "+" : "-")
        << std::setw(2) << std::setfill('0') << std::abs(hrs)
        << std::setw(2) << std::setfill('0') << min;
    return oss.str();
}
#endif

static std::string getTempFileName(const std::string &extension)
{
    static int id = 0;
    char buff[L_tmpnam];
    std::string tmp_name(std::tmpnam(buff));
    tmp_name.append(std::to_string(++id));
    tmp_name.append(extension);
    return tmp_name;
}

static bool writeFile(const std::string &filePath, const std::string &data)
{
    std::ofstream file(filePath, std::ios::out);
    if (file.is_open()) {
        file.write(data.c_str(), data.length());
        if (file.fail()) {
            file.close();
            return false;
        }
        file.close();
        return true;
    }
    return false;
}

class CMailMessage::CMailMessagePrivate
{
public:
    CMailMessagePrivate()
    {}
    ~CMailMessagePrivate()
    {
        while (!tmp_files.empty()) {
            std::remove(tmp_files.top().c_str());
            tmp_files.pop();
        }
    }
#ifdef WIN32_USING_MAPI
    bool sendMailMAPI(std::string to, std::string subject, std::string msg)
    {
        to.insert(0, "SMTP:");
        if (HMODULE lib = LoadLibrary(L"mapi32.dll")) {
            ULONG (WINAPI *_MAPISendMail)(LHANDLE, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
            *(FARPROC*)&_MAPISendMail = GetProcAddress(lib, "MAPISendMail");
            if (_MAPISendMail) {
                std::string tmp_name = getTempFileName(".html");
                if (!writeFile(tmp_name, msg)) {
                    FreeLibrary(lib);
                    return false;
                }
                tmp_files.push(tmp_name);

                MapiRecipDesc recip[1] = { {0} };
                recip[0].ulRecipClass = MAPI_TO;
                recip[0].lpszAddress = &to[0];
                recip[0].lpszName = &to[0];

                std::string fileName = ""; // Forces HTML attachment to be rendered as email body

                MapiFileDesc mapiFile[1] = { {0} };
                mapiFile[0].nPosition = (ULONG)-1;
                mapiFile[0].lpszPathName = &tmp_name[0];
                mapiFile[0].lpszFileName = &fileName[0];

                std::string msgType = "IPM.Note";

                MapiMessage mapiMsg = { 0 };
                mapiMsg.lpszMessageType = &msgType[0];
                mapiMsg.lpRecips = recip;
                mapiMsg.nRecipCount = 1;
                mapiMsg.lpszSubject = &subject[0];
                mapiMsg.lpszNoteText = NULL;
                mapiMsg.lpFiles = mapiFile;
                mapiMsg.nFileCount = 1;
                mapiMsg.ulReserved = CP_UTF8;

                ULONG nSent = _MAPISendMail(NULL, (ULONG_PTR)HWND_DESKTOP, &mapiMsg, MAPI_LOGON_UI, 0);
                FreeLibrary(lib);
                return (nSent == SUCCESS_SUCCESS || nSent == MAPI_E_USER_ABORT);
            }
            FreeLibrary(lib);
        }
        return false;
    }
#else
    bool openEML(const std::string &to, const std::string &subject, const std::string &msg)
    {
        std::ostringstream data;
        data << "From: " << /*from <<*/ "\n"
             << "To: " << to << "\n"
             << "Subject: " << subject << "\n"
             << "Date: " << getFormattedDate() << "\n"
             << "X-Unsent: 1\n"
             << "MIME-Version: 1.0\n"
             << "Content-Type: text/html; charset=UTF-8\n"
             << "\n" << msg << "\n";

        std::string tmp_name = getTempFileName(".eml");
        if (writeFile(tmp_name, data.str())) {
            tmp_files.push(tmp_name);
#ifdef __APPLE__

#else
# ifdef _WIN32
            std::replace(tmp_name.begin(), tmp_name.end(), '\\', '/');
# endif
            return Utils::openUrl(QString::fromStdString(tmp_name));
#endif
        }
        return false;
    }
#endif

    std::stack<std::string> tmp_files;
};

CMailMessage::CMailMessage() :
    pimpl(new CMailMessagePrivate)
{}

CMailMessage::~CMailMessage()
{
    delete pimpl, pimpl = nullptr;
}

CMailMessage &CMailMessage::instance()
{
    static CMailMessage inst;
    return inst;
}

#ifdef WIN32_USING_MAPI
int CMailMessage::checkMAPIClient()
{
    if (!isMAPIClientAssigned()) {
        std::vector<std::wstring> clients;
        getMailClients(clients);
        if (clients.empty()) {
            return MAPIClientEmpty;
        }
        auto client = selectClient(clients);
        if (client.empty()) {
            return MapiClientCancel;
        }
        if (!assignMAPIClient(client)) {
            return MAPIClientError;
        }
    }
    return MAPIClientOK;
}
#endif

bool CMailMessage::sendMail(const std::string &to, const std::string &subject, const std::string &msg)
{
#ifdef WIN32_USING_MAPI
    return pimpl->sendMailMAPI(to, subject, msg);
#else
    return pimpl->openEML(to, subject, msg);
#endif
}
