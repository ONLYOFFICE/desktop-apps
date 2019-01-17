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

#include "casctabdata.h"

CAscTabData::CAscTabData(const QString& t, CefType wt)
    : _title(t), _is_changed(false), _is_closed(false)
    , _is_local(false)
    , _panel_id(-1)
    , _vtype(wt)
    , _url()
    , _typeContent(etUndefined)
{}

CAscTabData::CAscTabData(const QString& t, AscEditorType ct)
    : _title(t), _is_changed(false), _is_closed(false)
    , _panel_id(-1)
    , _url()
    , _typeContent(ct)
{
    switch (ct) {
    case etDocument:
    case etSpreadsheet:
    case etPresentation:
        _vtype = cvwtEditor;
        break;
    default:
        _vtype = cvwtSimple;
        break;
    }
}

void CAscTabData::setTitle(const QString& t)
{
    _title = t;
}

QString CAscTabData::title(bool orig) const
{
    return !orig && _is_changed ? _title + "*": _title;
}

void CAscTabData::setChanged(bool s)
{
    _is_changed = s;
}

bool CAscTabData::changed() const
{
    return _is_changed;
}

void CAscTabData::setViewId(int id)
{
    _panel_id = id;
}

int CAscTabData::viewId() const
{
    return _panel_id;
}

void CAscTabData::close() {
    _is_closed = true;
}

void CAscTabData::reuse()
{
    _is_closed = false;
}

bool CAscTabData::closed() const
{
    return _is_closed;
}

CefType CAscTabData::viewType() const
{
    return _vtype;
}

void CAscTabData::setUrl(const wstring& u)
{
    _url = u;
}

void CAscTabData::setUrl(const QString& u)
{
    _url = u.toStdWString();
}

wstring CAscTabData::url() const
{
    return _url;
}

bool CAscTabData::isViewType(CefType vt) const
{
    return vt == _vtype;
}

AscEditorType CAscTabData::contentType()
{
    return _typeContent;
}

void CAscTabData::setContentType(AscEditorType t)
{
    _typeContent = t;
}

void CAscTabData::setLocal(bool l)
{
    _is_local = l;
}

bool CAscTabData::local() const
{
    return _is_local;
}

bool CAscTabData::eventLoadSupported() const
{
    return _event_load_supported;
}

void CAscTabData::setEventLoadSupported(bool value)
{
    _event_load_supported = value;
}
