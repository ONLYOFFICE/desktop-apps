#ifndef JUMPLIST_H
#define JUMPLIST_H

#define NTDDI_VERSION NTDDI_WIN7
#define WIN32_LEAN_AND_MEAN
#define STRICT_TYPED_ITEMIDS
#define ICON_OFFSET 14
#define MIN_TASK_NUM 3
#define MAX_TASK_NUM 4

#include <QtGlobal>
#include <QStringList>
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <strsafe.h>

#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>


HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl, int index)
{
    IShellLink *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        WCHAR szAppPath[MAX_PATH];
        if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)))
        {
            hr = psl->SetPath(szAppPath);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetArguments(pszArguments);
                if (SUCCEEDED(hr))
                {
                    hr = psl->SetIconLocation(szAppPath, index + ICON_OFFSET);
                    Q_UNUSED(hr);
                    IPropertyStore *pps;
                    hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT propvar;
                        hr = InitPropVariantFromString(pszTitle, &propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->SetValue(PKEY_Title, propvar);
                            if (SUCCEEDED(hr))
                            {
                                hr = pps->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                                }
                            }
                            PropVariantClear(&propvar);
                        }
                        pps->Release();
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        psl->Release();
    }
    return hr;
}

HRESULT _AddTasksToList(ICustomDestinationList *pcdl, QStringList list)
{
    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        PCWSTR args[] = {
            L"--new:word",
            L"--new:cell",
            L"--new:slide",
            L"--new:form"
        };

        IShellLink * psl;
        for (int i = 0; i < MAX_TASK_NUM && i < list.size(); i++) {
            hr = _CreateShellLink(args[i], list[i].replace('_', ' ').toStdWString().c_str(), &psl, i);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            IObjectArray * poa;
            hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
            if (SUCCEEDED(hr))
            {
                hr = pcdl->AddUserTasks(poa);
                poa->Release();
            }
        }
        poc->Release();
    }
    return hr;
}

void CreateJumpList(const QStringList &list)
{
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr)) {
        UINT cMinSlots;
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
        if (SUCCEEDED(hr))
        {
            hr = _AddTasksToList(pcdl, list);
            if (SUCCEEDED(hr))
            {
                hr = pcdl->CommitList();
            }
        }
        poaRemoved->Release();
        pcdl->Release();
    }
    Q_UNUSED(hr);
}

void DeleteJumpList()
{
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr)) {
        hr = pcdl->DeleteList(NULL);
        pcdl->Release();
    }
    Q_UNUSED(hr);
}

#endif // JUMPLIST_H
