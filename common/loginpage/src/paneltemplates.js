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
    function FileTemplateModel(attributes) {
        Model.prototype.constructor.call(this);
        Object.assign(this, attributes);

        const skip_file_extension = n => {
            const p = n.search(/\.\w+$/);
            return p < 0 ? n : n.substring(0, p)
        }

        this.name   = skip_file_extension(attributes.name) || 'Template';
        this.descr  = attributes.descr || '';
    };

    FileTemplateModel.prototype = new Model();
    FileTemplateModel.prototype.constructor = FileTemplateModel;

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
        this.emptyPanelContent = `<div id="frame"></div>`;

        const _html = `<div class='action-panel ${args.action}'>
                            <div class='flexbox content-box'>
                                <div class='lr-flex'>
                                    <h3 class='table-caption' l10n>${_lang.actTemplates}</h3>
                                    <div id='idx-nav-templates'>
                                        <a data-value='local' class='nav-item' l10n>${_lang.tplPanelLocal}</a>
                                        <a data-value='cloud' class='nav-item' l10n>${_lang.tplPanelCloud}</a>
                                        <a data-value='test' class='nav-item' l10n>Cloud old</a>
                                    </div>
                                </div>
                                <section class='themed-sroll' panel='local'>
                                    <div class='table-box flex-fill'>
                                        <div id='idx-local-tpls' class='table-templates list'></div>
                                    </div>
                                </section>
                                <section class='themed-sroll' panel='cloud'>
                                    <div class='table-box flex-fill'>
                                        <div id='idx-cloud-tpls' class='table-templates list'></div>
                                    </div>
                                </section>
                                <section panel='test'>${this.emptyPanelContent}</section>
                            </div>
                    </div>`;

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 0;
        args.itemtext = _lang.actTemplates;

        baseView.prototype.constructor.call(this, args);
    };

    ViewTemplates.prototype = Object.create(baseView.prototype);
    ViewTemplates.prototype.constructor = ViewTemplates;

    utils.fn.extend(ViewTemplates.prototype, {
        listitemtemplate: function(info) {
            const type = utils.formatToEditor(info.type);
            const badge = !this.svgicons ? `<i class="badge ${type}"></i>` : 
                                `<svg class="badge"><use xlink:href="#tpltype-${type}"></use></svg>`;
            const icon_el = !info.icon ? `<svg class='icon'><use xlink:href='#template-item'></use></svg>`:
                                            `<div class="box"><img src="${info.icon}">${badge}</div>`
            return `<div id="${info.uid}" class='item'>
                        <div class='icon'>
                            ${icon_el}
                        </div>
                        <span class='title'>${info.name}</span>
                    </div>`;
        }
    });

    utils.fn.extend(ControllerTemplates.prototype, (function() {
        let iframe;
        let panel_local,
            panel_cloud;
        let local_templates,
            cloud_templates;
        let _page_num = 0, 
            isLoading = false, 
            totalPages = null;
    
        // TODO: for tests only. uncomment static url before release
        let test_url = localStorage.templatesdomain ? localStorage.templatesdomain : 'https://templates.onlyoffice.com';
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

        const _on_nav_item_click = function(e) {
            $('.nav-item', this.view.$panel).removeClass('selected');
            const $item = $(e.target);
            $item.addClass('selected');

            const panel = $item.data('value');
            $('[panel]', this.view.$panel).hide();
            $(`[panel=${panel}]`, this.view.$panel).show();

            this.view.$panel.removeClass('local cloud test').addClass($item.data('value'));
        }


        function _init_collection() {
            local_templates = new Collection({
                view: $('section[panel=local]', this.view.$panel),
                list: $('#idx-local-tpls', this.view.$panel),
            });

            local_templates.events.erased.attach(collection => {
                collection.list.parent().addClass('empty');
            });

            local_templates.events.inserted.attach((collection, model) => {
                let $item = this.view.listitemtemplate(model);

                collection.list.append($item);
                collection.list.parent().removeClass('empty');
            });

            local_templates.events.click.attach((collection, model) => {
                sdk.command('create:new', JSON.stringify({'template': {id:model.id, type:model.type, path: model.path}}));
            });

            local_templates.events.contextmenu.attach(function(collection, model, e){
                // ppmenu.actionlist = 'recent';
                // ppmenu.hideItem('files:explore', (!model.islocal && !model.dir) || !model.exist);
                // ppmenu.show({left: e.clientX, top: e.clientY}, model);
            });

            local_templates.events.changed.attach(function(collection, model){
                // let $el = collection.list.find('#' + model.uid);
                // if ( $el ) $el[model.exist ? 'removeClass' : 'addClass']('unavail');
            });

            cloud_templates = new Collection({
                view: $('section[panel=local]', this.view.$panel),
                list: $('#idx-cloud-tpls', this.view.$panel),
            });

            cloud_templates.events.erased.attach(collection => {
                collection.list.parent().addClass('empty');
            });

            cloud_templates.events.inserted.attach((collection, model) => {
                let $item = this.view.listitemtemplate(model);

                collection.list.append($item);
                collection.list.parent().removeClass('empty');
            });

            cloud_templates.events.click.attach((collection, model) => {
                window.sdk.openTemplate(model.path, model.name);
            });

            local_templates.empty();
            cloud_templates.empty();
        }

        const _on_update_template = function(index) {

        }

        const _on_add_local_templates = function(tmpls) {
            local_templates.empty();
            for (let item of tmpls) {
                var model = new FileTemplateModel(item);

                local_templates.add(model);
            }
        }

        const _on_add_cloud_templates = function(data) {
            let c = 0,
                icon_url;
            for (let i of data) {
                const info = i['attributes']
                const file_ext = info['form_exts']['data'][0]['attributes']['ext'];
                icon_url =  info.template_image?.data?.attributes?.formats?.xxsmall?.url ||
                info.template_image?.data?.attributes?.formats?.thumbnail?.url;
                const item = {
                    uid: i.id,
                    name: info['name_form'],
                    descr: info['template_desc'],
                    path: info.file_oform?.data[0].attributes.url,
                    type: utils.fileExtensionToFileFormat(file_ext),
                    icon: icon_url,
                };
                const model = new FileTemplateModel(item);
                cloud_templates.add(model);

                console.log(++c, 'model', i.id, model.name);
            }
        }

        const loadTemplates = function() {
            if (isLoading || (totalPages !== null && _page_num >= totalPages)) return;
    
            _page_num++;  
            isLoading = true;
    
            const _url = `https://oforms.teamlab.info/dashboard/api/oforms?populate=*&locale=en&pagination[page]=${_page_num}`;
            // const _url = `https://oforms.onlyoffice.com/dashboard/api/oforms?populate=*&locale=en&pagination[page]=${_page_num}`;
            fetch(_url)
                .then(r => r.json())
                .then(d => {
                    if (!!d.data) {
                        _on_add_cloud_templates(d.data);
    
                        if (totalPages === null) {
                            totalPages = d.meta.pagination.pageCount;
                        }
                    }
                    isLoading = false;
                })
        };
    
        const Scroll = function() {
            const $cloudPanel = $('section[panel=cloud]', this.view.$panel);
            const scrollTop = $cloudPanel.scrollTop();
            const scrollHeight = $cloudPanel[0].scrollHeight;
            const clientHeight = $cloudPanel[0].clientHeight;
    
            if (scrollTop + clientHeight >= scrollHeight - 100) {
                loadTemplates();
            }
        };

        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);
                this.view.render();
                errorBox.render({
                    parent: $('#frame')[0],
                    lang: utils.Lang.id,
                    page: 'templates',
                });

                this.view.$panel.addClass('local');
                $('.nav-item[data-value=local]', this.view.$panel).addClass('selected');
                $('[panel]', this.view.$panel).hide();
                $(`[panel=local]`, this.view.$panel).show();

                if ( window.utils.isWinXp ) {
                    $('#idx-nav-templates', this.view.$panel).hide();
                    this.view.$panel.addClass('win_xp');
                } else {
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
                }

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

                const mq = "screen and (-webkit-min-device-pixel-ratio: 1.01) and (-webkit-max-device-pixel-ratio: 1.99), " +
                                            "screen and (min-resolution: 1.01dppx) and (max-resolution: 1.99dppx)";

                const mql = window.matchMedia(mq);
                this.view.svgicons = !mql.matches;
                mql.addEventListener('change', e => {
                    this.view.svgicons = !e.target.matches;
                });

                $('.nav-item', this.view.$panel).click(_on_nav_item_click.bind(this));
                window.sdk.on('onupdatetemplate', _on_update_template.bind(this));
                window.sdk.on('onaddtemplates', _on_add_local_templates.bind(this));

                _init_collection.call(this);

                const _reload_templates = l => {
                    let ls = [l];
                    if (utils.Lang.id.length > 2) {
                        ls.push(l[3] == '_' ? l.replaceAll('_', '-') : l.replaceAll('_', '-'), l.substring(0,2));
                    }
                    ls.push("en-US","en_US","en");
                    window.sdk.LocalFileTemplates(ls);
                };
                _reload_templates(utils.Lang.id);

                CommonEvents.on('lang:changed', (ol, nl) => {
                    _reload_templates(nl);
                });

                loadTemplates();
                
                $('section[panel=cloud]', this.view.$panel).on('scroll', Scroll.bind(this));

                return this;
            }
        };
    })());
}();