window.DialogConnectIntro = function(params) {
  "use strict";

  let connectHandler = params.onConnect || (() => {});
  const events = { close: params.onclose };

  let $el, $title, $body;
  var _template = `
    <dialog class="dlg dlg-connect-intro">
      <div class="title">
        <label class="caption">${utils.Lang.loginTitleStart}</label>
        <span class="tool close"></span>
      </div>
      <div class="body"></div>
    </dialog>
  `;

  let _body_template = params.bodyTemplate || '<div/>';
   
 function onCloseClick(e) {
    close();
  };

  function close(opts) {
    $el.remove();
    if (events.close) {
      events.close(opts);
    }
  }

  function _bind_events() {
    $title.find('.tool.close').on('click', onCloseClick);
    $el.on('close', onCloseClick);

    $body.find('.link-connect-now').on('click', e => {
      close();
      connectHandler(e);
    });

    $body.on('click', '.btn.login', e => {
      close();
      connectHandler(e, $(e.currentTarget).data('cprov'));
    });

    $body.on('click', '.link', e => {
      close();
      connectHandler(e);
    });
  }

  return {
    show: function() {
      $el = $('#placeholder').append(_template).find('.dlg-connect-intro');
      $title = $el.find('.title');
      $body = $el.find('.body');
      $el.width(590);

      $body.html(_body_template);
      _bind_events();

      $el.on('click', function (e) {
        if (e.target === $el.get(0)) {
          close();
        }
      });

      $el.get(0).showModal();
      $el.addClass('scaled');
    }
  };
};