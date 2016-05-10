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

    Templates.insertFilesTable({holder:'#box-recovery', id: 'tbl-filesrcv', caption: utils.Lang.listRecoveryTitle, coltitle:false});
    Templates.insertFilesTable({holder:'#box-recent', caption: utils.Lang.listRecentFileTitle, coltitle:false});
    

    !window.app && (window.app = {controller:{}});
    !window.app.controller && (window.app.controller = {});

    window.app.controller.folders = (new ControllerFolders({})).init();
    window.app.controller.about = (new ControllerAbout).init();
    if (!!window.ControllerPortals)
        window.app.controller.portals = (new ControllerPortals({})).init();

    /* popup menu */
    var menuFiles = new Menu({
        id: 'pp-menu-files',
        items: [{
            caption: utils.Lang.menuFileOpen,
            action: 'files:open'
        },{
            caption: utils.Lang.menuRemoveModel,
            action: 'files:forget'
        },{
            caption: utils.Lang.menuClear,
            action: 'files:clear'
        }]
    });

    menuFiles.init('#placeholder');
    menuFiles.events.itemclick.attach(clickMenuFiles);
    /**/    
   
    { 
        let $el = $('.action-panel.open');
        $el.find('#btn-openlocal').click(function() {
            openFile(OPEN_FILE_FOLDER, '');
        }).text(utils.Lang.btnBrowse);
    }

    $('h3.createnew').text(utils.Lang.actCreateNew);
    $('a[action="new:docx"]').text(utils.Lang.newDoc);
    $('a[action="new:xlsx"]').text(utils.Lang.newXlsx);
    $('a[action="new:pptx"]').text(utils.Lang.newPptx);
    $('a[action=recent]').text(utils.Lang.actRecentFiles);

    var $boxRecovery = $('.action-panel.recent #box-recovery');
    var $listRecovery = $boxRecovery.find('.table-files.list');
    var $headerRecovery = $boxRecovery.find('.header');
    var $scrboxRecovery = $boxRecovery.find('.flex-fill');

    var $boxRecent = $('.action-panel.recent #box-recent');
    // var $listRecent = $boxRecent.find('.table-files.list');
    var $scrboxRecent = $boxRecent.find('.flex-fill');
    var separatorHeight = $('<div id="recovery-sep"></div>').insertAfter($('#box-recovery')).height();

    // $('button#btn-add').click(function(e) {
    //     let info = {type:'pptx', name:'New Document.txt', descr:'e:/from/some/portal'};
    //     recentCollection.add( new FileModel(info) );
    // });

    // $('button#btn-add2').click(function(e) {
    //     let info = {type:'pptx', name:'New Document.txt', descr:'e:/from/some/folder'};
    //     recoveryCollection.add( new FileModel(info) );

    //     $boxRecovery[recoveryCollection.size() > 0 ? 'show' : 'hide']();
    //     $('#recovery-sep')[recoveryCollection.size() > 0 ? 'show' : 'hide']();
    //     sizeRecoveryList();
    // });

    // 

        // bug: recent panel has the wrong height if 'wellcome' panel is showed firstly
        $('.tool-menu > .menu-item > a[action=recent]').on('click.once', function(e){
            $(e.target).off('.once');
            sizeRecoveryList();
        });

    function sizeRecoveryList() {
        // set fixed height for scrollbar appearing. 
        var _available_height = $boxRecent.parents('.action-panel').height();
        var _box_recent_height = _available_height;

        if ($boxRecovery.is(':hidden')) {
            // $boxRecent.height($boxRecent.parent().height());
        } else {
            _available_height -= separatorHeight
            _box_recent_height *= 0.5; 

            $boxRecovery.height(_available_height * 0.5);

            var $table_box = $boxRecovery.find('.table-box');
            if ( !$table_box.hasScrollBar() ) {
                let _new_recovery_height = $table_box.find('.table-files.list').height() + $headerRecovery.height();
                $boxRecovery.height(_new_recovery_height);
                _box_recent_height = _available_height - _new_recovery_height;
            }
        }

        /*$boxRecent.height() != _box_recent_height &&*/ $boxRecent.height(_box_recent_height);
    };

    window.sdk.on('onupdaterecents', params=>{
        recentCollection.empty();

        var files = utils.fn.parseRecent(params);
        for (let item of files) {
            recentCollection.add( new FileModel(item) );
        }

        /* set offset when the scroll bar appear */
        // $listRecent.find('tr > td:last-child').css('padding-left', $scrboxRecent.hasScrollBar() ? Scroll_offset : '');
        // $listRecentDirs.find('tr > td:last-child').css('padding-left', $scrboxRecentDirs.hasScrollBar() ? Scroll_offset : '');
    });

    window.onupdaterecovers = function(params) {
        recoveryCollection.empty();

        var files = utils.fn.parseRecent(params);
        for (let item of files) {
            recoveryCollection.add( new FileModel(item) );
        }

        $boxRecovery[recoveryCollection.size() > 0 ? 'show' : 'hide']();
        $('#recovery-sep')[recoveryCollection.size() > 0 ? 'show' : 'hide']();
        sizeRecoveryList();
    };

    $(window).resize(function(){
        Menu.closeAll();
        sizeRecoveryList();
    });

    /** recent collection **/
    recentCollection = new Collection({
        view: $('.action-panel.recent #box-recent')
        ,list: '.table-files.list'
    });
    recentCollection.events.erased.attach(function(collection){
        collection.view.find(collection.list).parent().addClass('empty');
    });
    recentCollection.events.inserted.attach(function(collection, model){
        let $list = collection.view.find('.table-files.list');
        
        let $item = $(Templates.produceFilesItem(model));
        $list.append($item);
        $list.parent().removeClass('empty');
    });
    recentCollection.events.click.attach(function(collection, model){
        openFile(OPEN_FILE_RECENT, model.fileid);
    });
    recentCollection.events.contextmenu.attach(function(collection, model, e){
        menuFiles.actionlist = 'recent';
        menuFiles.show({left: e.clientX, top: e.clientY}, model);
    });
    recentCollection.empty();    
    /**/

    /** recent collection **/
    recoveryCollection = new Collection({
        view: $('.action-panel.recent #box-recovery')
        ,list: '.table-files.list'
    });
    recoveryCollection.events.inserted.attach(function(collection, model){
        let $list = collection.view.find('.table-files.list');
        
        let $item = $(Templates.produceFilesItem(model));
        $list.append($item);    
    });
    recoveryCollection.events.click.attach(function(collection, model){
        openFile(OPEN_FILE_RECOVERY, model.fileid);
    });
    recoveryCollection.events.contextmenu.attach(function(collection, model, e){
        menuFiles.actionlist = 'recovery';
        menuFiles.show({left: e.clientX, top: e.clientY}, model);
    });
    /**/

    /* test information */
    // var info = {portal:"https://testinfo.teamlab.info",user:"Maxim Kadushkin",email:"Maxim.Kadushkin@avsmedia.net"};
    // PortalsStore.keep(info);
    /* **************** */

    if (!localStorage.welcome) {
        Templates.createWelcomePanel('.action-panel.welcome');
        selectAction('welcome');

        localStorage.setItem('welcome', 'have been');
    } else 
        selectAction('recent');

    $('.newportal').click(function(){
        window.open(utils.defines.links.regnew);
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
        if (window.AscDesktopEditor) {
            window.AscDesktopEditor.LocalFileRecovers();
            window.AscDesktopEditor.LocalFileRecents();

            window.AscDesktopEditor.execCommand('app:onready', '');
        } 
    }, 50);
});

