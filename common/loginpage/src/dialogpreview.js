function CloudTemplateDialog(model, params = {}) {
    params = {
        ...params,
        dialogClass: 'dlg-template-preview',
        titleText: utils.Lang.actPreviewTemplates,
        defaultWidth: 456,
    };

    Dialog.call(this, params);
    this.model = model;
}

CloudTemplateDialog.prototype = Object.create(Dialog.prototype);
CloudTemplateDialog.prototype.constructor = CloudTemplateDialog;

CloudTemplateDialog.prototype.show = function(width) {
    Dialog.prototype.show.call(this, width);

    const {$el} = this.getElements();

    $el.find('.btn.btn--landing').on('click', () => {
        this.close();
        window.sdk.openTemplate(this.model.path, this.model.name);
    });
};

window.CloudTemplateDialog = CloudTemplateDialog;