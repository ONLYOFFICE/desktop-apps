window.DragAndDropFileZone = function() {
    "use strict";

    let $el, $title;

    const _template = `
      <div class="drag-and-drop-zone">
        <h1 class="text-headline-1" l10n>${utils.Lang.areaOpenFile}</h1>
        <div class="zone">
            <svg class="icon">
                <use xlink:href="#box-open"></use>
            </svg>
            <p class="text-body text-secondary" l10n>${utils.Lang.labelDropFile}</p>
            <a class="text-body" l10n>${utils.Lang.labelSelectFile}</a>
        </div>
      </div>
    `;


    return {
        render: function(parentElement) {
            $el = parentElement.append(_template).find('.drag-and-drop-zone');
            $title = $el.find('h1');

            const $openLink = $el.find('a');
            $openLink.bind('click', function() {
                openFile(OPEN_FILE_FOLDER, '');
            });
        },
        detach: function() {
            $el.remove();
        },
        hide: function() {
            $el.hide()
        },
        show: function() {
            $el.show()
        },
        hideTitle: function() {
            $title.hide()
            $el.css('padding-top', '16px');
        },
        showTitle: function() {
            $title.show();
            $el.css('padding-top', '');
        }
    }
};