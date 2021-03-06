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
S3_BUCKET ?= repo-doc-onlyoffice-com
RELEASE_BRANCH ?= unstable

BRANDING_DIR ?= $(PWD)/branding

ifeq ($(COMPANY_NAME), ONLYOFFICE)
	PACKAGE_NAME := $(COMPANY_NAME_LOW)-$(PRODUCT_NAME_LOW)
else
	PACKAGE_NAME := $(COMPANY_NAME_LOW)
endif
PACKAGE_VERSION := $(PRODUCT_VERSION)-$(BUILD_NUMBER)

UNAME_M ?= $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
	RPM_ARCH = x86_64
	DEB_ARCH = amd64
	TAR_ARCH = x64
	URPMI_ARCH = x86_64
	APT_RPM_ARCH = x86_64
	ARCHITECTURE = 64
endif
ifneq ($(filter %86,$(UNAME_M)),)
	RPM_ARCH = i686
	DEB_ARCH = i386
	URPMI_ARCH = i586
	APT_RPM_ARCH = i586
	TAR_ARCH = x86
	ARCHITECTURE = 32
endif

RPM_BUILD_DIR = $(PWD)/rpm/builddir
DEB_BUILD_DIR = $(PWD)/deb
DEB_ASTRA_BUILD_DIR = $(PWD)/deb-astra
APT_RPM_BUILD_DIR = $(PWD)/apt-rpm/builddir
URPMI_BUILD_DIR = $(PWD)/urpmi/builddir
TAR_BUILD_DIR = $(PWD)/tar

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
DEB_PACKAGE_DIR = $(PWD)
DEB_ASTRA_PACKAGE_DIR = $(PWD)
APT_RPM_PACKAGE_DIR = $(APT_RPM_BUILD_DIR)/RPMS/$(APT_RPM_ARCH)
URPMI_PACKAGE_DIR = $(URPMI_BUILD_DIR)/RPMS/$(URPMI_ARCH)
TAR_PACKAGE_DIR = $(TAR_BUILD_DIR)/$(TAR_ARCH)

RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(APT_RPM_ARCH).rpm
URPMI = $(URPMI_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(URPMI_ARCH).rpm
DEB = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb
DEB_ASTRA = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)-astra_$(DEB_ARCH).deb
TAR = $(TAR_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(TAR_ARCH).tar.gz

RPM_URI       := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/centos/$(notdir $(RPM))
APT_RPM_URI   := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/altlinux/$(notdir $(APT_RPM))
URPMI_URI     := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/rosa/$(notdir $(URPMI))
DEB_URI       := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/ubuntu/$(notdir $(DEB))
DEB_ASTRA_URI := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/astra/$(notdir $(DEB_ASTRA))
TAR_URI       := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/linux/$(notdir $(TAR))

DEPLOY_JSON = deploy.json

ifeq ($(COMPANY_NAME), ONLYOFFICE)
	DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/$(PRODUCT_NAME_LOW)
	DESKTOPEDITORS_EXEC := $(PACKAGE_NAME)
else
	DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/desktopeditors
	MEDIAVIEWER_PREFIX := $(COMPANY_NAME_LOW)/mediaviewer
	DESKTOPEDITORS_EXEC := $(COMPANY_NAME_LOW)-desktopeditors
	IMAGEVIEWER_EXEC := $(COMPANY_NAME_LOW)-imageviewer
	VIDEOPLAYER_EXEC := $(COMPANY_NAME_LOW)-videoplayer
endif

LINUX_DEPS += common/usr/share/applications/variables.m4
BIN_FILES += common/usr/bin/$(DESKTOPEDITORS_EXEC)
DESKTOP_FILES += common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop
ifeq ($(COMPANY_NAME), ONLYOFFICE)
	SYMLINK += common/usr/bin/desktopeditors
else
	BIN_FILES += common/usr/bin/$(IMAGEVIEWER_EXEC)
	BIN_FILES += common/usr/bin/$(VIDEOPLAYER_EXEC)
	DESKTOP_FILES += common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop
	DESKTOP_FILES += common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop
	SYMLINK += common/usr/bin/$(COMPANY_NAME_LOW)
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.docx.desktop
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.xlsx.desktop
	DEB_DEPS += common/usr/share/flyfm/templates/desktopeditors.pptx.desktop
endif
LINUX_DEPS += $(BIN_FILES)
LINUX_DEPS += $(DESKTOP_FILES)
LINUX_DEPS += $(SYMLINK)

DEB_DEPS += deb/debian/changelog
DEB_DEPS += deb/debian/control
DEB_DEPS += deb/debian/postinst
DEB_DEPS += deb/debian/postrm
DEB_DEPS += deb/debian/prerm
DEB_DEPS += deb/debian/rules
DEB_DEPS += deb/debian/$(PACKAGE_NAME).dirs
DEB_DEPS += deb/debian/$(PACKAGE_NAME).install
DEB_DEPS += deb/debian/$(PACKAGE_NAME).links

DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.docx.desktop
DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.xlsx.desktop
DEB_ASTRA_DEPS += common/usr/share/flyfm/templates/desktopeditors.pptx.desktop
DEB_ASTRA_DEPS += deb-astra/debian/changelog
DEB_ASTRA_DEPS += deb-astra/debian/control
DEB_ASTRA_DEPS += deb-astra/debian/postinst
DEB_ASTRA_DEPS += deb-astra/debian/postrm
DEB_ASTRA_DEPS += deb-astra/debian/prerm
DEB_ASTRA_DEPS += deb-astra/debian/rules
DEB_ASTRA_DEPS += deb-astra/debian/$(PACKAGE_NAME).dirs
DEB_ASTRA_DEPS += deb-astra/debian/$(PACKAGE_NAME).install
DEB_ASTRA_DEPS += deb-astra/debian/$(PACKAGE_NAME).links

INSTALLER_DEPENDENCY += desktopeditor

USR_LIB_X64 := /usr/lib/x86_64-linux-gnu

SRC += ../../../../build_tools/out/linux_$(ARCHITECTURE)/$(DESKTOPEDITORS_PREFIX)
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	SRC += ../../../../build_tools/out/linux_$(ARCHITECTURE)/$(MEDIAVIEWER_PREFIX)
endif


INSTALLER := qt-installer/$(PACKAGE_NAME)-installer-$(TAR_ARCH)
#QT_LIBS := $(shell qmake -query QT_INSTALL_LIBS)
#QT_PLUGINS := $(shell qmake -query QT_INSTALL_PLUGINS)

PLATFORM := linux_64_static_stdlib

M4_PARAMS += -D M4_COMPANY_NAME='$(COMPANY_NAME)'
M4_PARAMS += -D M4_PRODUCT_NAME='$(PRODUCT_NAME)'
M4_PARAMS += -D M4_PUBLISHER_NAME='$(PUBLISHER_NAME)'
M4_PARAMS += -D M4_PUBLISHER_URL='$(PUBLISHER_URL)'
M4_PARAMS += -D M4_SUPPORT_MAIL='$(SUPPORT_MAIL)'
M4_PARAMS += -D M4_SUPPORT_URL='$(SUPPORT_URL)'
M4_PARAMS += -D M4_PACKAGE_NAME='$(PACKAGE_NAME)'
M4_PARAMS += -D M4_PACKAGE_VERSION='$(PACKAGE_VERSION)'
M4_PARAMS += -D M4_PACKAGE_EDITION='$(PACKAGE_EDITION)'
M4_PARAMS += -D M4_DESKTOPEDITORS_PREFIX='$(DESKTOPEDITORS_PREFIX)'
M4_PARAMS += -D M4_DESKTOPEDITORS_EXEC='$(DESKTOPEDITORS_EXEC)'
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	M4_PARAMS += -D M4_MEDIAVIEWER_PREFIX='$(MEDIAVIEWER_PREFIX)'
	M4_PARAMS += -D M4_IMAGEVIEWER_EXEC='$(IMAGEVIEWER_EXEC)'
	M4_PARAMS += -D M4_VIDEOPLAYER_EXEC='$(VIDEOPLAYER_EXEC)'
endif
M4_PARAMS += -D M4_DEB_ARCH='$(DEB_ARCH)'
ifdef ASTRALINUX_KEYS_DIR
	M4_PARAMS += -D M4_ASTRALINUX_KEYS_DIR='$(abspath $(ASTRALINUX_KEYS_DIR))'
endif

RPM_PARAMS += --define '_company_name $(COMPANY_NAME)'
RPM_PARAMS += --define '_product_name $(PRODUCT_NAME)'
RPM_PARAMS += --define '_publisher_name $(PUBLISHER_NAME)'
RPM_PARAMS += --define '_publisher_url $(PUBLISHER_URL)'
RPM_PARAMS += --define '_support_mail $(SUPPORT_MAIL)'
RPM_PARAMS += --define '_package_name $(PACKAGE_NAME)'
RPM_PARAMS += --define '_package_edition $(PACKAGE_EDITION)'
RPM_PARAMS += --define '_product_version $(PRODUCT_VERSION)'
RPM_PARAMS += --define '_build_number $(BUILD_NUMBER)'
RPM_PARAMS += --define '_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)'
RPM_PARAMS += --define '_desktopeditors_exec $(DESKTOPEDITORS_EXEC)'
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	RPM_PARAMS += --define '_mediaviewer_prefix $(MEDIAVIEWER_PREFIX)'
	RPM_PARAMS += --define '_imageviewer_exec $(IMAGEVIEWER_EXEC)'
	RPM_PARAMS += --define '_videoplayer_exec $(VIDEOPLAYER_EXEC)'
endif
RPM_PARAMS += --define '_binary_payload w7.xzdio'

.PHONY: all clean branding rpm deb deb-astra apt-rpm tar deploy deploy-deb deploy-deb-astra deploy-rpm deploy-apt-rpm deploy-urpmi deploy-tar desktopeditor qt-redist libstdc++-addon libc-addon

all: rpm deb apt-rpm urpmi tar 
#installer

rpm: $(RPM)

deb: $(DEB)

deb-astra: $(DEB_ASTRA)

tar: $(TAR)

apt-rpm: $(APT_RPM)

urpmi: $(URPMI)

installer: $(INSTALLER)

clean:
	rm -rfv \
		common/etc \
		common/opt \
		$(LINUX_DEPS) \
		$(DEB_DEPS) \
		$(DEB_BUILD_DIR)/debian/.debhelper \
		$(DEB_BUILD_DIR)/debian/files \
		$(DEB_BUILD_DIR)/debian/$(PACKAGE_NAME)* \
		$(DEB_PACKAGE_DIR)/*.deb \
		$(DEB_PACKAGE_DIR)/*.ddeb \
		$(DEB_PACKAGE_DIR)/*.buildinfo \
		$(DEB_PACKAGE_DIR)/*.changes \
		$(DEB_ASTRA_BUILD_DIR) \
		$(RPM_BUILD_DIR) \
		$(APT_RPM_BUILD_DIR) \
		$(URPMI_BUILD_DIR) \
		$(TAR_BUILD_DIR) \
		rpm/$(PACKAGE_NAME).spec \
		apt-rpm/$(PACKAGE_NAME).spec \
		urpmi/$(PACKAGE_NAME).spec \
		$(INSTALLER)\
		*.tar.gz\
		$(DEPLOY_JSON)

branding: $(BRANDING_DIR)
	cp -rf $(BRANDING_DIR)/* $(PWD)/

desktopeditor: branding $(LINUX_DEPS)
	mkdir -p common/opt
	cp -rft common/opt $(SRC)

	cp -rf ../../../common/package/license/agpl-3.0.htm common/opt/desktopeditors/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE common/opt/desktopeditors

	chmod 755 common/opt/desktopeditors/DesktopEditors
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	chmod 755 common/opt/mediaviewer/ImageViewer
	chmod 755 common/opt/mediaviewer/VideoPlayer
endif

qt-redist:
	cp -r -L -t common/opt/desktopeditors/ \
		$(QT_PLUGINS)/bearer \
		$(QT_PLUGINS)/imageformats \
		$(QT_PLUGINS)/platforms \
		$(QT_PLUGINS)/platformthemes \
		$(QT_PLUGINS)/printsupport \
		$(QT_LIBS)/libQt5Core.so.5 \
		$(QT_LIBS)/libQt5DBus.so.5 \
		$(QT_LIBS)/libQt5Gui.so.5 \
		$(QT_LIBS)/libQt5PrintSupport.so.5 \
		$(QT_LIBS)/libQt5Widgets.so.5 \
		$(QT_LIBS)/libQt5X11Extras.so.5 \
		$(QT_LIBS)/libQt5XcbQpa.so.5
		# $(USR_LIB_X64)/libicudata.so.52 \
		# $(USR_LIB_X64)/libicui18n.so.52 \
		# $(USR_LIB_X64)/libicuuc.so.52

$(RPM): desktopeditor rpm/$(PACKAGE_NAME).spec
	cd rpm && setarch $(RPM_ARCH) rpmbuild -bb \
		--define "_topdir $(RPM_BUILD_DIR)" \
		--define "_package_arch $(RPM_ARCH)" \
		$(RPM_PARAMS) \
		package.spec

$(DEB): desktopeditor $(DEB_DEPS)
	cd deb && dpkg-buildpackage -b -uc -us

$(DEB_ASTRA): desktopeditor $(DEB_ASTRA_DEPS)
	cd deb-astra && dpkg-buildpackage -b -uc -us

$(APT_RPM): desktopeditor apt-rpm/$(PACKAGE_NAME).spec
	cd apt-rpm && setarch $(APT_RPM_ARCH) rpmbuild -bb \
		--define "_topdir $(APT_RPM_BUILD_DIR)" \
		--define "_package_arch $(APT_RPM_ARCH)" \
		$(RPM_PARAMS) \
		package.spec

$(URPMI): desktopeditor urpmi/$(PACKAGE_NAME).spec
	cd urpmi && setarch $(URPMI_ARCH) rpmbuild -bb \
		--define "_topdir $(URPMI_BUILD_DIR)" \
		--define "_package_arch $(URPMI_ARCH)" \
		$(RPM_PARAMS) \
		package.spec

$(TAR): desktopeditor
	mkdir -p $(TAR_PACKAGE_DIR)
ifeq ($(COMPANY_NAME), ONLYOFFICE)
	tar -czf $@ --owner=root --group=root -C common/opt desktopeditors
else
	tar -czf $@ --owner=root --group=root -C common/opt desktopeditors mediaviewer
endif

$(INSTALLER): $(INSTALLER_DEPENDENCY)
	cd qt-installer && ./make-install.sh

deploy-rpm: $(RPM)
	aws s3 cp --no-progress --acl public-read \
		$(RPM) s3://$(S3_BUCKET)/$(RPM_URI)

deploy-deb: $(DEB)
	aws s3 cp --no-progress --acl public-read \
		$(DEB) s3://$(S3_BUCKET)/$(DEB_URI)

deploy-deb-astra: $(DEB_ASTRA)
	aws s3 cp --no-progress --acl public-read \
		$(DEB_ASTRA) s3://$(S3_BUCKET)/$(DEB_ASTRA_URI)

deploy-apt-rpm: $(APT_RPM)
	aws s3 cp --no-progress --acl public-read \
		$(APT_RPM) s3://$(S3_BUCKET)/$(APT_RPM_URI)

deploy-urpmi: $(URPMI)
	aws s3 cp --no-progress --acl public-read \
		$(URPMI) s3://$(S3_BUCKET)/$(URPMI_URI)

deploy-tar: $(TAR)
	aws s3 cp --no-progress --acl public-read \
		$(TAR) s3://$(S3_BUCKET)/$(TAR_URI)

deploy-installer: $(INSTALLER)
	aws s3 cp $(INSTALLER) s3://$(S3_BUCKET)/$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/ --acl public-read       

common/usr/bin/$(DESKTOPEDITORS_EXEC) : common/usr/bin/desktopeditors.m4
common/usr/bin/$(IMAGEVIEWER_EXEC) : common/usr/bin/imageviewer.m4
common/usr/bin/$(VIDEOPLAYER_EXEC) : common/usr/bin/videoplayer.m4
common/usr/share/applications/variables.m4 : branding
common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop : common/usr/share/applications/desktopeditors.desktop.m4 common/usr/share/applications/variables.m4
common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop : common/usr/share/applications/imageviewer.desktop.m4 common/usr/share/applications/variables.m4
common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop : common/usr/share/applications/videoplayer.desktop.m4 common/usr/share/applications/variables.m4
common/usr/bin/desktopeditors : common/usr/bin/$(DESKTOPEDITORS_EXEC)
common/usr/bin/$(COMPANY_NAME_LOW) : common/usr/bin/$(DESKTOPEDITORS_EXEC)
deb/debian/$(PACKAGE_NAME).dirs : deb/debian/package.dirs.m4
deb/debian/$(PACKAGE_NAME).install : deb/debian/package.install.m4
deb/debian/$(PACKAGE_NAME).links : deb/debian/package.links.m4

$(BIN_FILES) $(DESKTOP_FILES) deb/debian/$(PACKAGE_NAME).dirs deb/debian/$(PACKAGE_NAME).install deb/debian/$(PACKAGE_NAME).links :
	m4 -I'common/usr/share/applications' $(M4_PARAMS) $< > $@

deb-astra/debian/changelog : deb/debian/changelog.m4 branding
deb-astra/debian/control : deb/debian/control.m4 branding
deb-astra/debian/$(PACKAGE_NAME).dirs : deb/debian/package.dirs.m4 branding
deb-astra/debian/$(PACKAGE_NAME).install : deb/debian/package.install.m4 branding
deb-astra/debian/$(PACKAGE_NAME).links : deb/debian/package.links.m4 branding
deb-astra/debian/postinst : deb/debian/postinst.m4 branding
deb-astra/debian/postrm : deb/debian/postrm.m4 branding
deb-astra/debian/prerm : deb/debian/prerm.m4 branding
deb-astra/debian/rules.m4 : branding

deb-astra/debian/changelog deb-astra/debian/control deb-astra/debian/$(PACKAGE_NAME).dirs deb-astra/debian/$(PACKAGE_NAME).install deb-astra/debian/$(PACKAGE_NAME).links deb-astra/debian/postinst deb-astra/debian/postrm deb-astra/debian/prerm :
	m4 $(M4_PARAMS) -D M4_PACKAGE_VERSION='$(PACKAGE_VERSION)-astra' $< > $@

$(SYMLINK) :
	ln -srf $< $@

% : %.m4
	m4 $(M4_PARAMS)	$< > $@

%/$(PACKAGE_NAME).spec : %/package.spec 
	cp $< $@

comma := ,
json_edit = cp -f $(1) $(1).tmp; jq $(2) $(1).tmp > $(1); rm -f $(1).tmp

$(DEPLOY_JSON):
	echo '{}' > $@
	$(call json_edit, $@, '. + { \
		product:  "$(PRODUCT_NAME_LOW)"$(comma) \
		version:  "$(PRODUCT_VERSION)"$(comma) \
		build:    "$(BUILD_NUMBER)" \
	}')
	$(call json_edit, $@, '.items += [{ \
		platform: "ubuntu"$(comma) \
		title:    "Debian 8 9 10$(comma) Ubuntu 14 16 18 20 and derivatives"$(comma) \
		path:     "$(DEB_URI)" \
	}]')
ifneq ($(COMPANY_NAME), ONLYOFFICE)
	$(call json_edit, $@, '.items += [{ \
		platform: "astralinux"$(comma) \
		title:    "Astra Linux Smolenks 1.6"$(comma) \
		path:     "$(DEB_ASTRA_URI)" \
	}]')
endif
	$(call json_edit, $@, '.items += [{ \
		platform: "centos"$(comma) \
		title:    "Centos 7$(comma) Redhat 7$(comma) Fedora latest and derivatives"$(comma) \
		path:     "$(RPM_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "altlinux"$(comma) \
		title:    "Altlinux p8 p9"$(comma) \
		path:     "$(APT_RPM_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "rosa"$(comma) \
		title:    "Rosa p8"$(comma) \
		path:     "$(URPMI_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "linux"$(comma) \
		title:    "Linux portable"$(comma) \
		path:     "$(TAR_URI)" \
	}]')

DEPLOY_TARGETS += deploy-rpm deploy-deb deploy-apt-rpm deploy-urpmi deploy-tar
ifneq ($(COMPANY_NAME), ONLYOFFICE)
DEPLOY_TARGETS += deploy-deb-astra
endif

deploy: $(DEPLOY_TARGETS) $(DEPLOY_JSON)
#deploy-installer
