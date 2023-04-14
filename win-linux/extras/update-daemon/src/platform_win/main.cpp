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

#include "utils.h"
#include "platform_win/svccontrol.h"
#include "classes/platform_win/capplication.h"
#include "classes/cupdatemanager.h"
#include "../../src/defines.h"

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  gSvcStopEvent = NULL;


VOID WINAPI SvcMain(DWORD argc, LPTSTR *argv);
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcReportEvent(LPTSTR);


int __cdecl _tmain (int argc, TCHAR *argv[])
{
    if (argc > 1) {
        if (lstrcmpi(argv[1], _T("--install")) == 0) {
            SvcControl::SvcInstall();
            if (argc > 2)
                SvcControl::DoUpdateSvcDesc(argv[2]);
            SvcControl::DoStartSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--delete")) == 0) {
            SvcControl::DoStopSvc();
            SvcControl::DoDeleteSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--start")) == 0) {
            SvcControl::DoStartSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--stop")) == 0) {
            SvcControl::DoStopSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--enable")) == 0) {
            SvcControl::DoEnableSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--disable")) == 0) {
            SvcControl::DoDisableSvc();
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--description")) == 0) {
            if (argc > 2)
                SvcControl::DoUpdateSvcDesc(argv[2]);
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--update_dacl")) == 0) {
            //SvcControl::DoUpdateSvcDacl(pTrusteeName);
            return 0;
        } else
        if (lstrcmpi(argv[1], _T("--run-as-app")) == 0) {
            CSocket socket(0, INSTANCE_SVC_PORT);
            if (!socket.isPrimaryInstance())
                return 0;

            CApplication app;
            CUpdateManager upd;
            socket.onMessageReceived([&app](void *buff, size_t bufsize) {
                if (strcmp((const char*)buff, "stop") == 0)
                    app.exit(0);
            });
            return app.exec();
        }
    }

    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        {(LPTSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)SvcMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(DispatchTable) == 0) {
       NS_Utils::ShowMessage(L"ServiceCtrlDispatcher returned error:", true);
       return GetLastError();
    }

    return 0;
}

VOID WINAPI SvcMain(DWORD argc, LPTSTR *argv)
{
    if (argc > 1) {
        if (lstrcmpi(argv[1], _T("--log")) == 0) {
            NS_Logger::AllowWriteLog();
        }
    }

    gSvcStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, SvcCtrlHandler);
    if (gSvcStatusHandle == NULL) {
        wstring err(ADVANCED_ERROR_MESSAGE);
        SvcControl::SvcReportEvent(err.c_str());
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory(&gSvcStatus, sizeof(gSvcStatus));
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.
    //   Create an event. The control handler function, SvcCtrlHandler,
    //   signals this event when it receives the stop control code.
    gSvcStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (gSvcStopEvent == NULL) {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        wstring err(ADVANCED_ERROR_MESSAGE);
        SvcControl::SvcReportEvent(err.c_str());
        return;
    }

    // Report running status when initialization is complete.
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    CUpdateManager upd;
    upd.aboutToQuit([]() {
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    });
    WaitForSingleObject(gSvcStopEvent, INFINITE);    
    CloseHandle(gSvcStopEvent);
}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    switch (dwCtrl) {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        // Signal the service to stop.
        SetEvent(gSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

VOID ReportSvcStatus(DWORD currState, DWORD exitCode, DWORD waitHint)
{
    static DWORD dwCheckPoint = 1;

    gSvcStatus.dwCurrentState = currState;
    gSvcStatus.dwWin32ExitCode = exitCode;
    gSvcStatus.dwWaitHint = waitHint;

    if (currState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((currState == SERVICE_RUNNING) ||
            (currState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    if (SetServiceStatus(gSvcStatusHandle, &gSvcStatus) == FALSE) {
        wstring err(ADVANCED_ERROR_MESSAGE);
        SvcControl::SvcReportEvent(err.c_str());
    }
}
