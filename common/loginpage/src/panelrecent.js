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

+function () {
    'use strict'
    var ControllerRecent = function (args = {}) {
        args.caption = 'Recent files';
        args.action =
            this.action = "recents";
        this.view = new ViewRecent(args);
    };

    ControllerRecent.prototype = Object.create(baseController.prototype);
    ControllerRecent.prototype.constructor = ControllerRecent;
    const isSvgIcons = window.devicePixelRatio >= 2 || window.devicePixelRatio === 1;
    var ViewRecent = function (args) {
        var _lang = utils.Lang;

        // args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        // localStorage.removeItem('welcome');

        //language=HTML
        const welcomeBannerTemplate = !localStorage.getItem('welcome') ? `
            <div id="area-welcome">
                <h2 l10n>${_lang.welWelcome}</h2>
                <p l10n class="text-normal">${_lang.welDescr}</p>
                <p l10n class="text-normal">${_lang.welNeedHelp}</p>
            </div>` : '';

        //language=HTML
        args.tplPage = `
            <div class="action-panel ${args.action}">
                <div class="recent-panel-container">
                    <div class="search-bar hidden">
                        <h1 l10n>${_lang.welWelcome}</h1>
                    </div>

                    <section id="area-document-creation-grid"></section>
                    ${welcomeBannerTemplate}
                    <section id="area-dnd-file"></section>

                    <div id="box-container">
                        <div id="box-recovery">
                            <div class="file-list-title">
                                <h3 l10n>${_lang.listRecoveryTitle}</h3>
                            </div>
                            <div class="file-list-head text-normal">
                                <div class="col-name" l10n>${_lang.colFileName}</div>
                                <div class="col-location" l10n>${_lang.colLocation}</div>
                                <div class="col-date" l10n>${_lang.colLastOpened}</div>
                            </div>
                            <div class="file-list-body scrollable"></div>
                        </div>

                        <div id="box-recent">
                            <div class="file-list-title">
                                <h3 l10n>${_lang.listRecentFileTitle}</h3>
                            </div>
                            <div class="file-list-head text-normal">
                                <div class="col-name" l10n>${_lang.colFileName}</div>
                                <div class="col-location" l10n>${_lang.colLocation}</div>
                                <div class="col-date" l10n>${_lang.colLastOpened}</div>
                            </div>
                            <div class="file-list-body scrollable"></div>
                        </div>
                    </div>
                </div>
            </div>`;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 0;
        // args.itemtext = _lang.actRecentFiles;
        args.tplItem = 'nomenuitem';

        baseView.prototype.constructor.call(this, args);
    };

    ViewRecent.prototype = Object.create(baseView.prototype);
    ViewRecent.prototype.constructor = ViewRecent;
    utils.fn.extend(ViewRecent.prototype, {
        render: function () {
            baseView.prototype.render.apply(this, arguments);

            if (!localStorage.getItem('welcome')) {
                localStorage.setItem('welcome', '0');
            }

            this.$boxRecovery = this.$panel.find('#box-recovery');
            this.$boxRecent = this.$panel.find('#box-recent');
            this.$panelContainer = this.$panel.find('.recent-panel-container');
        },
        listitemtemplate: function (info) {
            let id = !!info.uid ? (` id="${info.uid}"`) : '';
            info.crypted === undefined && (info.crypted = false);
            const dotIndex = info.name.lastIndexOf('.');
            const fullName = info.name;
            if (dotIndex !== -1) {
                info.ext = info.name.substring(dotIndex);
                info.name = info.name.substring(0, dotIndex);
            } else {
                info.ext = '';
            }

            // todo: crypted icon

            //language=HTML
            let _tpl = `
                <div ${id} class="row text-normal">
                    <div class="col-name" title="${fullName}">
                        <div class="icon">
                            <svg class="icon" data-iconname="${info.type === 'folder' ? 'folder' : `${info.format}`}"
                                 data-precls="tool-icon">
                                <use xlink:href="#${info.type === 'folder' ? 'folder-small' : info.format}"></use>
                            </svg>
                            ${info.crypted ? `<svg class="icon" data-iconname="shield" data-precls="tool-icon">
                                                <use xlink:href="#shield"></use>
                                              </svg>` : ''}
                            ${!isSvgIcons ? `<i class="icon tool-icon ${info.type === 'folder' ? 'folder' : `${info.format}`}"></i>` : ''}
                        </div>
                        <p class="name">${info.name}</p>
                        <span class="ext">${info.ext}</span>
                    </div>
                    <div class="col-location" title="${info.descr}">
                        <!--              todo: icon here          -->
                        ${info.descr}
                    </div>
            `;

            if (info.type !== 'folder') {
                //language=HTML
                _tpl += `
                    <div class="col-date"><p>${info.date}</p></div>`;

                //language=HTML
                _tpl += `
                    <div class="col-pin">
                        <button id="${info.uid}-pin-btn" class="btn-quick">
                            <svg class="icon">
                                <use xlink:href="#pin"/>
                            </svg>
                            ${!isSvgIcons ? '<i class="icon tool-icon pin"></i>' : ''}
                        </button>
                    </div>`;

                //language=HTML
                _tpl += `
                    <div class="col-more">
                        <button id="${info.uid}-more-btn" class="btn-quick more">
                            <svg class="icon">
                                <use xlink:href="#more"/>
                            </svg>
                            ${!isSvgIcons ? '<i class="icon tool-icon more"></i>' : ''}
                        </button>
                    </div>`;
            }

            return _tpl + '</div>';
        },
        onscale: function (pasteSvg) {
            let elm, icoName, parent,
                emptylist = $('[class*="text-emptylist"]', '#box-recent');
            emptylist.toggleClass('text-emptylist text-emptylist-svg');

            if (pasteSvg && !emptylist.find('svg').length)
                emptylist.prepend($('<svg class = "icon"><use xlink:href="#folder-big"></use></svg>'));

            $('#box-recent .cicon').each(function () {
                elm = $(this);
                parent = elm.parent();
                if (parent.hasClass('crypted-svg') || parent.hasClass('crypted'))
                    parent.toggleClass('crypted-svg crypted');

                if (!pasteSvg || !!$('svg', elm).length) return;

                icoName = $('i.icon', elm).attr('class').split(' ').filter((cls) => cls != 'icon' && cls != 'img-format');
                elm.append($(`<svg class = "icon"><use xlink:href="#${icoName}"></use></svg>`));
                if (parent.hasClass('crypted-svg'))
                    elm.append($('<svg class = "shield"><use xlink:href="#shield"></use></svg>'));
            });

            $('#box-recent-folders td.cicon').each(function () {
                elm = $(this)
                parent = elm.parent();
                if (parent.hasClass('crypted-svg') || parent.hasClass('crypted'))
                    parent.toggleClass('crypted-svg crypted');
                if (!pasteSvg || !!$('svg', elm).length) return;

                elm.append($('<svg class = "icon  folder"> <use xlink:href="#folder-small"></use></svg>'));
                if (parent.hasClass('crypted-svg'))
                    elm.append($('<svg class = "shield"><use xlink:href="#shield"></use></svg>'));

            });
        }
    });

    window.ControllerRecent = ControllerRecent;

    String.prototype.hashCode = function () {
        var hash = 0, i, chr;
        if (this.length === 0) return hash;

        for (i = this.length; !(--i < 0);) {
            chr = this.charCodeAt(i);
            hash = ((hash << 5) - hash) + chr;
            hash = hash & hash; // Convert to 32bit integer
        }

        return hash;
    };

    utils.fn.extend(ControllerRecent.prototype, (function () {
        let collectionRecents, collectionRecovers;
        let ppmenu;
        const ITEMS_LOAD_RANGE = 40;

        const _add_recent_block = function () {
            if (!this.rawRecents || !Object.keys(this.rawRecents).length) return;

            const _raw_block = this.rawRecents.slice(this.recentIndex, this.recentIndex + ITEMS_LOAD_RANGE);
            const _files = utils.fn.parseRecent(_raw_block);

            let _check_block = {};

            for (let item of _files) {
                var model = new FileModel(item);
                model.set('hash', item.path.hashCode());

                if (!!this.rawRecents) {
                    collectionRecents.add(model);
                    _check_block[model.get('hash')] = item.path;
                } else return;
            }

            const _new_items_count = Object.keys(_check_block).length;
            if (_new_items_count) {
                if (this.appready) {
                    sdk.execCommand('files:check', JSON.stringify(_check_block));
                }

                Object.assign(this.check_list, _check_block);
            }

            if (_new_items_count === ITEMS_LOAD_RANGE) {
                setTimeout(e => {
                    this.recentIndex += ITEMS_LOAD_RANGE;
                    _add_recent_block.call(this);
                }, 10);
            } else {
                this.rawRecents = undefined;
            }

            // this.view.$boxRecent.css('display', collectionRecents.size() > 0 ? 'flex' : 'none');

            if (collectionRecents.size() > 0 || collectionRecovers.size() > 0) {
                this.dndZone.hide();
            }
        };

        var _on_recents = function (params) {
            this.rawRecents = undefined;

            setTimeout(e => {
                this.rawRecents = params;
                this.recentIndex = 0;

                collectionRecents.empty();
                _add_recent_block.call(this);
            }, 10)
        };

        var _on_recovers = function (params) {
            collectionRecovers.empty();

            var files = utils.fn.parseRecent(params);
            for (let item of files) {
                collectionRecovers.add(new FileModel(item));
            }

            this.view.$boxRecovery.css('display', collectionRecovers.size() > 0 ? 'flex' : 'none');

            if (collectionRecents.size() > 0 || collectionRecovers.size() > 0) {
                this.dndZone.hide();
            }
        };

        function addContextMenuEventListener(collection, model, view, actionList) {
            $(`#${model.uid}-pin-btn`, view).click((e) => {
                e.stopPropagation();
                model.set('pinned', !model.pinned);
                model.set('pinid', !model.pinned ? model.fileid : -model.fileid);
            })

            $(`#${model.uid}-more-btn`, view).click((e) => {
                e.stopPropagation();

                if (Menu.opened) {
                    Menu.closeAll();
                    return;
                }

                ppmenu.actionlist = actionList;
                if (actionList === 'recovery') {
                    ppmenu.hideItem('files:explore', true);
                    ppmenu.hideItem('files:pin', true);
                    ppmenu.hideItem('files:unpin', true);
                } else {
                    ppmenu.hideItem('files:explore', (!model.islocal && !model.dir) || !model.exist);
                    ppmenu.hideItem(model.pinned ? 'files:pin' : 'files:unpin', true);
                    ppmenu.hideItem(model.pinned ? 'files:unpin' : 'files:pin', false);
                }

                ppmenu.showUnderElem(e.currentTarget, model, $('body').hasClass('rtl') ? 'left' : 'right');
            })
        }

        function handlePin(collection, model) {
            let $el = $('#' + model.uid, collection.list);
            if ($el.length) {
                const f = collection.items.find((elem) => {
                    return model.pinid <= 0 ? elem.pinid < model.pinid : elem.pinid > model.pinid
                });

                if (f) {
                    const $item = $('#' + f.uid, collection.list);
                    $el.insertBefore($item);
                } else if (!f && model.pinid > 0) {
                    $el.appendTo(collection.list);
                } else {
                    $el.prependTo(collection.list);
                }
            }
        }

        function _init_collections() {
            let _cl_rcbox = this.view.$boxRecent,
                _cl_rvbox = this.view.$boxRecovery;

            collectionRecents = new Collection({
                view: _cl_rcbox,
                list: _cl_rcbox.find('.file-list-body')
            });

            collectionRecents.events.erased.attach(collection => {
                collection.list.parent().addClass('empty');
            });

            collectionRecents.events.inserted.attach((collection, model) => {
                let $item = $(this.view.listitemtemplate(model));

                if (model.pinned) {
                    const $pinned = collection.list.children('.row.pinned');
                    if ($pinned.length) {
                        $item.insertAfter($pinned.last());
                    } else {
                        $item.prependTo(collection.list);
                    }
                } else {
                    collection.list.append($item);
                }

                $item[model.pinned ? 'addClass' : 'removeClass']('pinned');

                addContextMenuEventListener(collection, model, this.view.$panel, 'recent');

                collection.list.parent().removeClass('empty');
            });

            collectionRecents.events.click.attach((collection, model) => {
                // var _portal = model.descr;
                // if ( !model.islocal && !app.controller.portals.isConnected(_portal) ) {
                // app.controller.portals.authorizeOn(_portal, {type: 'fileid', id: model.fileid});
                // } else {
                openFile(OPEN_FILE_RECENT, model);
                // }
            });

            collectionRecents.events.contextmenu.attach(function (collection, model, e) {
                ppmenu.actionlist = 'recent';
                ppmenu.hideItem('files:explore', (!model.islocal && !model.dir) || !model.exist);
                ppmenu.hideItem(model.pinned ? 'files:pin' : 'files:unpin', true);
                ppmenu.hideItem(model.pinned ? 'files:unpin' : 'files:pin', false);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });

            collectionRecents.events.changed.attach(function (collection, model, property) {
                let $el = collection.list.find('#' + model.uid);
                if ($el) {
                    $el[model.exist ? 'removeClass' : 'addClass']('unavail');
                    if (property['pinned'] !== undefined) {
                        // handlePin(collection, model);
                        sdk.setRecentFilePinned(model.get('fileid'), property['pinned']);
                        $el[model.pinned ? 'addClass' : 'removeClass']('pinned');
                    }

                    if (property.pinid != undefined) {
                        handlePin(collection, model);
                    }
                }
            });

            collectionRecents.empty();

            /**/

            collectionRecovers = new Collection({
                view: _cl_rvbox,
                list: _cl_rvbox.find('.file-list-body')
            });
            collectionRecovers.events.inserted.attach((collection, model) => {
                collection.list.append(this.view.listitemtemplate(model));
                addContextMenuEventListener(collection, model, this.view.$panel, 'recovery');
            });
            collectionRecovers.events.click.attach((collection, model) => {
                openFile(OPEN_FILE_RECOVERY, model);
            });
            collectionRecovers.events.contextmenu.attach((collection, model, e) => {
                ppmenu.actionlist = 'recovery';
                ppmenu.hideItem('files:explore', true);
                ppmenu.hideItem('files:pin', true);
                ppmenu.hideItem('files:unpin', true);
                ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });
        };

        function _init_ppmenu() {
            ppmenu = new Menu({
                id: 'pp-menu-files',
                className: 'with-icons',
                bottomlimitoffset: 10,
                items: [
                    {caption: utils.Lang.menuFileOpen, action: 'files:open', icon: '#folder'},
                    {caption: utils.Lang.menuFilePin, action: 'files:pin', icon: '#pin20'},
                    {caption: utils.Lang.menuFileUnpin, action: 'files:unpin', icon: '#unpin20'},
                    {caption: utils.Lang.menuFileExplore, action: 'files:explore', icon: '#gofolder'},
                    {caption: utils.Lang.menuRemoveModel, action: 'files:forget', icon: '#remove'},
                    {caption: '--'},
                    {caption: utils.Lang.menuClear, action: 'files:clear', variant: 'negative'}
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
            } else if (/\:unpin/.test(action)) {
                collectionRecents.find('uid', data.uid)?.set('unpinned', true);
            } else if (/\:clear/.test(action)) {
                if (menu.actionlist === 'recent') {
                    window.sdk.LocalFileRemoveAllRecents();
                    if (collectionRecovers.size() === 0) {
                        this.dndZone.show();
                    }
                } else {
                    window.sdk.LocalFileRemoveAllRecovers();
                    if (collectionRecents.size() === 0) {
                        this.dndZone.show();
                    }
                }
            } else if (/\:forget/.test(action)) {
                $('#' + data.uid, this.view.$panel).addClass('lost');

                if (menu.actionlist === 'recent') {
                    window.sdk.LocalFileRemoveRecent(parseInt(data.fileid));
                    if (collectionRecovers.size() === 0) {
                        this.dndZone.show();
                    }
                } else {
                    window.sdk.LocalFileRemoveRecover(parseInt(data.fileid));
                    if (collectionRecents.size() === 0) {
                        this.dndZone.show();
                    }
                }
            } else if (/\:explore/.test(action)) {
                if (menu.actionlist == 'recent') {
                    sdk.execCommand('files:explore', JSON.stringify({
                        path: data.path,
                        id: data.fileid,
                        hash: data.hash
                    }));
                }
            }
        };

        // note: search function
        function _on_filter_recents(e) {
            console.log('on recents filter', e.target.value)

            const _filter = e.target.value;
            if (!_filter.length) {
                $('.table-files tr.hidden', this.view.$panel).removeClass('hidden')

                collectionRecents.items.forEach(model => model.set('hidden', false));
            } else {
                const re = new RegExp(_filter, "gi");
                collectionRecents.items.forEach(model => {
                    const _path = model.get('path');
                    if (!re.test(_path)) {
                        $('#' + model.uid, this.view.$panel).addClass('hidden');
                        model.set('hidden', true);
                    } else if (model.get('hidden')) {
                        $('#' + model.uid, this.view.$panel).removeClass('hidden');
                        model.set('hidden', false);
                    }
                });
            }
        }

        return {
            init: function () {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();
                this.check_list = {};

                _init_collections.call(this);
                _init_ppmenu.call(this);

                window.sdk.on('onupdaterecents', _on_recents.bind(this));
                window.sdk.on('onupdaterecovers', _on_recovers.bind(this));
                window.sdk.on('on_native_message', (cmd, param) => {
                    if (/files:checked/.test(cmd)) {
                        let fobjs = JSON.parse(param);
                        if (fobjs) {
                            for (let obj in fobjs) {
                                let value = JSON.parse(fobjs[obj]);
                                let model = collectionRecents.find('hash', parseInt(obj));
                                if (model) {
                                    model.get('exist') != value && model.set('exist', value);
                                }
                            }
                        }
                    } else if (/file\:skip/.test(cmd)) {
                        sdk.LocalFileRemoveRecent(parseInt(param));
                    } else if (/app\:ready/.test(cmd)) {
                        if (Object.keys(this.check_list).length) {
                            setTimeout(() => {
                                sdk.execCommand('files:check', JSON.stringify(this.check_list));
                            }, 100);
                        }

                        this.appready = true;
                    }
                });

                CommonEvents.on("icons:svg", this.view.onscale);
                CommonEvents.on('portal:authorized', (data) => {
                    if (data.type == 'fileid') {
                        let fileid = data.id;
                        // openFile(OPEN_FILE_RECENT, fileid);
                    }

                    console.log('portal authorized');
                });

                this.dndZone = new DnDFileZone();
                this.dndZone.render(this.view.$panel.find("#area-dnd-file"));

                const docGrid = new DocumentCreationGrid({
                    documentTypes: [
                        {
                            id: 'word',
                            title: utils.Lang.newDoc,
                            langKey: 'newDoc',
                            formatLabel: {
                                value: 'DOCX',
                                gradientColorStart: '#4298C5',
                                gradientColorEnd: '#2D84B2',
                                bgColorWinXP: '#287ca9',
                            },
                            icon: '#docx-big',
                        },
                        {
                            id: 'cell',
                            title: utils.Lang.newXlsx,
                            langKey: 'newXlsx',
                            formatLabel: {
                                value: 'XLSX',
                                gradientColorStart: '#5BB514',
                                gradientColorEnd: '#318C2B',
                                bgColorWinXP: '#3aa133',
                            },
                            icon: '#xlsx-big',
                        },
                        {
                            id: 'slide',
                            title: utils.Lang.newPptx,
                            langKey: 'newPptx',
                            formatLabel: {
                                value: 'PPTX',
                                gradientColorStart: '#F4893A',
                                gradientColorEnd: '#DE7341',
                                bgColorWinXP: '#f36700',
                            },
                            icon: '#pptx-big',
                        },
                        {
                            id: 'form',
                            title: utils.Lang.newForm,
                            langKey: 'newForm',
                            formatLabel: {
                                value: 'PDF',
                                gradientColorStart: '#F36653',
                                gradientColorEnd: '#D2402D',
                                bgColorWinXP: '#e54d39',
                            },
                            icon: '#pdf-big',
                        }
                    ],
                    onDocumentSelect: (docType) => {
                        window.sdk.command("create:new", docType);
                    }
                });

                docGrid.render(this.view.$panel.find("#area-document-creation-grid"));

                $('#idx-recent-filter', this.view.$panel).on('input', _on_filter_recents.bind(this));

                return this;
            },
            getRecents: function () {
                return collectionRecents;
            },
            getRecovers: function () {
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