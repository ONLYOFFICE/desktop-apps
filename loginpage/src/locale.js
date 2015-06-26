/*
 * (c) Copyright Ascensio System SIA 2010-2016
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

var en = {
    '#auth-wellcome':'Login to ONLYOFFICE',
    '#auth-error':'Login failed. Please, check the entered data and try again.',
    '#auth-portal:placeholder':'portal',
    '#auth-email:placeholder':'email',
    '#auth-pass:placeholder':'password',
    '#label-pass-forgot':'Forgot password?',
    '#link-restore':'Restore',
    '#btn-login':'Login',
    '#text-welcome':'Welcome to ONLYOFFICE Desktop!',
    '#text-description':'A new fast solution for work with documents using your ONLYOFFICE'
}

var ru = {
    '#auth-wellcome':'Войти в ONLYOFFICE',
    '#auth-error':'Ошибка авторизации. Пожалуйста, проверьте введенные данные.',
    '#auth-portal:placeholder':'Портал',
    '#auth-email:placeholder':'Е-майл',
    '#auth-pass:placeholder':'Пароль',
    '#label-pass-forgot':'Забыли пароль?',
    '#link-restore':'Восстановить',
    '#btn-login':'Войти',
    '#text-welcome':'Добро пожаловать в ONLYOFFICE Desktop!',
    '#text-description':'Новое быстрое решение для работы с документами на вашем портале ONLYOFFICE'
}

var de = {
    '#auth-wellcome':'Melden Sie sich bei ONLYOFFICE and',
    '#auth-error':'Anmeldung fehlgeschlagen. Überprüfen Sie die eingegebenen Daten und versuchen Sie es erneut.',
    '#auth-portal:placeholder':'Portal',
    '#auth-email:placeholder':'E-Mail',
    '#auth-pass:placeholder':'Kennwort',
    '#label-pass-forgot':'Kennwort vergessen?',
    '#link-restore':'Wiederherstellen',
    '#btn-login':'Anmelden',
    '#text-welcome':'Willkommen bei !ONLYOFFICE Desktop!',
    '#text-description':'Die neue schnelle Lösung für die Bearbeitung der Dokumente mithilfe von Ihrem ONLYOFFICE'
}

function getUrlParams() {
    var e,
    a = /\+/g,  
    r = /([^&=]+)=?([^&]*)/g,
    d = function (s) { return decodeURIComponent(s.replace(a, " ")); },
    q = window.location.search.substring(1),
    urlParams = {};

    while (e = r.exec(q))
        urlParams[d(e[1])] = d(e[2]);

    return urlParams;
}

(function applyLocale(lang) {
    var lobj = window[lang];
    if (!!lang && lang.length && !!lobj) {
        var elem, res, s;
        for (var prop in lobj) {
            s = lobj[prop];            
            res = /([^\:]+)\:?(\w+)?$/.exec(prop);
            if (res) {
                elem = $(res[1]);

                if (elem.length) {
                    if (res[2] && res[2].length) {
                        elem.attr(res[2], s);
                    } else {
                        // if (elem.is('button')) 
                            elem.html(s);
                    }
                }
            }
        }
    } 
})(getUrlParams()['lang']);
