const locale = {
    en: {
        msgNoConn: 'Internet connection failed...',
        msgNoConnDesc: 'Check connection',
        msgFileNoConnDesc: "You are unable to edit the document because the Internet connection is lost or restricted. Please check your connection and re-open the document to continue.",
        msgTemplatesNoConnDesc: "Couldn't load this section because you are experiencing possible network issues. Please check your internet connection and try again.",
    }
}

locale.ru = {
    msgNoConn: 'Страница не доступна...',
    msgNoConnDesc: 'Проверьте соединение',
}

locale.tr = n => locale[locale.lang][n] || locale.en[n];
window.i18n = locale;