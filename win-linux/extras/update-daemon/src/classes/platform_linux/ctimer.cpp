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
#include <glib.h>


class CTimer::CTimerPrivate
{
public:
    CTimerPrivate() = default;
    ~CTimerPrivate() = default;

    static gboolean timerCallback(gpointer data)
    {
        auto* pimpl = static_cast<CTimerPrivate*>(data);
        if (pimpl->callback)
            pimpl->callback();

        return G_SOURCE_CONTINUE;
    }

    std::function<void(void)> callback;
    guint timerId = 0;
};


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
        g_source_remove(pimpl->timerId);
        pimpl->timerId = 0;
    }
}

bool CTimer::start(unsigned int timeout, std::function<void(void)> callback)
{
    stop();
    pimpl->callback = std::move(callback);    
    pimpl->timerId = g_timeout_add(timeout, CTimerPrivate::timerCallback, pimpl);
    return pimpl->timerId != 0;
}
