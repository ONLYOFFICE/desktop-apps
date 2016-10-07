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

'use strict';
$(document).ready(function() {
    $('.tool-menu').on('click', '> .menu-item > a', onActionClick);
    $('.tool-quick-menu .menu-item a').click(onNewFileClick);


    !window.app && (window.app = {controller:{}});
    !window.app.controller && (window.app.controller = {});

    window.app.controller.recent = (new ControllerRecent).init();
    window.app.controller.folders = (new ControllerFolders).init();
    window.app.controller.about = (new ControllerAbout).init();
    if (!!window.ControllerPortals)
        window.app.controller.portals = (new ControllerPortals({})).init();

    $('h3.createnew').text(utils.Lang.actCreateNew);
    $('a[action="new:docx"]').text(utils.Lang.newDoc);
    $('a[action="new:xlsx"]').text(utils.Lang.newXlsx);
    $('a[action="new:pptx"]').text(utils.Lang.newPptx);


    if (!localStorage.welcome) {
        app.controller.welcome = (new ControllerWelcome).init();
        selectAction('welcome');

        localStorage.setItem('welcome', 'have been');
    } else 
        selectAction('recent');

    $('#placeholder').on('click', '.newportal', function(){
        CommonEvents.fire("portal:create");
    });

    if (!window.LoginDlg) {
        $('.tools-connect').hide();
        hideAction('connect');
    }

    if (!utils.inParams.waitingloader)
        setLoaderVisible(false);

    /* test information */
    // var arr = [
    //     {"id":0,"type":utils.defines.FileFormat.FILE_CROSSPLATFORM_PDF,"path":"https://testinfo.teamlab.info/New Document.docx","modifyed":"11.12.2015 18:45"},
    //     {"id":1,"type":utils.defines.FileFormat.FILE_CROSSPLATFORM_DJVU,"path":"C:\\Users\\maxim.kadushkin\\Documents\\DPIConfig_SmallPCs.docx","modifyed":"11.12.2015 18:22"},
    //     {"id":2,"type":utils.defines.FileFormat.FILE_CROSSPLATFORM_XPS,"path":"/Users/ayuzhin/Develop/Web/test.html","modifyed":"11.12.2015 17:58"},
    //     {"id":3,"type":utils.defines.FileFormat.FILE_PRESENTATION_PPTX,"path":"https://testinfo.teamlab.info\\Sadfasd.docx","modifyed":"10.12.2015 17:25"},
    //     {"id":4,"type":utils.defines.FileFormat.FILE_SPREADSHEET_CSV,"path":"https://testinfo.teamlab.info\\Sadfasd.docx","modifyed":"10.12.2015 17:25"},
    //     {"id":5,"type":utils.defines.FileFormat.FILE_SPREADSHEET_ODS,"path":"https://testinfo.teamlab.info\\Sadfasd.docx","modifyed":"10.12.2015 17:25"},
    //     {"id":6,"type":utils.defines.FileFormat.FILE_SPREADSHEET_XLS,"path":"https://testinfo.teamlab.info\\Office 365 Value Added Reseller Guide.docx","modifyed":"10.12.2015 16:46"}
    // ];
    // window.onupdaterecents(arr);
    // window.onupdaterecovers(arr);
    /* **************** */

    setTimeout(()=>{
        if (window.sdk) {
            window.sdk.LocalFileRecovers();
            window.sdk.LocalFileRecents();

            window.sdk.execCommand('app:onready', '');
        } 
    }, 50);
});

function onActionClick(e) {
    var $el = $(this);
    var action = $el.attr('action');

    if (action == 'open' && 
            !app.controller.recent.getRecents().size() && 
                !app.controller.recent.getRecovers().size()) 
    {
        openFile(OPEN_FILE_FOLDER, '');
    } else {
        $('.tool-menu > .menu-item').removeClass('selected');
        $el.parent().addClass('selected');
        $('.action-panel').hide();
        $('.action-panel.' + action).show(0,()=>{
            // bug: recent panel has the wrong height if 'wellcome' panel is showed firstly
            if (action == 'recent') {
                app.controller.recent.view.updatelistsize();
            }
        });
    }
};

