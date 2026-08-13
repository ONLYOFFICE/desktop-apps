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

#include "ascdialog.h"
#include <QEventLoop>


CAscDialog::CAscDialog(QWidget *parent, const QRect &rc)
    : CWindowPlatform(parent, rc)
    , m_loop(new QEventLoop)
    , m_result(DialogCode::Rejected)
{
    m_ignoreMinSize = true;
    setWindowFlags(windowFlags() | Qt::Dialog);
    setMinimumSize(0, 0);
    setGeometry(adjustWindowRect(rc, m_dpiRatio));
}

CAscDialog::~CAscDialog()
{
    delete m_loop;
    m_loop = nullptr;
}

/** Public **/

void CAscDialog::setModal()
{
    setWindowModality(Qt::ApplicationModal);
}

void CAscDialog::accept(int result)
{
    m_result = result;
    m_loop->exit();
}

void CAscDialog::reject()
{
    m_result = DialogCode::Rejected;
    m_loop->exit();
}

int CAscDialog::exec()
{
    show(false);
    m_loop->exec();
    hide();
    return m_result;
}

/** Protected **/

bool CAscDialog::event(QEvent *ev)
{
    switch (ev->type())
    {
    case QEvent::Close:
    {
        if (m_loop->isRunning())
        {
            reject();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return CWindowPlatform::event(ev);
}

void CAscDialog::onLayoutDirectionChanged()
{}

/** Private **/

