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

#ifdef _WIN32
# include "utils.h"
# include "version.h"
#else
# include <libnotify/notify.h>
# include <unordered_map>
#endif
#include <QTextDocumentFragment>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#include "components/cnotification.h"
#include "defines.h"

#define NOTIF_TIMEOUT_MS 10000
#ifdef __linux__
# define toTStr(qstr)     qstr.toStdString()
#else
# define toTStr(qstr)     qstr.toStdWString()
#endif
#define TEXT_SKIP        toTStr(QObject::tr("Skip"))
#define TEXT_REMIND      toTStr(QObject::tr("Later"))
#define TEXT_INSTALL     toTStr(QObject::tr("Install"))
#define TEXT_INSLATER    toTStr(QObject::tr("Later"))
#define TEXT_RESTART     toTStr(QObject::tr("Restart"))
#define TEXT_SAVEANDINS  toTStr(QObject::tr("Install"))
#define TEXT_DOWNLOAD    toTStr(QObject::tr("Download"))

using namespace WinDlg;

#ifdef __linux__
# define addAction(action, label) notify_notification_add_action(ntf, action, label, NOTIFY_ACTION_CALLBACK(action_callback), (void*)&pimpl->ntfMap, NULL);

typedef std::unordered_map<NotifyNotification*, FnVoidInt> NtfMap;

static void action_callback(NotifyNotification *ntf, char *action, void *data)
{
    if (!data)
        return;
    int res = strcmp(action, "inslater") == 0 ? DLG_RESULT_INSLATER :
              strcmp(action, "restart") == 0 ?  DLG_RESULT_RESTART :
              strcmp(action, "skip") == 0 ?     DLG_RESULT_SKIP :
              strcmp(action, "remind") == 0 ?   DLG_RESULT_REMIND :
              strcmp(action, "install") == 0 ?  DLG_RESULT_INSTALL :
              strcmp(action, "saveins") == 0 ?  DLG_RESULT_INSTALL :
              strcmp(action, "download") == 0 ? DLG_RESULT_DOWNLOAD : -1;

    NtfMap *ntfMap = (NtfMap*)data;
    if (ntfMap->find(ntf) != ntfMap->end()) {
        if (FnVoidInt callback = ntfMap->at(ntf))
            callback(res);
    }
}

static void on_close(NotifyNotification *ntf, void *data)
{
    if (!data)
        return;
    NtfMap *ntfMap = (NtfMap*)data;
    auto it = ntfMap->find(ntf);
    if (it != ntfMap->end()) {
        g_object_unref(ntf);
        ntfMap->erase(it);
    }
}

static bool isNotificationsEnabled()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (conn.isConnected()) {
        QDBusInterface itf("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.DBus.Properties", conn);
        if (itf.isValid()) {
            QDBusMessage msg = itf.call("Get", "org.freedesktop.Notifications", "Inhibited");
            if (msg.type() == QDBusMessage::ReplyMessage && msg.arguments().size() > 0) {
                QVariant var = msg.arguments().at(0);
                if (var.canConvert<QDBusVariant>()) {
                    QVariant res = var.value<QDBusVariant>().variant();
                    if (res.type() == QVariant::Bool && !res.toBool())
                        return true;
                }
            }
        }
    }
    return false;
}
#else
# include "platform_win/wintoastlib.h"

using namespace WinToastLib;

class ToastHandler : public IWinToastHandler {
public:
    ToastHandler(DlgBtns _dlgBtns, FnVoidInt _callback) : dlgBtns(_dlgBtns), callback(_callback)
    {}
    void toastActivated() const // The user clicked in this toast
    {}
    void toastActivated(int actionIndex) const // The user clicked on button #actionIndex
    {
        int res = -1;
        switch (actionIndex) {
        case 0: res = (dlgBtns == DlgBtns::mbInslaterRestart) ? DLG_RESULT_INSLATER : DLG_RESULT_SKIP; break;
        case 1: res = (dlgBtns == DlgBtns::mbInslaterRestart) ? DLG_RESULT_RESTART : DLG_RESULT_REMIND; break;
        case 2: res = (dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_DOWNLOAD : DLG_RESULT_INSTALL; break;
        default:
            break;
        }
        if (callback)
            callback(res);
    }
    void toastFailed() const // Error showing current toast
    {
        if (callback)
            callback(NOTIF_FAILED);
    }
    void toastDismissed(WinToastDismissalReason state) const
    {
        switch (state) {
        case UserCanceled: // The user dismissed this toast
            break;
        case ApplicationHidden: // The application hide the toast using ToastNotifier.hide()
            break;
        case TimedOut: // The toast has timed out
            break;
        default: // Toast not activated
            break;
        }
    }

private:
    DlgBtns dlgBtns;
    FnVoidInt callback;
};
#endif

class CNotification::CNotificationPrivate
{
public:
#ifdef __linux__
    ~CNotificationPrivate()
    {
        clear();
    }
    void clear()
    {
        for (auto it = ntfMap.begin(); it != ntfMap.end();) {
            NotifyNotification *ntf = it->first;
            it = ntfMap.erase(it);
            notify_notification_close(ntf, NULL);
            g_object_unref(ntf);
        }
    }

