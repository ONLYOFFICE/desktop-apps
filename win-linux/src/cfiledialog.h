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

#ifndef CFILEDIALOG_H
#define CFILEDIALOG_H

#include <QObject>
#include <QMap>

#ifdef _WIN32
#include "win/qwinwidget.h"

class CFileDialogWrapper : public QWinWidget
{
    Q_OBJECT

public:
    explicit CFileDialogWrapper(HWND);
#else
class CFileDialogWrapper : public QObject
{
    Q_OBJECT

public:
    explicit CFileDialogWrapper(QWidget * p = 0);
#endif
    ~CFileDialogWrapper();

    bool    modalSaveAs(QString&);

//    QString modalOpen(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR);
    QStringList modalOpen(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR, bool multi = false);
    QString     modalOpenSingle(const QString&, const QString& filter = QString(), QString * selectedFilter = Q_NULLPTR);

    QString     modalOpenImage(const QString&);
    QStringList modalOpenImage(const QString&, bool list);
    QStringList modalOpenImages(const QString&);
    QString     modalOpenPlugin(const QString&);
    QStringList modalOpenPlugin(const QString&, bool list);
    QStringList modalOpenPlugins(const QString&);
    QStringList modalOpenAny(const QString&, bool multi = false);

    void    setFormats(std::vector<int>&);
    int     getFormat();

private:
    QString getFilter(const QString&) const;
    int getKey(const QString &value);
    QString joinFilters() const;

    QString m_filters;
    QMap<int, QString> m_mapFilters;
    int m_format;

signals:

public slots:
};

#endif // CFILEDIALOG_H
