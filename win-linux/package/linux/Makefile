PWD := $(shell pwd)

COMPANY_NAME ?= ONLYOFFICE
PRODUCT_NAME ?= Desktop Editors
PRODUCT_NAME_SHORT ?= $(subst $(empty) $(empty),,$(PRODUCT_NAME))

COMPANY_NAME_LOW = $(shell echo $(COMPANY_NAME) | tr A-Z a-z)
PRODUCT_NAME_LOW = $(shell echo $(PRODUCT_NAME_SHORT) | tr A-Z a-z)

PUBLISHER_NAME ?= Ascensio System SIA
PUBLISHER_URL ?= http://onlyoffice.com
SUPPORT_URL ?= http://support.onlyoffice.com
SUPPORT_MAIL ?= support@onlyoffice.com

PRODUCT_VERSION ?= 0.0.0
BUILD_NUMBER ?= 0
PACKAGE_EDITION ?= cool

BRANDING_DIR ?= $(PWD)/branding

PACKAGE_NAME := $(COMPANY_NAME_LOW)-$(PRODUCT_NAME_LOW)
PACKAGE_VERSION := $(PRODUCT_VERSION)-$(BUILD_NUMBER)

UNAME_M ?= $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
	RPM_ARCH = x86_64
	DEB_ARCH = amd64
	TAR_ARCH = x86_64
	PLATFORM = linux_64
endif
ifneq ($(filter aarch%,$(UNAME_M)),)
	RPM_ARCH = aarch64
	DEB_ARCH = arm64
	TAR_ARCH = aarch64
	PLATFORM = linux_arm64
endif

