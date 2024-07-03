window.DragAndDropFileZone = function() {
    "use strict";

    let $el;

    const _template = `
      <div class="drag-and-drop-zone">
        <svg class="icon">
            <use xlink:href="#box-open"></use>
        </svg>
        <p class="text-body text-secondary">${utils.Lang.listRecentDropZone}</p>
        <a class="text-body">${utils.Lang.menuFileSelect}</a>
      </div>
    `;


    return {
        render: function(parentElement) {
            $el = parentElement.append(_template).find('.drag-and-drop-zone');
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
        }
    }
};