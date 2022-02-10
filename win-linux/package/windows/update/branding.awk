BEGIN {
	onlyoffice = "1";

	# Appcast Vars
	AppcastProductTitle = "ONLYOFFICE Desktop Editors";
	if (Prod) {
		AppcastBaseUrl = "https://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice";
		AppcastChangesPath = AppcastBaseUrl "/changes";
		AppcastUpdatesPath = AppcastBaseUrl "/updates";
	}
	else {
		AppcastBaseUrl = "https://s3.eu-west-1.amazonaws.com/repo-doc-onlyoffice-com/onlyoffice/" Branch "/windows/" Version "-" Build "/desktop";
		AppcastChangesPath = AppcastBaseUrl;
		AppcastUpdatesPath = AppcastBaseUrl;
	}

	# Changes Vars
	ChangesProductTitle = "ONLYOFFICE Desktop Editors";
	ChangesProductHeading = "ONLYOFFICE Desktop Editors";
	ChangesMoreUrl = "https://github.com/ONLYOFFICE/DesktopEditors/blob/master/" \
		"CHANGELOG.md#" gensub(/\./, "", "g", Version);
}