RPM_BUILD_DIR = $(PWD)/rpm/builddir
APT_RPM_BUILD_DIR = $(PWD)/apt-rpm/builddir
URPMI_BUILD_DIR = $(PWD)/urpmi/builddir
SUSE_RPM_BUILD_DIR = $(PWD)/suse-rpm/builddir
TAR_BUILD_DIR = $(PWD)/tar

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
APT_RPM_PACKAGE_DIR = $(APT_RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
URPMI_PACKAGE_DIR = $(URPMI_BUILD_DIR)/RPMS/$(RPM_ARCH)
SUSE_RPM_PACKAGE_DIR = $(SUSE_RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
TAR_PACKAGE_DIR = $(TAR_BUILD_DIR)

RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(RPM_RELEASE_SUFFIX).$(RPM_ARCH).rpm
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(APT_RPM_RELEASE_SUFFIX).$(RPM_ARCH).rpm
URPMI = $(URPMI_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(URPMI_RELEASE_SUFFIX).$(RPM_ARCH).rpm
SUSE_RPM = $(SUSE_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(SUSE_RPM_RELEASE_SUFFIX).$(RPM_ARCH).rpm
DEB = deb/$(PACKAGE_NAME)_$(PACKAGE_VERSION)$(DEB_RELEASE_SUFFIX)_$(DEB_ARCH).deb
DEB_ASTRA = deb-astra/$(PACKAGE_NAME)_$(PACKAGE_VERSION)$(DEB_ASTRA_RELEASE_SUFFIX)_$(DEB_ARCH).deb
TAR = $(TAR_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)$(TAR_RELEASE_SUFFIX)-$(TAR_ARCH).tar.xz
HELP_TAR = $(TAR_PACKAGE_DIR)/$(PACKAGE_NAME)-help-$(PACKAGE_VERSION)$(TAR_RELEASE_SUFFIX)-any.tar.xz

NEWS_RPM = common/usr/share/doc/$(PACKAGE_NAME)/NEWS
CHANGELOG_RPM = common/usr/share/doc/$(PACKAGE_NAME)/ChangeLog
CHANGELOG_DEB = deb/build/debian/changelog
CHANGELOG_DEB_ASTRA = deb-astra/build/debian/changelog

DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/$(PRODUCT_NAME_LOW)
DESKTOPEDITORS_EXEC := $(PACKAGE_NAME)
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	MEDIAVIEWER_PREFIX := $(COMPANY_NAME_LOW)/mediaviewer
	IMAGEVIEWER_EXEC := $(COMPANY_NAME_LOW)-imageviewer
	VIDEOPLAYER_EXEC := $(COMPANY_NAME_LOW)-videoplayer
endif

LINUX_DEPS += common/usr/share/applications/variables.m4
BIN_FILES += common/usr/bin/$(DESKTOPEDITORS_EXEC)
DESKTOP_FILES += common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop
SYMLINK += common/usr/bin/desktopeditors
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	BIN_FILES += common/usr/bin/$(IMAGEVIEWER_EXEC)
	BIN_FILES += common/usr/bin/$(VIDEOPLAYER_EXEC)
	DESKTOP_FILES += common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop
	DESKTOP_FILES += common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.docx.desktop
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.xlsx.desktop
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.pptx.desktop
endif
LINUX_DEPS += $(BIN_FILES)
LINUX_DEPS += $(DESKTOP_FILES)
LINUX_DEPS += $(SYMLINK)
LINUX_DEPS += $(CHANGELOG_RPM)

DEB_DEPS += $(CHANGELOG_DEB)
DEB_DEPS += deb/build/debian/compat
DEB_DEPS += deb/build/debian/control
DEB_DEPS += deb/build/debian/postinst
DEB_DEPS += deb/build/debian/postrm
DEB_DEPS += deb/build/debian/prerm
DEB_DEPS += deb/build/debian/rules
DEB_DEPS += deb/build/debian/source/format
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).dirs
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).install
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME).links
DEB_DEPS += deb/build/debian/$(PACKAGE_NAME)-help.install

DEB_ASTRA_DEPS += $(CHANGELOG_DEB_ASTRA)
DEB_ASTRA_DEPS += deb-astra/build/debian/compat
DEB_ASTRA_DEPS += deb-astra/build/debian/control
DEB_ASTRA_DEPS += deb-astra/build/debian/postinst
DEB_ASTRA_DEPS += deb-astra/build/debian/postrm
DEB_ASTRA_DEPS += deb-astra/build/debian/prerm
DEB_ASTRA_DEPS += deb-astra/build/debian/rules
DEB_ASTRA_DEPS += deb-astra/build/debian/source/format
DEB_ASTRA_DEPS += deb-astra/build/debian/$(PACKAGE_NAME).dirs
DEB_ASTRA_DEPS += deb-astra/build/debian/$(PACKAGE_NAME).install
DEB_ASTRA_DEPS += deb-astra/build/debian/$(PACKAGE_NAME).links
DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.docx.desktop
DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.xlsx.desktop
DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.pptx.desktop

SRC += ../../../../build_tools/out/$(PLATFORM)/$(DESKTOPEDITORS_PREFIX)
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	SRC += ../../../../build_tools/out/$(PLATFORM)/$(MEDIAVIEWER_PREFIX)
endif

M4_PARAMS += -D M4_COMPANY_NAME='$(COMPANY_NAME)'
M4_PARAMS += -D M4_PRODUCT_NAME='$(PRODUCT_NAME)'
M4_PARAMS += -D M4_PUBLISHER_NAME='$(PUBLISHER_NAME)'
M4_PARAMS += -D M4_PUBLISHER_URL='$(PUBLISHER_URL)'
M4_PARAMS += -D M4_SUPPORT_MAIL='$(SUPPORT_MAIL)'
M4_PARAMS += -D M4_SUPPORT_URL='$(SUPPORT_URL)'
M4_PARAMS += -D M4_PACKAGE_NAME='$(PACKAGE_NAME)'
M4_PARAMS += -D M4_PACKAGE_EDITION='$(PACKAGE_EDITION)'
M4_PARAMS += -D M4_DESKTOPEDITORS_PREFIX='$(DESKTOPEDITORS_PREFIX)'
M4_PARAMS += -D M4_DESKTOPEDITORS_EXEC='$(DESKTOPEDITORS_EXEC)'
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	M4_PARAMS += -D M4_MEDIAVIEWER_PREFIX='$(MEDIAVIEWER_PREFIX)'
	M4_PARAMS += -D M4_IMAGEVIEWER_EXEC='$(IMAGEVIEWER_EXEC)'
	M4_PARAMS += -D M4_VIDEOPLAYER_EXEC='$(VIDEOPLAYER_EXEC)'
	M4_PARAMS += -D M4_SCHEME_HANDLER='$(COMPANY_NAME_LOW)'
else
	M4_PARAMS += -D M4_SCHEME_HANDLER='oo-office'
endif
M4_PARAMS += -D M4_DEB_ARCH='$(DEB_ARCH)'
M4_DEB_PARAMS = $(M4_PARAMS)
M4_DEB_PARAMS += -D M4_PACKAGE_VERSION='$(PACKAGE_VERSION)$(DEB_RELEASE_SUFFIX)'
M4_DEB_ASTRA_PARAMS = $(M4_PARAMS)
M4_DEB_ASTRA_PARAMS += -D M4_ASTRALINUX_KEYS_DIR='$(abspath $(ASTRALINUX_KEYS_DIR))'
M4_DEB_ASTRA_PARAMS += -D M4_ASTRALINUX_SIGN_IMAGE='$(ASTRALINUX_SIGN_IMAGE)'
M4_DEB_ASTRA_PARAMS += -D M4_PACKAGE_VERSION='$(PACKAGE_VERSION)$(DEB_ASTRA_RELEASE_SUFFIX)'

RPM_PARAMS += --define '_company_name $(COMPANY_NAME)'
RPM_PARAMS += --define '_product_name $(PRODUCT_NAME)'
RPM_PARAMS += --define '_publisher_name $(PUBLISHER_NAME)'
RPM_PARAMS += --define '_publisher_url $(PUBLISHER_URL)'
RPM_PARAMS += --define '_support_mail $(SUPPORT_MAIL)'
RPM_PARAMS += --define '_package_name $(PACKAGE_NAME)'
RPM_PARAMS += --define '_package_edition $(PACKAGE_EDITION)'
RPM_PARAMS += --define '_product_version $(PRODUCT_VERSION)'
RPM_PARAMS += --define '_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)'
RPM_PARAMS += --define '_desktopeditors_exec $(DESKTOPEDITORS_EXEC)'
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	RPM_PARAMS += --define '_mediaviewer_prefix $(MEDIAVIEWER_PREFIX)'
	RPM_PARAMS += --define '_imageviewer_exec $(IMAGEVIEWER_EXEC)'
	RPM_PARAMS += --define '_videoplayer_exec $(VIDEOPLAYER_EXEC)'
	RPM_PARAMS += --define '_scheme_handler $(COMPANY_NAME_LOW)'
else
	RPM_PARAMS += --define '_scheme_handler oo-office'
endif
RPM_PARAMS += --define '_binary_payload w7.xzdio'

CHANGES_DIR ?= ../../../common/changes
CHANGES_HTML = changes.html
CHANGES_VERSIONS = $(shell echo $(PRODUCT_VERSION); { ls -1 $(CHANGES_DIR) \
	| grep -E '^[0-9]+' | grep -v $(PRODUCT_VERSION) | sort -Vur; })

all: tar deb deb-astra rpm apt-rpm suse-rpm urpmi

ifeq ($(COMPANY_NAME), ONLYOFFICE)
tar: $(TAR) $(HELP_TAR)
else
tar: $(TAR)
endif

deb: $(DEB)

deb-astra: $(DEB_ASTRA)

rpm: $(RPM)

apt-rpm: $(APT_RPM)

suse-rpm: $(SUSE_RPM)

urpmi: $(URPMI)

clean:
	rm -rfv \
		common/etc \
		common/opt \
		common/help \
		$(LINUX_DEPS) \
		deb/build \
		deb/*.buildinfo \
		deb/*.changes \
		deb/*.ddeb \
		deb/*.deb \
		deb-astra \
		$(RPM_BUILD_DIR) \
		$(APT_RPM_BUILD_DIR) \
		$(URPMI_BUILD_DIR) \
		$(SUSE_RPM_BUILD_DIR) \
		$(TAR_BUILD_DIR) \
		rpm/$(PACKAGE_NAME).spec \
		apt-rpm/$(PACKAGE_NAME).spec \
		urpmi/$(PACKAGE_NAME).spec \
		suse-rpm/$(PACKAGE_NAME).spec \
		$(dir $(CHANGELOG_RPM)) \
		$(CHANGES_DIR)/*/changes_??? \
		$(CHANGES_DIR)/*/news_???

branding: $(BRANDING_DIR)
	cp -rf $(BRANDING_DIR)/* $(PWD)/

desktop: branding $(LINUX_DEPS)
	mkdir -p common/opt
	cp -rft common/opt $(SRC)

	cp -rf ../../../common/package/license/agpl-3.0.htm common/opt/desktopeditors/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE common/opt/desktopeditors

	cp -rf ../../../common/package/mimetypes common/opt/desktopeditors

ifeq ($(COMPANY_NAME), ONLYOFFICE)
	for path in common/opt/desktopeditors/editors/web-apps/apps/*/main/resources/help; do \
		path_new=$$(echo $$path | sed 's|/opt/|/help/|'); \
		mkdir -p $$path_new; \
		mv -fT $$path $$path_new; \
	done
endif

	chmod 755 common/opt/desktopeditors/DesktopEditors
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	chmod 755 common/opt/mediaviewer/ImageViewer
	chmod 755 common/opt/mediaviewer/VideoPlayer
endif

$(RPM): desktop rpm/$(PACKAGE_NAME).spec
	cd rpm && rpmbuild -bb \
		--define "_topdir $(RPM_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(RPM_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		package.spec

deb/build/debian/% : deb/template/%
	mkdir -pv $(@D) && cp -fv $< $@

deb/build/debian/% : deb/template/%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

$(CHANGELOG_DEB_ASTRA) : $(CHANGELOG_DEB)
	mkdir -pv $(@D)
	cp -v $(CHANGELOG_DEB) $@

$(CHANGELOG_DEB) :
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date -R -f -); \
		else \
			date=$$(date -R); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		mkdir -pv $(CHANGES_DIR)/$$v; \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			sed $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e '/DOCTYPE/,/div class/d' \
				-e '/div class/,/html/d' \
				-e '/<h1>/d' \
				-e '/<ul>/d' \
				-e '/<\/ul>/d' \
				-e 's/<\/li>//g' \
				-e 's/<\/h4>/:/g' \
				-e 's/<\/h2>/ ]/g' \
				-e 's/<code>//g' \
				-e 's/<\/code>//g' \
				-e 's/ \{,\}<li>/    - /g' \
				-e 's/ \{,\}<h4>/  * /g' \
				-e 's/ \{,\}<h2>/\n  [ /g' \
				-e '/^$$/d' \
				| sed '1{/^$$/d}' \
				> $(CHANGES_DIR)/$$v/changes_deb; \
		else \
			echo "  * $(PRODUCT_VERSION)" > $(CHANGES_DIR)/$$v/changes_deb; \
		fi; \
		if [ $$v = $(PRODUCT_VERSION) ]; then \
			m4 $(M4_DEB_PARAMS) -D M4_DATE="$$date" \
				deb/template/changelog.m4 \
				| sed '2r '$(CHANGES_DIR)/$$v/changes_deb >> $@; \
		else \
			m4 $(M4_DEB_PARAMS) -D M4_PACKAGE_VERSION=$$v -D M4_DATE="$$date" \
				deb/template/changelog.m4 \
				| sed '2r '$(CHANGES_DIR)/$$v/changes_deb >> $@; \
		fi; \
	done

