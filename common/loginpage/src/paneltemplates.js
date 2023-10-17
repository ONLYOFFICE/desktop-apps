/*
 * (c) Copyright Ascensio System SIA 2010-2022
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

+function(){ 'use strict'
    const ControllerTemplates = function(args={}) {
        args.caption = 'Templates';
        args.action =
        this.action = 'templates';
        this.view = new ViewTemplates(args);
    };

    ControllerTemplates.prototype = Object.create(baseController.prototype);
    ControllerTemplates.prototype.constructor = ControllerTemplates;

    window.ControllerTemplates = ControllerTemplates;

    function createIframe(config) {
        var iframe = document.createElement("iframe");

        iframe.width = "100%";
        iframe.height = "100%";
        iframe.align = "top";
        iframe.frameBorder = 0;
        // iframe.name = "frameEditor";
        iframe.allowFullscreen = true;

        return iframe;
    }

    var ViewTemplates = function(args) {
        var _lang = utils.Lang;

        const msg = 'Oops! Something went wrong :(<br>Check internet connection';
        this.emptyPanelContent = `<div id="frame">`;

        args.tplPage = `<div class="action-panel ${args.action}">${this.emptyPanelContent}</div></div>`;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 0;
        args.itemtext = _lang.actTemplates;

        baseView.prototype.constructor.call(this, args);
    };

    ViewTemplates.prototype = Object.create(baseView.prototype);
    ViewTemplates.prototype.constructor = ViewTemplates;

    utils.fn.extend(ControllerTemplates.prototype, (function() {
        let iframe;

        // TODO: for tests only. uncomment static url before release
        let test_url = localStorage.templatesdomain ? localStorage.templatesdomain : 'https://oforms.onlyoffice.com';
        const _url_templates = `${test_url}/{0}?desktop=true`;
        // const _url_templates = "https://oforms.onlyoffice.com/{0}?desktop=true";

        const _create_and_inject_iframe = () => {
            const re = /(theme-(?!type)[\w-]+)/.exec(document.body.className);
            const theme_id = re ? re[1] : 'theme-light';

            const iframe = createIframe({});
            iframe.src = `${_url_templates.replace('{0}',utils.Lang.id.substring(0,2))}&theme=${theme_id}`;

            const target = document.getElementById("frame");
            target.parentNode && target.parentNode.replaceChild(iframe, target);
            return iframe;
        }

        const _remove_frame = content => {
            if ( !!iframe ) {
                iframe.parentElement.innerHTML = content;
                iframe = null;
            }
        }

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);
                this.view.render();
                errorBox.render({
                    parent: $('#frame')[0],
                    lang: utils.Lang.id,
                });

                const _check_url_avail = () => {
                    if ( !iframe ) {
                        fetch(_url_templates.replace('{0}', 'en'), {mode: 'no-cors'}).
                            then(r => {
                                if ( r.status == 200 || r.type == 'opaque' ) {
                                    iframe = _create_and_inject_iframe();
                                }
                            }).
                            catch(e => console.error('error on check templates url', e));
                    }
                }

                _check_url_avail();

                CommonEvents.on('panel:show', panel => {
                    if ( !iframe && panel == this.action ) {
                        _check_url_avail();
                    }
                });

                CommonEvents.on('lang:changed', (old, newlang) => {
                    window.errorBox.translate(newlang);

                    if ( !!iframe ) {
                        // iframe.contentWindow.postMessage(JSON.stringify({lang: newlang}));
                        _remove_frame(this.view.emptyPanelContent);
                        _check_url_avail();
                    }
                });

                CommonEvents.on('theme:changed', (theme, type) => {
                    if ( !!iframe ) {
                        // iframe.contentWindow.postMessage(JSON.stringify({theme: {name: theme, type: type}}));
                        _remove_frame(this.view.emptyPanelContent);
                        _check_url_avail();
                    }
                });

                // if ( !!localStorage.templatespanel ) {
                    // let iframe;
                    // if ( navigator.onLine ) {
                    //     iframe = _create_and_inject_iframe();
                    // } else {
                    //     CommonEvents.on('panel:show', panel => {
                    //         if ( !iframe && panel == this.action && navigator.onLine) {
                    //             iframe = _create_and_inject_iframe();
                    //         }
                    //     });
                    // }
                // } else {
                //     this.view.$menuitem.find('> a').click(e => {
                //         window.sdk.command("open:template", 'external');
                //         e.preventDefault();
                //         e.stopPropagation();
                //     });
                // }

                return this;
            }
        };
    })());
}();