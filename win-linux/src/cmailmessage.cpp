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
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <stack>
#ifdef __APPLE__

#else
# include "utils.h"
# ifdef _WIN32
#  include <mapi.h>
# endif
#endif


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
        while (!eml_paths.empty()) {
            std::remove(eml_paths.top().c_str());
            eml_paths.pop();
        }
    }

    std::stack<std::string> eml_paths;
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

void CMailMessage::openEML(const std::string &to, const std::string &subject, const std::string &msg)
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
#ifdef __APPLE__

#else
# ifdef _WIN32
        std::replace(tmp_name.begin(), tmp_name.end(), '\\', '/');
# endif
        Utils::openUrl(QString::fromStdString(tmp_name));
#endif
        pimpl->eml_paths.push(tmp_name);
    }
}

#ifdef _WIN32
bool CMailMessage::sendMailMAPI(std::string to, std::string subject, std::string msg)
{
    to.insert(0, "SMTP:");
    if (HMODULE lib = LoadLibrary(L"mapi32.dll")) {
        ULONG (WINAPI *_MAPISendMail)(LHANDLE, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
        *(FARPROC*)&_MAPISendMail = GetProcAddress(lib, "MAPISendMail");
        if (_MAPISendMail) {
            std::string tmp_name = getTempFileName(".html");
            if (!writeFile(tmp_name, msg))
                return false;
            pimpl->eml_paths.push(tmp_name);

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
            return (nSent == SUCCESS_SUCCESS || nSent == MAPI_E_USER_ABORT);
        }
        FreeLibrary(lib);
    }
    return false;
}
#endif
