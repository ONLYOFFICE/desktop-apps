window.WelcomeComponent = function() {
    "use strict";

    let $el;

    const _template = `
      <div class="welcome-component">
        <h1 l10n>${utils.Lang.welcomeTitle}</h1>
        <div l10n>${utils.Lang.welcomeDescription}</div>
      </div>
    `;


    return {
        render: function(parentElement) {
            $el = parentElement.append(_template).find('.welcome-component');
        },
        detach: function() {
            $el.remove();
        },
    }
};