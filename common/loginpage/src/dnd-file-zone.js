window.DnDFileZone = function () {
	"use strict";

	let $el;

	//language=HTML
	const _template = `
        <div class="dnd-zone">
            <div class="content">
                <p class="text-normal" l10n>${utils.Lang.labelDropFile}</p>
                <button class="btn btn--primary" l10n>${utils.Lang.labelSelectFile}</button>
            </div>
        </div>`;


	return {
		render: function (parentElement) {
			$el = parentElement.append(_template).find('.drag-and-drop-zone');

			const $openLink = $el.find('a');

			$openLink.bind('click', function () {
				openFile(OPEN_FILE_FOLDER, '');
			});
		},
		detach: function () {
			$el.remove();
		},
		hide: function () {
			$el.hide()
		},
		show: function () {
			$el.show()
		}
	}
};