/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 */

#ifndef CUPSAVAILABILITY_H
#define CUPSAVAILABILITY_H

#include <QDir>
#include <QFile>

#ifdef __linux__
inline bool cupsLocalSocketAvailable()
{
    return QFile::exists(QStringLiteral("/run/cups/cups.sock"))
        || QFile::exists(QStringLiteral("/var/run/cups/cups.sock"));
}

inline bool cupsRemoteServerConfigured()
{
    if (qEnvironmentVariableIsSet("CUPS_SERVER"))
        return true;

    QFile file(QDir::homePath() + QStringLiteral("/.cups/client.conf"));
    if (!file.open(QIODevice::ReadOnly))
        return false;

    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.startsWith(QLatin1String("ServerName")) || line.startsWith(QLatin1String("ServerHostname")))
            return true;
    }
    return false;
}
#endif

#endif // CUPSAVAILABILITY_H
