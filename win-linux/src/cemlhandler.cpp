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

#include "cemlhandler.h"
#include "defines.h"
#include "utils.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <QUuid>
#include <QDir>
#include <QStack>


static QString getFormattedDate()
{
    QDateTime now = QDateTime::currentDateTime();
    QTimeZone timeZone = QTimeZone::systemTimeZone();
    now = now.toTimeZone(timeZone);
    QString formattedDate = now.toString("ddd, dd MMM yyyy HH:mm:ss");

    int offsetSec = timeZone.offsetFromUtc(now);
    int hrs = offsetSec / 3600;
    int min = (offsetSec % 3600) / 60;
    QString offsetStr = QString("%1%2%3")
                            .arg(hrs >= 0 ? "+" : "-")
                            .arg(qAbs(hrs), 2, 10, QChar('0'))
                            .arg(qAbs(min), 2, 10, QChar('0'));
    return formattedDate + " " + offsetStr;
}

class CEmlHandler::CEmlHandlerPrivate
{
public:
    CEmlHandlerPrivate()
    {}
    ~CEmlHandlerPrivate()
    {
        while (!eml_paths.isEmpty())
            QFile::remove(eml_paths.pop());
    }

    QStack<QString> eml_paths;
};

CEmlHandler::CEmlHandler() :
    pimpl(new CEmlHandlerPrivate)
{}

CEmlHandler::~CEmlHandler()
{
    delete pimpl, pimpl = nullptr;
}

CEmlHandler &CEmlHandler::instance()
{
    static CEmlHandler inst;
    return inst;
}

void CEmlHandler::openEML(const QString &from, const QString &to, const QString &subject, const QString &msg)
{
    QString data(QString("From: %1\nTo: %2\nSubject: %3\n").arg(from, to, subject));
    data.append(QString("Date: %1\n").arg(getFormattedDate()));
    data.append("X-Unsent: 1\n");
    data.append("MIME-Version: 1.0\n");
    // data.append("Content-Type: text/plain; charset=UTF-8\n");
    data.append("\n");
    data.append(msg);
    data.append("\n");

    QString tmp_name = QDir::tempPath() + QString("/%1%2.eml").arg(FILE_PREFIX, QUuid::createUuid().toString().remove('{').remove('}'));
    if (Utils::writeFile(tmp_name, data.toUtf8())) {
        Utils::openUrl(tmp_name);
        pimpl->eml_paths.push(tmp_name);
    }
}
