BEGIN {
	onlyoffice = "1";

	# Appcast Vars
	AppcastProductTitle = "ONLYOFFICE Desktop Editors";
	AppcastBaseUrl = "https://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice";
	AppcastChangesPath = AppcastBaseUrl "/changes";
	AppcastUpdatesPath = AppcastBaseUrl "/updates";

	# Changes Vars
	ChangesProductTitle = "ONLYOFFICE Desktop Editors";
	ChangesProductHeading = "ONLYOFFICE Desktop Editors";
	ChangesMoreUrl = "https://github.com/ONLYOFFICE/DesktopEditors/blob/master/" \
		"CHANGELOG.md#" gensub(/\./, "", "g", Version);
}
