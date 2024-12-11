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

#include "cjson.h"
#include "cjson_p.h"
#include <codecvt>


static std::string TStrToUtf8(const tstring &str)
{
#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8_conv;
    return utf8_conv.to_bytes(str);
#else
    return str;
#endif
}

class JsonObjectPrivate
{
public:
    json_object_s *obj = nullptr;
};

class JsonValuePrivate
{
public:
    json_value_s *val = nullptr;
};

class JsonDocumentPrivate
{
public:
    json_value_s *root = nullptr;
};


JsonValue::JsonValue() : pimpl(new JsonValuePrivate)
{}

JsonValue::JsonValue(const JsonValue &jval) : JsonValue()
{
    pimpl->val = jval.pimpl->val;
}

JsonValue::~JsonValue()
{
    delete pimpl, pimpl = nullptr;
}

JsonValue& JsonValue::operator=(const JsonValue &jval)
{
    if (this == &jval)
        return *this;
    pimpl->val = jval.pimpl->val;
    return *this;
}

JsonObject JsonValue::toObject()
{
    JsonObject jobj;
    if (pimpl->val && pimpl->val->type == json_type_object)
        jobj.pimpl->obj = (json_object_s*)pimpl->val->payload;
    return jobj;
}

tstring JsonValue::toTString()
{
    tstring str;
    if (pimpl->val && pimpl->val->type == json_type_string) {
        json_string_s *jstr = (json_string_s*)pimpl->val->payload;
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        str = converter.from_bytes(std::string(jstr->string, jstr->string_size));
#else
        str = std::string(jstr->string, jstr->string_size);
#endif
    }
    return str;
}


JsonObject::JsonObject() : pimpl(new JsonObjectPrivate)
{}

JsonObject::JsonObject(const JsonObject &jobj) : JsonObject()
{
    pimpl->obj = jobj.pimpl->obj;
}

JsonObject::~JsonObject()
{
    delete pimpl, pimpl = nullptr;
}

JsonObject& JsonObject::operator=(const JsonObject &jobj)
{
    if (this == &jobj)
        return *this;
    pimpl->obj = jobj.pimpl->obj;
    return *this;
}

JsonValue JsonObject::value(const tstring &key)
{
    std::string utf8_key = TStrToUtf8(key);
    JsonValue jval;
    json_object_element_s *element;
    if (pimpl->obj && (element = pimpl->obj->start) != NULL) {
        do {
            if (strcmp(element->name->string, utf8_key.c_str()) == 0) {
                jval.pimpl->val = element->value;
                break;
            }
        } while ((element = element->next) != NULL);
    }
    return jval;
}

bool JsonObject::contains(const tstring &key)
{
    std::string utf8_key = TStrToUtf8(key);
    json_object_element_s *element;
    if (pimpl->obj && (element = pimpl->obj->start) != NULL) {
        do {
            if (strcmp(element->name->string, utf8_key.c_str()) == 0)
                return true;
        } while ((element = element->next) != NULL);
    }
    return false;
}


JsonDocument::JsonDocument() : pimpl(new JsonDocumentPrivate)
{}

JsonDocument::JsonDocument(const tstring &json) : JsonDocument()
{
    std::string utf8_json = TStrToUtf8(json);
    pimpl->root = json_parse(utf8_json.c_str(), utf8_json.length());
}

JsonDocument::~JsonDocument()
{
    if (pimpl->root)
        free(pimpl->root);
    delete pimpl, pimpl = nullptr;
}

JsonObject JsonDocument::object()
{
    JsonObject obj;
    if (pimpl->root && pimpl->root->type == json_type_object && pimpl->root->payload)
        obj.pimpl->obj = (json_object_s*)pimpl->root->payload;
    return obj;
}
