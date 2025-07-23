window.AboutDialog = function(params) {
  "use strict";

  params = Object.assign({}, params, {
    template: `
      <dialog class="dlg dlg-about">
        <div class="title">
          <label class="text-headline" l10n>${utils.Lang.actAbout}</label>
          <span class="tool close"></span>
        </div>
        <div class="body"></div>
      </dialog>
    `
  });

  const dlg = Dialog(params);

  return {
    setBody: dlg.setBody,
    show: function() { 
      dlg.show('dlg-about', 576)
      const {$title, $body} = dlg.getElements();
      $title.find('.text-headline').text(utils.Lang.actAbout);
      $body.find('#idx-about-version span[l10n]').text(utils.Lang.strVersion);

    }
  };
};