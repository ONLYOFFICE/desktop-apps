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

'use strict';

var nCounter = 0;
function Event(sender) {
    this._sender = sender;
    this._listeners = [];
}

Event.prototype = {
    attach : function (listener) {
        return this._listeners.push(listener);
    },
    detach : function (value) {
        if ( !Number.isInteger(value) )
            value = this._listeners.indexOf(value);

        if ( !(value < 0) ) {
            this._listeners.splice(value, 1);
        }
    },
    clear: function() {
        this._listeners = [];
    },
    notify : function (args) {
        var index;
        var _args = Array.from(arguments);
        _args.unshift(this._sender);

        for (index = 0; index < this._listeners.length; index += 1) {
            // this._listeners[index](this._sender, args);
            this._listeners[index].apply(this, _args);
        }
    }
};

function Collection(attributes) {
    this.items = [];
    this.view = attributes.view;
    this.list = attributes.list;

    var _time = Date.now();
    this.on_item_changed = function(model) {
        this.events.changed.notify(model);
    }.bind(this);

    this.on_item_click = function(e) {
        Menu.opened && Menu.closeAll();

        if (Date.now()-_time > 800) {
            _time = Date.now();
            this.events.click.notify(e.data);
        }

        e.preventDefault();
        return false;
    }.bind(this);

    this.on_item_ctxmenu = function(e) {
        this.events.contextmenu.notify(e.data, e);
        e.preventDefault();
    }.bind(this);

    this.events = {};
    this.events.changed = new Event(this);
    this.events.erased = new Event(this);
    this.events.inserted = new Event(this);
    this.events.click = new Event(this);
    this.events.contextmenu = new Event(this);
};

Collection.prototype.add = function(item) {    
    item.changed.attach(this.on_item_changed);

    this.items.push(item);
    this.events.inserted.notify(item);

    $('#' + item.uid).off('click contextmenu');
    $('#' + item.uid).on('click', item, this.on_item_click);
    $('#' + item.uid).on('contextmenu', item, this.on_item_ctxmenu);
};

Collection.prototype.find = function(key, val) {
    return this.items.find(function(elem, i, arr){
        return elem[key] == val;
    });
};

Collection.prototype.empty = function() {
    this.items.forEach(function(model, i, a) {
        $('#'+model.uid).off();
    });

    this.items = [];

    if (!!this.list) this.view.find(this.list).empty();
    
    this.events.erased.notify();
};

Collection.prototype.size = function() {
    return this.items.length;
};

function Model(attributes) {
    var attr = attributes || {};

    this.prefix = attr.prefix || 'asc-gen';
    this.uid = this.prefix + ++nCounter;
    this.changed = new Event(this);
};

Model.prototype.set = function(key, value, opts) {
    this[key] = value;

    let args = {};
    args[key] = value;

    if ( !opts || opts.silent !== true )
        this.changed.notify(args);
};

Model.prototype.get = function(key) {
    return this[key];
};

function PortalModel(attributes) {
    Model.prototype.constructor.call(this, {prefix: 'asc-portal-'});

    this.name   = attributes.portal && utils.skipUrlProtocol(attributes.portal);
    this.path   = attributes.portal || '';
    this.logged = false;
    this.user   = attributes.user || '';
    this.email  = attributes.email || '';
    this.provider = attributes.provider || 'asc';
};

PortalModel.prototype = Object.create(Model.prototype); /*new Model();*/
PortalModel.prototype.constructor = PortalModel;

function FileModel(attributes) {
    Model.prototype.constructor.call(this);

    this.name   = attributes.name || '';
    this.descr  = attributes.descr || '';
    this.path   = attributes.path;
    this.date   = attributes.date;
    this.type   = attributes.type;
    this.fileid = attributes.id;
    this.exist  = true;
    this.islocal = !/^https?:\/\//.test(this.path)
};

FileModel.prototype = new Model();
FileModel.prototype.constructor = FileModel;
