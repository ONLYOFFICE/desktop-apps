+function() {
    function AboutDialog(params) {
        params = params || {};
        Dialog.call(this, {
            dialogClass: 'dlg-about',
            titleText: utils.Lang.actAbout,
            defaultWidth: 570,
            onclose: params.onclose
        });
    }

    AboutDialog.prototype = Object.create(Dialog.prototype);
    AboutDialog.prototype.constructor = AboutDialog;

    AboutDialog.prototype.inShow = function() {
        var $title = this.getElements().$title;
        $title.find('.caption').text(utils.Lang.actAbout);
    };

    window.AboutDialog = AboutDialog;
}();