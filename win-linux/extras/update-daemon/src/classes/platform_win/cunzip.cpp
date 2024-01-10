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
#include "platform_win/utils.h"
#include <atlbase.h>
#include <Shldisp.h>


class CUnzip::CUnzipPrivate
{
public:
    CUnzipPrivate()
    {}
    ~CUnzipPrivate()
    {}

    int extractRecursively(IShellDispatch *pISD, const CComPtr<Folder> &pSrcFolder, const wstring &destFolder)
    {
        CComPtr<FolderItems> pItems;
        if (FAILED(pSrcFolder->Items(&pItems)))
            return UNZIP_ERROR;

        long itemCount = 0;
        if (FAILED(pItems->get_Count(&itemCount)))
            return UNZIP_ERROR;

        for (int i = 0; i < itemCount; i++) {
            if (!run)
                return UNZIP_ABORT;

            CComPtr<FolderItem> pItem;
            if (FAILED(pItems->Item(CComVariant(i), &pItem)))
                return UNZIP_ERROR;

            VARIANT_BOOL isFolder = VARIANT_FALSE;
            if (FAILED(pItem->get_IsFolder(&isFolder)))
                return UNZIP_ERROR;

            if (isFolder == VARIANT_TRUE) {
                // Source path
                CComPtr<Folder> pSubFolder;
                if (FAILED(pISD->NameSpace(CComVariant(pItem), &pSubFolder)))
                    return UNZIP_ERROR;

                // Dest path
                BSTR bstrName;
                if (FAILED(pItem->get_Name(&bstrName)))
                    return UNZIP_ERROR;

                wstring targetFolder = destFolder + L"\\" + bstrName;
                SysFreeString(bstrName);
                if (CreateDirectory(targetFolder.c_str(), NULL) == 0)
                    return UNZIP_ERROR;

                int res = extractRecursively(pISD, pSubFolder, targetFolder);
                if (res != UNZIP_OK)
                    return res;

            } else {
                CComPtr<Folder> pDestFolder;
                if (FAILED(pISD->NameSpace(CComVariant(destFolder.c_str()), &pDestFolder)))
                    return UNZIP_ERROR;
                if (FAILED(pDestFolder->CopyHere(CComVariant(pItem), CComVariant(1024 | 512 | 16 | 4))))
                    return UNZIP_ERROR;
            }
        }
        return UNZIP_OK;
    }

    int unzipArchive(const wstring &zipFilePath, const wstring &folderPath)
    {
        if (!NS_File::fileExists(zipFilePath) || !NS_File::dirExists(folderPath))
            return UNZIP_ERROR;

        wstring file = NS_File::toNativeSeparators(zipFilePath);
        wstring path = NS_File::toNativeSeparators(folderPath);

        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            return UNZIP_ERROR;

        IShellDispatch *pShell = NULL;
        hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShell));
        if (FAILED(hr)) {
            CoUninitialize();
            return UNZIP_ERROR;
        }

        CComPtr<Folder> pSrcFolder;
        if (FAILED(pShell->NameSpace(CComVariant(file.c_str()), &pSrcFolder))) {
            pShell->Release();
            CoUninitialize();
            return UNZIP_ERROR;
        }
        int res = extractRecursively(pShell, pSrcFolder, path);
        pSrcFolder.Release();
        pShell->Release();
        CoUninitialize();
        return res;
    }

    FnVoidInt complete_callback = nullptr;
    std::atomic_bool run;
    std::future<void> future;
};

CUnzip::CUnzip() :
    pimpl(new CUnzipPrivate)
{
    pimpl->run = false;
}

CUnzip::~CUnzip()
{
    pimpl->run = false;
    if (pimpl->future.valid())
        pimpl->future.wait();
    delete pimpl, pimpl = nullptr;
}

void CUnzip::extractArchive(const wstring &zipFilePath, const wstring &folderPath)
{
    pimpl->run = false;
    if (pimpl->future.valid())
        pimpl->future.wait();
    pimpl->run = true;
    pimpl->future = std::async(std::launch::async, [=]() {
        int res = pimpl->unzipArchive(zipFilePath, folderPath);
        if (pimpl->complete_callback)
            pimpl->complete_callback(res);
    });
}

void CUnzip::stop()
{
    pimpl->run = false;
}

void CUnzip::onComplete(FnVoidInt callback)
{
    pimpl->complete_callback = callback;
}
