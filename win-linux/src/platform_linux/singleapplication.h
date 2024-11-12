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

#ifndef SINGLE_APPLICATION_H
#define SINGLE_APPLICATION_H

#include <QApplication>
#include <QByteArray>
#pragma push_macro("signals")
#undef signals
#include <gio/gio.h>
#pragma pop_macro("signals")


class CSocket;
class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    explicit SingleApplication( int &argc, char *argv[]);
    ~SingleApplication();

    bool isPrimary();
    bool sendMessage(const QByteArray&);

signals:
    void receivedMessage(QByteArray message);

private slots:
    void invokeSignal(const QString&);

private:
    CSocket *m_socket = nullptr;
    GDBusConnection *m_conn = nullptr;
    std::string m_client_id;
    uint    m_subscrId = 0;
    bool    m_isPrimary = false;
};

#endif // SINGLE_APPLICATION_H