var recentCollection;
var recoveryCollection;

function onActionClick(e) {
    var $el = $(this);
    var action = $el.attr('action');

    if (action == 'open' && 
            !recentCollection.size() && !recoveryCollection.size()) {
        openFile(OPEN_FILE_FOLDER, '');
    } else {
        $('.tool-menu > .menu-item').removeClass('selected');
        $el.parent().addClass('selected');
        $('.action-panel').hide();
        $('.action-panel.' + action).show();
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

function onRecentClick(e) {
    openFile(e.data.rcv === true ? 
        OPEN_FILE_RECOVERY : OPEN_FILE_RECENT, e.data.id);

    e.preventDefault();
    return false;
}

function onRecentFolderClick(e) {
    openFile(OPEN_FILE_FOLDER, e.data.path);

    e.preventDefault();
    return false;
}

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

function clickMenuFiles(menu, action, data) {
    if (/\:open/.test(action)) {
        menu.actionlist == 'recent' ?
            openFile(OPEN_FILE_RECENT, data.fileid) :
            openFile(OPEN_FILE_RECOVERY, data.fileid);
    } else
    if (/\:clear/.test(action)) {
        menu.actionlist == 'recent' ?
            AscDesktopEditor.LocalFileRemoveAllRecents() :
            AscDesktopEditor.LocalFileRemoveAllRecovers();
    } else
    if (/\:forget/.test(action)) {
        menu.actionlist == 'recent' ?
            AscDesktopEditor.LocalFileRemoveRecent(parseInt(data.fileid)) :
            AscDesktopEditor.LocalFileRemoveRecover(parseInt(data.fileid));
    }
};

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
    if (window["AscDesktopEditor"]) {
        if (type == OPEN_FILE_FOLDER) {
            if (window.AscDesktopEditor.LocalFileOpen) {
                window.AscDesktopEditor.LocalFileOpen(params);
            } else {
                alert("desktop!!! (open)");
            }
        } else {
            var _method = type == OPEN_FILE_RECOVERY ? 
                            'LocalFileOpenRecover' : 'LocalFileOpenRecent';

            if (window.AscDesktopEditor[_method]) {
                window.AscDesktopEditor[_method](parseInt(params));
            } else {
                // alert("desktop!!! (" + _method + ": " + params + ")");
            }
        }
    } else {
        // alert("desktop!!! AscDesktopEditor object haven't found");
    }
}

function createFile(type) {
    if (window["AscDesktopEditor"] && window["AscDesktopEditor"]["LocalFileCreate"]) {
        window["AscDesktopEditor"]["LocalFileCreate"](type);
    } else {
        alert("desktop!!! (open)");
    }
}

document.getElementById('wrap').ondrop = function(e) {
    window["AscDesktopEditor"]["DropOfficeFiles"]();

    e.preventDefault();
    return false;
}

document.getElementById('wrap').ondragover = function (e) {
    e.dataTransfer.dropEffect = 'copy';

    e.preventDefault();
    return false;
};
