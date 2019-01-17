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

window.DialogConnect = function(params) {
    "use strict";

    !params && (params = {});
    !params.provider && (params.provider = 'asc');

    let $el, $title, $body;
    var _events = { close: params.onclose };
    var _template = `<dialog class="dlg dlg-connect-to">
                        <div class="title">
                            <label class="caption">Title</label>
                            <span class="tool close"></span>
                        </div>
                        <div class="body"></div>
                    </dialog>`;

    var _body_template = `<section>
                            <div id="panel-portal" class="sl-panel">
                                <div class='select-field dlg--option'>
                                    <section class='box-cmp-select'>
                                        <select class='combobox'></select>
                                    </section>
                                </div>
                                <div class="error-box">
                                    <p id="auth-error" class="msg-error">asd</p>
                                    <input id="auth-portal" type="text" name="" spellcheck="false" class="tbox dlg--option" placeholder="${utils.Lang.pshPortal}" value="">
                                </div>
                                <div style="height:10px;"></div>
                                <div class="lr-flex">
                                    <a class="text-sub link newportal" target="popup" href="javascript:void(0)">${utils.Lang.linkCreatePortal}</a>
                                    <span />
                                    <div>
                                        <img class="img-loader">
                                        <button id="btn-next" class="btn primary">${utils.Lang.btnConnect}</button>
                                    </div>
                                </div>
                            </div>
                        </section>`;

    function _set_title(title) {
        $title.find('.caption').text(title);
    };

    function _set_error(error, focusel) {
        let $label = $el.find('#auth-error');

        if ( !!error ) {
            $label.text(error).fadeIn(100);
            !!focusel && $body.find(focusel).addClass('error').focus();
        } else {
            $label.fadeOut(100);
            $body.find('.error').removeClass('error');
        }
    };

    let _clear_error = _set_error.bind(this,'');

    function _on_close_click(e) {
        _close();
    };

    function _close(opts) {
        // $el[0].hasAttribute('open') && $el[0].close();
        $el.remove();

        if ( _events.close ) {
            _events.close(opts);
        }
    };

    function _on_click_connect() {
        _clear_error();

        let provider = $body.find('select').val(),
            portal = $body.find('#auth-portal').val(),
            protocol = 'https://';

        var re_wrong_symb = /([\s\r\n\t\\]|%(?!\d{2}))/;
        if (!portal.length || re_wrong_symb.test(portal)) {
            _set_error(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        portal = /^(https?:\/{2})?([^\s\?\&]+)/i.exec(portal);
        if (!!portal && portal[2].length) {
            portal[1] && (protocol = portal[1]);
            portal = portal[2];
        } else {
            _set_error(utils.Lang.errLoginPortal, '#auth-portal');
            return;
        }

        /* skip last '/' for owncloud */
        if ( provider == 'ownc' ) {
            portal.endsWith('/') && (portal = portal.slice(0,-1));
        }
        if ( provider == 'ownc' || provider == 'nextc' ) {
            portal.endsWith('/index.php/login') && (portal = portal.slice(0,-16));
        }

        _disable_dialog(true);

        // portal += config.portals.checklist[farm];

        let _callback = (obj) => {
            if ( obj ) {
                if ( obj.status == 'success' ) {
                    _close({
                        portal:protocol+portal,
                        provider:provider
                    });
                } else {
                    if ( obj.response.status == 404 )
                        _set_error(utils.Lang.errLoginPortal, '#auth-portal');
                    else _set_error((obj.response && obj.response.statusText) || obj.status, '#auth-portal');
                }
            }

            _disable_dialog(false);
        };

        _require_portal_info(protocol + portal, provider).then(
            _callback,
            obj => {
                if ( obj.status == 'error' && obj.response.status == 404 ) {
                    protocol = protocol == "https://" ? "http://" : "https://";
                    return _require_portal_info(protocol + portal, provider);
                } else _callback(obj);
                return obj;
            }
        ).then( _callback, _callback );
    };

    function _bind_events() {
        $body.on('keypress', '.tbox', 
            function(e) {
                if (e.which == 13) {
                    if (/auth-portal/.test(e.target.id)) 
                        _on_click_connect(); 
                    // else
                    // if (/auth-email/.test(e.target.id)) 
                    //     $el.find('#auth-pass').focus(); else
                    // if (/auth-pass/.test(e.target.id)) {
                    //     $el.find('#btn-login').focus().click();
                    // }
                }
        });

        // $el.on('keyup', e => {
        //     if (e.which == 27) _close();
        // });
    };

    function _disable_dialog(disable) {
        $body.find('.tbox, #btn-login').prop('disabled', disable);
        $body.find('.img-loader')[disable?'show':'hide']();
    };

    function _require_portal_info(portal, provider) {
        !provider && (provider = 'asc');
        let _info = config.portals.checklist.find(i => i.id == provider);
        var _url = portal + _info.check.url;

        return new Promise((resolve, reject) => {
            $.ajax({
                url: _url,
                crossOrigin: true,
                crossDomain: true,
                timeout: 10000,
                headers: _info.check.headers,
                complete: function(e, status) {
                    status == 'success' ?
                        resolve({status:status, response:e}) :
                        reject({status:status, response:e});
                },
                error: function(e, status, error) {
                    reject({status:status, response:e});
                }
            });
        });
    };

    return {
        show: function() {
            $el = $('#placeholder').append(_template).find('.dlg-connect-to');
            $title = $el.find('.title');
            $body = $el.find('.body');

            $el.width(450);
            $title.find('.tool.close').bind('click', _on_close_click);

            _set_title( utils.Lang.loginTitleStart );
            $body.html( _body_template );

            let $combo = $el.find('select');
            let _clouds = config.portals.checklist;
            if ( _clouds.length == 1 && _clouds[0].id == 'asc' ) {
                $combo.append(`<option value='asc'>onlyoffice</option>`);
                $combo.parents('.select-field').hide();
            } else {
                for (let c of _clouds) {
                    $combo.append(`<option value='${c.id}'>${c.name}</option>`);
                }
                $combo.val(params.provider);

                let $newportal = $el.find('.newportal').disable(!(params.provider=='asc'));
                $combo.on('change', e => {
                    $newportal.disable(!(e.target.value=='asc'));
                    _clear_error();
                });

                $combo.selectpicker();
            }
            
            let $portal = $body.find('#auth-portal');
            if ( !!params.portal ) $portal.val(utils.skipUrlProtocol(params.portal));
            $body.find('#btn-next').on('click', _on_click_connect);

            _bind_events();
            $el.get(0).showModal();
            $el.addClass('scaled');
            $el.on('close', _on_close_click);
            $body.find('.newportal').on('click', e => {CommonEvents.fire("portal:create"); _close();});

            setTimeout(()=>{$portal.focus();}, 50);
        },
        portalexists: _require_portal_info
    };
};