function selectAction(action) {
    $('.tool-menu > .menu-item').removeClass('selected');
    $('.tool-menu a[action='+action+']').parent().addClass('selected');
    $('.action-panel').hide();
    $('.action-panel.' + action).show();
};

function hideAction(action, hide) {
    var mitem = $('.tool-menu a[action='+action+']').parent();
    mitem.removeClass('extra')[hide===true?'show':'hide']();
    $('.action-panel.' + action)[hide===true?'show':'hide']();
};

function setLoaderVisible(isvisible, timeout) {
    setTimeout(function(){
        $('#loading-mask')[isvisible?'show':'hide']();
    }, timeout || 200);
}

var OPEN_FILE_RECOVERY = 1;
var OPEN_FILE_RECENT = 2;
var OPEN_FILE_FOLDER = 3;
var Scroll_offset = '16px';

function onNewFileClick(e) {
    if ($(e.currentTarget).parent().hasClass('disabled'))
        return;
    
    var me = this;
    if (me.click_lock===true) return;
    me.click_lock = true;

    var t = -1;
    switch (e.currentTarget.attributes['action'].value) {
    case 'new:docx': t = 0; break;
    case 'new:xlsx': t = 2; break;
    case 'new:pptx': t = 1; break;
    default: break;
    }

    createFile(t);

    setTimeout(function(){
        me.click_lock = false;
    }, utils.defines.DBLCLICK_LOCK_TIMEOUT);
}

window.sdk.on('on_native_message', function(cmd, param) {
    if (cmd == 'portal:logout') {
        // var short_name = utils.skipUrlProtocol(param);
        // var model = portalCollection.find('name', short_name);
        // !!model && model.set('logged', false);
    } else 
    if (cmd == 'lic:active') {
        let is_active_license = param == '1';

        if (is_active_license) {
            let is_selected = $('.tool-menu > li.menu-item.selected > a[action=activate]').length > 0;
            is_selected && selectAction('recent');
        } else {
            !app.controller.activate &&
                (app.controller.activate = (new ControllerActivate({})).init());
        }

        if ( app.controller.activate ) {
            app.controller.activate.setPanelHidden(is_active_license);
            app.controller.activate.disableCtrls(false);
        }
    } else 
    if (cmd == 'lic:selectpanel') {
        selectAction('activate');
        app.controller.activate.setPanelHidden(false);
    } else 
    if (cmd == 'lic:sendkey') {
        // $('a[action=activate]').parent()['show']();
    } else
    if (/^panel\:hide/.test(cmd)) {
        let hide = param == '1';
        let panel = /\:hide\:(\w+)$/.exec(cmd)[1];

        if (panel.length) {
            hideAction(panel, hide);
        }
    } else
    if (/app\:ready/) {
        setLoaderVisible(false);
    }
    
    console.log(cmd, param);
});

function openFile(type, params) {
    if (window.sdk) {
        if (type == OPEN_FILE_FOLDER) {
            window.sdk.LocalFileOpen(params);
        } else {
            var _method = type == OPEN_FILE_RECOVERY ? 
                            'LocalFileOpenRecover' : 'LocalFileOpenRecent';

            window.sdk[_method](parseInt(params));
        }
    } 
}

function createFile(type) {
    if (window.sdk) {
        window.sdk["LocalFileCreate"](type);
    } 
}

document.getElementById('wrap').ondrop = function(e) {
    window.sdk["DropOfficeFiles"]();

    e.preventDefault();
    return false;
}

document.getElementById('wrap').ondragover = function (e) {
    e.dataTransfer.dropEffect = 'copy';

    e.preventDefault();
    return false;
};
