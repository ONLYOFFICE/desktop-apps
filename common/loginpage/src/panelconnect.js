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

/*
    "connect to portal" page
    controller + view
*/

+function(){ 'use strict'
    var ControllerPortals = function(args) {
        args.caption = 'Connect to portal';
        args.action = 
        this.action = "connect";

        this.view = new ViewPortals(args);
    };

    ControllerPortals.prototype = Object.create(baseController.prototype);
    ControllerPortals.prototype.constructor = ControllerPortals;

    var ViewPortals = function(args) {
        var _lang = utils.Lang;

        args.id&&(args.id=`id=${args.id}`)||(args.id='');

        var _html = `<div ${args.id} class="action-panel ${args.action}">` +
                      '<div id="box-empty-portals" class="empty flex-center">' +
                        '<section class="center-box">'+
                          `<h3 class="empty-title" style="margin:0 0 42px;">${_lang.portalEmptyTitle}</h3>`+
                          '<img class="img-connect">' +
                          `<h4 class="text-description" style="margin:38px 0 6px;color:#666666;">${_lang.portalEmptyDescr}</h4>` +
                          '<div class="tools-connect">'+
                            `<button class="btn primary newportal">${_lang.btnCreatePortal}</button>`+
                            '<section class="link-connect">'+
                              `<label>${_lang.textHavePortal}</label><a class="login link" href="#">${_lang.btnConnect}</a>`+
                            '</section>'+
                          '</div>'+
                        '</section>'+
                      '</div>'+
                      '<div id="box-portals">' +
                        '<div class="flexbox">'+
                          `<h3 class="table-caption">${_lang.portalListTitle}</h3>`+
                          '<div class="table-box flex-fill"><table class="table-files list"></table></div>'+
                          '<div class="lst-tools">'+
                            `<button id="btn-addportal" class="btn login">${_lang.btnAddPortal}</button>`+
                          '</div>'+
                        '</div>'+
                      '</div>' +
                    '</div>';

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 2;
        args.itemtext = _lang.actConnectTo;

        baseView.prototype.constructor.call(this, args);
    };

    ViewPortals.prototype = Object.create(baseView.prototype);
    ViewPortals.prototype.constructor = ViewPortals;

    utils.fn.extend(ViewPortals.prototype, {
        render: function() {
            baseView.prototype.render.apply(this, arguments);

            this.$panelNoPortals = this.$panel.find('#box-empty-portals');
            this.$panelPortalList = this.$panel.find('#box-portals');

            if ( !localStorage['commercial'] ) {
                var onadsclick = (e) => {
                    let $el = $(e.target);
                    let action = $el.attr('action');

                    let $title = this.$panel.find('h3.empty-title'),
                        $descr = this.$panel.find('h4.text-description'),
                        $img = this.$panel.find('img.img-connect');

                    if (/^custom/.test(action)) {
                        $('.action-panel').hide();
                        this.$panel.show();
                        $('.tool-menu > .menu-item').removeClass('selected');

                        if (/verhistory$/.test(action)) {
                            $title.html(utils.Lang.adsTitle1);
                            $descr.html(utils.Lang.adsDescr1);
                            $img.removeClass('docreview').addClass('verhistory');
                            this.$adsItemHistory.addClass('selected');
                        } else {
                            $title.html(utils.Lang.adsTitle2);
                            $descr.html(utils.Lang.adsDescr2);
                            $img.removeClass('verhistory').addClass('docreview');
                            this.$adsItemReview.addClass('selected');
                        }
                    } else
                    if (/^connect/.test(action)) {
                        $title.html(utils.Lang.portalEmptyTitle)
                        $descr.html(utils.Lang.portalEmptyDescr);
                        $img.removeClass('verhistory').removeClass('docreview');
                    }
                };

                let action = 'custom ads-verhistory';
                this.$adsItemHistory = this.renderMenuItem(`<li class="menu-item"><a action='${action}'>${utils.Lang.adsToolItem1}</a></li>`);
                this.$adsItemHistory.on('click', onadsclick);

                action = 'custom ads-docreview';
                this.$adsItemReview = this.renderMenuItem(`<li class="menu-item"><a action='${action}'>${utils.Lang.adsToolItem2}</a></li>`);
                this.$adsItemReview.on('click', onadsclick);

                $(this.menuContainer).find('[action=connect]').parent().on('click', onadsclick);
            }
        },
        portaltemplate: function(info) {
            return `<tr id=${info.elid}><td class="row-cell cportal primary">${utils.skipUrlProtocol(info.portal)}</td>` +
                          `<td class="row-cell cuser minor"><span>${info.user}</span></td>` +
                          `<td class="row-cell cemail minor"><span>${info.email}</span></td>` +
                          '<td class="cell-tools">'+
                          '<div class="hlayout">'+
                            '<button class="btn-quick logout img-el"></button>'+
                          '</span>'+
                        '</td>';
        }
    });

    window.ControllerPortals = ControllerPortals;
    utils.fn.extend(ControllerPortals.prototype, (function() {
        let collection,
            ppmenu;
        let dlgLogin;

        function _on_context_menu(menu, action, data) {
            var model = data;
            if (/\:open/.test(action)) {
                model.logged ?
                    window.sdk.execCommand("portal:open", model.path) :
                        doLogin(model.path, model.email);
            } else
            if (/\:logout/.test(action)) {
                _do_logout.call(this, model.name);
            } else
            if (/\:forget/.test(action)) {
                model.removed = true;
                _do_logout.call(this, model.name);
            }
        };

        function _do_login(portal, user) {
            if ( !dlgLogin ) {
                dlgLogin = new LoginDlg();
                dlgLogin.onsuccess(info => {
                    if ( info.status == 'sso' ) {
                        window.sdk.execCommand("portal:open", info.provider);
                    } else
                    if ( info.status == 'user' ) {
                        window.sdk.execCommand("portal:open", info.data.portal);

                        dlgLogin.onclose();
                        PortalsStore.keep(info);
                        _update_portals.call(this);

                        window.selectAction('connect');
                    }
                });
                dlgLogin.onclose(code=>{
                    dlgLogin = undefined;
                });
                dlgLogin.show(portal, user);
            }
        };

        function _authorize(portal, user, data) {
            if ( !dlgLogin ) {
                dlgLogin = new LoginDlg();
                dlgLogin.onsuccess(info => {
                    dlgLogin.onclose();
                    PortalsStore.keep(info);
                    _update_portals.call(this);

                    CommonEvents.fire('portal:authorized', [data]);
                });
                dlgLogin.onclose(code=>{
                    dlgLogin = undefined;
                });
                dlgLogin.show(portal, user);
            }
        };

        function _do_logout(info) {
            // var model = portalCollection.find('name', info);
            // model && model.set('logged', false);

            window.sdk.execCommand("portal:logout", info);
        };

        function _update_portals() {
            collection.empty();

            /* fill portals list */
            var portals = PortalsStore.portals();

            if (portals.length) {
                !localStorage['commercial'] &&
                    localStorage.setItem('commercial', 'showed');

                let auth_arr = {};
                for (let rec of portals) {
                    var pm = new PortalModel(rec);

                    auth_arr[pm.name] = '';
                    collection.add(pm);
                }

                window.sdk && window.sdk.checkAuth && window.sdk.checkAuth(auth_arr);

                this.view.$panelNoPortals.hide();
                this.view.$panelPortalList.show();

                if ( !!this.view.$adsItemReview ) {
                    this.view.$adsItemReview.hide();
                    this.view.$adsItemHistory.hide();
                }
            } else {
                this.view.$panelNoPortals.show();
                this.view.$panelPortalList.hide();
            }
        };

        var _init_collection = function() {
                collection = new Collection({
                    view: this.view.$panelPortalList,
                    list: '.table-files.list'
                });

                collection.events.changed.attach((collection, model) => {
                    this.view.$panelPortalList.find('#' + model.uid)[model.logged?'addClass':'removeClass']('logged');
                });

                collection.events.inserted.attach((collection, model) => {
                    let $listPortals = collection.view.find('.table-files.list');
                    let $item = $(this.view.portaltemplate({
                        portal: model.name,
                        user: model.user,
                        email: model.email,
                        elid: model.uid
                    }));
                    
                    $item.find('.logout').click(model.name, e => {
                        _do_logout(e.data);

                        e.stopPropagation && e.stopPropagation();
                        return false;
                    });
                    
                    $listPortals.append($item);
                });

                collection.events.click.attach((collection, model)=>{
                    model.logged ?
                        window.sdk.execCommand("portal:open", model.path) :
                        _do_login.call(this, model.path, model.email);
                });

                collection.events.contextmenu.attach((collection, model, e)=>{
                    ppmenu.disableItem('portal:logout', !model.logged);
                    ppmenu.show({left: e.clientX, top: e.clientY}, model);
                });
        };

        var _init_ppmenu = function() {
            ppmenu = new Menu({
                id: 'pp-menu-portals',
                items: [{
                    caption: utils.Lang.menuFileOpen,
                    action: 'portal:open'
                },{
                    caption: utils.Lang.menuLogout,
                    action: 'portal:logout'
                },{
                    caption: utils.Lang.menuRemoveModel,
                    action: 'portal:forget'
                }]
            });

            ppmenu.init('#placeholder');
            ppmenu.events.itemclick.attach( _on_context_menu.bind(this) );
        };

        var _apply_auth = function(obj) {
            for (let i in obj) {
                let model = collection.find('name', i);

                if (model) model.set('logged', obj[i].length > 0);
            };
        };

        var _on_create_portal = function() {
            dlgLogin && dlgLogin.close();
            window.sdk.execCommand('portal:create', '');
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                window.sdk.on('on_check_auth', _apply_auth.bind(this));
                window.sdk.on('on_native_message', (cmd, param)=>{
                    let res = /portal:logout(\:cancel)?/.exec(cmd);
                    if (!!res && !!res[0]) {
                        var short_name = utils.skipUrlProtocol(param);
                        var model = collection.find('name', short_name);

                        if (!!model) {
                            if (!res[1]) {
                                model.set('logged', false);
                                model.removed &&
                                    PortalsStore.forget(param) && _update_portals.call(this);
                            } else
                                delete model.removed;
                        }
                    } else
                    if (/portal:login/.test(cmd)) {
                        let obj = JSON.parse(utils.fn.decodeHtml(param));
                        if ( obj ) {
                            var model = collection.find('name', utils.skipUrlProtocol(obj.domain));
                            if ( model ) {
                                !model.get('logged') && model.set('logged', true);
                            } else {
                                let info = {
                                    portal: obj.domain,
                                    user: obj.displayName,
                                    email: obj.email
                                };

                                info.portal.endsWith('/') &&
                                    (info.portal = info.portal.slice(0,-1));

                                PortalsStore.keep(info);
                                _update_portals.call(this);
                            }
                        }
                    }
                });

                _init_collection.call(this);
                _update_portals.call(this);
                _init_ppmenu.call(this);

                $('body').on('click', '.login', e=>{
                    _do_login.call(this);
                });

                window.CommonEvents.on('portal:create', _on_create_portal);

                return this;
            },
            isConnected: function(portal) {
                var model = collection.find('name', utils.skipUrlProtocol(portal));
                return model && model.logged;
            },
            authorizeOn: function(portal, data) {
                var model = collection.find('name', utils.skipUrlProtocol(portal));
                if ( !model ) {
                    _authorize.call(this, portal, undefined, data);
                } else
                if ( !model.logged ) {
                    _authorize.call(this, portal, model.email, data);
                }
            }
        };
    })());
}();

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerPortals({});
//     p.init();

//     !window.app && (window.app = {controller:{}});
//     !window.app.controller && (window.app.controller = {});
//     window.app.controller.portals = p;
// });