window.AboutDialog = function(params) {
    "use strict";

    !params && (params = {});

    let $el, $dialogTitle, $dialogBody, pendingBody;
    const events = {close: params.onclose};

    const _template = `
        <dialog class="dlg dlg-about">
            <div class="title">
                <label class="text-headline" l10n>${utils.Lang.actAbout}</label>
                <span class="tool close"/>
            </div>
            <div class="body"/>
        </dialog>
    `;

    function onCloseClick(e) {
        close();
    };

    function close(opts) {
        $el.get(0).close(); 
        if (events.close) {
            events.close(opts);
        }
    }


    return {
        setBody: function(data) {
            if ($dialogBody) {
                $dialogBody.html(data);
            } else {
                pendingBody = data;
            }
        },
        show: function () {
            if ($el && $el.length) {
                const dlg = $el.get(0);
                if (!dlg.open) dlg.showModal(); 
                return;
            }

            $el = $('#placeholder').append(_template).find('.dlg-about');
            $el.width(576);
           

            $dialogTitle = $el.find('.title');
            $dialogTitle.find('.tool.close').bind('click', onCloseClick);
            $dialogTitle.find('.text-headline').text(utils.Lang.actAbout);
          
            $dialogBody = $el.find('.body');
            if (pendingBody) {
                $dialogBody.html(pendingBody);
                pendingBody = null;
            }
            $dialogBody.find('#idx-about-version span[l10n]').text(utils.Lang.strVersion); 

            $el.get(0).showModal();
            $el.addClass('scaled');
            $el.on('close', onCloseClick);

            $(document).on('click', function(e) {
                if (e.target === $el.get(0)) {
                    close();
                }
            });
        }
    }
};