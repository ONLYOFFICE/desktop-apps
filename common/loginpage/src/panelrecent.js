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
        this.action = "recents";
        this.view = new ViewRecent(args);
    };

    ControllerRecent.prototype = Object.create(baseController.prototype);
    ControllerRecent.prototype.constructor = ControllerRecent;
    var isSvgIcons = window.devicePixelRatio >= 2 || window.devicePixelRatio == 1;
    var ViewRecent = function(args) {
        var _lang = utils.Lang;

        // args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        let _html = `<div class="action-panel ${args.action}">
                      <h2 class="text-headline-1" l10n>${_lang.areaCreateFile}</h2>
                      <section id="box-create-new">
                      </section>
                      <section id="dnd-file-zone">
                      </section>
                      <section id="welcome-box">
                      </section>
                      <div class="recent-flex-box">
                        <h2 class="text-headline-1" l10n>${_lang.listRecentFileTitle}</h2>
                        <div id="box-recovery" class="recent-box-wrapper">
                          <div class="flexbox">
                            <h3 class="table-caption text-headline-2" l10n>${_lang.listRecoveryTitle}</h3>
                            <div class="table-box flex-fill">
                              <table id="tbl-filesrcv" class="table-files list"></table>
                            </div>
                          </div>
                        </div>
                        
                        <div id="box-pinned" class="recent-box-wrapper">
                            <div class="flexbox">
                                <h3 class="table-caption text-headline-2" l10n>${_lang.listPinnedTitle}</h3>
                                <div class="table-box flex-fill">
                                    <table class="table-files list"></table>
                                </div>
                            </div>
                        </div>

                        <div id="box-today" class="recent-box-wrapper">
                            <div class="flexbox">
                                <h3 class="table-caption text-headline-2" l10n>${_lang.listToday}</h3>
                                <div class="table-box flex-fill">
                                    <table class="table-files list"></table>
                                </div>
                            </div>
                        </div>

                        <div id="box-recent" class="recent-box-wrapper flex-fill">
                          <div class="flexbox">
                            <div style="display:none;">
                              <h3 class="table-caption" l10n>${_lang.listEarlier}</h3>
                              <input type="text" id="idx-recent-filter" style="display:none;">
                            </div>
                            <h3 class="table-caption text-headline-2" l10n>${_lang.listEarlier}</h3>
                            <div class="table-box flex-fill">
                              <table class="table-files list"></table>
                              <h4 class="text-emptylist${isSvgIcons ? '-svg' : ''} img-before-el" l10n>
                                  ${isSvgIcons ? '<svg class="icon"><use xlink:href="#folder-big"></use></svg>' : ''}
                                  ${_lang.textNoFiles}
                              </h4>
                            </div>
                          </div>
                        </div>
                      </div>
                    </div>`;

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = -1;
        args.tplItem = 'nomenuitem';
        // args.itemtext = _lang.actRecentFiles;

        baseView.prototype.constructor.call(this, args);
    };

    ViewRecent.prototype = Object.create(baseView.prototype);
    ViewRecent.prototype.constructor = ViewRecent;
    utils.fn.extend(ViewRecent.prototype, {
        render: function() {
            baseView.prototype.render.apply(this, arguments);

            this.$boxRecovery = this.$panel.find('#box-recovery');
            this.$boxRecent = this.$panel.find('#box-recent');
            this.$boxPinned = this.$panel.find('#box-pinned');
            this.$boxToday = this.$panel.find('#box-today');
        },
        listitemtemplate: function(info) {
            let id = !!info.uid ? (` id="${info.uid}"`) : '';
            info.crypted == undefined && (info.crypted = false);
            const editor_type = utils.editorByFileFormat(info.type);

            var _tpl = `<tr${id} class="${info.crypted ? `crypted${isSvgIcons ?'-svg':''}` : ''}" ${editor_type?`data-editor-type="${editor_type}"`:''}>
                          <td class="row-cell cicon">
                            <i class="icon ${info.type=='folder'?'img-el folder':`img-format ${info.format}`}" />
                        ${!isSvgIcons ?'':
                            `<svg class = "icon ${info.type=='folder'?'folder':''}">
                                <use xlink:href="#${info.type=='folder'?'folder-small':`${info.format}`}"></use>
                            </svg>
                            ${info.crypted?'<svg class = "shield"> <use xlink:href="#shield"></use></svg>':''}`
                        }
                          </td>
                          <td class="row-cell cname">
                            <p class="name text-body">${info.name}</p>
                            <p class="descr text-caption">${info.descr}</p>
                          </td>`;

            if (info.type != 'folder') {
                _tpl += `<td class="row-cell cbutton cpin">
                    <button id="${info.uid}-pin-btn">
                        <svg class="icon" data-iconname="pin" data-precls="tool-icon">
                            <use href="#pin"/>
                        </svg>
                    </button>
                </td>`;
                _tpl += `<td class="row-cell cdate text-caption">${info.date}</td>`;
                _tpl += `<td class="row-cell cbutton">
                    <button id="${info.uid}-more-btn">
                        <svg class="icon" data-iconname="more" data-precls="tool-icon">
                            <use href="#more"/>
                        </svg>
                    </button>
                </td>`;
            }

            return _tpl;
        },
        onscale: function (pasteSvg) {
            let elm,icoName, elmIcon, parent,
                emptylist = $('[class*="text-emptylist"]', '#box-recent');
            emptylist.toggleClass('text-emptylist text-emptylist-svg');

            if(pasteSvg && !emptylist.find('svg').length)
                emptylist.prepend($('<svg class = "icon"><use xlink:href="#folder-big"></use></svg>'));

            $('#box-recent .cicon').each(function () {
                 elm = $(this);
                 parent = elm.parent();
                 if(parent.hasClass('crypted-svg') || parent.hasClass('crypted'))
                     parent.toggleClass('crypted-svg crypted');

                 if(!pasteSvg || !!$('svg',elm).length) return;

                 icoName = $('i.icon', elm).attr('class').split(' ').filter((cls) => cls != 'icon' && cls != 'img-format');
                 elm.append($(`<svg class = "icon"><use xlink:href="#${icoName}"></use></svg>`));
                 if(parent.hasClass('crypted-svg'))
                     elm.append($('<svg class = "shield"><use xlink:href="#shield"></use></svg>'));
            });

            $('#box-recent-folders td.cicon').each(function (){
                elm=$(this)
                parent = elm.parent();
                if(parent.hasClass('crypted-svg') || parent.hasClass('crypted'))
                    parent.toggleClass('crypted-svg crypted');
                if(!pasteSvg || !!$('svg',elm).length) return;

                elm.append($('<svg class = "icon  folder"> <use xlink:href="#folder-small"></use></svg>'));
                if(parent.hasClass('crypted-svg'))
                    elm.append($('<svg class = "shield"><use xlink:href="#shield"></use></svg>'));

                });
        },
        updatelistsize: function() {
            // set fixed height for scrollbar appearing. 
            let _available_height = this.$panel.height();
            let _box_recent_height = _available_height;

            const updateBoxHeight = (box) => {
                if (box.find('tr').size() > 0) {
                    _available_height -= /*separatorHeight*/40;
                    _box_recent_height *= 0.5;

                    box.height(_available_height * 0.5);

                    const $tableBox = box.find('.table-box');
                    if (!$tableBox.hasScrollBar()) {
                        const newBoxHeight = $tableBox.find('.table-files.list').height() + /*$headerRecovery.height()*/46;
                        box.height(newBoxHeight);

                        _box_recent_height = _available_height - newBoxHeight;
                    }
                }
            }

            [this.$boxPinned,this.$boxToday, this.$boxRecovery].map(el => updateBoxHeight(el));

            /*$boxRecent.height() != _box_recent_height &&*/
            this.$boxRecent.height(_box_recent_height);
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
        let collectionRecents, collectionRecovers, collectionPinned, collectionToday;
        let ppmenu;
        let panelCreateNew;
        let dragAndDropZone;
        let welcomeComponent;
        const ITEMS_LOAD_RANGE = 40;

        const isToday = (dateString) => {
            const [datePart, _] = dateString.split(' ');
            const [day, month, year] = datePart.split('.').map(Number);
            const date = new Date(year, month - 1, day);

            return new Date().toDateString() === date.toDateString();
        }

        const _add_recent_block = function() {
            if ( !this.rawRecents || !Object.keys(this.rawRecents).length ) return;

            const _raw_block = this.rawRecents.slice(this.recentIndex, this.recentIndex + ITEMS_LOAD_RANGE);
            const _files = utils.fn.parseRecent(_raw_block);

            let _check_block = {};

            for (let item of _files) {
                var model = new FileModel(item);
                model.set('hash', item.path.hashCode());
                model.events.changed.attach(_on_pin_recent);

                if (!!this.rawRecents) {
                    if (isToday(model.date)) {
                        collectionToday.add(model);
                    } else {
                        collectionRecents.add(model);
                    }

                    _check_block[model.get('hash')] = item.path;
                } else return;
            }

            const _new_items_count = Object.keys(_check_block).length;
            if ( _new_items_count ) {
                if ( this.appready ) {
                    sdk.execCommand('files:check', JSON.stringify(_check_block));
                }

                Object.assign(this.check_list, _check_block);
            }

            if ( _new_items_count == ITEMS_LOAD_RANGE ) {
                setTimeout(e => {
                    this.recentIndex += ITEMS_LOAD_RANGE;
                    _add_recent_block.call(this);
                }, 10);
            } else {
                this.rawRecents = undefined;
                CommonEvents.fire('recent:filter', [null]);
            }
        };

        var _on_recents = function(params) {
            this.rawRecents = undefined;

            setTimeout(e => {
                this.rawRecents = params;
                this.recentIndex = 0;

                collectionRecents.empty();
                collectionToday.empty();
                collectionPinned.empty();

                this.view.$boxPinned.hide();
                this.view.$boxToday.hide();

                _add_recent_block.call(this);
            }, 10)
        };

        var _on_recovers = function(params) {
            collectionRecovers.empty();

            var files = utils.fn.parseRecent(params);
            for (let item of files) {
                collectionRecovers.add( new FileModel(item) );
            }

            this.view.$boxRecovery[collectionRecovers.size() > 0 ? 'show' : 'hide']();
            // this.view.updatelistsize();
        };

        function bindButtons(collection, model, view) {
            $(`#${model.uid}-pin-btn`, view).click((e) => {
                e.preventDefault();
                e.stopPropagation();

                model.set('pinned', !model.pinned);
            })

            $(`#${model.uid}-more-btn`, view).click((e) => {
                e.preventDefault();
                e.stopPropagation();
                collection.events.contextmenu.notify(model, e);
            })
        }

        function _on_pin_recent(model, property) {
            if ( property['pinned'] !== undefined ) {
                sdk.setRecentFilePinned(model.get('fileid'), property['pinned']);
            }
        }

        function _init_collections() {
            let _cl_rcbox = this.view.$panel.find('#box-recent'),
                _cl_rvbox = this.view.$panel.find('#box-recovery'),
                boxPinned = this.view.$panel.find('#box-pinned'),
                boxToday = this.view.$panel.find('#box-today');

            // Pinned
            //

            collectionPinned = new Collection({
                view: boxPinned,
                list: boxPinned.find('.table-files.list')
            });

            collectionPinned.events.click.attach((collection, model) => {
                openFile(OPEN_FILE_RECENT, model);
            });

            collectionPinned.events.inserted.attach((collection, model)=>{
                let $item = this.view.listitemtemplate(model);

                collection.list.append($item);
                let $el = collection.list.find('#' + model.uid);
                $el.addClass('pinned');

                bindButtons(collection, model, this.view.$panel);

                this.view.$boxPinned.show();
                // this.view.updatelistsize();
            });

            collectionPinned.events.deleted.attach((collection, model)=> {
                collection.list.find('#' + model.uid)?.remove();

                if (collection.size() === 0) {
                    this.view.$boxPinned.hide();
                    // this.view.updatelistsize();
                }
            });

            collectionPinned.events.contextmenu.attach(function(collection, model, e){
                ppmenu.actionlist = 'recent';
                ppmenu.hideItem('files:unpin', model.dir || !model.pinned || !model.exist);
                ppmenu.hideItem('files:pin', model.dir || model.pinned || !model.exist);
                ppmenu.hideItem('files:explore', (!model.islocal && !model.dir) || !model.exist);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });

            collectionPinned.events.changed.attach((collection, model) => {
                if (!model.pinned) {
                    collection.remove(model);

                    if (isToday(model.date)) {
                        collectionToday.add(model);
                    } else {
                        collectionRecents.add(model);
                    }

                    return;
                }

                let $el = collection.list.find('#' + model.uid);
                if ($el) {
                    $el[model.exist ? 'removeClass' : 'addClass']('unavail');
                }
            });

            collectionPinned.events.erased.attach(collection => {
                collection.list.parent().addClass('empty');
            });


            const createRecentlyCollection = (box) => {
                const collectionRecently = new Collection({
                    view: box,
                    list: box.find('.table-files.list')
                });


                collectionRecently.events.erased.attach(collection => {
                    collection.list.parent().addClass('empty');
                });

                collectionRecently.events.deleted.attach((collection, model)=> {
                    collection.list.find('#' + model.uid)?.remove();
                });

                collectionRecently.events.click.attach((collection, model) => {
                    openFile(OPEN_FILE_RECENT, model);
                });

                collectionRecently.events.contextmenu.attach((collection, model, e) => {
                    ppmenu.actionlist = 'recent';
                    ppmenu.hideItem('files:unpin', model.dir || !model.pinned || !model.exist);
                    ppmenu.hideItem('files:pin', model.dir || model.pinned || !model.exist);
                    ppmenu.hideItemAfter('files:unpin', model.dir || !model.exist);
                    ppmenu.hideItem('files:explore', (!model.islocal && !model.dir) || !model.exist);
                    ppmenu.show({left: e.clientX, top: e.clientY}, model);
                });

                collectionRecently.events.changed.attach((collection, model) => {
                    if (model.pinned) {
                        collection.remove(model);
                        collectionPinned.add(model);
                        return;
                    }

                    let $el = collection.list.find('#' + model.uid);
                    if ($el) {
                        $el[model.exist ? 'removeClass' : 'addClass']('unavail');
                    }
                });

                collectionRecently.empty();

                return collectionRecently;
            }

            // Today
            //

            collectionToday = createRecentlyCollection(boxToday);

            collectionToday.events.inserted.attach((collection, model) => {
                if (model.pinned) {
                    collection.remove(model);
                    collectionPinned.add(model);
                    return;
                }

                let $item = this.view.listitemtemplate(model);
                collection.list.append($item);

                bindButtons(collection, model, this.view.$panel);

                this.view.$boxToday.show();
                // this.view.updatelistsize();
            });

            // Recents
            //

            collectionRecents = createRecentlyCollection(_cl_rcbox);

            collectionRecents.events.inserted.attach((collection, model) => {
                if (model.pinned) {
                    collection.remove(model);
                    collectionPinned.add(model);
                    return;
                }

                let $item = this.view.listitemtemplate(model);
                collection.list.append($item);

                bindButtons(collection, model, this.view.$panel);

                collection.list.parent().removeClass('empty');
            });

            // Recovers
            //

            collectionRecovers = new Collection({
                view: _cl_rvbox,
                list: _cl_rvbox.find('.table-files.list')
            });
            collectionRecovers.events.inserted.attach((collection, model)=>{
                collection.list.append( this.view.listitemtemplate(model) );
                bindButtons(collection, model, this.view.$panel);
            });
            collectionRecovers.events.click.attach((collection, model)=>{
                openFile(OPEN_FILE_RECOVERY, model);
            });
            collectionRecovers.events.contextmenu.attach((collection, model, e)=>{
                ppmenu.actionlist = 'recovery';
                ppmenu.hideItem('files:unpin', true);
                ppmenu.hideItem('files:pin', true);
                ppmenu.hideItemAfter('files:unpin', true);
                ppmenu.hideItem('files:explore', true);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });
        };

        function _init_ppmenu() {
            ppmenu = new Menu({
                id: 'pp-menu-files',
                bottomlimitoffset: 10,
                items: [
                    {
                        caption: utils.Lang.menuFileOpen,
                        action: 'files:open'
                    },
                    {
                        caption: '--',
                    },
                    {
                        caption: utils.Lang.menuFilePin,
                        action: 'files:pin',
                        icon: 'pin20'
                    },
                    {
                        caption: utils.Lang.menuFileUnpin,
                        action: 'files:unpin',
                        icon: 'unpin'
                    },
                    {
                        caption: '--',
                    },
                    {
                        caption: utils.Lang.menuFileExplore,
                        action: 'files:explore',
                        icon: 'gofolder'
                    },
                    {
                        caption: utils.Lang.menuRemoveModel,
                        action: 'files:forget',
                        icon: 'remove'
                    },
                    {
                        caption: '--',
                    },
                    {
                        caption: utils.Lang.menuClear,
                        action: 'files:clear'
                    },
                ]
            });

            ppmenu.init('#placeholder');
            ppmenu.events.itemclick.attach(_on_context_menu.bind(this));
        };

        function _on_context_menu(menu, action, data) {
            if (/\:open/.test(action)) {
                menu.actionlist == 'recent' ?
                    openFile(OPEN_FILE_RECENT, data) :
                    openFile(OPEN_FILE_RECOVERY, data);
            } else if (/\:pin/.test(action)) {
                collectionRecents.find('uid', data.uid)?.set('pinned', true);
                collectionToday.find('uid', data.uid)?.set('pinned', true);
            } else if (/\:unpin/.test(action)) {
                collectionPinned.find('uid', data.uid)?.set('pinned', false);
            } else if (/\:clear/.test(action)) {
                menu.actionlist == 'recent' ?
                    window.sdk.LocalFileRemoveAllRecents() :
                    window.sdk.LocalFileRemoveAllRecovers();
            } else if (/\:forget/.test(action)) {
                $('#' + data.uid, this.view.$panel).addClass('lost');
                setTimeout(e => {
                    menu.actionlist == 'recent' ?
                        window.sdk.LocalFileRemoveRecent(parseInt(data.fileid)) :
                        window.sdk.LocalFileRemoveRecover(parseInt(data.fileid));}
                , 300); // 300ms - duration of item's 'collapse' transition
            } else if (/\:explore/.test(action)) {
                if (menu.actionlist == 'recent') {
                    sdk.execCommand('files:explore', data.path);
                }
            }
        };

        function _on_filter_recents(e) {
            console.log('on recents filter', e.target.value)

            const _filter = e.target.value;
            if ( !_filter.length ) {
                $('.table-files tr.hidden', this.view.$panel).removeClass('hidden')

                collectionRecents.items.forEach(model => model.set('hidden', false));
            } else {
                const re = new RegExp(_filter, "gi");
                collectionRecents.items.forEach(model => {
                    const _path = model.get('path');
                    if ( !re.test(_path) ) {
                        $('#' + model.uid, this.view.$panel).addClass('hidden');
                        model.set('hidden', true);
                    } else
                    if ( model.get('hidden') ) {
                        $('#' + model.uid, this.view.$panel).removeClass('hidden');
                        model.set('hidden', false);
                    }
                });
            }
        }

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();
                this.check_list = {};

                panelCreateNew = new PanelCreateNew();
                panelCreateNew.render(this.view.$panel.find("#box-create-new"));

                dragAndDropZone = new DragAndDropFileZone();
                dragAndDropZone.render(this.view.$panel.find("#dnd-file-zone"));
                dragAndDropZone.hide();

                if (!localStorage.welcome) {
                    welcomeComponent = new WelcomeComponent();
                    welcomeComponent.render(this.view.$panel.find("#welcome-box"));
                }

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
                    // this.view.updatelistsize();
                });
                CommonEvents.on("icons:svg", this.view.onscale);
                CommonEvents.on('portal:authorized', (data)=>{
                    if ( data.type == 'fileid' ) {
                        let fileid = data.id;
                        // openFile(OPEN_FILE_RECENT, fileid);
                    }

                    console.log('portal authorized');
                });
                CommonEvents.on('recent:filter', this.filterRecents);

                $('#box-recent .table-box').scroll(e => {
                    if ( Menu.opened )
                        Menu.closeAll();
                });

                $('#idx-recent-filter', this.view.$panel).on('input', _on_filter_recents.bind(this));

                return this;
            },
            getRecents: function() {
                return collectionRecents;
            },
            getRecovers: function() {
                return collectionRecovers;
            },
            filterRecents: function(doctype) {
                if (welcomeComponent) {
                    if (localStorage.welcome) {
                        welcomeComponent.detach();
                        welcomeComponent = null;
                        $('.recent-flex-box').show();
                    } else {
                        $('.recent-flex-box').hide();
                        return;
                    }
                }

                $('.recent-box-wrapper .table-files').removeClass('filter-word filter-cell filter-slide filter-pdfe');
                panelCreateNew.filter(doctype);
                const $title = $('.recent-flex-box > .text-headline-1')
                if (doctype) {
                    $('.recent-box-wrapper .table-files').addClass(`filter-${doctype}`);
                }

                let totalItems = 0;
                $('.recent-box-wrapper').each(function() {
                    const items = $(this).find(`tr[data-editor-type${doctype ? `="${doctype}"` : ''}]`);
                    $(this)[items.size() === 0 ? 'hide' : 'show']();

                    totalItems+=items.size()
                });

                $title[totalItems === 0 ? 'hide' : 'show']();
                dragAndDropZone[totalItems === 0 ? 'show' : 'hide']();
            },
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