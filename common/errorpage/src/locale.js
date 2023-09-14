const langs = {
    en: {
        msgNoConn: 'Internet connection failed...',
        msgNoConnDesc: 'Check connection',
        msgFileNoConnDesc: "You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and re-open the document to continue.",
        msgTemplatesNoConnDesc: "Couldn't load this section because you are experiencing possible network issues. Please check your internet connection and try again.",
    }
}

langs.ru = {
    msgNoConn: 'Страница не доступна...',
    msgNoConnDesc: 'Проверьте соединение',
}


const locale = function() {
    let lang;
    return {
        tr: function(n) {
            return (langs[lang] && langs[lang][n]) || langs.en[n];
        },
        set_lang: function(l) {
            lang = l;
        },
    }
}

window.i18n = new locale;