    NtfMap ntfMap;
#endif
    int isInit = -1;
};


CNotification& CNotification::instance()
{
    static CNotification inst;
    return inst;
}

CNotification::CNotification() :
    pimpl(new CNotificationPrivate)
{}

CNotification::~CNotification()
{
    delete pimpl, pimpl = nullptr;
#ifdef __linux__
    if (notify_is_initted())
        notify_uninit();
#endif
}

bool CNotification::init()
{
    if (pimpl->isInit != -1)
        return pimpl->isInit;

#ifdef __linux__
    pimpl->isInit = notify_init(WINDOW_TITLE);
#else
    if (Utils::getWinVersion() < Utils::WinVer::Win10) {
        pimpl->isInit = 0;
        return false;
    }
    WinToast::instance()->setAppName(TEXT(WINDOW_TITLE));
    WinToast::instance()->setAppUserModelId(WinToast::configureAUMI(TEXT(VER_COMPANYNAME_STR), TEXT(VER_PRODUCTNAME_STR), TEXT(VER_PRODUCTNAME_STR), TEXT(VER_FILEVERSION_STR)));
    pimpl->isInit = WinToast::instance()->initialize();
#endif
    return pimpl->isInit;
}

void CNotification::clear()
{
#ifdef __linux__
    pimpl->clear();
#else
    WinToast::instance()->clear();
#endif
}

bool CNotification::show(const QString &msg, const QString &content, DlgBtns dlgBtns, const FnVoidInt &callback)
{
#ifdef __linux__
    if (!isNotificationsEnabled())
        return false;
    QString lpText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    NotifyNotification *ntf = notify_notification_new(lpText.toLocal8Bit().data(), content.toLocal8Bit().data(), NULL);
    g_signal_connect(G_OBJECT(ntf), "closed", G_CALLBACK(on_close), (void*)&pimpl->ntfMap);
    notify_notification_set_urgency(ntf, NotifyUrgency::NOTIFY_URGENCY_NORMAL);
    notify_notification_set_timeout(ntf, NOTIF_TIMEOUT_MS);
    pimpl->ntfMap[ntf] = callback;

    if (callback) {
        switch (dlgBtns) {
        case DlgBtns::mbInslaterRestart:
            addAction("inslater", TEXT_INSLATER.c_str());
            addAction("restart", TEXT_RESTART.c_str());
            break;
        case DlgBtns::mbSkipRemindInstall:
            addAction("skip", TEXT_SKIP.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("install", TEXT_INSTALL.c_str());
            break;
        case DlgBtns::mbSkipRemindSaveandinstall:
            addAction("skip", TEXT_SKIP.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("saveins", TEXT_SAVEANDINS.c_str());
            break;
        case DlgBtns::mbSkipRemindDownload:
            addAction("skip", TEXT_SKIP.c_str());
            addAction("remind", TEXT_REMIND.c_str());
            addAction("download", TEXT_DOWNLOAD.c_str());
            break;
        default:
            break;
        }
    }
    return notify_notification_show(ntf, NULL);
#else
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    std::wstring lpContent = content.toStdWString();

    WinToastTemplate tmpl(WinToastTemplate::WinToastTemplateType::Text02);
    tmpl.setTextField(lpText, WinToastTemplate::FirstLine);
    tmpl.setTextField(lpContent, WinToastTemplate::SecondLine);
    tmpl.setDuration(WinToastTemplate::Duration::System);
    tmpl.setExpiration(NOTIF_TIMEOUT_MS);

    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:
        tmpl.addAction(TEXT_INSLATER);
        tmpl.addAction(TEXT_RESTART);
        break;
    case DlgBtns::mbSkipRemindInstall:
        tmpl.addAction(TEXT_SKIP);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_INSTALL);
        break;
    case DlgBtns::mbSkipRemindSaveandinstall:
        tmpl.addAction(TEXT_SKIP);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_SAVEANDINS);
        break;
    case DlgBtns::mbSkipRemindDownload:
        tmpl.addAction(TEXT_SKIP);
        tmpl.addAction(TEXT_REMIND);
        tmpl.addAction(TEXT_DOWNLOAD);
        break;
    default:
        break;
    }
    return WinToast::instance()->showToast(tmpl, new ToastHandler(dlgBtns, callback)) > -1;
#endif
}
