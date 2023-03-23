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

#ifndef CUPDATEMANAGER_H
#define CUPDATEMANAGER_H

#include <QObject>
#include <QTimer>
#include <ctime>
#include "csocket.h"

using std::wstring;


enum UpdateMode {
    DISABLE=0, SILENT=1, ASK=2
};

class CUpdateManager: public QObject
{
    Q_OBJECT
public:
    explicit CUpdateManager(QObject *parent = nullptr);
    ~CUpdateManager();

    void setNewUpdateSetting(const QString& _rate);
    void cancelLoading();
    void skipVersion();
    int  getUpdateMode();
    QString getVersion() const;
    void scheduleRestartForUpdate();
    void handleAppClose();
    void loadUpdates();
    void installUpdates();

public slots:
    void checkUpdates();

signals:
    void progresChanged(const int percent);

private:
    void init();
    void clearTempFiles(const QString &except = QString());
    void updateNeededCheking();
    void onCheckFinished(bool error, bool updateExist, const QString &version, const QString &changelog);
    void unzipIfNeeded();
    void savePackageData(const QString &version = QString(), const QString &fileName = QString());
    bool sendMessage(int cmd, const wstring &param1 = L"null", const wstring &param2 = L"null",
                        const wstring &param3 = L"null");

    struct PackageData;
    struct SavedPackageData;
    PackageData      *m_packageData;
    SavedPackageData *m_savedPackageData;

    bool        m_restartForUpdate = false,
                m_lock = false;

//    QTimer      *m_pTimer = nullptr;
//    time_t      m_lastCheck;

    QTimer      *m_pCheckOnStartupTimer = nullptr;
    wstring     m_checkUrl;

    class DialogSchedule;
    DialogSchedule *m_dialogSchedule = nullptr;

    CSocket *m_socket = nullptr;

private slots:
    void onLoadCheckFinished(const QString &filePath);
    void showUpdateMessage(QWidget *parent);
    void onLoadUpdateFinished(const QString &filePath);
    void showStartInstallMessage(QWidget *parent);
    void onProgressSlot(const int percent);
    void onError(const QString &error);
    void criticalMsg(QWidget *parent, const QString &msg);
};

#endif // CUPDATEMANAGER_H
