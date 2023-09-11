function getUrlParams() {
    let e;
    const a = /\+/g,  // Regex for replacing addition symbol with a space
        r = /([^&=]+)=?([^&]*)/g,
        d = s => decodeURIComponent(s.replace(a, " ")),
        q = window.location.search.substring(1),
        urlParams = {};

    while (e = r.exec(q))
        urlParams[d(e[1])] = d(e[2]);

    return urlParams;
}

const nativevars = window.RendererProcessVariable;
let lang, theme, page;

if ( nativevars ) {
    lang = nativevars.lang;
    theme = nativevars?.theme?.type;
}

const params = getUrlParams();
window.i18n.lang = lang || (params["lang"] || 'en').split(/[\-\_]/)[0];
!theme && (theme = params['uitheme'] || 'light');
page = params['page'];

if ( theme == 'dark' ) {
    document.body.classList.add('theme-type-dark')
}

const ms = document.getElementById("idx-msg-short");
if ( ms ) ms.innerText = window.i18n.tr("msgNoConn");

const ml = document.getElementById("idx-msg-long");
if ( ml ) {
    if ( page == 'file' )
        ml.innerText = window.i18n.tr("msgFileNoConnDesc");
    else
    if ( page == 'templates')
        ml.innerText = window.i18n.tr("msgTemplatesNoConnDesc");
    else ml.innerText = window.i18n.tr("msgNoConnDesc");
}
