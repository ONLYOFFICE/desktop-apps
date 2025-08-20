// +function() {
  function AboutDialog(params = {}) {
    params = {...params, ...{
                dialogClass: 'dlg-about',
                titleText: utils.Lang.actAbout,
                defaultWidth: 570,
                onclose: params.onclose
            }};

    Dialog.call(this, params);

    this.pendingBody = null;
    this.cover = true; 
  }

  AboutDialog.prototype = Object.create(Dialog.prototype);
  AboutDialog.prototype.constructor = AboutDialog;

  AboutDialog.prototype.setBody = function(html) {
    this.pendingBody = html;  
    if (this.$body) {
      this.$body.html(html);
    }
  };

  // window.AboutDialog = AboutDialog;
// }();