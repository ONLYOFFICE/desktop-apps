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

#include "cunzip.h"
#include "platform_linux/utils.h"
#include <archive.h>
#include <archive_entry.h>
#include <cstring>

#define BLOCK_SIZE 10240


int unzipArchive(const string &zipFilePath, const string &folderPath, std::atomic_bool &run, string &error)
{
    if (!NS_File::fileExists(zipFilePath) || !NS_File::dirExists(folderPath)) {
        error = "Archive path is empty or dest dir not exist";
        return UNZIP_ERROR;
    }

    struct archive *arch = archive_read_new();
    archive_read_support_filter_xz(arch);
    archive_read_support_format_tar(arch);
    if (archive_read_open_filename(arch, zipFilePath.c_str(), BLOCK_SIZE) != ARCHIVE_OK) {
        error = "Cannot open archive";
        archive_read_free(arch);
        return UNZIP_ERROR;
    }

    int res = ARCHIVE_OK;
    int ex_code = UNZIP_OK;
    struct archive_entry *entry;
    while ((res = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
        if (!run) {
            ex_code = UNZIP_ABORT;
            break;
        }

        const char *entryname = archive_entry_pathname(entry);
        if (!entryname) {
            error = "Invalid entry name";
            break;
        }

        char outpath[1024] = {0};
        snprintf(outpath, sizeof(outpath), "%s/%s", folderPath.c_str(), entryname);

        if (archive_entry_filetype(entry) == AE_IFREG) {
            archive_entry_set_pathname(entry, outpath);
            res = archive_read_extract(arch, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM
                                                       | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS
                                                       | ARCHIVE_EXTRACT_NO_OVERWRITE);
            if (res != ARCHIVE_OK) {
                error = "Cannot extract entry";
                break;
            }
        }
    }

    if (res != ARCHIVE_EOF && ex_code != UNZIP_ABORT) {
        error = string("Error reading archive: ") + archive_error_string(arch);
        ex_code = UNZIP_ERROR;
    }

    archive_read_close(arch);
    archive_read_free(arch);
    return ex_code;
}

CUnzip::CUnzip()
{
    m_run = false;
}

CUnzip::~CUnzip()
{
    m_run = false;
    if (m_future.valid())
        m_future.wait();
}

void CUnzip::extractArchive(const string &zipFilePath, const string &folderPath)
{
    m_run = false;
    if (m_future.valid())
        m_future.wait();
    m_run = true;
    m_future = std::async(std::launch::async, [=]() {
        string error;
        int res = unzipArchive(zipFilePath, folderPath, m_run, error);
        if (!error.empty())
            fprintf(stderr, "%s", error.c_str());
        if (m_complete_callback)
            m_complete_callback(res);
    });
}

void CUnzip::stop()
{
    m_run = false;
}

void CUnzip::onComplete(FnVoidInt callback)
{
    m_complete_callback = callback;
}
