function PreviewTemplateDialog(model, params = {}) {
    params = {
        ...params,
        dialogClass: 'dlg-template-preview',
        titleText: utils.Lang.actPreviewTemplates,
        defaultWidth: 800,
    };

    Dialog.call(this, params);
    this.model = model;
}

PreviewTemplateDialog.prototype = Object.create(Dialog.prototype);
PreviewTemplateDialog.prototype.constructor = PreviewTemplateDialog;

PreviewTemplateDialog.prototype.show = function(width) {
    Dialog.prototype.show.call(this, width);

    const {$el} = this.getElements();

    $el.find('.btn.btn--landing').on('click', () => {
        this.close();
        window.sdk.openTemplate(this.model.path, this.model.name);
    });
};

window.PreviewTemplateDialog = PreviewTemplateDialog;