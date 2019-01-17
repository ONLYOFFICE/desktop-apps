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

#include <QCoreApplication>

#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include "Windows.h"

typedef HRESULT (__stdcall *SetCurrentProcessExplicitAppUserModelIDProc)(PCWSTR AppID);

bool SetAppUserModelId()
{
    bool result = false;

    // try to load Shell32.dll
    HMODULE _lib_shell32 = LoadLibrary(L"shell32.dll");
    if ( _lib_shell32 != NULL ) {
        // see if the function is exposed by the current OS
        SetCurrentProcessExplicitAppUserModelIDProc setCurrentProcessExplicitAppUserModelId =
            reinterpret_cast<SetCurrentProcessExplicitAppUserModelIDProc>(GetProcAddress(_lib_shell32, "SetCurrentProcessExplicitAppUserModelID"));

        if ( setCurrentProcessExplicitAppUserModelId != NULL ) {
            result = setCurrentProcessExplicitAppUserModelId(QString(APP_USER_MODEL_ID).toStdWString().c_str()) == S_OK;
        }

        FreeLibrary(_lib_shell32);
    }

    return result;
}


int main(int argc, char *argv[])
{
    SetAppUserModelId();

    QCoreApplication a(argc, argv);
//    return a.exec();

    QStringList _cmdArgs(QCoreApplication::arguments().mid(1));
    QFileInfo _fi(QString::fromLocal8Bit(argv[0]));

    qputenv("Path", "./converter;" + qgetenv("Path"));

#define APP_LAUNCH_NAME "./DesktopEditors.exe"

//    QProcess::startDetached(fi.absolutePath() + APP_LAUNCH_NAME, _cmdArgs, fi.absolutePath());
    QProcess::startDetached(_fi.absolutePath() + "./editors.exe", _cmdArgs, _fi.absolutePath());

    return 0;
}
