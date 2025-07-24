window.Dialog = function(params) {
  "use strict";

  params = params || {};
  const events = { close: params.onclose };

  const dialogClass = params.dialogClass || 'dlg';
  const titleText = params.titleText || '';
  const bodyTemplate = params.bodyTemplate || '';

  let $el, $title, $body;

  const template = `
    <dialog class="dlg ${dialogClass}">
      <div class="title">
        <label class="caption">${titleText}</label>
        <span class="tool close"></span>
      </div>
      <div class="body">${bodyTemplate}</div>
    </dialog>
  `;

  function show(width) {
    $el = $('#placeholder').append(template).find(`.${dialogClass}`);
    $el.width(width || 500);

    $title = $el.find('.title');
    $body = $el.find('.body');

    if (bodyTemplate) $body.html(bodyTemplate);

    $title.find('.tool.close').on('click', close);
    $el.on('close', close);

    $(document).on('click', clickHandler);

    $el.get(0).showModal();
    $el.addClass('scaled');
  }

  function clickHandler(e) {
    if (e.target === $el.get(0)) {
      close();
    }
  }

  function close(opts) {
    $el.remove();
    $(document).off('click', clickHandler);
    if (events.close) events.close(opts);
  }

  function setBody(html) {
    $body.html(html);
  }

  function getElements() {
    return { $el, $title, $body };
  }

  return {
    show,
    close,
    setBody,
    getElements
  };
};