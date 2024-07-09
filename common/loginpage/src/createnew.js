+function(){
    'use strict'

    const panelCreateNew = function(args={}) {
        let $html;
        const _panel_tmpl = `<div class="box">
                                <ul id="idx-list-create-blank" class="list-create-doc">
                                </ul>
                                <div class="separator"></div>
                                <ul id="idx-list-create-from-template" class="list-create-doc">
                                </ul>
                            </div>`;

        const _item_tmpl = info => `<li class="create-new-item" data-editor="${info.editor}">
                                        <a class="item-wrap">
                                            <svg class="icon icon-newdoc">
                                                <use href="#${info.icon}"></use>
                                            </svg>
                                            <span l10n class="">${info.title}</span>
                                        </a>
                                    </li>`;
        const _arr_new_doc = [{
                            editor: 'word',
                            icon:'newword',
                            title: utils.Lang.newDoc,
                        },{
                            editor: 'cell',
                            icon:'newcell',
                            title: utils.Lang.newXlsx,
                        },{
                            editor: 'slide',
                            icon:'newslide',
                            title: utils.Lang.newPptx,
                        },{
                            editor: 'form',
                            icon:'newpdf',
                            title: utils.Lang.newForm,
                        // },{
                        //     editor: 'pdfopen',
                        //     icon:'openpdf',
                        //     title: 'Open PDF'
                        }];

        const _render = function(p) {
            const $parent = $(p);
            if ( $parent.length ) {
                $html = $(_panel_tmpl);
                const $box_new = $html.find('#idx-list-create-blank');

                _arr_new_doc.forEach(item => {
                    const i = _item_tmpl(item);

                    $box_new.append(i);
                });

                $parent.html($html);

                $html.on('click', '.item-wrap', _on_click_create_new);
            }
        }

        const _on_click_create_new = e => {
            const t = $(e.currentTarget).parent().data('editor');
            if ( !!t ) window.sdk.command("create:new", t);
        }

        const _filter_by_editor = function (editor) {
            if ( $html ) {
                if ( editor ) {
                    $html.find(`.create-new-item:not([data-editor=${editor}])`).hide();
                    $html.find(`.create-new-item[data-editor=${editor}]`).show();
                } else {
                    $html.find(`.create-new-item`).show();
                }
            }
        }

        return {
            render: _render,
            filter: _filter_by_editor,
            loadtemplates: function(array) {}
        }
    }

    window.PanelCreateNew = panelCreateNew;
}();
