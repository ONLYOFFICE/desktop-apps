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

#include "singleapplication.h"
#include "utils.h"
#include <QThread>

#define RETRIES_DELAY_MS 100


SingleApplication::SingleApplication(int &argc, char *argv[], const QString&) :
    QApplication(argc, argv)
{
    m_socket = new CSocket(0, Utils::getInstAppPort(), false, true);
    if (m_socket->isPrimaryInstance()) {
        m_isPrimary = true;
        m_socket->onMessageReceived([=](void *buff, size_t size) {
            QString data = QString::fromLocal8Bit((const char*)buff, size);
            QMetaObject::invokeMethod(this, "invokeSignal", Qt::QueuedConnection, Q_ARG(QString, data));
        });
    }
}

SingleApplication::~SingleApplication()
{
    if (m_socket)
        delete m_socket, m_socket = nullptr;
}

bool SingleApplication::isPrimary()
{
    return m_isPrimary;
}

bool SingleApplication::sendMessage(const QByteArray &msg)
{
    if (m_isPrimary)
        return false;

    QThread::msleep(RETRIES_DELAY_MS);
    CSocket socket(Utils::getInstAppPort(), 0, false, true);
    return socket.sendMessage((void*)msg.data(), msg.size());
}

void SingleApplication::invokeSignal(const QString &data)
{
    emit receivedMessage(data.toUtf8());
}
