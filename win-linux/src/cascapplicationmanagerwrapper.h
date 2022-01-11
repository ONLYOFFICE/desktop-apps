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
#include "cthemes.h"

#define SEND_TO_ALL_START_PAGE nullptr

#ifdef Q_OS_WIN
typedef HWND ParentHandle;
#else
typedef QWidget* ParentHandle;
#endif


struct sWinTag {
    int     type;
    size_t  handle;

    bool operator==(const sWinTag& other) const
    {
        return other.handle == this->handle;
    }
};

enum class CScalingFactor
{
    SCALING_FACTOR_1,
    SCALING_FACTOR_1_25,
    SCALING_FACTOR_1_5,
    SCALING_FACTOR_1_75,
    SCALING_FACTOR_2,
};

class CAscApplicationManagerWrapper;
class CAscApplicationManagerWrapper_Private;
typedef CAscApplicationManagerWrapper AscAppManager;

class CAscApplicationManagerWrapper : public QObject, public QAscApplicationManager, CCefEventsTransformer
{
    Q_OBJECT
protected:
    void addStylesheets(CScalingFactor, const std::string&);

private:
    std::vector<size_t> m_vecEditors;

    std::map<CScalingFactor, std::vector<std::string>> m_mapStyles;

    std::map<int, CCefEventsGate *> m_receivers;
    std::map<int, CSingleWindow *> m_winsReporter;

    uint m_closeCount = 0;
    uint m_countViews = 0;
    std::wstring m_closeTarget;

    CWindowsQueue<sWinTag> * m_queueToClose;
    CEventDriver m_eventDriver;
    CMainWindow * m_pMainWindow = nullptr;

    std::shared_ptr<CAppUpdater> m_updater;
    std::shared_ptr<CThemes> m_themes;
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
    bool applySettings(const std::wstring& wstrjson);
    void sendSettings(const std::wstring& opts);
    void applyTheme(const std::wstring&, bool force = false);

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
    void onFileChecked(const QString&, int, bool);
    void onEditorWidgetClosed();


public:
    static CAscApplicationManagerWrapper & getInstance();
    static CAscApplicationManager * createInstance();

    CSingleWindow * createReporterWindow(void *, int);

    static void             startApp();
    static void             initializeApp();
    static void             gotoMainWindow(size_t pw = 0);
    static void             handleInputCmd(const std::vector<std::wstring>&);
    static void             closeMainWindow();
    static void             closeEditorWindow(const size_t);

    static void             editorWindowMoving(const size_t, const QPoint&);
    static CMainWindow *    mainWindow();
    static const CEditorWindow *  editorWindowFromHandle(size_t);
    static void             sendCommandTo(QCefView * target, const QString& cmd, const QString& args = "");
    static void             sendCommandTo(CCefView * target, const std::wstring& cmd, const std::wstring& args = L"");
    static std::wstring     userSettings(const std::wstring& name);
    static void             setUserSettings(const std::wstring& name, const std::wstring& value);

    static void             sendEvent(int type, void * data);
    static QString          getWindowStylesheets(double);
    static QString          getWindowStylesheets(CScalingFactor);
    static bool             canAppClose();
    static QCefView *       createViewer(QWidget * parent);
    static QString          newFileName(int format);
    static CThemes &        themes();

    static ParentHandle     windowHandleFromId(int id);

    static void             destroyViewer(int id);
    static void             destroyViewer(QCefView * v);

    static void             cancelClose();
    static void checkUpdates();

    uint logoutCount(const std::wstring& portal) const;
    void Logout(const std::wstring& portal);
    void launchAppClose();

    void OnEvent(NSEditorApi::CAscCefMenuEvent *);
    bool event(QEvent *event);
private:
    friend class CAscApplicationManagerWrapper_Private;
    std::unique_ptr<CAscApplicationManagerWrapper_Private> m_private;

    CAscApplicationManagerWrapper(CAscApplicationManagerWrapper_Private *);
};

#endif // QASCAPPLICATIONMANAGER

