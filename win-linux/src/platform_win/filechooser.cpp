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

#include <QDir>
#include <QSettings>
#ifndef __OS_WIN_XP
# include <Shobjidl.h>
#endif
#include "filechooser.h"

using std::wstring;
#ifndef __OS_WIN_XP
using std::vector;
using std::pair;
typedef vector<IShellItem *> VectorShellItems;
typedef vector<pair<wstring, wstring>> specvector;

auto itemsFromItemArray(IShellItemArray * items)
{
    VectorShellItems out;
    DWORD itemCount = 0;
    if (FAILED(items->GetCount(&itemCount)) || itemCount == 0)
        return out;

    out.reserve(itemCount);
    for (DWORD i = 0; i < itemCount; ++i) {
        IShellItem * item = nullptr;
        if (SUCCEEDED(items->GetItemAt(i, &item)))
            out.push_back(item);
    }

    return out;
}

auto stringToFilters(const wstring& wstr) -> specvector {
    specvector v;

    if ( !wstr.empty() ) {
        auto _parce_filter_string = [](const wstring& fullstr, size_t start, size_t stop) -> pair<wstring, wstring> {
            wstring filter_name = fullstr.substr(start, stop - start);
            size_t mid = filter_name.find(L"(") + 1;
            wstring filter_pattern = filter_name.substr(mid, filter_name.find(L")", mid) - mid);

            std::replace(begin(filter_pattern), end(filter_pattern), ' ', ';');
            return make_pair(filter_name, filter_pattern);
        };

        wstring _delim = L";;";
        size_t _curr = wstr.find(_delim),
            _prev = 0;
        while ( _curr != wstring::npos ) {
            v.push_back(_parce_filter_string(wstr,_prev,_curr));

            _prev = _curr + 2;
            _curr = wstr.find(_delim, _prev);
        }

        v.push_back(_parce_filter_string(wstr, _prev, wstr.size()));
    }

    return v;
}
#endif

