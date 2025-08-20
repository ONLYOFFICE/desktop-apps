+function() {
    function DialogConnectIntro(params) {
        params = params || {};
        Dialog.call(this, {
            dialogClass: 'dlg-connect-intro',
            titleText: utils.Lang.loginTitleStart,
            bodyTemplate: params.bodyTemplate || '<div/>',
            defaultWidth: 590,
            onclose: params.onclose
        });

        this.connectHandler = params.onConnect || (() => {});
    }

    DialogConnectIntro.prototype = Object.create(Dialog.prototype);
    DialogConnectIntro.prototype.constructor = DialogConnectIntro;

    DialogConnectIntro.prototype.show = function(width) {
        Dialog.prototype.show.call(this, width);

        this.$body.on('click', '.link-connect-now, .link, .btn.login', (e) => {
            this.close();
            this.connectHandler(e, $(e.currentTarget).data('cprov'));
        });
    }

    window.DialogConnectIntro = DialogConnectIntro;
}();