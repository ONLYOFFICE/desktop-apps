
function checkScaling() {
    const matches = {
        'pixel-ratio__1_25': `screen and (-webkit-min-device-pixel-ratio: 1.25) and (-webkit-max-device-pixel-ratio: 1.49),
                                screen and (min-resolution: 1.25dppx) and (max-resolution: 1.49dppx)`,
        'pixel-ratio__1_5': `screen and (-webkit-min-device-pixel-ratio: 1.5) and (-webkit-max-device-pixel-ratio: 1.74),
                                screen and (min-resolution: 1.5dppx) and (max-resolution: 1.74dppx)`,
        'pixel-ratio__1_75': `screen and (-webkit-min-device-pixel-ratio: 1.75) and (-webkit-max-device-pixel-ratio: 1.99),
                                screen and (min-resolution: 1.75dppx) and (max-resolution: 1.99dppx)`,
        'pixel-ratio__2': `screen and (-webkit-min-device-pixel-ratio: 2), screen and (min-resolution: 2dppx), screen and (min-resolution: 192dpi)`
    };

    for (var c in matches) {
        const media_query_list = window.matchMedia(matches[c]);
        media_query_list.addEventListener('change', function(cls, e) {
            if ( e.matches ) {
                document.body.className = document.body.className.replace(/pixel-ratio__[\d_]+/gi, '').trim();
                document.body.classList.add(cls);
            }
        }.bind(this, c));

        if ( media_query_list.matches ) {
            document.body.classList.add(c);
            // break;
        }
    }
}

checkScaling();

var params = (function() {
    var e,
        a = /\+/g,  // Regex for replacing addition symbol with a space
        r = /([^&=]+)=?([^&]*)/g,
        d = function (s) { return decodeURIComponent(s.replace(a, " ")); },
        q = window.location.search.substring(1),
        urlParams = {};

    while (e = r.exec(q))
        urlParams[d(e[1])] = d(e[2]);

    return urlParams;
})();

var ui_theme_name = params.uitheme || localStorage.getItem("ui-theme");
if ( !!ui_theme_name ) {
    if ( /^{".+"}$/.test(ui_theme_name) )
        ui_theme_name = /id\":\"([\w\d_-]+)/.exec(ui_theme_name)[1];

    const theme_type = ui_theme_name == 'theme-dark' ? 'theme-type-dark' : 'theme-type-light';
    document.body.classList.add(ui_theme_name, theme_type);
}
