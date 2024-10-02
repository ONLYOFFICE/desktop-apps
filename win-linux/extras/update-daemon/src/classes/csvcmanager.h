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

#ifndef CSVCMANAGER_H
#define CSVCMANAGER_H

#include "classes/csocket.h"
#include <future>
#ifdef _WIN32
# include "classes/platform_win/cdownloader.h"
# include "classes/platform_win/cunzip.h"
#else
# include "classes/platform_linux/cdownloader.h"
# include "classes/platform_linux/cunzip.h"
#endif

typedef std::function<void(void)> FnVoidVoid;
using std::future;


class CSvcManager
{
public:
    explicit CSvcManager();
    ~CSvcManager();

    /* callback */
    void aboutToQuit(FnVoidVoid callback);

private:
    void init();
    void onQueryResponse(const int error, const int lenght);
    void onCompleteUnzip(const int error);
    void onCompleteSlot(const int error, const tstring &filePath);
    void onProgressSlot(const int percent);
    void unzipIfNeeded(const tstring &filePath, const tstring &newVersion);
    void clearTempFiles(const tstring &prefix, const tstring &except = tstring());
    void startReplacingFiles(const tstring &packageType, const bool restartAfterUpdate);
    void startReplacingService(const bool restartAfterUpdate);
#ifdef _WIN32
    void startInstallPackage();
#endif

    struct PackageData;
    struct SavedPackageData;
    PackageData      *m_packageData;
    SavedPackageData *m_savedPackageData;

    FnVoidVoid   m_quit_callback = nullptr;
    tstring      m_checkUrl,
                 m_currVersion,
                 m_ignVersion,
                 m_newVersion;
    bool         m_lock = false;
    int          m_downloadMode,
                 m_packageType;
    future<void> m_future_clear;
    CSocket     *m_socket = nullptr;
    CDownloader *m_pDownloader = nullptr;
    CUnzip      *m_pUnzip = nullptr;

    enum Mode {
        CHECK_UPDATES=0, DOWNLOAD_CHANGELOG=1, DOWNLOAD_UPDATES=2
    };
    enum Package {
        ISS = 0, MSI, Portable, Other
    };
};

#endif // CSVCMANAGER_H
