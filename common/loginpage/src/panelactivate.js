/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
    'activate' panel 
    controller + view
*/

+function(){ 'use strict'
    var ControllerActivate = function(args) {
        args.caption = 'Activate panel';
        args.action = 
        this.action = "activate";

        this.view = new ViewActivate(args);    
    };

    ControllerActivate.prototype = Object.create(baseController.prototype);
    ControllerActivate.prototype.constructor = ControllerActivate;

    var ViewActivate = function(args) {
        var _lang = utils.Lang;

        let _html = `<div class="action-panel ${args.action}">` +
                      '<div class="flexbox">' +
                        '<section>'+
                          `<h3 class="text-welcome">${_lang.licPanelTitle}</h3>` +
                          `<h4 class="text-description">${_lang.licPanelDescr}</h4>` +
                          `<input id="txt-key-activate" class="tbox" type="text" placeholder="${_lang.licKeyHolder}">` +
                          '<div class="lr-flex">'+
                            `<a class="text-sub link buynow" href="#">${_lang.licGetLicense}</a>` +
                            '<span />'+ 
                            `<div><img class="img-loader"><button class="btn primary doactivate">${_lang.btnActivate}</button></div>` +
                          '</div>' +
                        '</section>'+
                      '</div>'+
                    '</div>';

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 3;
        args.itemtext = _lang.actActivate;

        baseView.prototype.constructor.call(this, args);
    };

    ViewActivate.prototype = Object.create(baseView.prototype);
    ViewActivate.prototype.constructor = ViewActivate;

    window.ControllerActivate = ControllerActivate;

    utils.fn.extend(ControllerActivate.prototype, {
            onclick_activate: function(e) {
                var $tb = this.view.$panel.find('#txt-key-activate'),
                    key = $tb.val();
                if (!!key) {
                    this.disableCtrls(true);
                    this.view.$panel.find('.img-loader').show();
                    window.sdk.execCommand("app:activate", key);
                }
            },
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                this.view.$panel.find('.doactivate').click(this.onclick_activate.bind(this));
                this.view.$panel.find('#txt-key-activate').on('keypress', e => {
                    if (e.which == 13) {
                        this.onclick_activate(e);
                    }
                });
                this.view.$panel.find('.buynow').click(()=>{
                    window.sdk.execCommand('app:buynow', '');
                });

                return this;
            }
            ,disableCtrls: function(disable) {
                disable = disable ? 'disable' : '';
                this.view.$panel.find('.doactivate,#txt-key-activate').prop('disabled', disable);
                this.view.$panel.find('.img-loader').hide();
            }
            ,setPanelHidden: function(hidden) {
                if (hidden) {
                    this.view.$menuitem['hide']();
//                    this.view.$panel['hide']();
                } else {
                    this.view.$menuitem['show']();
                    this.view.$panel.find('#txt-key-activate').focus();
                }
            }
    });
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerActivate({});
//     p.init();
// });