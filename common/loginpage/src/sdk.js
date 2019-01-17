/*
 * (c) Copyright Ascensio System SIA 2010-2019
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


+function() { "use strict"
    let _events = [
                    'onchildframemessage'
                    , 'onupdaterecents'
                    , 'onupdaterecovers'
                    , 'on_native_message'
                    , 'on_check_auth'
                    , 'onChangeCryptoMode'
                ];

    let subscribers = {
        any: [] // event type: subscribers
    };

    var sdk = window.AscDesktopEditor;

    sdk.on = function(type, fn, context) {
        type = type || 'any';
        fn = typeof fn === 'function' ? fn : context[fn];
        if (typeof subscribers[type] === "undefined") {
            subscribers[type] = [];
        }

        subscribers[type].push({ fn: fn, context: context || this });
    };

    sdk.remove = function(type, fn, context) {
        notifySubscribers('unsubscribe', type, fn, context);
    };

    sdk.fire = function(type, publication) {
        notifySubscribers('publish', type, publication);
    };

    sdk.command = function() {
        window.AscDesktopEditor.execCommand.apply(this, arguments);
    };

    sdk.externalClouds = function() {
        let _clouds = sdk.GetExternalClouds();
        if ( _clouds ) {
            for (let c of _clouds) {
                (!c.check || !c.check.url) && (c.check = {url:''});
                if ( !c.check.url.startsWith('/') )
                    c.check.url = '/'.concat(c.check.url);
            }
        } else {
            _clouds = [{ id: "asc",
                            name: "ONLYOFFICE",
                            check: {url:"/api/2.0/capabilities.json"} }];
        }

        return _clouds;
    };

    sdk.encrypt = {
        ENCRYPT_MODE_NONE: 0,
        ENCRYPT_MODE_SIMPLE: 1,
        ENCRYPT_MODE_STANDARD: 2,
        ENCRYPT_MODE_ADVANCED: 3
    };

    sdk.CryptoMode = sdk.GetCryptoMode();
    sdk.encrypt.mode = function(mode, pass) {
        if ( !!mode ) sdk.SetCryptoMode(pass, mode);
        else return sdk.CryptoMode;
    };

    sdk.encrypt.available = function() {
        return sdk.GetSupportCryptoModes();
    };

    window.onChangeCryptoMode = e => {
        sdk.CryptoMode = e;
    };

    var notifySubscribers = function(action, type, arg, context) {
        var pubtype = type || 'any',
            pubsubscribers = subscribers[pubtype],
            max = pubsubscribers ? pubsubscribers.length : 0;

        for (let i = 0; i < max; i += 1) {
            if (action === 'publish') {
                // Call our observers, passing along arguments
                 pubsubscribers[i].fn.apply(pubsubscribers[i].context, arg);
            } else {
                if (pubsubscribers[i].fn === arg && pubsubscribers[i].context === context) {
                    pubsubscribers.splice(i, 1);
                }
            }
        }
    };

    for (let e of _events) {
        window[e] = function() {
            notifySubscribers('publish', e, arguments)
        };
    };

    window.sdk = sdk;
}();
