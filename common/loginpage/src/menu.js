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

'use strict';

var Menu = function(args) {
    this.config = Object.assign({}, args);
    this.id = args.id;
    this.items = args.items;

    this.prefix = args.prefix || 'asc-gen';
    this.events = {};
    this.events.itemclick = new ModelEvent(this);
};

Menu.prototype.init = function(parent) {
    var me = this;
    var _tpl_ = '<div id="%id" class="menu-container">'+
                    '<div class="dropdown-toggle" data-toggle="dropdown"></div>'+
                    '<ul class="dropdown-menu" role="menu">' +
                    '</ul></div>';

    var $container = $(_tpl_.replace(/\%id/, this.id)).appendTo(parent);
    var $list = $container.find('ul');

    var _tpl_item_ = '<li><a id="%id" class="dd-item" tabindex="-1" type="menuitem" l10n>%icon%caption</a></li>',
        _tpl_divider_ = '<li class="divider"></li>';

    this.items.forEach(function(item) {
        let $item;

        !item.id && (item.id = me.prefix + ++nCounter);
        if (item.caption === '--') {
            $item = $(_tpl_divider_);
        } else {
            let iconHtml = '';
            if (item.icon) {
                if (item.icon.startsWith('#')) {
                    iconHtml = `<svg class="menu-icon"><use xlink:href="${item.icon}"></use></svg>`;
                } else {
                    iconHtml = `<i class="menu-icon ${item.icon}"></i>`;
                }
            }

            $item = $(_tpl_item_
                .replace(/%caption/, item.caption)
                .replace(/%id/, item.id)
                .replace(/%icon/, iconHtml));

            if (item.variant) {
                $item.find('a').attr('data-variant', item.variant);
            }

            $item.on('click', {'action': item.action}, function(e) {
                if ( !e.target.hasAttribute('disabled') )
                    me.events.itemclick.notify(e.data['action'], me.contextdata);
            });
        }

        $list.append($item);
    });

    $container.on({
        'hide.bs.dropdown': function(){
            // console.log('before hide menu');
        },
        'hidden.bs.dropdown': function() {
            me.contextdata = [];
            // console.log('after hide menu');
        }
    });
};

Menu.prototype.show = function(pos, data) {
    $('.menu-container').removeClass('open');
    let $el = $('#'+this.id);

    let $dd = $el.find('.dropdown-menu');
    let _right = $dd.width() + pos.left,
        _rlimit = $(document).width();
    if (!!_right && _right > _rlimit) {
        pos.left -= (_right - _rlimit + 4);
    }

    let _top = $dd.height() + pos.top,
        _blimit = $(document).height();
    if (!!this.config.bottomlimitoffset)
        _blimit -= this.config.bottomlimitoffset;
    if (!!_top && _top > _blimit) {
        pos.top -= (_top - _blimit + 4);
    }

    $el.css(pos);
    $dd.dropdown('toggle');
    this.contextdata = data;
    Menu.opened = true;
};

Menu.prototype.showUnderElem = function(el, data) {
    $('.menu-container').removeClass('open');
    let $el = $('#'+this.id);
    const $rel = $(el);

    // const $rel.width();
    const pos = $rel.position();
    pos.top += $rel.height() + 2;

    const $dd = $el.find('.dropdown-menu');
    pos.left -= $dd.outerWidth() - $rel.outerWidth();

    $el.css(pos);
    $dd.dropdown('toggle');
    this.contextdata = data;
    Menu.opened = true;
};

Menu.prototype.disableItem = function(action, disable) {
    for (let item of this.items) {
        if (item.action == action) {
            let $el = $('#' + item.id);
            if ( disable ) {
                $el.attr('disabled', 'disabled');
                $el.parent()['addClass']('disabled');
            } else {
                !!$el.attr('disabled') && $el.removeAttr('disabled');
                $el.parent()['removeClass']('disabled');
            }
        }
    }
};

Menu.prototype.hideItem = function(action, hide) {
    for (let item of this.items) {
        if (item.action == action) {
            $('#' + item.id).parent()
                [hide?'hide':'show']();
        }
    }
};

Menu.opened = false;

Menu.closeAll = function() {
    $('.menu-container.open').removeClass('open');
    Menu.opened = false;
};

window.Menu = Menu;

$(document).on('keydown', function(e) {
    if (e.keyCode == 27) {
        // $('.menu-container').removeClass('open');
        Menu.closeAll();
    }
});

$(window).resize(function(){
    Menu.closeAll();
});
