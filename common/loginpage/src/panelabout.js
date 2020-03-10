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
    'about' panel
    controller + view
*/

+function(){ 'use strict'
    var ControllerAbout = function(args={}) {
        args.caption = 'About panel';
        this.action = "about";
    };

    ControllerAbout.prototype = Object.create(baseController.prototype);
    ControllerAbout.prototype.constructor = ControllerAbout;

    var ViewAbout = function(args) {
        var _lang = utils.Lang;

        args.tplPage = `<div class="action-panel ${args.action}"></div>`;
        args.itemcls = 'bottom extra';
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        // args.itemindex = 3;
        args.itemtext = _lang.actAbout;

        baseView.prototype.constructor.call(this, args);
    };

    ViewAbout.prototype = Object.create(baseView.prototype);
    ViewAbout.prototype.constructor = ViewAbout;
    ViewAbout.prototype.paneltemplate = function(args) {
        var _opts = args.opts;
        !!_opts.active && (_opts.edition = !!_opts.edition ? _opts.edition + ' ' + _opts.active : _opts.active);
        _opts.edition = !!_opts.edition ? `<div class="ver-edition">${_opts.edition}</div>` : '';

        var _lang = utils.Lang;
        let _html = '<div class="flexbox">'+
                        '<div class="box-ver">' +
                          `<div class="img-el ver-logo ${_opts.logocls}"></div><p></p>`+
                          `<div class="ver-version">${_opts.appname} ${_lang.strVersion} ${_opts.version}</div>${_opts.edition}<p></p>`+
                          `<a class="ver-checkupdate link" href="#" l10n>${_lang.checkUpdates}</a><p />`+
                          `<div class="ver-copyright">${_opts.rights}</div>`+
                          `<a class="ver-site link" target="popup" href="${_opts.link}">${_opts.site}</a>`+
                        '</div>'+
                        // '<div class="box-license flex-fill">'+
                        //   '<iframe id="framelicense" src="license.htm"></iframe>'+
                        // '</div>'+
                    '</div>';

        return _html;
    };
    ViewAbout.prototype.renderpanel = function(template) {
        this.$panel && this.$panel.empty();
        this.$panel.append(template);
    };

    window.ControllerAbout = ControllerAbout;

    utils.fn.extend(ControllerAbout.prototype, {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                let args = {action: this.action};

                window.sdk.on('on_native_message', (cmd, param) => {
                    if (/app\:version/.test(cmd)) {
                        try {
                            args.opts = JSON.parse( $('<div>').html(param).text() );
                        } catch (e) {
                            delete args.opts;
                        }

                        if (args.opts) {
                            !args.opts.site && (args.opts.site = utils.skipUrlProtocol(args.opts.link));
                        }

                        if (!this.view) {
                            this.view = new ViewAbout(args);
                            this.view.render();
                            this.view.$menuitem.removeClass('extra');
                        } 

                        this.view.renderpanel(this.view.paneltemplate(args));
                        this.view.$panel.find('.ver-checkupdate').on('click', (e) => {
                            window.sdk.execCommand('update', 'check');
                        });
                        this.view.$panel.find('.ver-checkupdate')[this.updates===true?'show':'hide']();
                    } else
                    if (/^updates:turn/.test(cmd)) {
                        this.updates = param == 'on';

                        if ( this.view ) {
                            this.view.$panel.find('.ver-checkupdate')[this.updates?'show':'hide']();
                        }
                    }
                });

                return this;
            }
    });
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerAbout({});
//     p.init();
// });