$(NEWS_RPM) :
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date -I -f -); \
		else \
			date=$$(date -I); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		mkdir -pv $(CHANGES_DIR)/$$v; \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			sed $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e '/DOCTYPE/,/div class/d' \
				-e '/div class/,/html/d' \
				-e 's/<\/li>//g' \
				-e 's/<\/h4>/:/g' \
				-e 's/<\/h2>//g' \
				-e 's/<code>//g' \
				-e 's/<\/code>//g' \
				-e 's/ \{,\}<li>/  - /g' \
				-e 's/ \{,\}<h4>/- /g' \
				-e 's/ \{,\}<h2>//g' \
				-e '/<ul>/d' \
				-e '/<\/ul>/d' \
				-e '/<h1>/d' \
				-e '/^$$/d' \
				> $(CHANGES_DIR)/$$v/news_rpm; \
		else \
			echo "$(PRODUCT_VERSION)" > $(CHANGES_DIR)/$$v/news_rpm; \
		fi; \
		m4 $(M4_PARAMS) -D M4_VERSION=$$v -D M4_DATE_SHORT="$$date" \
			common/usr/share/doc/NEWS.m4 \
			| sed '2r '$(CHANGES_DIR)/$$v/news_rpm >> $@; \
	done

$(CHANGELOG_RPM) : $(NEWS_RPM)
	mkdir -pv $(@D)
	for v in $(CHANGES_VERSIONS); do \
		if [ -f $(CHANGES_DIR)/$$v/$(CHANGES_HTML) ]; then \
			date=$$(sed -n $(CHANGES_DIR)/$$v/$(CHANGES_HTML) \
				-e 's/{{DATE}}/'$$(date -I)'/' \
				-e 's/.*<span class="releasedate"> - \(.*\)<\/span>.*/\1/p' \
				| date +'%a %b %d %Y' -f -); \
		else \
			date=$$(date +'%a %b %d %Y'); \
		fi; \
		echo "VERSION: $$v; DATE: $$date"; \
		sed $(CHANGES_DIR)/$$v/news_rpm \
			-e 's/^    - /      - /' \
			-e 's/^  - /    - /' \
			-e 's/^- /  - /' \
			-e 's/^[^ *]/- &/' \
			> $(CHANGES_DIR)/$$v/changes_rpm; \
		m4 $(M4_PARAMS) -D M4_VERSION=$$v -D M4_DATE="$$date" \
			common/usr/share/doc/ChangeLog.m4 \
			| sed '1r '$(CHANGES_DIR)/$$v/changes_rpm >> $@; \
	done

