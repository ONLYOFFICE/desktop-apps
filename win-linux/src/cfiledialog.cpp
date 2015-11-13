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

#include "cfiledialog.h"
#include <QFileDialog>

#include <QDebug>

#if defined(_WIN32)
CFileDialogWrapper::CFileDialogWrapper(HWND hParentWnd) : QWinWidget(hParentWnd)
#else
CFileDialogWrapper::CFileDialogWrapper(QWidget * parent) : QObject(parent)
#endif
{

}

CFileDialogWrapper::~CFileDialogWrapper()
{

}

bool CFileDialogWrapper::showModal(QString& fileName)
{
    QString filter = tr("All files (*.*)"), ext_in;
    QRegExp reExtension("\\.(\\w{1,10})$");
    if (!(reExtension.indexIn(fileName) < 0)) {
        ext_in = reExtension.cap(1);
        filter.prepend(getFilter(ext_in) + ";;");
    }

    QWidget * p = qobject_cast<QWidget *>(parent());
//    QFileDialog dlg(p);
//    fileName = dlg.getSaveFileName(p, tr("Save As"), fileName, filter);
    fileName = QFileDialog::getSaveFileName(p, tr("Save As"), fileName, filter);

    return fileName.length() > 0;
}

QString CFileDialogWrapper::getFilter(const QString& extension) const
{
    QString out = extension.toLower();
    if (extension.contains(QRegExp("^docx?$"))) {
        return tr("Word Document") + " (*." + out +")";
    } else
    if (extension.contains(QRegExp("^xlsx?$"))) {
        return tr("Excel Workbook") + " (*." + out + ")";
    } else
    if (extension.contains(QRegExp("^pptx?$"))) {
        return tr("PowerPoint Presentation") + " (*." + out + ")";
    } else {
        out.replace(0, 1, extension.left(1).toUpper());
        return tr("%1 File (*.%2)").arg(out).arg(out.toLower());
    }
}
