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


/*
*   new inherited controller declaration
*   panel 'recent'
*/

+function(){ 'use strict'
    var ControllerRecent = function(args={}) {
        args.caption = 'Recent files';
        args.action =
        this.action = "recent";
        this.view = new ViewRecent(args);
    };

    ControllerRecent.prototype = Object.create(baseController.prototype);
    ControllerRecent.prototype.constructor = ControllerRecent;

    var ViewRecent = function(args) {
        var _lang = utils.Lang;

        // args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        let _html = `<div class="action-panel ${args.action}">` +
                      '<div class="flexbox">' +
                        '<div id="box-recovery" class="flex-item">' +
                          '<div class="flexbox">'+
                            `<h3 class="table-caption" l10n>${_lang.listRecoveryTitle}</h3>`+
                            '<div class="table-box flex-fill">'+
                              '<table id="tbl-filesrcv" class="table-files list"></table>'+
                            '</div>' +
                          '</div>' +
                        '</div>' +
                        '<div id="recovery-sep"></div>' +
                        '<div id="box-recent" class="flex-item flex-fill">' +
                          '<div class="flexbox">'+
                            `<h3 class="table-caption" l10n>${_lang.listRecentFileTitle}</h3>`+
                            '<div class="table-box flex-fill">'+
                              '<table class="table-files list"></table>'+
                              '<h4 class="text-emptylist img-before-el" l10n>' + _lang.textNoFiles + '</h4>' +
                            '</div>' +
                          '</div>' +
                        '</div>' +
                      '</div>' +
                    '</div>';

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 0;
        args.itemtext = _lang.actRecentFiles;

        baseView.prototype.constructor.call(this, args);
    };

    ViewRecent.prototype = Object.create(baseView.prototype);
    ViewRecent.prototype.constructor = ViewRecent;
    utils.fn.extend(ViewRecent.prototype, {
        render: function() {
            baseView.prototype.render.apply(this, arguments);

            this.$boxRecovery = this.$panel.find('#box-recovery');
            this.$boxRecent = this.$panel.find('#box-recent');
        },
        listitemtemplate: function(info) {
            let id = !!info.uid ? (` id="${info.uid}"`) : '';

            var _tpl = `<tr${id}>
                          <td class="row-cell cicon">
                            <span class="icon ${info.type=='folder'?'img-before-el':'img-format'} ${info.type}" />
                          </td>
                          <td class="row-cell cname">
                            <p class="name primary">${info.name}</p>
                            <p class="descr minor">${info.descr}</p>
                          </td>`;

            if (info.type != 'folder')
                _tpl += `<td class="row-cell cdate minor">${info.date}</td>`;

            return _tpl;
        },
        updatelistsize: function() {
            // set fixed height for scrollbar appearing. 
            var _available_height = this.$panel.height();
            var _box_recent_height = _available_height;

            if (!this.$boxRecovery.find('tr').size()) {
                // $boxRecent.height($boxRecent.parent().height());
            } else {
                _available_height -= /*separatorHeight*/40;
                _box_recent_height *= 0.5; 

                this.$boxRecovery.height(_available_height * 0.5);

                var $table_box = this.$boxRecovery.find('.table-box');
                if ( !$table_box.hasScrollBar() ) {
                    let _new_recovery_height = $table_box.find('.table-files.list').height() + /*$headerRecovery.height()*/46;
                    this.$boxRecovery.height(_new_recovery_height);

                    _box_recent_height = _available_height - _new_recovery_height;
                }
            }

            /*$boxRecent.height() != _box_recent_height &&*/ this.$boxRecent.height(_box_recent_height);
        }
    });

    window.ControllerRecent = ControllerRecent;

    String.prototype.hashCode = function() {
        var hash = 0, i, chr;
        if (this.length === 0) return hash;

        for (i = this.length; !(--i < 0);) {
            chr   = this.charCodeAt(i);
            hash  = ((hash << 5) - hash) + chr;
            hash  = hash & hash; // Convert to 32bit integer
        }

        return hash;
    };

    utils.fn.extend(ControllerRecent.prototype, (function() {
        let collectionRecents, collectionRecovers;
        let ppmenu;

        var _on_recents = function(params) {
            collectionRecents.empty();

            var files = utils.fn.parseRecent(params);
            for (let item of files) {
                var model = new FileModel(item);
                model.set('hash', item.path.hashCode());

                collectionRecents.add(model);

                this.check_list[model.get('hash')] = item.path;
            }

            if ( this.appready && Object.keys(this.check_list).length ) {
                sdk.execCommand('files:check', JSON.stringify(this.check_list));
            }
        };

        var _on_recovers = function(params) {
            collectionRecovers.empty();

            var files = utils.fn.parseRecent(params);
            for (let item of files) {
                collectionRecovers.add( new FileModel(item) );
            }

            this.view.$boxRecovery[collectionRecovers.size() > 0 ? 'show' : 'hide']();
            this.view.$panel.find('#recovery-sep')[collectionRecovers.size() > 0 ? 'show' : 'hide']();
            this.view.updatelistsize();
        };

        function _init_collections() {
            let _cl_rcbox = this.view.$panel.find('#box-recent'),
                _cl_rvbox = this.view.$panel.find('#box-recovery');

            collectionRecents = new Collection({
                view: _cl_rcbox,
                list: _cl_rcbox.find('.table-files.list')
            });

            collectionRecents.events.erased.attach(collection => {
                collection.list.parent().addClass('empty');
            });

            collectionRecents.events.inserted.attach((collection, model) => {
                let $item = this.view.listitemtemplate(model);

                collection.list.append($item);
                collection.list.parent().removeClass('empty');
            });

            collectionRecents.events.click.attach((collection, model) => {
                // var _portal = model.descr;
                // if ( !model.islocal && !app.controller.portals.isConnected(_portal) ) {
                    // app.controller.portals.authorizeOn(_portal, {type: 'fileid', id: model.fileid});
                // } else {
                    openFile(OPEN_FILE_RECENT, model.fileid);
                // }
            });

            collectionRecents.events.contextmenu.attach(function(collection, model, e){
                ppmenu.actionlist = 'recent';
                ppmenu.hideItem('files:explore', !model.islocal && !model.dir);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });

            collectionRecents.events.changed.attach(function(collection, model){
                let $el = collection.list.find('#' + model.uid);
                if ( $el ) $el[model.exist ? 'removeClass' : 'addClass']('unavail');
            });

            collectionRecents.empty();

            /**/

            collectionRecovers = new Collection({
                view: _cl_rvbox,
                list: _cl_rvbox.find('.table-files.list')
            });
            collectionRecovers.events.inserted.attach((collection, model)=>{
                collection.list.append( this.view.listitemtemplate(model) );
            });
            collectionRecovers.events.click.attach((collection, model)=>{
                openFile(OPEN_FILE_RECOVERY, model.fileid);
            });
            collectionRecovers.events.contextmenu.attach((collection, model, e)=>{
                ppmenu.actionlist = 'recovery';
                ppmenu.hideItem('files:explore', true);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });
        };

        function _init_ppmenu() {
            ppmenu = new Menu({
                id: 'pp-menu-files',
                items: [{
                    caption: utils.Lang.menuFileOpen,
                    action: 'files:open'
                },{
                    caption: utils.Lang.menuFileExplore,
                    action: 'files:explore'
                },{
                    caption: utils.Lang.menuRemoveModel,
                    action: 'files:forget'
                },{
                    caption: utils.Lang.menuClear,
                    action: 'files:clear'
                }]
            });

            ppmenu.init('#placeholder');
            ppmenu.events.itemclick.attach(_on_context_menu.bind(this));
        };

        function _on_context_menu(menu, action, data) {
            if (/\:open/.test(action)) {
                menu.actionlist == 'recent' ?
                    openFile(OPEN_FILE_RECENT, data.fileid) :
                    openFile(OPEN_FILE_RECOVERY, data.fileid);
            } else
            if (/\:clear/.test(action)) {
                menu.actionlist == 'recent' ?
                    window.sdk.LocalFileRemoveAllRecents() :
                    window.sdk.LocalFileRemoveAllRecovers();
            } else
            if (/\:forget/.test(action)) {
                $('#' + data.uid, this.view.$panel).addClass('lost');
                setTimeout(e => {
                    menu.actionlist == 'recent' ?
                        window.sdk.LocalFileRemoveRecent(parseInt(data.fileid)) :
                        window.sdk.LocalFileRemoveRecover(parseInt(data.fileid));}
                , 300); // 300ms - duration of item's 'collapse' transition
            } else
            if (/\:explore/.test(action)) {
                if (menu.actionlist == 'recent') {
                    sdk.execCommand('files:explore', data.path);
                }
            }
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();
                this.check_list = {};

                _init_collections.call(this);
                _init_ppmenu.call(this);

                window.sdk.on('onupdaterecents', _on_recents.bind(this));
                window.sdk.on('onupdaterecovers', _on_recovers.bind(this));
                window.sdk.on('on_native_message', (cmd, param)=>{
                    if (/files:checked/.test(cmd)) {
                        let fobjs = JSON.parse(param);
                        if ( fobjs ) {
                            for (let obj in fobjs) {
                                let value = JSON.parse(fobjs[obj]);
                                let model = collectionRecents.find('hash', parseInt(obj));
                                if ( model ) {
                                    model.get('exist') != value && model.set('exist', value);
                                }
                            }
                        }
                    } else
                    if (/file\:skip/.test(cmd)) {
                        sdk.LocalFileRemoveRecent(parseInt(param));
                    } else
                    if (/app\:ready/.test(cmd)) {
                        if ( Object.keys(this.check_list).length ) {
                            setTimeout(()=>{
                                sdk.execCommand('files:check', JSON.stringify(this.check_list));
                            }, 100);
                        }

                        this.appready = true;
                    }
                });

                $(window).resize(()=>{
                    this.view.updatelistsize();
                });

                CommonEvents.on('portal:authorized', (data)=>{
                    if ( data.type == 'fileid' ) {
                        let fileid = data.id;
                        openFile(OPEN_FILE_RECENT, fileid);
                    }

                    console.log('portal authorized');
                });

                return this;
            },
            getRecents: function() {
                return collectionRecents;
            },
            getRecovers: function() {
                return collectionRecovers;
            }
        };
    })());
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerRecent({});
//     p.init();
// });