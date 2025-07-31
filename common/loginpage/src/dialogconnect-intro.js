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

    DialogConnectIntro.prototype.inShow = function() {
        var el = this.getElements();
        var $title = el.$title, $body = el.$body;

        $title.find('.tool.close').on('click', () => {
            this.close();
        });

        $body.on('click', '.link-connect-now, .link, .btn.login', (e) => {
            this.close();
            this.connectHandler(e, $(e.currentTarget).data('cprov'));
        });

        $body.find('[l10n="portalEmptyDescr"]').text(utils.Lang.portalEmptyDescr);
        $body.find('[l10n="portalEmptyAdv1"]').text(utils.Lang.portalEmptyAdv1);
        $body.find('.btn--landing').text(utils.Lang.btnCreatePortal);
        $body.find('label[l10n]').text(utils.Lang.textHavePortal);
        $body.find('.login.link').text(utils.Lang.btnConnect);
    };

    window.DialogConnectIntro = DialogConnectIntro;
}();