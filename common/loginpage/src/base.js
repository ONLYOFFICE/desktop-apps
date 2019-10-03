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
*   base controller declaration
*/

+function() {
    var controller = function(args) {
        this.$menuitem = this.$panel = undefined;
    };

    controller.prototype.view = undefined;
    controller.prototype.init = function() {
        this.view && this.view.init();
    };

    var view = function(args) {
        this.rendered = false;

        var _action = args.action ? `action="${args.action}"` : '';
        var _itemcls = 'menu-item' + (args.itemcls?` ${args.itemcls}`:'');

        this.tplPage = args.tplPage || '<div class="center-panel">Hello, Word!</div>';
        this.tplItem = args.tplItem || `<li class="${_itemcls}"><a l10n ${_action}>${args.itemtext}</a></li>`;
        this.menuContainer = args.menu || '';
        this.panelContainer = args.field || '';
        this.opts = args;
    };

    view.prototype.init = function() {
    };

    view.prototype.render = function() {
        if (!this.rendered) {
            this.rendered = true;

            let _index = this.opts.itemindex;
            this.$menuitem = this.renderMenuItem.call(this, this.tplItem, _index);

            if (this.tplPage != 'empty') {
                let $parentview = $(this.panelContainer);
                if (_index >= 0) {
                    let $panels = $parentview.children();
                    this.$panel = $panels.size() > _index ?
                        $(this.tplPage).insertBefore($panels.eq(_index)) : $(this.tplPage).appendTo($parentview);

                        let a = $panels.eq(_index);
                        let b = 1;
                } else {
                    this.$panel = $(this.tplPage).appendTo($parentview);
                }
            }
        }
    };

    view.prototype.renderMenuItem = function(node, index) {
        let $itemout;
        if (node != 'nomenuitem') {
            $itemout = $(node);

            let _ib = utils.fn.getToolMenuItemOrder($itemout);
            if ( !_ib.item ) {
                let $menu = $(this.menuContainer);
                $itemout.appendTo($menu);
            } else {
                _ib.after ? $itemout.insertAfter(_ib.item) : $itemout.insertBefore(_ib.item);
            }
        }

        return $itemout;
    };

    window.baseView = view;
    window.baseController = controller;
}();

