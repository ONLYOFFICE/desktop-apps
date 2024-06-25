{

!window.l10n && (window.l10n = {boxNoConnect:{}});
!window.l10n.boxNoConnect && (window.l10n.boxNoConnect = {});

const langs = {
    en: {
        msgNoConn: 'Internet connection failed...',
        msgNoConnDesc: 'Check connection',
        msgFileNoConn: 'Check your Internet connection',
        msgFileNoConnDesc: "You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and reopen the document to continue.",
        msgTemplatesNoConn: 'Check your Internet connection',
        msgTemplatesNoConnDesc: "Couldn't load this section because you are experiencing possible network issues. Please check your Internet connection and try again.",
        msgFileError: 'Oops! Something went wrong',
        msgFileErrorDesc: "We lost access to your file due to a lack of memory or some other reason. Please don't worry and try reopening the file. Close this tab to continue.",
    }
}

const error_box = function() {
    let _page;
    const _tr = (n, l) => {
        return (l10n.boxNoConnect[l] && l10n.boxNoConnect[l][n]) || langs.en[n];
    }

    const page_config = {
        "def": ["msgNoConn", "msgNoConnDesc"],
        "cloudfile": ["msgFileNoConn", "msgFileNoConnDesc"],
        "templates": ["msgTemplatesNoConn", "msgTemplatesNoConnDesc"],
        "fileerr": ["msgFileError", "msgFileErrorDesc", "something_wrong"],
    };

    const _fix_lang = l => {
        if ( l && l10n && l10n.boxNoConnect ) {
            if ( l10n.boxNoConnect[l])
                return l;

            l = l.split(/[\-\_]/)[0];
            if ( l10n.boxNoConnect[l] )
                return l;
        }

        return 'en';
    }

    return {
        render: function(args = {}) {
            _page = args.page || 'def';
            const svg_id = page_config[_page] && page_config[_page][2] ? page_config[_page][2] : "connection_error";

            const html_ = `
                <section class="box-connection-error center">
                    <svg class="icon">
                        <use href="#${svg_id}"></use>
                    </svg>
                    <label id="idx-msg-short" class="description description__short"></label>
                    <label id="idx-msg-long" class="description description__long"></label>
                </section>`;

            if ( !args.parent ) args.parent = document.body;
            args.parent.insertAdjacentHTML('beforeend', html_);

            this.translate(args.lang);
        },
        translate: function(lang) {
            const page = !page_config[_page] ? 'def' : _page;
            lang = _fix_lang(lang);

            const ms = document.getElementById("idx-msg-short");
            if ( ms ) ms.innerText = _tr(page_config[page][0], lang);

            const ml = document.getElementById("idx-msg-long");
            if ( ml ) {
                ml.innerText = _tr(page_config[page][1], lang);

                // if ( _page == 'file' ) {
                //     ml.innerText = _tr("msgFileNoConnDesc", lang);
                // } else
                // if ( _page == 'templates') {
                //     ml.innerText = _tr("msgTemplatesNoConnDesc", lang);
                // } else ml.innerText = _tr("msgNoConnDesc", lang);
            }
        },
    }
}

window.errorBox = new error_box;
}

