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

const sdk = window.AscDesktopEditor;
if ( sdk ) {
    sdk.execCommand('error:page', 'open');
}

const nativevars = window.RendererProcessVariable;
let lang, theme, page;

if ( nativevars ) {
    lang = nativevars.lang;
    theme = nativevars?.theme?.type;
}

const params = getUrlParams();
!theme && (theme = params['uitheme'] || 'light');
page = params['page'];

errorBox.render({
    page: params['page'],
    lang: lang || params["lang"],
});

if ( theme == 'dark' ) {
    document.body.classList.add('theme-type-dark')
}
