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

#ifndef CASCAPPLICATIONMANAGERWRAPPER
#define CASCAPPLICATIONMANAGERWRAPPER

#include "qascapplicationmanager.h"
#include <QObject>
#include <QMutex>
#include <vector>
#include <memory>
#include "ccefeventstransformer.h"
#include "ccefeventsgate.h"
#include "ceditorwindow.h"
#include "cwindowsqueue.h"
#include "ceventdriver.h"

#ifdef _WIN32
#include "win/mainwindow.h"
#include "win/csinglewindow.h"
#else
#include "linux/cmainwindow.h"
#include "linux/singleapplication.h"
#include "linux/csinglewindow.h"
#endif

#include "cappupdater.h"

#define SEND_TO_ALL_START_PAGE nullptr

#ifdef Q_OS_WIN
typedef HWND ParentHandle;
#else
typedef QWidget* ParentHandle;
#endif


using namespace std;

struct sWinTag {
    int     type;
    size_t  handle;

    bool operator==(const sWinTag& other) const
    {
        return other.handle == this->handle;
    }
};

class CAscApplicationManagerWrapper;
typedef CAscApplicationManagerWrapper AscAppManager;

class CAscApplicationManagerWrapper : public QObject, public QAscApplicationManager, CCefEventsTransformer
{
    Q_OBJECT

private:
    vector<size_t> m_vecWindows;
    vector<size_t> m_vecEditors;
    vector<QString> m_vecStyles;
    vector<QString> m_vecStyles2x;

    map<int, CCefEventsGate *> m_receivers;
    map<int, CSingleWindow *> m_winsReporter;

    uint m_closeCount = 0;
    uint m_countViews = 0;
    wstring m_closeTarget;

    CWindowsQueue<sWinTag> * m_queueToClose;
    CEventDriver m_eventDriver;

    std::shared_ptr<CAppUpdater> m_updater;
public:
    CWindowsQueue<sWinTag>& closeQueue();
    CEventDriver& commonEvents();

private:
    CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&);
    void operator =(CAscApplicationManagerWrapper const&);

    CAscApplicationManagerWrapper();
    ~CAscApplicationManagerWrapper();

    void StartSaveDialog(const std::wstring& sName, unsigned int nId);
    bool processCommonEvent(NSEditorApi::CAscCefMenuEvent *);
    void broadcastEvent(NSEditorApi::CAscCefMenuEvent *);
    bool applySettings(const wstring& wstrjson);
    void sendSettings(const wstring& opts);

    CMainWindow * mainWindowFromViewId(int uid) const;
    CEditorWindow * editorWindowFromViewId(int uid) const;
    CEditorWindow * editorWindowFromUrl(const QString&) const;

public:
    static void bindReceiver(int view_id, CCefEventsGate * const receiver);
    static void unbindReceiver(int view_id);
    static void unbindReceiver(const CCefEventsGate * receiver);

signals:
    void coreEvent(void *);

public slots:
    void onCoreEvent(void *);
    void onDownloadSaveDialog(const std::wstring& name, uint id);
    void onQueueCloseWindow(const sWinTag&);


public:
    static CAscApplicationManagerWrapper & getInstance();
    static CAscApplicationManager * createInstance();

    CSingleWindow * createReporterWindow(void *, int);

    static void             startApp();
    static void             initializeApp();
    static CMainWindow *    createMainWindow(QRect&);
    static void             closeMainWindow(const size_t);
    static void             closeEditorWindow(const size_t);

    static void             processMainWindowMoving(const size_t, const QPoint&);
    static void             editorWindowMoving(const size_t, const QPoint&);
    static uint             countMainWindow();
    static CMainWindow *    topWindow();
    static const CEditorWindow *  editorWindowFromHandle(size_t);
    static void             sendCommandTo(QCefView * target, const QString& cmd, const QString& args = "");
    static void             sendCommandTo(CCefView * target, const wstring& cmd, const wstring& args = L"");

    static void             sendEvent(int type, void * data);
    static QString          getWindowStylesheets(int);
    static bool             canAppClose();
    static QCefView *       createViewer(QWidget * parent);
    static QString          newFileName(int format);

    static ParentHandle     windowHandleFromId(int id);

    static void             destroyViewer(int id);
    static void             destroyViewer(QCefView * v);

    static void             cancelClose();
    static void checkUpdates();

    void manageUndocking(int uid, const std::wstring& action);
    uint logoutCount(const wstring& portal) const;
    void launchAppClose();

    void OnEvent(NSEditorApi::CAscCefMenuEvent *);
    bool event(QEvent *event);
private:
    class CAscApplicationManagerWrapper_Private;
    std::unique_ptr<CAscApplicationManagerWrapper_Private> m_private;
};

#endif // QASCAPPLICATIONMANAGER

