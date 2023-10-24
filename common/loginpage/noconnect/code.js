{

!window.l10n && (window.l10n = {boxNoConnect:{}});
!window.l10n.boxNoConnect && (window.l10n.boxNoConnect = {});

const langs = {
    en: {
        msgNoConn: 'Internet connection failed...',
        msgNoConnDesc: 'Check connection',
        msgFileNoConnDesc: "You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and re-open the document to continue.",
        msgTemplatesNoConnDesc: "Couldn't load this section because you are experiencing possible network issues. Please check your internet connection and try again.",
    }
}

const error_box = function() {
    let _page;
    const _tr = (n, l) => {
        return (l10n.boxNoConnect[l] && l10n.boxNoConnect[l][n]) || langs.en[n];
    }

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
            const html_ = `
                <section class="box-connection-error center">
                    <svg class="icon">
                        <use href="#connection_error"></use>
                    </svg>
                    <label id="idx-msg-short" class="description description__short"></label>
                    <label id="idx-msg-long" class="description description__long"></label>
                </section>`;

            if ( !args.parent ) args.parent = document.body;
            args.parent.insertAdjacentHTML('beforeend', html_);

            _page = args.page;
            this.translate(args.lang);
        },
        translate: function(lang) {
            lang = _fix_lang(lang);

            const ms = document.getElementById("idx-msg-short");
            if ( ms ) ms.innerText = _tr("msgNoConn", lang);

            const ml = document.getElementById("idx-msg-long");
            if ( ml ) {
                if ( _page == 'file' )
                    ml.innerText = _tr("msgFileNoConnDesc", lang);
                else
                if ( _page == 'templates')
                    ml.innerText = _tr("msgTemplatesNoConnDesc", lang);
                else ml.innerText = _tr("msgNoConnDesc", lang);
            }
        },
    }
}

window.errorBox = new error_box;
}

