/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#include "utils.h"
#include "defines.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#ifdef _WIN32
#include "shlobj.h"
#endif

#include <QDebug>

QStringList * Utils::getInputFiles(const QStringList& inlist)
{
    QStringList * _ret_files_list = new QStringList;

    QStringListIterator i(inlist); i.next();
    while (i.hasNext()) {
        QFileInfo info(i.next());
        if (info.isFile()) {
            _ret_files_list->append(info.absoluteFilePath());
        }
    }

    return _ret_files_list;
}

bool Utils::firstStart(bool restore)
{
    bool _is_active;
    QString _file_name;
#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
        _file_name = QString::fromWCharArray(szPath);
        _file_name.append("\\").append(LIC_KEY_FILENAME);
    }
#else
    _file_name = QString("/var/lib").append(APP_DATA_PATH).append("/../.").append(LIC_KEY_FILENAME);
#endif

    GET_REGISTRY_USER(_reg_user);
    _is_active = !QFileInfo(_file_name).exists();
    if (_is_active) {
        _is_active = !_reg_user.contains("license");
    } else
    if (restore) {
        _reg_user.setValue("license", "ȒѬ Ї");
    }

    return _is_active;
}

bool Utils::markFirstStart()
{
    QString _file_name;
#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
        _file_name = QString::fromWCharArray(szPath);
        _file_name.append("\\").append(LIC_KEY_FILENAME);
    }
#else
    _file_name = QString("/var/lib").append(APP_DATA_PATH).append("/../.").append(LIC_KEY_FILENAME);
#endif

    GET_REGISTRY_USER(_reg_user);
    bool _is_active = !QFileInfo(_file_name).exists();
    _is_active && (_is_active = !_reg_user.contains("license"));

    if (_is_active) {
        QFile _file(_file_name);
        if (_file.open(QFile::WriteOnly)) {
            _file.write("ȒѬ Ї", 7);
            _file.close();

#ifdef _WIN32
            SetFileAttributes(_file_name.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
        }

        _reg_user.setValue("license", "ȒѬ Ї");
    }

    return true;
}

QString Utils::getLocalUsedPath(int t)
{
    GET_REGISTRY_USER(_reg_user)

    QString _path;
    if (t == LOCAL_PATH_OPEN)
        _path = _reg_user.value("openPath").value<QString>(); else
        _path = _reg_user.value("savePath").value<QString>();

    return _path.length() > 0 && QDir(_path).exists() ?
        _path : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void Utils::createTempLicense()
{
    QString _file_name = licenseDir();
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    QFile _file(_file_name);
    if (_file.open(QFile::WriteOnly)) {
        _file.write("Ȓ»оЇ", 7);
        _file.close();

#ifdef _WIN32
        SetFileAttributes(_file_name.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
}

bool Utils::isTempLicense()
{
    QString _file_name = licenseDir();
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    return QFileInfo(_file_name).exists();
}

void Utils::removeTempLicense()
{
    QString _file_name = licenseDir();
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    QFile::remove(_file_name);
}

QString Utils::licenseDir()
{
    return QString::fromStdWString(licenseDirW());
}

wstring Utils::licenseDirW()
{
    std::wstring sAppData(L"");
#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
        sAppData = std::wstring(szPath);
        std::replace(sAppData.begin(), sAppData.end(), '\\', '/');
        sAppData.append(QString(APP_LICENSE_PATH).toStdWString());
    }

    if (sAppData.size()) {
        QDir().mkpath(QString::fromStdWString(sAppData));
    }

#else
    sAppData = QString("/var/lib").append(APP_LICENSE_PATH).toStdWString();
    QFileInfo fi(QString::fromStdWString(sAppData));
    if (!QDir().mkpath(fi.absoluteFilePath())) {
        if (!fi.isWritable()) {
            // TODO: check directory permissions and warn the user
            qDebug() << "directory permission error";
        }
    }
#endif

    return sAppData;
}

