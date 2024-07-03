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
    var ControllerFolders = function(args={}) {
        args.caption = 'Recent folders';
        args.action =
        this.action = "folders";
        this.view = new ViewFolders(args);
    };

    ControllerFolders.prototype = Object.create(baseController.prototype);
    ControllerFolders.prototype.constructor = ControllerFolders;

    var isSvgIcons = window.devicePixelRatio >= 2 || window.devicePixelRatio == 1;
    var ViewFolders = function(args) {
        var _lang = utils.Lang;

        args.id&&(args.id=`"id=${args.id}"`)||(args.id='');

        let _html = `
        <div ${args.id} class="action-panel ${args.action}">
            <div id="box-recent-folders">
                <div class="flexbox">
                    <h3 class="table-caption" l10n>${_lang.listRecentDirTitle}</h3>
                    <section id="dnd-file-zone"></section>
                    <div class="table-box flex-fill">
                        <table ${args.id} class="table-files list"></table>
                        <h4 class="text-emptylist${isSvgIcons ? '-svg' : ''} img-before-el" l10n>
                            ${isSvgIcons ? '<svg><use xlink:href="#folder-big"></use></svg>' : ''}
                            ${_lang.textNoFiles}
                        </h4>
                    </div>
                    <div id="box-open-acts" class="lst-tools">
                        <button id="btn-openlocal" class="btn btn--primary" l10n>${_lang.btnBrowse}</button>
                    </div>
                </div>
            </div>
        </div>`;

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = -1;
        args.tplItem = 'nomenuitem';
        // args.itemtext = _lang.actOpenLocal

        baseView.prototype.constructor.call(this, args);
    };

    ViewFolders.prototype = Object.create(baseView.prototype);
    ViewFolders.prototype.constructor = ViewFolders;

    window.ControllerFolders = ControllerFolders;

    utils.fn.extend(ControllerFolders.prototype, (function() {
        var _on_update = function(params) {
            var _dirs = utils.fn.parseRecent(params, 'folders'), $item;

            var $boxRecentDirs = this.view.$panel.find('#box-recent-folders');
            var $listRecentDirs = $boxRecentDirs.find('.table-files.list');

            $listRecentDirs.empty();
            for (let dir of _dirs) {
                if (!utils.getUrlProtocol(dir.full)) {
                    $item = $(app.controller.recent.view.listitemtemplate(dir));

                    $item.click({path: dir.full}, e=>{
                        openFile(OPEN_FILE_FOLDER, e.data.path);

                        e.preventDefault();
                        return false;
                    });
                    $listRecentDirs.append($item);
                }
            }
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);

                this.view.render();

                const dragAndDropZone = new DragAndDropFileZone();
                dragAndDropZone.render(this.view.$panel.find("#dnd-file-zone"));

                this.view.$panel.find('#btn-openlocal').click(()=>{
                    openFile(OPEN_FILE_FOLDER, '');
                });
                window.CommonEvents.on("icons:svg", (pasteSvg)=>{
                    let emptylist = $('[class*="text-emptylist"]', '#box-recent-folders');
                    emptylist.toggleClass('text-emptylist text-emptylist-svg');
                    if(pasteSvg && !emptylist.find('svg').length)
                        emptylist.prepend($('<svg class = "empty-folder"><use xlink:href="#folder-big"></use></svg>'));
                });
                window.sdk.on('onupdaterecents', _on_update.bind(this));

                return this;
            }
        };
    })());
}();

/*
*   controller definition
*/

// window.CommonEvents.on('main:ready', function(){
//     var p = new ControllerFolders({});
//     p.init();
// });