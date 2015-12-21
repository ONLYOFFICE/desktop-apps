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

    var linkForgotPass = 'Forgot password?';
    var linkCreatePortal = 'Create a portal';
    var dlgLoginTitle = 'Connect portal';
    var btnLogin = 'Login';
    var errLogin = 'Wrong portal name or login or email';
    var errLoginPortal = 'Check the portal name';
    var errLoginEmail = 'Check the email address';
    var errLoginServer = 'Wrong server answer during login';
    var errLoginAuth = 'Error on query user information';
    var errLoginPass = 'Check the password';

    var $el, $mask;
    var _tpl = '<div class="dlg dlg-login">' +
                  '<div class="title">'+
                    '<label class="caption">'+dlgLoginTitle+'</label>'+
                    '<span class="tool close"></span>'+
                  '</div>'+
                  '<div class="body">'+
                    '<div class="logo"></div>'+
                    '<section id="box-lbl-error">'+
                      '<p id="auth-error" class="msg-error">' + errLogin + '</p>' +
                    '</section>'+
                    '<input id="auth-portal" type="text" name="" spellcheck="false" class="tbox auth-control first" placeholder="portal" value="">' +
                    '<input id="auth-email" type="text" name="" spellcheck="false" class="tbox auth-control" placeholder="email" value="">' +
                    '<input id="auth-pass" type="password" name="" spellcheck="false" class="tbox auth-control last" placeholder="password" value="">' +
                    '<div id="box-btn-login">'+
                      '<a id="link-restore" class="text-sub link" target="popup" href="https://www.onlyoffice.com/signin.aspx">' + linkForgotPass + '</a>'+
                      '<span />'+ 
                      '<div><img class="img-loader">' +
                      '<button id="btn-login" class="btn primary">'+btnLogin+'</button></div>'+
                    '</div>'+
                    '<div class="separator"></div>'+
                    '<div style="text-align:left;">'+
                      '<a id="link-create" class="text-sub link" target="popup" href="https://www.onlyoffice.com/registration.aspx">' + linkCreatePortal + '</a>'+
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
        hideLoginError();

        portal = $el.find('#auth-portal').val();
        email = $el.find('#auth-email').val();

        var re_wrong_symb = /[\s\\]/;
        if (!portal.length || re_wrong_symb.test(portal)) {
            showLoginError(errLoginPortal, '#auth-portal');
            return;
        }

        if (!email.length || re_wrong_symb.test(email)) {
            showLoginError(errLoginEmail, '#auth-email');
            return;
        }

        portal = /^(https?:\/{2})?([^\/]+)/i.exec(portal);
        if (!!portal && portal[2].length) {
            portal[1] && (protocol = portal[1]);
            portal = portal[2];
        } else {
            showLoginError(errLoginPortal, '#auth-portal');
            return;
        }

        var pass = $el.find('#auth-pass').val();
        if (!pass || pass.length < 0) {
            showLoginError(errLoginPass, '#auth-pass');
            return;
        }

        var url         = protocol + portal + "/api/2.0/authentication.json";
        var check_url   = protocol + portal + "/api/2.0/people/@self.json";        

        disableDialog(true);
        // setLoaderVisible(true);
        if (checkResourceExists(check_url) == 0) {
            showLoginError(errLoginPortal, '#auth-portal');
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

    function showLoginError(error, el) {
        let $lbl = $el.find('#auth-error');

        !!error && $lbl.text(error);
        $lbl.fadeIn(100);

        !!el && $el.find(el).addClass('error').focus();
        disableDialog(false);
    };

    function hideLoginError() {
        $el.find('#auth-error').fadeOut(100);
        $el.find('.error').removeClass('error');
    };

    window.onchildframemessage = function(message, framename) {
        if (framename == 'frameLogin') {
            if (message.length) {
                var obj;
                try {
                    obj = JSON.parse(message);
                } catch (e) {
                    disableDialog(false);
                }

                if (obj) {
                    if (obj.statusCode != 201) {
                        console.log('server error: ' + obj.statusCode);
                        showLoginError(errLoginServer);
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
                    showLoginError(errLoginServer);
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
                        showLoginError(errLoginAuth);
                    }
                } else {
                    console.log('authentication error: ' + status);
                    showLoginError(errLoginAuth);
                    // setLoaderVisible(false);
                }
            },
            error: function(e, status, error) {
                console.log('server error: ' + status + ', ' + error);
                showLoginError(errLoginAuth);
                // setLoaderVisible(false);
            }
        };

        $.ajax(opts);
    };

    function bindEvents() {
        $el.find('.body').on('keypress', '.auth-control', 
            function(e) {
                if (e.which == 13) {
                    if (/auth-portal/.test(e.target.id)) 
                        $el.find('#auth-email').focus(); else
                    if (/auth-email/.test(e.target.id)) 
                        $el.find('#auth-pass').focus(); else
                    if (/auth-pass/.test(e.target.id)) {
                        $el.find('#btn-login').focus().click();
                    }
                }
        });

        $el.on('keyup',
            function(e) {
                if (e.which == 27) {
                    onCloseClick();
                }
        });
    };

    function disableDialog(disable) {
        $el.find('.tbox, #btn-login').prop('disabled', disable);
        $el.find('.img-loader')[disable?'show':'hide']();
    };

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

            $el.width(450).height(470);

            $el.find('.tool.close').bind('click', onCloseClick);
            $el.find('#btn-login').click(onLoginClick);

            bindEvents();
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
