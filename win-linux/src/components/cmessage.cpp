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

#include "cmessage.h"
#include "backend/cmessage_qt.h"
#include "utils.h"
#ifdef _WIN32
# ifndef __OS_WIN_XP
#  include "backend/cmessage_win32.h"
# endif
#else
# include "backend/cmessage_gtk.h"
#endif


CMessage::CMessage(QWidget *parent)
    : m_pQtBackend(nullptr)
    , m_pPlatformBackend(nullptr)
{
    if (WindowHelper::useNativeDialog())
    {
#ifdef _WIN32
# ifndef __OS_WIN_XP
        m_pPlatformBackend = new MessagePlatformBackend(parent);
# endif
#else
        m_pPlatformBackend = new MessagePlatformBackend(parent);
#endif
    }
    if (!m_pPlatformBackend)
        m_pQtBackend = new MessageQtBackend(parent);
}

CMessage::~CMessage()
{
    if (m_pQtBackend)
    {
        delete m_pQtBackend;
        m_pQtBackend = nullptr;
    }
    if (m_pPlatformBackend)
    {
        delete m_pPlatformBackend;
        m_pPlatformBackend = nullptr;
    }
}

int CMessage::showMessage(QWidget *parent, const QString &msg, Icon msgIcon,
                          Buttons msgBtns, const Config &conf)
{
    if (WindowHelper::useNativeDialog())
    {
#ifdef _WIN32
# ifndef __OS_WIN_XP
        return MessagePlatformBackend::showMessage(parent, msg, msgIcon, msgBtns, conf);
# endif
#else
        return MessagePlatformBackend::showMessage(parent, msg, msgIcon, msgBtns, conf);
#endif
    }
    return MessageQtBackend::showMessage(parent, msg, msgIcon, msgBtns, conf);
}

void CMessage::confirm(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, Icon::Confirm);
}

void CMessage::info(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, Icon::Info);
}

void CMessage::warning(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, Icon::Warning);
}

void CMessage::error(QWidget *parent, const QString &msg)
{
    showMessage(parent, msg, Icon::Error);
}

void CMessage::setButtons(std::initializer_list<QString> btns)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setButtons(btns)
        : m_pQtBackend->setButtons(btns);
}

void CMessage::setButtons(Buttons btns)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setButtons(btns)
        : m_pQtBackend->setButtons(btns);
}

void CMessage::setDefaultButton(DefaultButton btn)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setDefaultButton(btn)
        : m_pQtBackend->setDefaultButton(btn);
}

void CMessage::setIcon(Icon msgIcon)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setIcon(msgIcon)
        : m_pQtBackend->setIcon(msgIcon);
}

void CMessage::setText(const QString &text)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setText(text)
        : m_pQtBackend->setText(text);
}

void CMessage::setConfig(const Config &conf)
{
    m_pPlatformBackend
        ? m_pPlatformBackend->setConfig(conf)
        : m_pQtBackend->setConfig(conf);
}

int CMessage::exec()
{
    return m_pPlatformBackend
               ? m_pPlatformBackend->exec()
               : m_pQtBackend->exec();
}
