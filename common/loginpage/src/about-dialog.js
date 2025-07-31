+function() {
  function AboutDialog(params) {
    params = params || {};
    Dialog.call(this, {
        dialogClass: 'dlg-about',
        titleText: utils.Lang.actAbout,
        defaultWidth: 570,
        onclose: params.onclose
    });

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

  AboutDialog.prototype.inShow = function() {
    var $title = this.getElements().$title;
    $title.find('.caption').text(utils.Lang.actAbout);
    if (this.pendingBody && this.$body && this.$body.is(':empty')) {
      this.$body.html(this.pendingBody);
    }
  };

  AboutDialog.prototype.show = function() {
    if (this.$el && this.$el.length) {
      if (!this.$el.get(0).open) this.$el.get(0).showModal();
      this.inShow(); 
      return;
    }

    Dialog.prototype.show.call(this);
  };

  window.AboutDialog = AboutDialog;
}();