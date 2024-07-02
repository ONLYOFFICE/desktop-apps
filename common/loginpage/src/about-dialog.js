window.AboutDialog = function(params) {
    "use strict";

    !params && (params = {});

    let $el, $dialogTitle, $dialogBody;
    const events = {close: params.onclose};

    const _template = `
        <dialog class="dlg dlg-about">
            <div class="title">
                <label class="caption" l10n>${utils.Lang.actAbout}</label>
                <span class="tool close"/>
            </div>
            <div class="body"/>
        </dialog>
    `;

    function onCloseClick(e) {
        close();
    };

    function close(opts) {
        $el.remove();
        if (events.close) {
            events.close(opts);
        }
    }

    function setBody(data) {
        $dialogBody.html(data);
    }

    return {
        setBody: function(data) {
            $dialogBody.html(data);
        },
        show: function () {
            $el = $('#placeholder').append(_template).find('.dlg-about');
            $el.width(450);

            $dialogTitle = $el.find('.title');
            $dialogTitle.find('.tool.close').bind('click', onCloseClick);
            // _set_title( utils.Lang.loginTitleStart );

            $dialogBody = $el.find('.body');

            $el.get(0).showModal();
            $el.addClass('scaled');
            $el.on('close', onCloseClick);
        }
    }
};