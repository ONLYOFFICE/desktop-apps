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
    'welcome' panel 
    controller + view
*/

+function(){ 'use strict'
    var ControllerWelcome = function(args={}) {
        args.caption = 'Welcome panel';
        args.action = 
        this.action = "welcome";

        this.view = new ViewWelcome(args);
    };

    ControllerWelcome.prototype = Object.create(baseController.prototype);
    ControllerWelcome.prototype.constructor = ControllerWelcome;

    var ViewWelcome = function(args) {
        var _lang = utils.Lang;

        var _html = `<div class="action-panel ${args.action}">
                      <div class="flexbox content-box">
                         <section class="center-box">
                              <div class='carousel'>
                                <figure class='carousel__slidebox'>                                
                                    <div class='carousel__slide active'>
                                        <p class='carousel__slide__text title' l10n>${_lang.welWelcome}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.welDescription}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect3' data-src='welcome'>
                                        </svg>
                                    </div>
                                    <div class='carousel__slide'>
                                        <p class='carousel__slide__text title' l10n>${_lang.emptySlide1Title}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.connect1Description}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect1' data-src='connect1'>
                                        </svg>
                                    </div>
                                    <div class='carousel__slide'>
                                        <p class='carousel__slide__text title' l10n>${_lang.connect2Title}</p>
                                        <p class='carousel__slide__text descr' l10n>${_lang.connect2Description}</p>
                                        <svg class='carousel__slide__img'>
                                            <use xlink:href='#connect2' data-src='connect2'>
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
                      </div>
                    </div>`;

        args.tplPage = _html;
        args.tplItem = 'nomenuitem';
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';

        baseView.prototype.constructor.call(this, args);
    };

    ViewWelcome.prototype = Object.create(baseView.prototype);
    ViewWelcome.prototype.constructor = ViewWelcome;

    window.ControllerWelcome = ControllerWelcome;

    utils.fn.extend(ControllerWelcome.prototype, {
        init: function() {
            baseController.prototype.init.apply(this, arguments);
            this.view.render();
            let carousel = {};
            var _scrollCarousel = function (direction) {
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
                    carousel.$items.filter('.pre-active-w').removeClass('pre-active-w').addClass('migrate');
                    carousel.$items.eq(_pre_index).removeClass('migrate pre-active-w active pro-active-w').addClass('pre-active-w');
                } else {
                    carousel.$items.filter('.pro-active-w').removeClass('pro-active-w').addClass('migrate');
                    carousel.$items.eq(_pro_index).removeClass('migrate pre-active-w active pro-active-w').addClass('pro-active-w');
                }

                carousel.$items.eq(_activeindex).removeClass('migrate pre-active-w pro-active-w').addClass('active');

                if ( direction == 'next' )
                    carousel.$items.eq(_pro_index).removeClass('migrate pre-active-w active pro-active-w').addClass('pro-active-w');
                else carousel.$items.eq(_pre_index).removeClass('migrate pre-active-w active pro-active-w').addClass('pre-active-w');
            };

            var _initCarousel = function() {
                let _$panel = this.view.$panel;
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
                 carousel.$items.eq(_pre_index).addClass('pre-active-w');
                 carousel.$items.eq(_pro_index).addClass('pro-active-w');

               _$panel.find('.carousel__scrolls > .carousel__scroll__btn')
                     .on('click', e => {
                         _scrollCarousel(e.target.getAttribute('value'));
                     });

                _on_theme_changed(localStorage.getItem('ui-theme-id'));
            };
            var _on_theme_changed = function(name,type) {if ( !!type )
                $('.carousel__slide__img > use').each((i, el) => {
                    const src = el.getAttribute('data-src');
                    if ( type == 'dark' )
                        el.setAttribute('xlink:href', `#${src}-dark`);
                    else el.setAttribute('xlink:href', `#${src}-light`);
                });};

            _initCarousel.call(this);
            window.CommonEvents.on('theme:changed',_on_theme_changed);

            return this;
        }
    });
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerWelcome({});
//     p.init();
// });