deb/build/debian/$(PACKAGE_NAME).% : deb/template/package.%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

deb/build/debian/$(PACKAGE_NAME)-help.% : deb/template/package-help.%.m4
	mkdir -pv $(@D) && m4 $(M4_DEB_PARAMS) $< > $@

deb-astra/build/debian/% : deb/template/%
	mkdir -pv $(@D) && cp -fv $< $@

deb-astra/build/debian/% : deb/template/%.m4
	mkdir -pv $(@D) && \
	m4 $(M4_DEB_ASTRA_PARAMS) $< > $@

deb-astra/build/debian/$(PACKAGE_NAME).% : deb/template/package.%.m4
	mkdir -pv $(@D) && \
	m4 $(M4_DEB_ASTRA_PARAMS) $< > $@

$(DEB): desktop $(DEB_DEPS)
	cd deb/build && dpkg-buildpackage -b -uc -us

$(DEB_ASTRA): desktop $(DEB_ASTRA_DEPS)
	cd deb-astra/build && dpkg-buildpackage -b -uc -us

$(APT_RPM): desktop apt-rpm/$(PACKAGE_NAME).spec
	cd apt-rpm && setarch $(RPM_ARCH) rpmbuild -bb \
		--define "_topdir $(APT_RPM_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(APT_RPM_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		package.spec

