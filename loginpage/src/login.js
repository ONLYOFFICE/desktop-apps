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


$(document).ready(function() {
    $('#auth-error').hide();

    var protocol = 'https://',
        startmodule = '/products/files/?desktop=true';
    var portal = undefined,
        email = undefined;

    $('#btn-login').click(function() {
        portal = document.getElementById('auth-portal').value;
        email = document.getElementById('auth-email').value;

        var re_wrong_symb = /[\s\\]/;
        if (!portal.length || re_wrong_symb.test(portal) || 
                !email.length || re_wrong_symb.test(email)) {
            showLoginError();
            return;
        } 

        var re_protocol = /^(https?:\/{2})/;
        if (re_protocol.test(portal)) {
            protocol = '';
        }

        // portal = /^(?:https?:\/{2})?([^\.]+\.[^\.]+\.\w+)/i.exec();
        // if (!!portal && portal[1].length) {
        //     portal = portal[1];
        // } else {
        //     showLoginError();
        //     return;
        // }

        var pass        = document.getElementById('auth-pass').value;
        var url         = protocol + portal + "/api/2.0/authentication.json";

        var iframe = document.createElement("iframe");
        iframe.name = "frameLogin";
        iframe.style.display = "none";

        iframe.addEventListener("load", function () {
            window.AscDesktopEditor.GetFrameContent("frameLogin");
        });

        document.body.appendChild(iframe);

        setLoaderVisible(true);
        sendData(url, {userName: email, password: pass}, iframe);
    });

    function setLoaderVisible(isvisible, timeout) {
        setTimeout(function(){
            $('#loading-mask')[isvisible?'show':'hide']();
        }, timeout || 200);
    }

    function showLoginError() {
        $('#auth-error').slideDown(100);
    }

    window.on_is_cookie_present = function(is_present, token) {
        is_present ?
            getUserInfo(token) :
            setLoaderVisible(false,1000);

    };

    // window["on_set_cookie"] = function(){};

    window.onchildframemessage = function(message, framename) {
        if (framename == 'frameLogin') {
            if (message.length) {
                var obj;
                try {
                    obj = JSON.parse(message);
                } catch (e) {}

                if (obj) {
                    if (obj.statusCode != 201) {
                        console.log('server error: ' + obj.statusCode);
                        showLoginError();
                    } else
                    if (!obj.response.sms) {
                        getUserInfo(obj.response.token);

                        setTimeout(function(){
                            var frame = document.getElementsByName('frameLogin');
                            frame && frame.length > 0 && frame[0].remove();
                        },10);

                        return;
                    }
                } else {
                    console.log('server error: wrong json');
                    showLoginError();
                }

                setLoaderVisible(false);
            }
        }
    };

    window.onkeydown = function(e) {
        var el = document.activeElement;
        if (!el || !/input|textarea/i.test(el.tagName)) 
            e.preventDefault();
    };

    $('#auth-portal').keypress(function(e){
        if ( e.which == 13 ) {
            $('#auth-email').focus();
        }
    });

    $('#auth-email').keypress(function(e){
        if ( e.which == 13 ) {
            $('#auth-pass').focus();
        }
    });

    $('#auth-pass').keypress(function(e){
        switch (e.which) {
        case 13:
            $('#btn-login').focus().click();
            break;
        }
    })
    .keydown(function(e){
        if ( e.which == 9 ) {
            $('#btn-login').focus();
            e.preventDefault();
        }
    });

    // $(document).on("contextmenu",function(e){
    //     if(e.target.nodeName != "INPUT" && e.target.nodeName != "TEXTAREA")
    //          e.preventDefault();
    // });

    function getUserInfo(token) {
        var _url_ = protocol + portal + "/api/2.0/people/@self.json";

        $.ajax({
            url: _url_,
            headers: {'Authorization': token},
            complete: function(e, status) {
                if (status == 'success') {
                    var obj = JSON.parse(e.responseText);
                    if (obj.statusCode == 200) {
                        window["AscDesktopEditor"]["js_message"]('login', 
                            JSON.stringify({
                                portal: portal,
                                user: obj.response                            
                            })
                        );

                        localStorage.setItem('ascportal', portal);
                        // window.AscDesktopEditor.setAuth(protocol + portal, portal, '/', token);
                        document.cookie = "asc_auth_key=" + token + ";domain=" + protocol + portal + ";path=/";
                        window.location.replace(protocol + portal + startmodule);
                    } else {
                        console.log('authentication error: ' + obj.statusCode);
                        showLoginError();
                    }
                } else {
                    console.log('authentication error: ' + status);
                    showLoginError();
                    setLoaderVisible(false);
                }
            },
            error: function(e, status, error) {
                console.log('server error: ' + status + ', ' + error);
                showLoginError();
                setLoaderVisible(false);
            }
        });
    }

    if (!!localStorage['ascportal'] && localStorage['ascportal'].length) {
        portal = localStorage['ascportal'];
    } else {
        var p = getUrlParams();
        if (p && !!p.portal) {
            portal = p.portal;
        }
    }

    if (!!portal && !!window.AscDesktopEditor && !!window.AscDesktopEditor.getAuth) {
        window.AscDesktopEditor.getAuth(portal);
        return;
    }

    setLoaderVisible(false);
});

function sendData(url, data, target) {
    var form = document.createElement("form");

    form.setAttribute("method", 'post');
    form.action = url;
    form.target = target.name;
    form.style.display = "none";

    for(var name in data) {
        var node = document.createElement("input");
        node.name  = name;
        node.value = data[name].toString();
        form.appendChild(node);
    }

    form.submit();
}
