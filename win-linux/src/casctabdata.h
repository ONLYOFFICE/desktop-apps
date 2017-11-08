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

#ifndef CASCTABDATA_H
#define CASCTABDATA_H


#include <QString>
#include "qcefview.h"

typedef CefViewWrapperType CefType;
using namespace std;

struct CAscTabData
{
public:
    CAscTabData(const QString &, CefType wt = cvwtEditor);
    CAscTabData(const QString &, AscEditorType ct);
    ~CAscTabData() {}

    void    setTitle(const QString&);
    void    setChanged(bool);
    void    setViewId(int);
//    void    setLocal(bool);
    void    setUrl(const wstring&);
    void    setUrl(const QString&);
    void    close();
    void    reuse();
    QString title(bool orig = false) const;
    bool    changed() const;
    int     viewId() const;
    bool    closed() const;
//    bool    local() const;
    CefType viewType() const;
    wstring url() const;
    bool    isViewType(CefType) const;

    AscEditorType   contentType();
    void            setContentType(AscEditorType);
private:
    QString _title;
    bool    _is_changed;
    bool    _is_closed;
//    bool    _is_local;
    int     _panel_id;
    CefType _vtype;
    wstring _url;

    AscEditorType _typeContent;
};

#endif // CASCTABDATA_H
