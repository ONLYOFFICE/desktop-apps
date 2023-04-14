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


bool StringToFolder(CComPtr<IShellDispatch> &pISD, CComPtr<Folder> &folder, const wstring &path)
{
    CComVariant vPath(CComBSTR(path.c_str()));
    vPath.ChangeType(VT_BSTR);
    HRESULT hr = pISD->NameSpace(vPath, &folder);
    return FAILED(hr) ? false : true;
}

int extractRecursively(CComPtr<IShellDispatch> &pISD, const CComPtr<Folder> &pSrcFolder,
                         const wstring &destFolder, CComVariant &vOptions, std::atomic_bool &run)
{
    CComPtr<FolderItems> pItems;
    HRESULT hr = pSrcFolder->Items(&pItems);
    if (FAILED(hr))
        return UNZIP_ERROR;

    long itemCount = 0;
    hr = pItems->get_Count(&itemCount);
    if (FAILED(hr))
        return UNZIP_ERROR;

    for (int i = 0; i < itemCount; i++) {
        if (!run)
            return UNZIP_ABORT;

        CComPtr<FolderItem> pItem;
        hr = pItems->Item(CComVariant(i), &pItem);
        if (FAILED(hr))
            return UNZIP_ERROR;

        CComBSTR srcPath;
        hr = pItem->get_Path(&srcPath);
        if (FAILED(hr))
            return UNZIP_ERROR;

        CComVariant vSrcPath(srcPath);
        vSrcPath.ChangeType(VT_BSTR);

        VARIANT_BOOL isFolder = VARIANT_FALSE;
        hr = pItem->get_IsFolder(&isFolder);
        if (FAILED(hr))
            return UNZIP_ERROR;

        if (isFolder == VARIANT_TRUE) {
            // Source path
            CComPtr<Folder> pSubFolder;
            hr = pISD->NameSpace(vSrcPath, &pSubFolder);
            if (FAILED(hr))
                return UNZIP_ERROR;

            // Dest path
            CComBSTR bstrName;
            hr = pItem->get_Name(&bstrName);
            if (FAILED(hr))
                return UNZIP_ERROR;

            wstring targetFolder(destFolder);
            targetFolder += L"\\";
            targetFolder += bstrName;
            if (CreateDirectory(targetFolder.c_str(), NULL) == 0)
                return UNZIP_ERROR;

            int res = extractRecursively(pISD, pSubFolder, targetFolder, vOptions, run);
            if (res != UNZIP_OK)
                return res;

        } else {
            CComPtr<Folder> pDestFolder;
            if (!StringToFolder(pISD, pDestFolder, destFolder))
                return UNZIP_ERROR;
            hr = pDestFolder->CopyHere(vSrcPath, vOptions);
            if (FAILED(hr))
                return UNZIP_ERROR;
        }
    }
    return UNZIP_OK;
}

int unzipArchive(const wstring &zipFilePath, const wstring &folderPath, std::atomic_bool &run)
{
    if (!NS_File::fileExists(zipFilePath) || !NS_File::dirExists(folderPath))
        return UNZIP_ERROR;

    wstring file = NS_File::toNativeSeparators(zipFilePath);
    wstring path = NS_File::toNativeSeparators(folderPath);

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return UNZIP_ERROR;

    CComPtr<IShellDispatch> pShell;
    hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShell));
    if (FAILED(hr)) {
        CoUninitialize();
        return UNZIP_ERROR;
    }

    CComPtr<Folder> pSrcFolder;
    if (!StringToFolder(pShell, pSrcFolder, file)) {
        CoUninitialize();
        return UNZIP_ERROR;
    }

    CComVariant vOptions(0);
    vOptions.vt = VT_I4;
    vOptions.lVal = 1024 | 512 | 16 | 4;
    int res = extractRecursively(pShell, pSrcFolder, path, vOptions, run);
    CoUninitialize();
    return res;
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

void CUnzip::extractArchive(const wstring &zipFilePath, const wstring &folderPath)
{
    m_run = false;
    if (m_future.valid())
        m_future.wait();
    m_run = true;
    m_future = std::async(std::launch::async, [=]() {
        int res = unzipArchive(zipFilePath, folderPath, m_run);
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
