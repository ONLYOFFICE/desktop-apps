window.AboutDialog = function(params) {
  "use strict";

  const dlg = Dialog({
    dialogClass: 'dlg-about',
    titleText: utils.Lang.actAbout,
    onclose: params?.onclose
  });

  return {
    setBody: dlg.setBody,
    show: function() { 
      dlg.show(570); 
      const {$title, $body} = dlg.getElements();
      $title.find('.caption').text(utils.Lang.actAbout);

    }
  };
};