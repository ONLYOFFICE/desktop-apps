window.WelcomeComponent = function() {
    "use strict";

    let $el;

    const _template = `
      <div class="welcome-component">
        <h1 l10n>${utils.Lang.welcomeTitle}</h1>
        <div>
            <p>${utils.Lang.welcomeWeArePleasedToWelcomeYou}</p>
            <p>${utils.Lang.welcomeWhatYouCanDoWithOnlyOffice}</p>
            <ul>
                <li><b>${utils.Lang.welcomeDocumentsSpreadsheetsPresentationsTitle}</b> ${utils.Lang.welcomeDocumentsSpreadsheetsPresentationsDescription}</li>
                <li><b>${utils.Lang.welcomePdfTitle}</b> ${utils.Lang.welcomePdfDescription}</li>
                <li><b>${utils.Lang.welcomeCloudTitle}</b> ${utils.Lang.welcomeCloudDescription}</li>
            </ul>
            <p>${utils.Lang.welcomeNeedHelp}</p>
            <p>${utils.Lang.welcomeSuccessfulCreativeWorkflow}</p>
            <p>${utils.Lang.welcomeOnlyOfficeTeam}</p>
        </div>
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