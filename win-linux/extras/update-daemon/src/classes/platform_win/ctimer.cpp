/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "ctimer.h"
#include <Windows.h>
#include <unordered_map>


class CTimer::CTimerPrivate
{
public:
    CTimerPrivate() = default;
    ~CTimerPrivate() = default;

    static VOID CALLBACK timerProc(HWND, UINT, UINT_PTR id, DWORD)
    {
        auto it = timerMap.find(id);
        if (it != timerMap.end())
        {
            auto* pimpl = it->second;
            if (pimpl->callback)
                pimpl->callback();
        }
    }

    // Maps Win32 timer IDs to CTimerPrivate instances.
    // Required because SetTimer() does not provide a user data pointer to timerProc.
    static std::unordered_map<UINT_PTR, CTimerPrivate*> timerMap;
    std::function<void(void)> callback;
    UINT_PTR timerId = 0;
};

std::unordered_map<UINT_PTR, CTimer::CTimerPrivate*> CTimer::CTimerPrivate::timerMap = {};


CTimer::CTimer()
    : pimpl(new CTimerPrivate)
{

}

CTimer::~CTimer()
{
    stop();
    delete pimpl; pimpl = nullptr;
}

bool CTimer::isActive() const
{
    return pimpl->timerId != 0;
}

void CTimer::stop()
{
    if (pimpl->timerId != 0)
    {
        KillTimer(nullptr, pimpl->timerId);
        CTimerPrivate::timerMap.erase(pimpl->timerId);
        pimpl->timerId = 0;
    }
}

bool CTimer::start(unsigned int timeout, std::function<void(void)> callback)
{
    stop();
    pimpl->callback = std::move(callback);
    pimpl->timerId = SetTimer(nullptr, 0, timeout, CTimerPrivate::timerProc);

    if (pimpl->timerId == 0)
        return false;

    CTimerPrivate::timerMap.emplace(pimpl->timerId, pimpl);
    return true;
}
