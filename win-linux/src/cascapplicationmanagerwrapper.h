/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include "applicationmanager.h"
#include <QObject>
#include <QMutex>
#include <vector>
#include <memory>
#include "ccefeventstransformer.h"

#ifdef _WIN32
#include "win/mainwindow.h"
#include "win/csinglewindow.h"
#else
#include "linux/cmainwindow.h"
#include "linux/singleapplication.h"
#include "linux/csinglewindow.h"
#endif

using namespace std;

class CAscApplicationManagerWrapper;
typedef CAscApplicationManagerWrapper AscAppManager;

class CAscApplicationManagerWrapper : public QObject, public CAscApplicationManager, CCefEventsTransformer
{
    Q_OBJECT

private:
    vector<size_t> m_vecWidows;
    vector<size_t> m_vecEditors;
    QMutex         m_oMutex;

private:
    CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&);
    void operator =(CAscApplicationManagerWrapper const&);

    CAscApplicationManagerWrapper();
    ~CAscApplicationManagerWrapper();

    void StartSaveDialog(const std::wstring& sName, unsigned int nId);
    void OnNeedCheckKeyboard();
    int  GetPlatformKeyboardLayout();
    void OnEvent(NSEditorApi::CAscCefMenuEvent *);

    CMainWindow * mainWindowFromViewId(int uid) const;
    CSingleWindow * editorWindowFromViewId(int uid) const;

signals:
    void coreEvent(void *);

public slots:
    void onCoreEvent(void *);


public:
    static CAscApplicationManager & getInstance();
    static CAscApplicationManager * createInstance();

    CSingleWindow * createReporterWindow(void *);

    static void             startApp();
    static void             initializeApp();
    static CMainWindow *    createMainWindow(QRect&);
    static void             closeMainWindow(const size_t);
    static void             closeEditorWindow(const size_t);
    static void             processMainWindowMoving(const size_t, const QPoint&);
    static uint             countMainWindow();
    static CMainWindow *    topWindow();

private:
    class CAscApplicationManagerWrapper_Private;
    std::unique_ptr<CAscApplicationManagerWrapper_Private> m_private;
};

#endif // QASCAPPLICATIONMANAGER

