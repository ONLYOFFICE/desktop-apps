window.DialogConnectIntro = function(params) {
  "use strict";

  let connectHandler = params.onConnect || (() => {});

  const dlg = Dialog({
    dialogClass: 'dlg-connect-intro',
    titleText: utils.Lang.loginTitleStart,
    bodyTemplate: params.bodyTemplate || '<div/>',
    onclose: params.onclose
  });

  return {
    show: function() {
      dlg.show(590); 
      const {$title, $body} = dlg.getElements();

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