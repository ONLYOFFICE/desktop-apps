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


LoginDlg = function() {
    "use strict";

    var $el, $mask;
    var _tpl = '<div class="dlg dlg-login">' +
                  '<div class="title">'+
                    '<label class="caption">Connect portal</label>'+
                    '<span class="tool close"></span>'+
                  '</div>'+
                  '<div class="body">'+
                    '<label>To join the portal, enter your credentials.</label>' +
                    '<input id="auth-portal" type="text" name="" spellcheck="false" class="tbox auth-control first" placeholder="portal" value="">' +
                    '<input id="auth-email" type="text" name="" spellcheck="false" class="tbox auth-control" placeholder="email" value="">' +
                    '<input id="auth-pass" type="password" name="" spellcheck="false" class="tbox auth-control last" placeholder="password" value="">' +
                    '<div id="box-btn-login">'+
                      '<a id="link-restore" class="text-sub link" target="popup" href="https://www.onlyoffice.com/signin.aspx">Forgot password?</a>'+
                      '<span />'+ 
                      '<button id="btn-login" class="btn primary">Login</button>'+
                    '</div>'+
                  '</div>'+
                '</div>';

    var protocol = 'https://',
        startmodule = '/products/files/?desktop=true';
    var portal = undefined,
        email = undefined;
    var events = {};

    function checkResourceExists(url) {
        var reader = new XMLHttpRequest();

        reader.open('get', url, false);
        reader.send(null);
        switch (reader.status) {
        case 0: case 401:
        case 200: return 1;
        case 404: return 0;
        default: return -1;
        }
    }

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
    };

    function onCloseClick(e) {
        doClose(0);
    };

    function doClose(code) {
        $mask.hide();
        $el.remove();

        if (events.close) {
            events.close(code);
        }
    };

    function onLoginClick(e) {
        portal = $el.find('#auth-portal').val();
        email = $el.find('#auth-email').val();

        var re_wrong_symb = /[\s\\]/;
        if (!portal.length || re_wrong_symb.test(portal) || 
                !email.length || re_wrong_symb.test(email)) {
            showLoginError();
            return;
        }

        portal = /^(https?:\/{2})?([^\/]+)/i.exec(portal);
        if (!!portal && portal[2].length) {
            portal[1] && (protocol = portal[1]);
            portal = portal[2];
        } else {
            showLoginError();
            return;
        }

        var pass        = $el.find('#auth-pass').val();
        var url         = protocol + portal + "/api/2.0/authentication.json";
        var check_url   = protocol + portal + "/api/2.0/people/@self.json";        

        // setLoaderVisible(true);
        if (checkResourceExists(check_url) == 0) {
            // TODO: divide to login error and wrong portal error
            showLoginError();
            // setLoaderVisible(false);
        } else {
            var iframe = document.createElement("iframe");
            iframe.name = "frameLogin";
            iframe.style.display = "none";

            iframe.addEventListener("load", function () {
                window.AscDesktopEditor.GetFrameContent("frameLogin");
            });

            document.body.appendChild(iframe);

            sendData(url, {userName: email, password: pass}, iframe);
        }
    };

    function showLoginError() {
        $('#auth-error').slideDown(100);
    }

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

                // setLoaderVisible(false);
            }
        }
    };

    function getUserInfo(token) {
        var _url_ = protocol + portal + "/api/2.0/people/@self.json";

        var opts = {
            url: _url_,
            crossOrigin: true,
            crossDomain: true,
            headers: {'Authorization': token},
            beforeSend: function (xhr) {
                // xhr.setRequestHeader ("Access-Control-Allow-Origin", "*");
            },
            complete: function(e, status) {
                if (status == 'success') {
                    var obj = JSON.parse(e.responseText);
                    if (obj.statusCode == 200) {
                        // deprecated
                        // ----------
                        // window["AscDesktopEditor"]["js_message"]('login', 
                        //     JSON.stringify({
                        //         portal: portal,
                        //         user: obj.response
                        //     })
                        // );

                        // localStorage.setItem('ascportal', portal);
                        document.cookie = "asc_auth_key=" + token + ";domain=" + protocol + portal + ";path=/;HttpOnly";
                        // window.location.replace(protocol + portal + startmodule);

                        if (!!events.success) {
                            let auth_info = {
                                portal: protocol + portal,
                                user: obj.response.displayName,
                                email: obj.response.email
                            };
                            events.success(auth_info);
                        }
                        doClose(1);
                    } else {
                        console.log('authentication error: ' + obj.statusCode);
                        showLoginError();
                    }
                } else {
                    console.log('authentication error: ' + status);
                    showLoginError();
                    // setLoaderVisible(false);
                }
            },
            error: function(e, status, error) {
                console.log('server error: ' + status + ', ' + error);
                showLoginError();
                // setLoaderVisible(false);
            }
        };

        $.ajax(opts);
    }

    return {
        show: function(portal, email) {
            $el = $('#placeholder').append(_tpl).find('.dlg-login');
            $mask = $('.modal-mask');

            let $p = $el.find('#auth-portal'),
                $e = $el.find('#auth-email'),
                $k = $el.find('#auth-pass');

            if (!!portal) {
                let sp = utils.getUrlProtocol(portal);
                !!sp && (protocol = sp);

                $p.val(utils.skipUrlProtocol(portal));
            }
            !!email && $e.val(email);

            $el.width(400).height(370);

            $el.find('.tool.close').bind('click', onCloseClick);
            $el.find('#btn-login').click(onLoginClick);

            $mask.show();

            $p.val().length > 0 ? 
                $e.val().length > 0 ? 
                    $k.focus() : $e.focus() : $p.focus();
        },
        onclose: function(callback) {
            if (!!callback)
                events.close = callback;
        },
        onsuccess: function(callback) {
            if (!!callback)
                events.success = callback;
        }
    };  
};

function doLogin() {

}
