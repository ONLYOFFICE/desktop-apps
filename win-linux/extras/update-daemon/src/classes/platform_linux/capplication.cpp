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

#include "capplication.h"
#include <glib.h>
#include <atomic>
#include <stdexcept>


struct CApplication::CApplicationPrivate
{
    std::atomic<bool> exiting{false};
    std::atomic<int> exitCode{0};
    GMainLoop *loop = nullptr;
    GMainContext *context = nullptr;
    static CApplication* instance;
};

CApplication* CApplication::CApplicationPrivate::instance = nullptr;

CApplication::CApplication()
    : pimpl(new CApplicationPrivate)
{
    if (CApplicationPrivate::instance)
        throw std::runtime_error("CApplication already exists");

    pimpl->context = g_main_context_default();
    if (!pimpl->context)
        throw std::runtime_error("Failed to get default GMainContext");

    pimpl->loop = g_main_loop_new(pimpl->context, FALSE);
    if (!pimpl->loop)
        throw std::runtime_error("Failed to create GLib main loop");

    CApplicationPrivate::instance = this;
}

CApplication::~CApplication()
{
    CApplicationPrivate::instance = nullptr;

    if (pimpl->loop)
        g_main_loop_unref(pimpl->loop);

    delete pimpl; pimpl = nullptr;
}

CApplication* CApplication::instance()
{
    return CApplicationPrivate::instance;
}

int CApplication::exec()
{
    g_main_loop_run(pimpl->loop);
    return pimpl->exitCode.load(std::memory_order_relaxed);
}

void CApplication::exit(int code)
{
    bool expected = false;
    if (!pimpl->exiting.compare_exchange_strong(expected, true))
        return;

    pimpl->exitCode.store(code, std::memory_order_relaxed);

    g_main_context_invoke(
        pimpl->context,
        +[](gpointer data) -> gboolean
        {
            auto* pimpl = static_cast<CApplicationPrivate*>(data);
            g_main_loop_quit(pimpl->loop);
            return G_SOURCE_REMOVE;
        },
        pimpl);
}
