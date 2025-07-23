window.DialogConnectIntro = function(params) {
  "use strict";

  let connectHandler = params.onConnect || (() => {});
  const bodyTemplate = params.bodyTemplate || '<div/>';

  params.template = `
    <dialog class="dlg dlg-connect-intro">
      <div class="title">
        <label class="caption">${utils.Lang.loginTitleStart}</label>
        <span class="tool close"></span>
      </div>
      <div class="body"></div>
    </dialog>
  `;

  const dlg = Dialog(params);

  return {
    show: function() {
      dlg.show('dlg-connect-intro', 590);
      const {$title, $body} = dlg.getElements();

      $body.html(bodyTemplate);

      $title.find('.tool.close').on('click', dlg.close);

      $body.on('click', '.link-connect-now, .link, .btn.login', function(e) {
        dlg.close();
        connectHandler(e, $(this).data('cprov'));
      });

      $body.find('[l10n="portalEmptyDescr"]').text(utils.Lang.portalEmptyDescr);
      $body.find('[l10n="portalEmptyAdv1"]').text(utils.Lang.portalEmptyAdv1);
      $body.find('.btn--landing').text(utils.Lang.btnCreatePortal);
      $body.find('label[l10n]').text(utils.Lang.textHavePortal);
      $body.find('.login.link').text(utils.Lang.btnConnect);
    }
  };
};