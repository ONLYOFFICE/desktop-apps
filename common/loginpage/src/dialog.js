+function() {
    function Dialog(params) {
        params = params || {};
        this.events = { close: params.onclose };
        this.dialogClass = params.dialogClass || 'dlg';
        this.titleText = params.titleText || '';
        this.bodyTemplate = params.bodyTemplate || '';
        this.defaultWidth = params.defaultWidth || 500;

        this.$el = null;
        this.$title = null;
        this.$body = null;
    }

    Dialog.prototype.template = function() {
        return `<dialog class="dlg ${this.dialogClass}">
            <div class="title">
                <label class="caption">${this.titleText}</label>
                <span class="tool close"></span>
            </div>
            <div class="body">${this.bodyTemplate}</div>
        </dialog>`;
    };

    Dialog.prototype.show = function(width) {
        this.$el = $('#placeholder').append(this.template()).find(`.${this.dialogClass}`);
        this.$el.width(width || this.defaultWidth);

        this.$title = this.$el.find('.title');
        this.$body = this.$el.find('.body');

        if (this.bodyTemplate)  this.$body.html(this.bodyTemplate);

        this.$title.find('.tool.close').on('click', () => this.close());
        this.$el.on('close', () => this.close());
        $(document).on('click', (e) => {
            if (e.target === this.$el.get(0)) {
                this.close();
            }
        });

        this.$el.get(0).showModal();
        this.$el.addClass('scaled');

        this.inShow();
    };

    Dialog.prototype.close = function(opts) {
        if (!this.cover && this.$el) {
            this.$el.remove();
        } else {
            this.$el?.get(0)?.close();
        }

        if (this.events.close) this.events.close(opts);
    };

    Dialog.prototype.setBody = function(html) {
        this.$body.html(html);
    };

    Dialog.prototype.getElements = function() {
        return {
            $el: this.$el,
            $title: this.$title,
            $body: this.$body
        };
    };

    Dialog.prototype.inShow = function() {};

    window.Dialog = Dialog;
}();