void nativeFileDialog(HWND parent_hwnd,
                      Win::Mode mode,
                      QStringList &filenames,
                      const wchar_t* title,
                      const wstring &file,
                      const wstring &path,
                      const wstring &flt,
                      QString* sel_filter,
                      bool sel_multiple)
{
#ifndef __OS_WIN_XP
    const CLSID rclsid = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
            CLSID_FileOpenDialog : CLSID_FileSaveDialog;
    const IID riid = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
            IID_IFileOpenDialog : IID_IFileSaveDialog;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog * pOpenDialog = nullptr;
        IFileSaveDialog * pSaveDialog = nullptr;
        LPVOID *ppv = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                reinterpret_cast<void**>(&pOpenDialog) : reinterpret_cast<void**>(&pSaveDialog);
        hr = CoCreateInstance(rclsid,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              riid,
                              ppv);

        if (SUCCEEDED(hr)) {
            hr = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                    pOpenDialog->SetTitle(title) : pSaveDialog->SetTitle(title);

            FILEOPENDIALOGOPTIONS dwFlags = FOS_PATHMUSTEXIST;
            hr = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                pOpenDialog->GetOptions(&dwFlags) : pSaveDialog->GetOptions(&dwFlags);
            if (SUCCEEDED(hr)) {
                if (sel_multiple && mode != Win::Mode::SAVE)
                    dwFlags |= FOS_ALLOWMULTISELECT;

                if (mode == Win::Mode::FOLDER)
                    dwFlags |= FOS_PICKFOLDERS;

                QSettings _r("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", QSettings::NativeFormat);
                if ( _r.value("Hidden", 1) == 1 )
                    dwFlags |= FOS_FORCESHOWHIDDEN;

                hr = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                        pOpenDialog->SetOptions(dwFlags | FOS_FILEMUSTEXIST) :
                        pSaveDialog->SetOptions(dwFlags & !FOS_OVERWRITEPROMPT);
            }

            specvector filters{stringToFilters(flt)};
            uint typeIndex = 1;
            COMDLG_FILTERSPEC * specOpenTypes = new COMDLG_FILTERSPEC[filters.size()];
            for (uint i{0}; i < filters.size(); ++i) {
                specvector::const_reference iter = filters.at(i);
                specOpenTypes[i] = {iter.first.c_str(), iter.second.c_str()};
                if (sel_filter) {
                    if ( !sel_filter->isEmpty() && iter.first.find(sel_filter->toStdWString()) == 0 )
                        typeIndex = i + 1;
                }
            }

            hr = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                pOpenDialog->SetFileTypes(filters.size(), specOpenTypes) :
                pSaveDialog->SetFileTypes(filters.size(), specOpenTypes);
            delete [] specOpenTypes;
            (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                    pOpenDialog->SetFileTypeIndex(typeIndex) :
                    pSaveDialog->SetFileTypeIndex(typeIndex);

            if (!path.empty()) {
                IShellItem * pItem = nullptr;
                hr = SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (SUCCEEDED(hr)) {
                    (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                            pOpenDialog->SetFolder(pItem) : pSaveDialog->SetFolder(pItem);
                    pItem->Release();
                }
            }

            if (!file.empty() && mode == Win::Mode::SAVE) {
                pSaveDialog->SetFileName(file.c_str());
                pSaveDialog->SetDefaultExtension(L"");
            }

            hr = (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                    pOpenDialog->Show(parent_hwnd) : pSaveDialog->Show(parent_hwnd);
            if (SUCCEEDED(hr)) {
                if (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) {
                    IShellItemArray * items = nullptr;
                    hr = pOpenDialog->GetResults(&items);
                    if (SUCCEEDED(hr) && items) {
                        VectorShellItems iarray = itemsFromItemArray(items);
                        for (IShellItem * item : iarray) {
                            PWSTR pszFilePath;
                            hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            if (SUCCEEDED(hr)) {
                                filenames.append(QString::fromStdWString(pszFilePath));
                                CoTaskMemFree(pszFilePath);
                            }
                        }

                        for (IShellItem * item : iarray)
                            item->Release();
                        items->Release();
                    }
                } else {
                    IShellItem * item;
                    hr = pSaveDialog->GetResult(&item);
                    if (SUCCEEDED(hr) && item) {
                        PWSTR pszFilePath;
                        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr)) {
                            filenames.append(QString::fromStdWString(pszFilePath).replace('\\', '/'));
                            CoTaskMemFree(pszFilePath);
                        }
                        item->Release();
                    }
                }
                if (mode != Win::Mode::FOLDER) {
                    uint typeIndex;
                    (mode == Win::Mode::OPEN) ? pOpenDialog->GetFileTypeIndex(&typeIndex) :
                                                pSaveDialog->GetFileTypeIndex(&typeIndex);
                    if (sel_filter && typeIndex > 0 && typeIndex <= filters.size()) {
                        specvector::const_reference iter = filters.at(typeIndex - 1);
                        *sel_filter = QString::fromStdWString(iter.first);
                    }
                }
            }
            (mode == Win::Mode::OPEN || mode == Win::Mode::FOLDER) ?
                    pOpenDialog->Release() : pSaveDialog->Release();
        }
        CoUninitialize();
    }
#else
#endif
}

QStringList Win::openWinFileChooser(QWidget *parent,
                                    Mode mode,
                                    const QString &title,
                                    const QString &file,
                                    const QString &path,
                                    const QString &filter,
                                    QString *sel_filter,
                                    bool sel_multiple)
{
    const int pos = file.lastIndexOf('/');
    QString _file = (pos != -1) ?
                file.mid(pos + 1) : file;
    const QString _path = (path.isEmpty() && pos != -1) ?
                file.mid(0, pos) : path;
    _file = _file.left(_file.lastIndexOf("."));

    QStringList filenames;
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;
    nativeFileDialog(parent_hwnd,
                     mode,
                     filenames,
                     title.toStdWString().c_str(),
                     _file.toStdWString(),
                     QDir::toNativeSeparators(_path).toStdWString(),
                     filter.toStdWString(),
                     sel_filter,
                     sel_multiple);

    return filenames;
}
