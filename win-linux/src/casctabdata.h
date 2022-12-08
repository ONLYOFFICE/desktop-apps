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

#ifndef CASCTABDATA_H
#define CASCTABDATA_H


#include <QString>
#include "qcefview.h"

typedef CefViewWrapperType CefType;

struct CAscTabData
{
public:
    CAscTabData(const QString &, CefType wt = cvwtEditor);
    CAscTabData(const QString &, AscEditorType ct);
    ~CAscTabData() {}

    void    setTitle(const QString&);
    void    setChanged(bool);
    void    setIsLocal(bool);
    void    setUrl(const std::wstring&);
    void    setUrl(const QString&);
    void    close();
    void    reuse();
    QString title(bool orig = false) const;
    bool    modified() const;
    bool    hasChanges() const;
    bool    closed() const;
    bool    isLocal() const;
    CefType viewType() const;
    std::wstring url() const;
    bool    isViewType(CefType) const;
    bool    eventLoadSupported() const;
    void    setEventLoadSupported(bool);
    void    setFeatures(const std::wstring&);
    std::wstring features() const;
    bool    hasFeature(const std::wstring&) const;

    AscEditorType   contentType() const;
    void            setContentType(AscEditorType);
private:
    QString _title;
    bool    _is_changed = false,
            _is_readonly = false,
            _has_changes = false;
    bool    _is_closed = false;
    bool    _is_local;
    CefType _vtype;
    std::wstring _url;
    bool    _event_load_supported = false;
    std::wstring _features;
    QString _str_readonly;

    AscEditorType _typeContent;
};

#endif // CASCTABDATA_H
