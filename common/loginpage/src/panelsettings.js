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
*/

+function(){ 'use strict'
    const THEME_TYPE_LIGHT = 'light';
    const THEME_TYPE_DARK = 'dark';
    const THEME_TYPE_SYSTEM = 'system';

    const THEME_ID_DEFAULT_LIGHT = 'theme-classic-light';
    const THEME_ID_DEFAULT_DARK = 'theme-dark';

    var ControllerSettings = function(args={}) {
        args.caption = 'Settings';
        args.action =
        this.action = "settings";
        this.view = new ViewSettings(args);
    };

    ControllerSettings.prototype = Object.create(baseController.prototype);
    ControllerSettings.prototype.constructor = ControllerSettings;

    var ViewSettings = function(args) {
        var _lang = utils.Lang;

        args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        let _html = `<div ${args.id} class='action-panel ${args.action}'>
                        <div id='box-settings'>
                            <div class='flexbox content-box'>
                                <h3 class='table-caption' l10n>${_lang.actSettings}</h3>
                                <div id='options-wrap'>
                                    <section class='settings-items'>
                                        <div class='settings-field'>
                                            <label class='sett__caption' l10n>${_lang.settUserName}</label>
                                            <div class='hbox sett--label-lift-top' id='sett-box-user'>
                                                <input type='text' class='tbox' spellcheck='false' maxlength='128'>
                                                <a class='link link--sizem link--gray' draggable='false' href='#' l10n>${_lang.settResetUserName}</a>
                                            </div>
                                        </div>
                                        <div class='settings-field settings-field-lang'>
                                            <label class='sett__caption' l10n>${_lang.settLanguage}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox' data-size="10"></select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id='opts-ui-scaling' style='display:none'>
                                            <label class='sett__caption' l10n>${_lang.settScaling}</label><label class='sett__caption'> *</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='0' l10n>${_lang.settOptScalingAuto}</option>
                                                        <option value='100'>100%</option>
                                                        <option value='125'>125%</option>
                                                        <option value='150'>150%</option>
                                                        <option value='175'>175%</option>
                                                        <option value='200'>200%</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-checkupdate" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settCheckUpdates}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='never' l10n>${_lang.settOptCheckNever}</option>
                                                        <option value='day' l10n>${_lang.settOptCheckDay}</option>
                                                        <option value='week' l10n>${_lang.settOptCheckWeek}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-ui-theme" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settUITheme}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='theme-system' l10n>${_lang.settOptThemeSystem}</option>
                                                        <option value='theme-light' l10n>${_lang.settOptThemeLight}</option>
                                                        <option value='theme-classic-light' l10n>${_lang.settOptThemeClassicLight}</option>
                                                        <option value='theme-dark' l10n>${_lang.settOptThemeDark}</option>
                                                        <option value='theme-contrast-dark' l10n>${_lang.settOptThemeContrastDark}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-launch-mode" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settOptLaunchMode}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='intab' l10n>${_lang.settOptLaunchInTab}</option>
                                                        <option value='inwindow' l10n>${_lang.settOptLaunchInWindow}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-spellcheck-mode" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settSpellcheckDetection}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='auto' l10n>${_lang.settOptScalingAuto}</option>
                                                        <option value='off' l10n>${_lang.settOptDisabled}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <!-- temporary elements section -->
                                        <div class='settings-field' style='display:none;'>
                                            <section class='switch-labeled hbox' id='sett-box-preview-mode'>
                                                <input type="checkbox" class="checkbox" id="sett-preview-mode">
                                                <label for="sett-preview-mode" class='sett__caption' l10n>${_lang.settOpenMode}</label>
                                            </section>
                                        </div>
                                        <!-- end section -->
                                    </section>
                                    <div class="lst-tools" id="sett-tools-dyn">
                                        <button class="btn btn--primary sett-btn--apply" id="sett-btn-apply" l10n>${_lang.setBtnApply}</button>
                                    </div>
                                </div>
                                <div class="lst-tools" id="sett-tools-stat">
                                    <button class="btn btn--primary sett-btn--apply" id="sett-btn-apply" l10n>${_lang.setBtnApply}</button>
                                    <!-- <strong class='sett__note' tooltip="${_lang.settAfterRestart}" tooltip-pos='top' l10n>i</strong> -->
                                </div>
                                <div class="spacer" />
                            </div>
                            <p id="caption-restart" class="sett__caption" style="display:none;text-align:left;margin-block-start:0.5em;"><label>* - </label><label l10n>${_lang.settAfterRestart}</label></p>
                        </div>
                    </div>`;

        args.tplPage = _html;
        args.itemcls = 'bottom';
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemtext = _lang.actSettings;

        baseView.prototype.constructor.call(this, args);
    };

    ViewSettings.prototype = Object.create(baseView.prototype);
    ViewSettings.prototype.constructor = ViewSettings;

    window.ControllerSettings = ControllerSettings;

    utils.fn.extend(ControllerSettings.prototype, (function() {
        let $btnApply,
            $userName,
            $chOpenMode;
        let $panel;
        let $optsLang,
            $optsUIScaling,
            $optsUITheme,
            $optsSpellcheckMode,
            $optsLaunchMode;

        function _set_user_name(name) {
            let me = this;

            $userName.val(name).removeClass('error');
            $btnApply.disable(false);
        };

        function _apply_theme(name, type) {
            if ( name == 'theme-system' ) {
                name = get_default_theme(get_system_theme_type());
            }

            if ( !$("body").hasClass(name) ) {
                const _type = (type == 'dark' || /theme-(?:[a-z]+-)?dark(?:-[a-z]*)?/.test(name)) ? 'theme-type-dark' : 'theme-type-light';
                const _cls = document.body.className.replace(/theme-[\w-]+/gi,'').trim();
                document.body.className = `${_cls?_cls+' ':''}${name} ${_type}`;

                localStorage.setItem('ui-theme-id', name);
                CommonEvents.fire('theme:changed', [name]);
            }
        };

        const _validate_user_name = name => {
            // return /^[\p{L}\p{M}\p{N}'"\.\- ]+$/u.test(name);

            /* @winxpsupport
            *  we use chrome ver 49 on win xp,
            *  that version has no support for unicode in regexp
            */
            if (window.utils.inParams.osver == 'winxp' || /windows nt 5/i.test(navigator.appVersion)) return true;
            else return (new RegExp('^[\\p{L}\\p{M}\\p{N}\'"\\.\\- ]+$', 'iu')).test(name)
        };

        function _on_btn_apply(e) {
            let _user_new_name = $userName.val();
            if ( _user_new_name && _user_new_name.length &&
                    _validate_user_name(_user_new_name) ) 
            {
                let _doc_open_mode = $chOpenMode.prop('checked') ? 'view' : 'edit';
                let _new_settings = {
                    username:_user_new_name,
                    docopenmode: _doc_open_mode
                };

                if ( $optsLang.is(':visible') ) {
                    _new_settings.langid = $optsLang.find('select').val();

                    utils.Lang.change(_new_settings.langid);
                }

                let $optsupdatesrate = $('#opts-checkupdate', $panel);
                if ( $optsupdatesrate.is(':visible') ) {
                    let $combo = $('select', $optsupdatesrate);

                    _new_settings.checkupdatesrate = $combo.val();
                    $combo.selectpicker('refresh');
                }

                if ( $optsUIScaling ) {
                    _new_settings.uiscaling = $optsUIScaling.val();
                    $optsUIScaling.selectpicker('refresh');
                }

                if ( $optsUITheme ) {
                    _new_settings.uitheme = $optsUITheme.val();
                    $optsUITheme.selectpicker('refresh');

                    // _apply_theme(_new_settings.uitheme);
                }

                if ( $optsLaunchMode ) {
                    _new_settings.editorwindowmode = $optsLaunchMode.val() == 'inwindow';
                    $optsLaunchMode.selectpicker('refresh');
                }

                if ( $optsSpellcheckMode ) {
                    _new_settings.spellcheckdetect = $optsSpellcheckMode.val();
                    $optsSpellcheckMode.selectpicker('refresh');
                }

                sdk.command("settings:apply", JSON.stringify(_new_settings));
                $btnApply.disable(true);
                
                localStorage.setItem('username', _user_new_name);
                localStorage.setItem('docopenmode', _doc_open_mode);

                _lock_createnew(_doc_open_mode == 'view');
            } else {
                $userName.addClass('error');
            }
        };

        function _on_txt_user_change(e) {
            $userName.removeClass('error');
            
            if ( $btnApply.isdisabled() )
                $btnApply.disable(false);
        };

        function _on_lang_change(e) {
            let l = $optsLang.find('select').val(),
                c = utils.Lang.tr('setBtnApply', l);
            if ( !!c ) $btnApply.text(c);
            if ( $btnApply.isdisabled() ) {
                $btnApply.disable(false);
            }

            $optsLang.toggleClass('notted', true);
        };

        function _on_autoupdate_change() {
            if ( $btnApply.isdisabled() ) {
                $btnApply.disable(false);
            }
        };

        function _lock_createnew(lock) {
            lock === true ? $('.tool-quick-menu .menu-item').addClass('disabled') :
                    $('.tool-quick-menu .menu-item').removeClass('disabled');
        };

        function _on_app_message(cmd, param) {
            if (/^settings\:/.test(cmd)) {
                if (/username$/.test(cmd)) {
                    _set_user_name.call(this, param);
                } else
                if (/hasopened$/.test(cmd)) {
                    // $btnApply.parent().addClass('noted');
                    console.log('has opened editors');
                } else
                if (/init$/.test(cmd)) {
                    let opts;
                    try {
                        opts = JSON.parse( $('<div>').html(param).text() );
                    } catch (e) { /*delete opts;*/ }

                    if ( opts ) {
                        if ( opts.langs === 0 ) {
                            $panel.find('.settings-field-lang').hide();
                        } else
                        if ( opts.locale ) {
                            $panel.find('.settings-field-lang').show();
                            let $combo = $panel.find('.settings-field-lang select');

                            let def_lang;
                            for (let lang in opts.locale.langs) {
                                /^en/.test(lang) && (def_lang = lang);
                                $combo.append(`<option value='${lang}'>${opts.locale.langs[lang]}</option>`);
                            }

                            if ( !opts.locale.langs[opts.locale.current] ) {
                                opts.locale.current = opts.locale.current.substring(0,2);
                                if ( !opts.locale.langs[opts.locale.current] && !!def_lang ) {
                                    opts.locale.current = def_lang;
                                }
                            }

                            $combo.val(opts.locale.current);
                            $combo.selectpicker();

                            if ( opts.locale.restart ) {
                                $panel.find('.settings-field-lang label[l10n]').after(`<label class='sett__caption'>*</label>`);
                                $('#caption-restart', $panel).show();
                            }
                        }

                        if ( opts.uiscaling != undefined && !$optsUIScaling ) {
                            ($optsUIScaling = ($('#opts-ui-scaling', $panel).show().find('select')))
                            .val(opts.uiscaling)
                            .selectpicker().on('change', e => {
                                $btnApply.isdisabled() && $btnApply.disable(false);
                            });

                            $('#caption-restart', $panel).show();
                        }

                        if ( !!opts.uitheme ) {
                            opts.uitheme == 'canuse' && (opts.uitheme = 'theme-light');
                            ($optsUITheme = ($('#opts-ui-theme', $panel).show().find('select')))
                            .val(opts.uitheme)
                            .selectpicker().on('change', e => {
                                $btnApply.isdisabled() && $btnApply.disable(false);
                            });

                            _apply_theme(opts.uitheme);
                        }

                        if ( opts.editorwindowmode !== undefined ) {
                            ($optsLaunchMode = ($('#opts-launch-mode', $panel).show().find('select')))
                            .val(opts.editorwindowmode ? 'inwindow' : 'intab')
                            .selectpicker().on('change', e => {
                                $btnApply.isdisabled() && $btnApply.disable(false);
                            });
                        }

                        if ( opts.spellcheckdetect !== undefined ) {
                            ($optsSpellcheckMode = ($('#opts-spellcheck-mode', $panel).show().find('select')))
                            .val(opts.spellcheckdetect)
                            .selectpicker().on('change', e => {
                                $btnApply.isdisabled() && $btnApply.disable(false);
                            });
                        }
                    }

                    $('.settings-field:visible:last').css('margin-bottom','0');
                } else
                if (/updates/.test(cmd)) {
                    let $settnode = $('#opts-checkupdate', $panel),
                        $combo = $('select', $settnode);

                    $combo.val(param);

                    if ( !$settnode.is(':visible') ) {
                        $settnode.show();
                        $combo.selectpicker();
                        $combo.on('change', _on_autoupdate_change.bind(this));
                    }
                }
            } else
            if (/uitheme:changed/.test(cmd)) {
                if ( !!$optsUITheme ) {
                    $optsUITheme.val(param)
                    $optsUITheme.selectpicker('refresh');
                }

                _apply_theme(param);
            }
        };

        const get_system_theme_type = () => window.matchMedia('(prefers-color-scheme: dark)').matches ? THEME_TYPE_DARK : THEME_TYPE_LIGHT; 
        const get_default_theme = type => type == THEME_TYPE_DARK ? THEME_ID_DEFAULT_DARK : THEME_ID_DEFAULT_LIGHT;
        const on_system_theme_dark = e =>
            sdk.command("system:changed", JSON.stringify({'colorscheme': e.target.matches ? THEME_TYPE_DARK:THEME_TYPE_LIGHT}));

        const on_window_resize = function(e) {
            if ( !this.resize_elems  ) return;

            if ( !this.resize_elems.opened  ) {
                this.resize_elems = {
                    statH: $('.settings .table-caption').outerHeight(true) + $('#sett-tools-dyn').outerHeight(true),
                    boxElem: $('#box-settings'),
                    settItems: $('section.settings-items'),
                    btnDyn: $('#sett-tools-dyn'),
                    btnStat: $('#sett-tools-stat'),
                }
            }

            this.resize_elems.settItems.height() + this.resize_elems.statH > this.resize_elems.boxElem.height() ?
                (this.resize_elems.btnDyn.hide(), this.resize_elems.btnStat.show()) : (this.resize_elems.btnDyn.show(), this.resize_elems.btnStat.hide());
        };

        const on_panel_show = function(panel) {
            if ( panel == this.action ) {
                !this.resize_elems && (this.resize_elems = { opened: false });

                if ( !this.resize_elems.opened ) {
                    on_window_resize.call(this);
                }
            }
        }

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                let me = this;
                me.view.$panel.find('#sett-box-user > a.link').on('click', e => {
                    sdk.command("settings:get", "username");
                });

                $panel = me.view.$panel;
                $btnApply = me.view.$panel.find('.sett-btn--apply');
                $userName = me.view.$panel.find('#sett-box-user > input');
                $chOpenMode = me.view.$panel.find('#sett-preview-mode');

                $btnApply.on('click', _on_btn_apply).prop('disabled', true);
                $userName.on('input', _on_txt_user_change);
                $chOpenMode.on('change', e => {
                    if ( $btnApply.prop('disabled') )
                        $btnApply.prop('disabled', false);
                });

                let _user_name = localStorage.getItem('username') || '';
                let _open_mode = localStorage.getItem('docopenmode') || 'edit';

                if ( _user_name ) {
                    $userName.val(_user_name);
                } else {
                    sdk.command("settings:get", "username");
                }

                if ( _open_mode == 'view' ) {
                    $chOpenMode.prop('checked', true);
                    _lock_createnew(true);
                }

                // if ( _open_mode == 'view' ) {
                //     sdk.command("settings:apply", JSON.stringify({docopenmode: _open_mode}));
                // }

                ($optsLang = $panel.find('.settings-field-lang')).hide();
                $optsLang.find('select').on('change', _on_lang_change.bind(this));

                $('select.combobox').on('rendered.bs.select', e => {
                    $(e.target).next().removeAttr('title');
                });

                window.sdk.on('on_native_message', _on_app_message.bind(this));
                window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', on_system_theme_dark.bind(this));
                // on_system_theme_dark({target: window.matchMedia('(prefers-color-scheme: dark)')});

                $(window).on('resize', on_window_resize.bind(this));
                CommonEvents.on('panel:show', on_panel_show.bind(this));

                return this;
            }
        };
    })());
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerSettings({});
//     p.init();
// });