$(URPMI): desktop urpmi/$(PACKAGE_NAME).spec
	cd urpmi && setarch $(RPM_ARCH) rpmbuild -bb \
		--define "_topdir $(URPMI_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(URPMI_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		package.spec

$(SUSE_RPM): desktop suse-rpm/$(PACKAGE_NAME).spec
	cd suse-rpm && rpmbuild -bb \
		--define "_topdir $(SUSE_RPM_BUILD_DIR)" \
		--define "_build_number $(BUILD_NUMBER)$(SUSE_RPM_RELEASE_SUFFIX)" \
		--define "_package_arch $(RPM_ARCH)" \
		--target $(RPM_ARCH) \
		$(RPM_PARAMS) \
		package.spec

$(TAR): desktop
	mkdir -p $(TAR_PACKAGE_DIR) common/opt/desktopeditors/app
	cp -ft common/opt/desktopeditors/app $(DESKTOP_FILES)
	tar -cI"xz -9T0" --owner=root --group=root -f"$@" -C common/opt desktopeditors

$(HELP_TAR): desktop
	mkdir -p $(TAR_PACKAGE_DIR)
	tar -cI"xz -9T0" --owner=root --group=root -f"$@" -C common/help desktopeditors

common/usr/bin/$(DESKTOPEDITORS_EXEC) : common/usr/bin/desktopeditors.m4
common/usr/bin/$(IMAGEVIEWER_EXEC) : common/usr/bin/imageviewer.m4
common/usr/bin/$(VIDEOPLAYER_EXEC) : common/usr/bin/videoplayer.m4
common/usr/share/applications/variables.m4 : branding
common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop : common/usr/share/applications/desktopeditors.desktop.m4 common/usr/share/applications/variables.m4
common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop : common/usr/share/applications/imageviewer.desktop.m4 common/usr/share/applications/variables.m4
common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop : common/usr/share/applications/videoplayer.desktop.m4 common/usr/share/applications/variables.m4
common/usr/bin/desktopeditors : common/usr/bin/$(DESKTOPEDITORS_EXEC)
common/usr/bin/$(COMPANY_NAME_LOW) : common/usr/bin/$(DESKTOPEDITORS_EXEC)

$(BIN_FILES) $(DESKTOP_FILES) :
	m4 -I'common/usr/share/applications' $(M4_PARAMS) $< > $@

$(SYMLINK) :
	ln -srf $< $@

% : %.m4
	m4 $(M4_PARAMS)	$< > $@

%/$(PACKAGE_NAME).spec : %/package.spec 
	cp $< $@

.PHONY: all tar deb deb-astra rpm apt-rpm suse-rpm urpmi clean desktop branding
