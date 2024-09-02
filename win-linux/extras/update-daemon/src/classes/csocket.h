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

#ifndef CSOCKET_H
#define CSOCKET_H

#include <string>
#include <vector>
#include <functional>
#ifdef _WIN32
# include <tchar.h>
# define tchar wchar_t
# define tstringstream std::wstringstream
# define tstring std::wstring
# define to_tstring to_wstring
#else
# define _T(str) str
# define tchar char
# define tstringstream std::stringstream
# define tstring std::string
# define to_tstring to_string
#endif

using std::size_t;

typedef std::function<void(void*, size_t)> FnVoidData;
typedef std::function<void(const char*)> FnVoidCharPtr;


enum MsgCommands {
    MSG_CheckUpdates = 0,
    MSG_LoadUpdates,
    MSG_LoadCheckFinished,
    MSG_LoadUpdateFinished,
    MSG_UnzipIfNeeded,
    MSG_ShowStartInstallMessage,
    MSG_StartReplacingFiles,
    MSG_ClearTempFiles,
    MSG_Progress,
    MSG_StopDownload,
    MSG_OtherError,
    MSG_RequestContentLenght,
    MSG_UnzipProgress,
    MSG_SetLanguage,
    MSG_StartReplacingService,
    MSG_StartInstallPackage
};

class CSocket
{
public:
    CSocket(int sender_port, int receiver_port, bool retry_connect = true, bool use_unique_addr = false);
    ~CSocket();

    /* callback */
    bool isPrimaryInstance();
    bool sendMessage(void *data, size_t size);
    bool sendMessage(int cmd, const tstring &param1 = _T(""), const tstring &param2 = _T(""));
    void onMessageReceived(FnVoidData callback);
    void onError(FnVoidCharPtr callback);
    int  parseMessage(void *data, std::vector<tstring> &params);

private:
    class CSocketPrv;
    CSocketPrv *pimpl = nullptr;
};

#endif // CSOCKET_H
