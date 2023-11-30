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

    const themes_map = {
        'theme-system': {
            text: 'Same as system',
            type: THEME_TYPE_SYSTEM,
        },
        'theme-light': {
            text: 'Light',
            type: 'light',
        },
        'theme-classic-light': {
            text: 'Classic Light',
            type: 'light',
        },
        'theme-dark': {
            text: 'Dark',
            type: 'dark',
        },
        'theme-contrast-dark': {
            text: 'Dark Contrast',
            type: 'dark',
        },
    }

    const nativevars = window.RendererProcessVariable;

        const create_colors_css = function (id, colors) {
            if ( !!colors && !!id ) {
                let _css_array = [':root .', id, '{'];
                for (var c in colors) {
                    _css_array.push('--', c, ':', colors[c], ';');
                }

                _css_array.push('}');
                return _css_array.join('');
            }
        }

        const write_theme_css = function (css, id) {
            if ( !!css ) {
                const $style = $(`style [data-theme-id=${id}]`);
                if ( $style.length )
                    $style.remove();

                let style = document.createElement('style');
                style.type = 'text/css';
                style.innerHTML = css;
                style.setAttribute('data-theme-id', id);
                document.getElementsByTagName('head')[0].appendChild(style);
            }
        }

        if ( nativevars.localthemes ) {
            for ( const t of nativevars.localthemes ) {
                const _css = create_colors_css(t.id, t.colors);
                if ( _css ) {
                    write_theme_css(_css, t.id);
                    themes_map[t.id] = {text: t.name, type: t.type, l10n: t.l10n};
                }
            }
        }

    const uitheme = { id: nativevars.theme.id, type: nativevars.theme.type }
    uitheme.set_id = function (id) {
        if ( id == 'theme-system' )
            this.adapt_to_system_theme();
        else this.id = id;
    }

    uitheme.is_theme_system = function () {
        return this.id == 'theme-system'
    }

    uitheme.is_system_theme_avalaible = function () {
        return t.system !== 'disabled'
    }

    uitheme.adapt_to_system_theme = function () {
        this.id = 'theme-system';
        this.type = this.is_system_theme_dark() ? 'dark' : 'light';
    }

    uitheme.relevant_theme_id = function () {
        if ( this.is_theme_system() )
            return this.is_system_theme_dark() ? 'theme-dark' : 'theme-classic-light';
        return this.id;
    }

    uitheme.is_system_theme_dark = function () {
        return this.get_system_theme_type() == 'dark';
    }

    uitheme.get_default_theme_for_type = type => type == THEME_TYPE_DARK ? THEME_ID_DEFAULT_DARK : THEME_ID_DEFAULT_LIGHT;

    uitheme.get_system_theme_type = () => {
        if ( nativevars.theme && !!nativevars.theme.system ) {
            return nativevars.theme.system !== 'disabled' ? nativevars.theme.system : THEME_TYPE_LIGHT;
        } else {
            return window.matchMedia('(prefers-color-scheme: dark)').matches ? THEME_TYPE_DARK : THEME_TYPE_LIGHT;
        }
    }


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
                                                    <select class='combobox' data-size="5">
                                                        <option value='0' l10n>${_lang.settOptScalingAuto}</option>
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
                                        <div class='settings-field' id="opts-autoupdate-mode" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settAUpdateMode}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option data-subtext="${_lang.settOptDescAUpdateSilent}" value='silent' l10n>${_lang.settOptAUpdateSilent}</option>
                                                        <option data-subtext="${_lang.settOptDescAUpdateAsk}" value='ask' l10n>${_lang.settOptAUpdateAsk}</option>
                                                        <option data-subtext="${_lang.settOptDescDisabled}" value='disabled' l10n>${_lang.settOptDisabled}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-autoupdate" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settCheckUpdates}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox'>
                                                        <option value='ask' l10n>${_lang.settOptEnabled}</option>
                                                        <option value='disabled' l10n>${_lang.settOptDisabled}</option>
                                                    </select>
                                                </section>
                                            </div>
                                        </div>
                                        <div class='settings-field' id="opts-ui-theme" style='display:none;'>
                                            <label class='sett__caption' l10n>${_lang.settUITheme}</label>
                                            <div class='sett--label-lift-top hbox'>
                                                <section class='box-cmp-select'>
                                                    <select class='combobox' data-size='5'></select>
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
                                        <div class='settings-field' style='display:none;'>
                                            <section class='switch-labeled hbox' id='sett-box-rtl-mode'>
                                                <input type="checkbox" class="checkbox" id="sett-rtl-mode">
                                                <label for="sett-rtl-mode" class='sett__caption' l10n>RTL Interface *</label>
                                            </section>
                                        </div>
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
            $optsLaunchMode,
            $optsAutoupdateMode;
        let $chRtl;

        function _set_user_name(name) {
            let me = this;

            $userName.val(name).removeClass('error');
            $btnApply.disable(false);
        };

        function _apply_theme(id, type) {
            uitheme.set_id(id);

            const theme_id = uitheme.relevant_theme_id();
            if ( !$("body").hasClass(theme_id) ) {
                const _type = (type == 'dark' || /theme-(?:[a-z]+-)?dark(?:-[a-z]*)?/.test(theme_id)) ? 'theme-type-dark' : 'theme-type-light';
                const _cls = document.body.className.replace(/theme-[\w-]+/gi,'').trim();
                document.body.className = `${_cls?_cls+' ':''}${theme_id} ${_type}`;

                CommonEvents.fire('theme:changed', [theme_id, themes_map[theme_id].type]);
            }
        };

        function _add_themes(objs) {
            const _combo = $('#opts-ui-theme select', $panel);
            if ( objs ) {
                !(objs instanceof Array) && (objs = [objs]);

                const _divider = _combo.find('[data-divider]');
                objs.forEach(t => {
                    const _css = create_colors_css(t.id, t.colors);
                    if ( _css ) {
                        write_theme_css(_css, t.id);
                        themes_map[t.id] = {text: t.name, type: t.type, l10n: t.l10n};

                        const _theme_title = t.l10n[utils.Lang.id] || t.name;
                        const _theme_menu_item = `<option value=${t.id} l10n>${_theme_title}</option>`;

                        if ( _divider.length )
                            _divider.before(_theme_menu_item);
                        else _combo.append(_theme_menu_item);
                    }
                });
                $optsUITheme.selectpicker('refresh');
            }
        }

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
                    _new_settings.checkupdatesinterval = $combo.val();
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

                if ( $optsAutoupdateMode ) {
                    _new_settings.autoupdatemode = $optsAutoupdateMode.val();
                    $optsAutoupdateMode.selectpicker('refresh');
                }

                if ( $optsSpellcheckMode ) {
                    _new_settings.spellcheckdetect = $optsSpellcheckMode.val();
                    $optsSpellcheckMode.selectpicker('refresh');
                }

                if ( $chRtl ) {
                    _new_settings.rtl = $chRtl.prop("checked");
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

                            const _themes = [{'theme-system': utils.Lang.settOptThemeSystem},
                                            {'theme-light': utils.Lang.settOptThemeLight},
                                            {'theme-classic-light': utils.Lang.settOptThemeClassicLight},
                                            {'theme-dark': utils.Lang.settOptThemeDark},
                                            {'theme-contrast-dark': utils.Lang.settOptThemeContrastDark}];

                            if ( nativevars.theme ) {
                                if ( nativevars.theme.system == 'disabled' )
                                    _themes.shift();
                            }

                            const _combo = $('#opts-ui-theme select', $panel).empty();
                            _themes.forEach(item => {
                                const entries = Object.entries(item)[0];
                                _combo.append(`<option value=${entries[0]} l10n>${entries[1]}</option>`);
                            });

                            if ( nativevars.localthemes ) {
                                for ( const t of nativevars.localthemes ) {
                                    const _theme_title = t.l10n[utils.Lang.id] || t.name;
                                    _combo.append(`<option value=${t.id} ml10n>${_theme_title}</option>`);
                                }
                            }

                            if ( !$optsUITheme ) {
                                ($optsUITheme = _combo)
                                .val(opts.uitheme)
                                .selectpicker().on('changed.bs.select', (e, index, selected, previous) => {
                                    if ( selected && e.target.value == 'add' ) {
                                        sdk.command("uitheme:add", "local");

                                        $optsUITheme.val(previous)
                                                    .selectpicker('refresh');
                                    } else {
                                        $btnApply.isdisabled() && $btnApply.disable(false);
                                    }
                                })
                                .parents('.settings-field').show();
                            } else {
                                $optsUITheme.val(opts.uitheme)
                                            .selectpicker('refresh');
                            }

                            if ( nativevars.theme ) {
                                if ( nativevars.theme.addlocal == 'on' ) {
                                    const _combo = $('#opts-ui-theme select', $panel);
                                    _combo.append(`<option data-divider="true"></option>
                                                    <option value="add" l10n>${utils.Lang.settOptThemeAddLocal}</option>`);

                                    $optsUITheme.selectpicker('refresh');
                                }
                            }
                        }
                        _apply_theme(!!opts.uitheme ? opts.uitheme : 'theme-classic-light');

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

                        if ( !!opts.updates ) {
                            if ( opts.updates.mode !== undefined ) {
                                // if ( !['ask', 'disabled'].includes(opts.updates.mode) )
                                    // opts.updates.mode = 'ask';                          // for 7.3. to workaround 'silent' mode

                                ($optsAutoupdateMode = ($('#opts-autoupdate-mode', $panel).show().find('select')))
                                // ($optsAutoupdateMode = ($('#opts-autoupdate', $panel).show().find('select')))
                                    .val(opts.updates.mode)
                                    .selectpicker().on('change', e => {
                                        $btnApply.isdisabled() && $btnApply.disable(false);
                                    });
                            }

                            if ( opts.updates.interval !== undefined ) {
                                let $settnode = $('#opts-checkupdate', $panel);

                                if ( !$settnode.is(':visible') ) {
                                    $settnode.show();
                                    $('select', $settnode)
                                        .val(opts.updates.interval)
                                        .selectpicker().on('change', e => {
                                            $btnApply.isdisabled() && $btnApply.disable(false);
                                        });
                                }
                            }
                        }
                    }

                    if ( opts.rtl !== undefined ) {
                        $chRtl = $('#sett-box-rtl-mode', $panel).parent().show().find('#sett-rtl-mode');
                        $chRtl.prop('checked', !!opts.rtl)
                            .on('change', e => {
                                $btnApply.prop('disabled') && $btnApply.prop('disabled', false);
                            });

                        document.body.setAttribute('dir', 'rtl');
                        document.body.classList.add('rtl');
                    }

                    $('.settings-field:visible:last').css('margin-bottom','0');
                } else
                if (/updates/.test(cmd)) {
                    // TODO: will be deprecated soon
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
            } else
            if (/uitheme:added/.test(cmd)) {
                console.log('theme added');

                let _theme;
                try {
                    _theme = JSON.parse(param);
                }
                catch (e) {}

                if ( _theme ) {
                    _add_themes(_theme);

                    $optsUITheme.val(_theme[0].id)
                                .selectpicker('refresh');

                    $btnApply.isdisabled() && $btnApply.disable(false);
                }
            } else
            if (/renderervars:changed/.test(cmd)) {
                let opts;
                try { opts = JSON.parse( $('<div>').html(param).text() ); }
                catch (e) { /*delete opts;*/ }

                if ( opts.theme && opts.theme.system ) {
                    window.RendererProcessVariable.theme.system = opts.theme.system;

                    const theme_id = $optsUITheme.val();
                    if ( theme_id == 'theme-system' )
                        _apply_theme(theme_id);
                }
            }
        };

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

        function _on_lang_changed(ol,nl) {
            $('option[value=silent]', this.view.$panel).attr('data-subtext', utils.Lang.settOptDescAUpdateSilent);
            $('option[value=ask]', this.view.$panel).attr('data-subtext', utils.Lang.settOptDescAUpdateAsk);
            $('option[value=disabled]', this.view.$panel).attr('data-subtext', utils.Lang.settOptDescDisabled);

            // for ( let k of Object.keys(themes_map) ) {
            //     const t = themes_map[k]
            //     if ( t.l10n ) {
            //         const _new_title = t.l10n[nl] || t.l10n[nl.substring(0,2)] || t.name;
            //         $(`option[value=${k}]`, $optsUITheme).text(_new_title);
            //     }
            // }
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                const _scaling = [100, 125, 150, 175, 200, 225, 250, 275, 300, 350, 400, 450, 500];
                let _scaling_items = '';
                _scaling.forEach(val => _scaling_items += `<option value="${val}">${val}%</option>`);
                $('#opts-ui-scaling .combobox', this.view.$panel).append($(_scaling_items));

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
                CommonEvents.on('lang:changed', _on_lang_changed.bind(this));

                return this;
            },
            currentTheme: function() {
                const name = uitheme.id;
                return {name: name, type: themes_map[name] ? themes_map[name].type : THEME_TYPE_LIGHT};
            },
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
