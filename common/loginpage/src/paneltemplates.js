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

    var ViewTemplates = function(args) {
        var _lang = utils.Lang;

        const msg = 'Oops! Something went wrong :(<br>Check internet connection';
        this.emptyPanelContent = `<div id="frame"></div>`;

        const _html = `<div class='action-panel ${args.action}'>
                            <div class='flexbox content-box'>
                                <div class='lr-flex'>
                                    <h3 class='table-caption' l10n>${_lang.actTemplates}</h3>
                                    <div class="search-container">
                                        <div class="icon-box">
                                            <svg class="icon search-icon" data-iconname="search" data-precls="tool-icon">
                                                <use href="#search"></use>
                                            </svg>
                                        </div>
                                        <input type="text" id="template-search" placeholder="${_lang.tplSearch}">
                                        <span class="tool close" id="template-clear" style="display: none;"></span>
                                    </div>
                                </div>
                                <div id='idx-nav-templates'>
                                    <a data-value='Documents' class='nav-item selected' l10n>${_lang.tplDocument}</a>
                                    <a data-value='Spreadsheets' class='nav-item' l10n>${_lang.tplSpreadsheet}</a>
                                    <a data-value='Presentations' class='nav-item' l10n>${_lang.tplPresentation}</a>
                                    <a data-value='PDFs' class='nav-item' l10n>${_lang.tplPDF}</a>
                                </div>
                                <div id="search-result" class="search-result" style="display: none;"></div>
                                    <div id="search-no-results" class="search-no-results" style="display: none;">
                                    <div class="icon-box">
                                        <svg class="icon nothing-found-light-icon" data-iconname="nothing-found-light" data-precls="tool-icon">
                                            <use id="idx-nothing-found-light" href="#nothing-found-light"></use>
                                            <use id="idx-nothing-found-dark" href="#nothing-found-dark"></use>
                                        </svg>
                                    </div>
                                    <p class="no-results-title">${_lang.tplNoResultsTitle}</p>
                                    <p class="no-results-text">${_lang.tplNoResultsText}</p> 
                                </div>
                                <section class='themed-sroll' panel='all'>
                                    <div class='table-box flex-fill'>
                                        <div class='table-templates list' id='idx-templates'></div>
                                    </div>
                                </section>
                            </div>
                        </div>`;

        args.tplPage = _html;
        args.menu = '.main-column.tool-menu';
        args.field = '.main-column.col-center';
        args.itemindex = 0;
        // args.itemtext = _lang.actTemplates;
        args.tplItem = 'nomenuitem';

        baseView.prototype.constructor.call(this, args);
    };

    ViewTemplates.prototype = Object.create(baseView.prototype);
    ViewTemplates.prototype.constructor = ViewTemplates;

    utils.fn.extend(ViewTemplates.prototype, {
        listitemtemplate: function(info) {
            const type = utils.formatToEditor(info.type);
            const badge = `<i class="badge ${type}"></i>`;
            const cloudIcon = info.isCloud ? `<svg class="icon cloud-icon" data-iconname="location-cloud" data-precls="tool-icon">
                                                <use href="#location-cloud"></use>
                                              </svg>` : 
                                              `<svg class="icon cloud-icon" data-iconname="location-local" data-precls="tool-icon">
                                                <use href="#location-local"></use>
                                              </svg>`;                    

            const icon_el = !info.icon ? `<svg class='icon icon--default'><use xlink:href='#template-item'></use></svg>
                                            <div class="box"><img>${badge}</div>` :
                                            `<div class="box"><img src="${info.icon}">${badge}</div>`;

            return `<div id="${info.uid}" class='item' data-type="${type}">
                        <div class="wrapper">
                            ${icon_el}
                        </div>
                        <div class="card">
                            <div class="badge-wrapper">${badge}</div>
                            <div class="title">${info.name}</div>
                            ${cloudIcon}
                        </div>
                    </div>`;
        }
    });

    utils.fn.extend(ControllerTemplates.prototype, (function() {
        let _page_num = 0, 
            isLoading = false, 
            totalPages = null; 

        const _on_nav_item_click = function(e) {
            $('.nav-item', this.view.$panel).removeClass('selected');
            const $item = $(e.target);
            $item.addClass('selected');

            const panel = $item.data('value');

            this.view.$panel.removeClass('Documents Spreadsheets Presentations PDFs').addClass(panel);

            applyFilter(this.view.$panel);
            $('.themed-sroll', this.view.$panel).scrollTop(0);
        };


        function _init_collection() {
           this.templates = new Collection({
                view: $('section[panel=all]', this.view.$panel),
                list: $('#idx-templates', this.view.$panel),
            });

            const setupCollection = (collection) => {
                collection.events.erased.attach(() => {
                    collection.list.parent().addClass('empty');
                });

                collection.events.inserted.attach((col, model) => {
                    let $item, isprepend = false;
                    if ( model instanceof Array ) {
                        const items = model;
                        $item = [], isprepend = !items[0].isCloud;
                        items.forEach(m => {
                            $item.push($(this.view.listitemtemplate(m)));
                        });
                    } else {
                        $item = $(this.view.listitemtemplate(model));
                    }

                    if ( isprepend === true ) {
                        col.list.prepend($item); 
                    } else {
                        col.list.append($item);
                    }
                    col.list.parent().removeClass('empty');
                    applyFilter(this.view.$panel);
                });

                collection.events.reset.attach((col, models) => {
                    let elms = [];
                    models.forEach(m => {
                        const $item = $(this.view.listitemtemplate(m));
                        elms.push($item);
                    });

                    if ( elms.length ) {
                        col.list.prepend(elms);
                        col.list.parent().removeClass('empty');

                        applyFilter(this.view.$panel);
                    }
                });

                collection.events.click.attach((col, model) => {
                    if (model.isCloud) {
                        window.sdk.openTemplate(model.path, model.name);
                    } else {
                        sdk.command('create:new', JSON.stringify({
                            template: {
                                id: model.id,
                                type: model.type,
                                path: model.path
                            }
                        }));
                    }
                });

                collection.events.changed.attach((collection, m, v) => {
                    if ( v && v.icon ) {
                        const $el = $(`#${m.uid}`, collection.list);
                        $el.find('svg.icon--default').hide();
                        $el.find('.box img').attr('src', m.icon);
                    }
                });
            };
            setupCollection(this.templates);
        }

        Collection.prototype.emptyLocal = function() {
            // const cloudItems = this.items.filter(item => item.isCloud);
            // this.empty();
            // cloudItems.forEach(item => {
            //     this.add(item);
            // });

            this.items.forEach(m => {
                    if ( !m.isCloud ) {
                        const el = document.getElementById(m.uid);
                        if ( el ) el.remove();
                    }
                });

            const cloud_items = this.items.filter(item => item.isCloud);
            this.items = cloud_items;
        };

        const _on_add_local_templates = function(tmpls) {
            const _func_ = () => {
                // this.templates.emptyLocal();

                let items = [];
                // [...tmpls]
                    // .reverse()
                (this.tmpls || tmpls)
                    .forEach(item => {
                        const type = utils.formatToEditor(item.type);
                        if (['word', 'cell', 'slide', 'pdf'].includes(type)) {
                            // this.templates.add(new FileTemplateModel(item));

                            const m = this.templates.find('path', item.path);
                            if ( !m ) {
                                items.push(new FileTemplateModel(item));
                            } else {
                                if ( !m.icon && item.icon ) {
                                    m.set('icon', item.icon);
                                }
                            }
                        }
                    });

                if ( items.length )
                    this.templates.add(items);
            };

            // if ( this.timer_id )
            //     this.tmpls = tmpls;
            // else {
            //     this.timer_id = setTimeout(e => {
            //         this.timer_id = undefined;
            //         _func_();

            //         delete this.tmpls;
            //     }, 2000);

                _func_();
            // }
        };

        const _on_add_cloud_templates = function(data) {
            let items = [];
            data.forEach(i => {
                const info = i['attributes'];
                if ( !info['form_exts']['data'].length ) return;

                const file_ext = info['form_exts']['data'][0]['attributes']['ext'],
                    id = i.id;
                if (!this.templates.items.some(t => t.uid === id)) {

                    const m = new FileTemplateModel({
                        uid: id,
                        name: info['name_form'],
                        descr: info['template_desc'],
                        path: info.file_oform ? info.file_oform.data[0].attributes.url : undefined,
                        type: utils.fileExtensionToFileFormat(file_ext),
                        icon: info.template_image ? info.template_image.data.attributes.formats.thumbnail.url : undefined,
                        isCloud: true,
                    });

                    items.push(m);
                    // this.templates.add(m);
                }
            });

            if ( items.length )
                this.templates.add(items);
        }
        
        const applyFilter = function($panel) {
            const selectedType = {
                'Documents': 'word',
                'Spreadsheets': 'cell',
                'Presentations': 'slide',
                'PDFs': 'pdf'
            }[$('.nav-item.selected', $panel).data('value')];

            const search = $('#template-search').val() || '';
            const searchСomparison = search.toLowerCase();
            let matchCount = 0;

            $('.table-templates.list .item', $panel).each(function () {
                const $item = $(this);
                const type = $item.data('type');
                const title = $item.find('.title').text().toLowerCase();

                const isMatch = searchСomparison ? title.includes(searchСomparison) : type === selectedType;

                $item.toggle(isMatch);
                if (isMatch) matchCount++;
            });

            $('#search-result', $panel).toggle(!!searchСomparison).text(`${utils.Lang.tplSearchResult} "${search}"`);
            $('#idx-nav-templates', $panel).toggle(!searchСomparison);
            $('#search-no-results', $panel).toggle(matchCount === 0);
        };
        
        const _loadTemplates = function(language) {
            const Langs = ['en', 'fr', 'de', 'es', 'pt', 'it', 'ja', 'zh', 'ar'];
            const shortLang = language ? language.split('_')[0].toLowerCase() : 'en';
            const locale = Langs.includes(shortLang) ? shortLang : 'en';
            if (isLoading || (totalPages !== null && _page_num >= totalPages)) return;

            _page_num++;  
            isLoading = true;

            const _domain = localStorage.templatesdomain ? localStorage.templatesdomain : 'https://oforms.onlyoffice.com'; // https://oforms.teamlab.info
            const _url = `${_domain}/dashboard/api/oforms?populate=*&locale=${locale}&pagination[page]=${_page_num}`;
            fetch(_url)
                .then(r => r.json())
                .then(d => {
                    isLoading = false;
                    if (d.data) {
                        _on_add_cloud_templates.call(this, d.data);
                        totalPages = d.meta.pagination.pageCount;
                        
                        if (_page_num < totalPages) {
                            _loadTemplates.call(this, language);
                        } 
                     }
                })
        };

        const loadAllPages = function() {
            if (isLoading) return;

            _page_num = 0;
            const self = this;
            const loadNext = () => {
                if (_page_num < totalPages || totalPages === null) {
                    _loadTemplates.call(self,  utils.Lang.id);
                    setTimeout(loadNext, 150); 
                }
            };

            loadNext(); 
        };

        const _resetPagination = function() {
            isLoading = false;
            totalPages = null;
            _page_num = 0;
            this.templates.empty();
        };
    
        return {
            init: function() {
                baseController.prototype.init.apply(this, arguments);
                this.view.render();

                _init_collection.call(this);

                if ( window.utils.isWinXp ) {
                    $('#idx-nav-templates', this.view.$panel).hide();
                } 

                const mq = "screen and (-webkit-min-device-pixel-ratio: 1.01) and (-webkit-max-device-pixel-ratio: 1.99), " +
                                            "screen and (min-resolution: 1.01dppx) and (max-resolution: 1.99dppx)";

                const mql = window.matchMedia(mq);
                this.view.svgicons = !mql.matches;
                mql.addEventListener('change', e => {
                    this.view.svgicons = !e.target.matches;
                });

                $('.nav-item', this.view.$panel).click(_on_nav_item_click.bind(this));
                _on_nav_item_click.call(this, { target: $('.nav-item.selected', this.view.$panel) });
                $('#template-search', this.view.$panel).on('input', () => {
                    applyFilter(this.view.$panel);
                    $('#template-clear', this.view.$panel).toggle($('#template-search', this.view.$panel).val().length > 0);
                });

                $('#template-clear', this.view.$panel).on('click', () => {
                    $('#template-search', this.view.$panel).val('').trigger('input');
                });
            
                window.sdk.on('onaddtemplates', _on_add_local_templates.bind(this));

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
                    _resetPagination.call(this);  
                    _reload_templates(nl);
                    _loadTemplates.call(this, nl);
                });

                loadAllPages.call(this);

                return this;
            }
        };
    })());
}();