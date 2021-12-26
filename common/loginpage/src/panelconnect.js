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
    "connect to portal" page
    controller + view
*/

+function(){ 'use strict'
    window.relpath = !/mac os/i.test(navigator.userAgent) ? '.' : '..';
    window.config = { portals: {
        update: function() {
            config.portals.checklist = sdk.externalClouds();

            if ( config.portals.checklist ) {
                let _providers = {};
                config.portals.checklist.forEach(item => {
                    _providers[item.provider] = item;
                });

                config.portals.providers = _providers;
                config.portals.providers.find = name => config.portals.providers[name];
            }
        }
    }};

    config.portals.update();

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

        var _html_empty_panel_with_carousel =
                        `<div id="box-empty-portals" class="empty flex-center">
                            <section class="center-box">
                              <h3 class="empty-title" l10n style="margin:0 0 60px;">${_lang.portalEmptyTitle}</h3>
                              <div class='carousel'>
                                <figure class='carousel__slidebox'>
                                    <div class='carousel__slide'>
                                        <p class='carousel__slide__text title' l10n>${_lang.emptySlide1Title}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.emptySlide1Text}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect1' data-src='connect1'>
                                        </svg>
                                    </div>
                                    <div class='carousel__slide'>
                                        <p class='carousel__slide__text title' l10n>${_lang.emptySlide2Title}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.emptySlide2Text}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect2' data-src='connect2'>
                                        </svg>
                                    </div>
                                    <div class='carousel__slide active'>
                                        <p class='carousel__slide__text title' l10n>${_lang.emptySlide3Title}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.emptySlide3Text}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect3' data-src='connect3'>
                                        </svg>
                                    </div>
                                </figure>
                                <nav class='carousel__scrolls'>
                                    <div class='carousel__scroll__btn prev' value='prev'></div>
                                    <div class='carousel__scroll__btn next' value='next'></div>
                                </nav>
                              </div>
                              <div class="tools-connect">
                                <button class="btn btn--landing newportal" l10n>${_lang.btnCreatePortal}</button>
                                <section class="link-connect">
                                  <label l10n>${_lang.textHavePortal}</label><a class="login link" l10n href="#">${_lang.btnConnect}</a>
                                </section>
                              </div>
                            </section>
                        </div>`;

        var _html_empty_panel_with_providers =
                            `<div id="box-empty-portals" class="empty flex-center">
                                <section id='connect-empty-var-2'>
                                    <h3 class="empty-title" style="margin:0;" l10n>${_lang.portalEmptyTitle}</h3>
                                    <h4 class='text-description' style='margin-bottom:50px;' l10n='portalEmptyDescr'>${_lang.portalEmptyDescr}</h4>
                                    <section class='tools-connect2'>
                                        <div id='box-providers-premium-button' />
                                        <div id="box-providers-buttons" style='font-size:0;' />
                                    </section>
                                    <h4 class='text-description separate-top' style='margin-bottom:8px;' l10n='portalEmptyAdv1'>${_lang.portalEmptyAdv1}</h4>
                                    <div class="tools-connect">
                                        <button class="btn btn--landing newportal" l10n>${_lang.btnCreatePortal}</button>
                                        <section class="link-connect">
                                            <label l10n>${_lang.textHavePortal}</label><a class="login link" href="#" l10n>${_lang.btnConnect}</a>
                                        </section>
                                    </div>
                                </section>
                            </div>`;

        var _html = `<div ${args.id} class="action-panel ${args.action}">
                      ${config.portals.checklist.length > 1 ? _html_empty_panel_with_providers : _html_empty_panel_with_carousel}
                      <div id="box-portals">
                        <div class="flexbox">
                          <h3 class="table-caption" l10n>${_lang.portalListTitle}</h3>
                          <div class="table-box flex-fill"><table class="table-files list"></table></div>
                          <div class="lst-tools">
                            <button id="btn-addportal" class="btn btn--primary login" l10n>${_lang.btnAddPortal}</button>
                          </div>
                        </div>
                      </div>
                    </div>`;

        if ( config.portals.checklist.length ) {
            const provider_button_template = (provider, name, icons) => {
                                                const icon_light = icons ? icons.themeLight.buttonLogo : '',
                                                        icon_dark = icons ? icons.themeDark.buttonLogo : '';
                                                const button_el = `<img class='icon icon__light' src='${relpath}/providers/${provider}/${icon_light}'></img>
                                                                    <img class='icon icon__dark' src='${relpath}/providers/${provider}/${icon_dark}'></img>`;
                                                return `<button class="btn btn--big btn--svg login" data-cprov='${provider}'>
                                                            ${!!icons ? button_el : name}
                                                        </button>`;
                                            }

            _html = $(_html);
            let $box = $('<div />');
            config.portals.checklist.forEach(item => {
                if ( !!item.icons && !!item.icons.themeLight ) {
                    const btn = provider_button_template(item.provider, item.name, item.icons);

                    item.provider != 'onlyoffice' ? $box.append(btn) :
                            _html.find('#box-providers-premium-button').append(btn);
                }
            });

            _html.find('#box-providers-buttons').append($box.children());
        }

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
        },
        portaltemplate: function(info, edit) {
            let _row = `<td class="row-cell cicon">
                            <svg class='icon'><use href='#${info.iconid}'></svg>
                        </td>
                        <td class="row-cell">
                            <p class="cportal primary">${utils.skipUrlProtocol(info.portal)}</p>
                            <p class="cuser minor">${info.user}${info.email.length && (' (' + info.email + ')') || ''}</p>
                        </td>
                        <td class="cell-tools">
                            <div class="hlayout">
                                <button class="btn-quick logout" tooltip="${utils.Lang.menuLogout}">
                                    <i class="icon img-el theme-inverted" />
                                </button>
                            </span>
                        </td>`;

            if ( !!info.themeicons ) {
                const icon_el = `<img class='icon icon__light' src='${info.themeicons.light}'></img>
                                    <img class='icon icon__dark' src='${info.themeicons.dark}'></img>`;
                _row = _row.replace(/<svg.+<\/svg>/, icon_el);
            } else
            if ( !!info.iconsrc ) {
                const icon_el = `<img class='icon icon_light' src='${info.iconsrc}'></img>`;
                _row = _row.replace(/<svg.+<\/svg>/, icon_el);
            }

            return edit===true ? _row : `<tr id=${info.elid}>${_row}</tr>`;
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
                const _provider_cfg = config.portals.providers[model.provider];
                const _entrypage = !_provider_cfg ? '/' : _provider_cfg.startPage;
                sdk.command("portal:open", JSON.stringify({
                        portal: !_provider_cfg || !_provider_cfg.entryPage ? model.path : _provider_cfg.entryPage,
                        provider: model.provider,
                        entrypage: _entrypage}));
            } else
            if (/\:logout/.test(action)) {
                _do_logout.call(this, model);
            } else
            if (/\:forget/.test(action)) {
                model.removed = true;
                _do_logout.call(this, model);
            }
        };

        var _do_connect = function(model) {
            let _dialog = new DialogConnect({
                portal: model.path,
                provider: model.provider,
                onclose: opts => {
                    if ( opts ) {
                        opts.type = 'outer';

                        const _pm = config.portals.checklist.find(e => e.provider == opts.provider);
                        opts.entrypage = !_pm ? '/' : _pm.startPage;

                        sdk.execCommand("auth:outer", JSON.stringify(opts));
                    }

                    _dialog = undefined;
                }
            });

            _dialog.show();
        };

        function _do_login(model) {
            if ( !dlgLogin ) {
                !model && (model = {})
                dlgLogin = new LoginDlg({
                    success: info => {
                        if ( info.type == 'sso' ) {
                            window.sdk.execCommand("auth:sso", JSON.stringify(info));
                        } else
                        if ( info.type == 'outer' ) {
                            window.sdk.execCommand("auth:outer", JSON.stringify(info));
                        } else
                        if ( info.type == 'user' ) {
                            window.sdk.execCommand("portal:open", JSON.stringify({portal:info.data.portal}));

                            // dlgLogin.close();
                            PortalsStore.keep(info.data);
                            _update_portals.call(this);

                            window.selectAction('connect');
                        }
                    },
                    close: code => {
                        dlgLogin = undefined;
                    }
                });

                dlgLogin.show({portal: model.path, provider: model.provider, email: model.email});
            }
        };

        if ( config.portals && !!config.portals.auth_use_api ) {
            _do_connect = _do_login;
        }

        function _authorize(portal, user, data) {
            if ( !dlgLogin ) {
                dlgLogin = new LoginDlg({
                    success: info => {
                        // dlgLogin.close();
                        PortalsStore.keep(info.data);
                        _update_portals.call(this);

                        CommonEvents.fire('portal:authorized', [data]);
                    },
                    close: code => {
                        dlgLogin = undefined;
                    }
                });

                dlgLogin.show({portal: portal, email: user});
            }
        };

        function _do_logout(model) {
            // var model = portalCollection.find('name', info);
            // model && model.set('logged', false);

            let info = {domain:model.path};
            const _provider = config.portals.providers.find(model.provider);
            if ( _provider && !!_provider.extraLogout )
                info.extra = _provider.extraLogout;

            window.sdk.execCommand('portal:logout', JSON.stringify(info));
        };

        function _update_portals() {
            collection.empty();

            /* fill portals list */
            var portals = PortalsStore.portals();

            if (portals.length) {
                let auth_arr = {};
                for (let rec of portals) {
                    var pm = new PortalModel(rec);

                    auth_arr[pm.name] = '';
                    collection.add(pm);
                }

                window.sdk && window.sdk.checkAuth && window.sdk.checkAuth(auth_arr);

                this.view.$panelNoPortals.hide();
                this.view.$panelPortalList.show();
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

                function _create_icon_id(provider) {
                    switch ( provider ) {
                    case 'onlyoffice':
                    case 'asc': return 'icon__asc';
                    case 'owncloud': return 'icon__ownc';
                    case 'nextcloud': return 'icon__nextc';
                    default: return 'icon__common';
                    }
                };

                function _get_icon_scr(provider) {
                    const _model = config.portals.checklist.find(e => (e.provider == provider))
                    return !!_model && !!_model.icons && !!_model.icons.themeLight.connectionsList ?
                                `${relpath}/providers/${_model.provider}/${_model.icons.themeLight.connectionsList}` : undefined;
                };

                function _get_theme_icons(provider) {
                    const _model = config.portals.checklist.find(e => (e.provider == provider))
                    return !!_model && !!_model.icons ?
                                { light: `${relpath}/providers/${_model.provider}/${_model.icons.themeLight.connectionsList}`,
                                    dark: `${relpath}/providers/${_model.provider}/${_model.icons.themeDark.connectionsList}` } : 
                                undefined;
                };

                collection.events.changed.attach((collection, model, value) => {
                    if ( !!value ) {
                        if ( value.logged != undefined )
                            this.view.$panelPortalList.find('#' + model.uid)[model.logged?'addClass':'removeClass']('logged');
                        else
                        if ( value.user ) {
                            let el = this.view.$panelPortalList.find('#' + model.uid);
                            el.html(
                                $(this.view.portaltemplate({
                                    portal: model.name,
                                    iconid: _create_icon_id(model.provider),
                                    iconsrc: _get_icon_scr(model.provider),
                                    themeicons: _get_theme_icons(model.provider),
                                    user: model.user,
                                    email: model.email}, true)));
                        } else
                        if ( value.removed != undefined ) {
                            value.removed ? $('#' + model.uid, this.view.$panelPortalList).addClass('lost') :
                                    $('#' + model.uid, this.view.$panelPortalList).removeClass('lost');
                        }
                    }
                });

                collection.events.inserted.attach((collection, model) => {
                    let $listPortals = collection.view.find('.table-files.list');
                    let $item = $(this.view.portaltemplate({
                        portal: model.name,
                        iconid: _create_icon_id(model.provider),
                        iconsrc: _get_icon_scr(model.provider),
                        themeicons: _get_theme_icons(model.provider),
                        user: model.user,
                        email: model.email,
                        elid: model.uid
                    }));
                    
                    $item.find('.logout').click(model, e => {
                        _do_logout(e.data);

                        e.stopPropagation && e.stopPropagation();
                        return false;
                    });
                    
                    $listPortals.append($item);
                });

                collection.events.click.attach((collection, model)=>{
                    _on_context_menu(undefined, 'portal:open', model);
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

                if (model) {
                    model.set('logged', false)

                    const _is_logged = obj[i].length > 0;
                    (new DialogConnect).portalexists(model.path, model.provider)
                            .then(data => {
                                data.status == 'success' && _is_logged && model.set('logged', true); 
                            }, error => {
                                $('#' + model.uid, this.view.$panelPortalList).toggleClass('unavail', true);
                            });
                }
            };
        };

        var _on_create_portal = function() {
            dlgLogin && dlgLogin.close();
            window.sdk.execCommand('portal:create', '');
        };

        var _on_login_message = function(info) {
            let _write_portal_cookie = portal => {
                let _re = /^(https?:\/{2})?([^\<\>\/\s]+)(\/[^\s]+)?/i.exec(portal);
                let _domain = _re[2];

                sdk.setCookie(portal, _domain, _re[3] || '/', "asc_auth_key", utils.fn.uuid());
            };

            let obj = JSON.parse(info);
            if ( obj && !!obj.displayName ) {
                var model = collection.find('name', utils.skipUrlProtocol(obj.domain));
                if ( model ) {
                    !obj.email && (obj.email = '');
                    if ( model.email == obj.email ) {
                        if ( !model.get('logged') ) {
                            model.set('logged', true);
                            if (model.provider != 'onlyoffice')
                                _write_portal_cookie(obj.domain);

                            if ( model.get('removed') ) {
                                model.set('removed', false);
                                PortalsStore.keep({
                                    portal: model.path,
                                    provider: model.provider,
                                    user: model.user,
                                    email: model.email
                                });
                            }

                            if ( model.get('user') != obj.displayName ) {
                                model.set('user', obj.displayName);

                                let _m = PortalsStore.get({portal:model.path});
                                if ( !!_m ) {
                                    _m['user'] = obj.displayName;
                                    PortalsStore.keep(_m);
                                }
                            }
                        }
                        return;
                    } else {
                        PortalsStore.forget(obj.domain);
                    }
                }


                let _p;
                !obj.provider && (obj.provider = 'onlyoffice');
                if ( !config.portals.checklist.find(i => i.provider == obj.provider) &&
                            (_p = config.portals.checklist.find(i => i.name.toLowerCase() == obj.provider.toLowerCase())) )
                    obj.provider = _p.provider;

                let info = {
                    portal: obj.domain,
                    provider: obj.provider,
                    user: obj.displayName,
                    email: obj.email
                };

                info.portal.endsWith('/') &&
                        (info.portal = info.portal.slice(0,-1));

                PortalsStore.keep(info);
                if ( obj.provider != 'onlyoffice' ) {
                    // sdk.setCookie(info.portal, utils.skipUrlProtocol(info.portal), "/", "asc_auth_key", utils.fn.uuid());
                    _write_portal_cookie(info.portal);

                    window.on_set_cookie = () => {
                        window.on_set_cookie = undefined;
                        _update_portals.call(this);
                    }
                } else {
                    _update_portals.call(this);
                }

                if ( $('.action-panel').filter('.welcome').is(':visible') ) {
                    window.selectAction('connect');
                }
            }
        };

        var _on_settings = function(cmd, params) {
            if (/init$/.test(cmd)) {
                if ( params.includes('\"portals\"\:') ) {
                    let opts;
                    try {
                        opts = JSON.parse(params);
                    } catch (e) { /*delete opts;*/ }

                    if ( opts && opts.portals && opts.portals.auth_use_api ) {
                        _do_connect = _do_login;
                    }
                } else
                if (params.includes('\"uitheme\"\:')) {
                    let opts = JSON.parse(params);

                    if ( !!opts.uitheme ) {
                        opts.uitheme == 'canuse' && (opts.uitheme = 'theme-light');

                        // if ( localStorage.getItem('ui-theme') != opts.uitheme )
                            _on_theme_changed(opts.uitheme);
                    }
                }
            }
        };

        let carousel = {};
        function _scrollCarousel(direction) {
            function __check_limits(v, max) {
                if ( v < 0 ) return max;
                else if ( v > max ) return 0;
                else return v;
            };

            let _activeindex = carousel.$items.filter('.active').index();
            direction == 'next' ? ++_activeindex : --_activeindex;

            _activeindex = __check_limits(_activeindex, carousel.$items.length - 1);

            let _pre_index = _activeindex - 1,
                _pro_index = _activeindex + 1;

            _pre_index = __check_limits(_pre_index, carousel.$items.length - 1);
            _pro_index = __check_limits(_pro_index, carousel.$items.length - 1);

            carousel.$items.eq(_activeindex).addClass('migrate');
            if ( direction == 'next' ) {
                carousel.$items.filter('.pre-active').removeClass('pre-active').addClass('migrate');
                carousel.$items.eq(_pre_index).removeClass('migrate pre-active active pro-active').addClass('pre-active');
            } else {
                carousel.$items.filter('.pro-active').removeClass('pro-active').addClass('migrate');
                carousel.$items.eq(_pro_index).removeClass('migrate pre-active active pro-active').addClass('pro-active');
            }

            carousel.$items.eq(_activeindex).removeClass('migrate pre-active pro-active').addClass('active');

            if ( direction == 'next' )
                carousel.$items.eq(_pro_index).removeClass('migrate pre-active active pro-active').addClass('pro-active');
            else carousel.$items.eq(_pre_index).removeClass('migrate pre-active active pro-active').addClass('pre-active');
        };

        function _initCarousel() {
            let _$panel = this.view.$panelNoPortals;
            carousel.$items = _$panel.find('.carousel__slide');
            let _activeindex = carousel.$items.filter('.active').index();

            if ( !(navigator.userAgent.indexOf("Windows NT 5.") < 0) ||
                    !(navigator.userAgent.indexOf("Windows NT 6.0") < 0) )
            {
                $('.carousel', _$panel).addClass('winxp');
            }

            let _pre_index = _activeindex - 1,
                _pro_index = _activeindex + 1;

            if ( _pre_index < 0 ) _pre_index = carousel.$items.length - 1;
            if ( _pro_index > carousel.$items.length - 1 ) _pro_index = 0;
            carousel.$items.eq(_pre_index).addClass('pre-active');
            carousel.$items.eq(_pro_index).addClass('pro-active');

            _$panel.find('.carousel__scrolls > .carousel__scroll__btn')
                .on('click', e => {
                    _scrollCarousel(e.target.getAttribute('value'));
                });

            _on_theme_changed(localStorage.getItem('ui-theme'));
        };

        function _on_lang_changed(ol,nl) {
            $('.btn-quick.logout',this.$panelPortalList).attr('tooltip',utils.Lang.menuLogout);
        };

        function _on_theme_changed(name) {
            $('.carousel__slide__img > use').each((i, el) => {
                const src = el.getAttribute('data-src');
                if ( name == 'theme-dark' )
                    el.setAttribute('xlink:href', `#${src}-dark`);
                else el.setAttribute('xlink:href', `#${src}-light`);
            });
        }

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
                                if ( model.removed ) {
                                    model.set('removed', true);
                                    PortalsStore.forget(param);
                                }
                            } else
                                delete model.removed;
                        }
                    } else
                    if (/portal:login/.test(cmd)) {
                        _on_login_message.call(this, param);
                    } else
                    if (/^settings\:/.test(cmd)) {
                        _on_settings.call(this, cmd, param);
                    } else
                    if (cmd == 'uitheme:changed') {
                        _on_theme_changed(param);
                    }
                });

                _init_collection.call(this);
                _update_portals.call(this);
                _init_ppmenu.call(this);
                _initCarousel.call(this);

                $('body').on('click', '.login', e=>{
                    let _data = $(e.currentTarget).data();
                    !_data ? _do_connect.call(this) : _do_connect.call(this, {provider:_data.cprov});
                });

                window.CommonEvents.on('portal:create', _on_create_portal);
                window.CommonEvents.on('lang:changed', _on_lang_changed);

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
            , collection: function() {
                return collection;
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
