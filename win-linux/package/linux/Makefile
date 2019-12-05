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

BRANDING_DIR ?= $(PWD)/branding

ifneq ($(COMPANY_NAME), R7-Office)
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
APT_RPM_BUILD_DIR = $(PWD)/apt-rpm/builddir
URPMI_BUILD_DIR = $(PWD)/urpmi/builddir
TAR_BUILD_DIR = $(PWD)/tar

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
DEB_PACKAGE_DIR = $(DEB_BUILD_DIR)/$(DEB_ARCH)
APT_RPM_PACKAGE_DIR = $(APT_RPM_BUILD_DIR)/RPMS/$(APT_RPM_ARCH)
URPMI_PACKAGE_DIR = $(URPMI_BUILD_DIR)/RPMS/$(URPMI_ARCH)
TAR_PACKAGE_DIR = $(TAR_BUILD_DIR)/$(TAR_ARCH)

REPO_NAME := repo

DEB_REPO := $(PWD)/$(REPO_NAME)
RPM_REPO := $(PWD)/rpm-$(REPO_NAME)
APT_RPM_REPO := $(PWD)/apt-rpm-$(REPO_NAME)
URPMI_REPO := $(PWD)/urpmi-$(REPO_NAME)

RPM_REPO_OS_NAME := centos

ifeq ($(PACKAGE_EDITION), cool)
RPM_REPO_OS_VER := 7
else
RPM_REPO_OS_VER := 6
endif
RPM_REPO_DIR = $(RPM_REPO_OS_NAME)/$(RPM_REPO_OS_VER)

DEB_REPO_OS_NAME := ubuntu

ifeq ($(PACKAGE_EDITION), cool)
DEB_REPO_OS_VER := trusty
else
DEB_REPO_OS_VER := precise
endif
DEB_REPO_DIR = $(DEB_REPO_OS_NAME)/$(DEB_REPO_OS_VER)

ALT_REPO_OS_NAME = ALTLinux

ifeq ($(PACKAGE_EDITION), cool)
ALT_REPO_OS_VER = p8
else
ALT_REPO_OS_VER = p7
endif
ALT_REPO_DIR = $(ALT_REPO_OS_NAME)/$(ALT_REPO_OS_VER)

ROSA_REPO_OS_NAME = rosa

ifeq ($(PACKAGE_EDITION), cool)
ROSA_REPO_OS_VER = r9
else
ROSA_REPO_OS_VER = r8
endif
ROSA_REPO_DIR = $(ROSA_REPO_OS_NAME)/$(ROSA_REPO_OS_VER)

OTHER_REPO_OS_NAME = other-linux

ifeq ($(PACKAGE_EDITION), cool)
OTHER_REPO_OS_VER = modern
else
OTHER_REPO_OS_VER = old
endif

OTHER_REPO_DIR = $(OTHER_REPO_OS_NAME)/$(OTHER_REPO_OS_VER)

RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(APT_RPM_ARCH).rpm
URPMI = $(URPMI_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(URPMI_ARCH).rpm
DEB = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb

ifneq ($(COMPANY_NAME), R7-Office)
	DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/$(PRODUCT_NAME_LOW)
	DESKTOPEDITORS_EXEC := $(PACKAGE_NAME)
else
	DESKTOPEDITORS_PREFIX := $(COMPANY_NAME_LOW)/desktopeditors
	MEDIAVIEWER_PREFIX := $(COMPANY_NAME_LOW)/mediaviewer
	DESKTOPEDITORS_EXEC := $(COMPANY_NAME_LOW)-desktopeditors
	IMAGEVIEWER_EXEC := $(COMPANY_NAME_LOW)-imageviewer
	VIDEOPLAYER_EXEC := $(COMPANY_NAME_LOW)-videoplayer
endif

LINUX_DEPS += common/usr/bin/$(DESKTOPEDITORS_EXEC)
LINUX_DEPS += common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop
ifneq ($(COMPANY_NAME), R7-Office)
	LINUX_DEPS += common/usr/bin/$(PRODUCT_NAME_LOW)
else
	LINUX_DEPS += common/usr/bin/$(COMPANY_NAME_LOW)
	LINUX_DEPS += common/usr/bin/$(IMAGEVIEWER_EXEC)
	LINUX_DEPS += common/usr/bin/$(VIDEOPLAYER_EXEC)
	LINUX_DEPS += common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop
	LINUX_DEPS += common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop
endif

DEB_DEPS += deb/debian/changelog
DEB_DEPS += deb/debian/control
DEB_DEPS += deb/debian/postinst
DEB_DEPS += deb/debian/postrm
DEB_DEPS += deb/debian/prerm
DEB_DEPS += deb/debian/rules
DEB_DEPS += deb/debian/$(PACKAGE_NAME).install

INSTALLER_DEPENDENCY += desktopeditor

USR_LIB_X64 := /usr/lib/x86_64-linux-gnu

SRC += ../../../../build_tools/out/linux_$(ARCHITECTURE)/$(DESKTOPEDITORS_PREFIX)
ifeq ($(COMPANY_NAME), R7-Office)
	SRC += ../../../../build_tools/out/linux_$(ARCHITECTURE)/$(MEDIAVIEWER_PREFIX)
endif

TAR = $(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(TAR_ARCH).tar.gz

INSTALLER := qt-installer/$(PACKAGE_NAME)-installer-$(TAR_ARCH)
#QT_LIBS := $(shell qmake -query QT_INSTALL_LIBS)
#QT_PLUGINS := $(shell qmake -query QT_INSTALL_PLUGINS)

PLATFORM := linux_64_static_stdlib

M4_PARAMS += -D M4_COMPANY_NAME="$(COMPANY_NAME)"
M4_PARAMS += -D M4_PRODUCT_NAME="$(PRODUCT_NAME)"
M4_PARAMS += -D M4_PACKAGE_NAME="$(PACKAGE_NAME)"
M4_PARAMS += -D M4_PACKAGE_VERSION="$(PACKAGE_VERSION)"
M4_PARAMS += -D M4_PACKAGE_EDITION="$(PACKAGE_EDITION)"
M4_PARAMS += -D M4_DESKTOPEDITORS_PREFIX="$(DESKTOPEDITORS_PREFIX)"
M4_PARAMS += -D M4_DESKTOPEDITORS_EXEC="$(DESKTOPEDITORS_EXEC)"
ifeq ($(COMPANY_NAME), R7-Office)
	M4_PARAMS += -D M4_MEDIAVIEWER_PREFIX="$(MEDIAVIEWER_PREFIX)"
	M4_PARAMS += -D M4_IMAGEVIEWER_EXEC="$(IMAGEVIEWER_EXEC)"
	M4_PARAMS += -D M4_VIDEOPLAYER_EXEC="$(VIDEOPLAYER_EXEC)"
endif
M4_PARAMS += -D M4_DEB_ARCH="$(DEB_ARCH)"
M4_PARAMS += -D M4_PUBLISHER_NAME="$(PUBLISHER_NAME)"
M4_PARAMS += -D M4_PUBLISHER_URL="$(PUBLISHER_URL)"
M4_PARAMS += -D M4_SUPPORT_MAIL="$(SUPPORT_MAIL)"
M4_PARAMS += -D M4_SUPPORT_URL="$(SUPPORT_URL)"

.PHONY: all clean deb rpm apt-rpm tar deploy deploy-deb deploy-rpm deploy-apt-rpm deploy-urpmi deploy-tar desktopeditor qt-redist libstdc++-addon libc-addon

all: deb rpm apt-rpm urpmi tar 
#installer

rpm: $(RPM)

deb: $(DEB)

tar: $(TAR)

apt-rpm: $(APT_RPM)

urpmi: $(URPMI)

installer: $(INSTALLER)

clean:
	rm -rfv \
		common/opt \
		$(LINUX_DEPS) \
		$(DEB_DEPS) \
		$(DEB_BUILD_DIR)/debian/.debhelper \
		$(DEB_BUILD_DIR)/debian/$(PACKAGE_NAME) \
		$(DEB_BUILD_DIR)/debian/files \
		$(DEB_BUILD_DIR)/debian/*.debhelper.log \
		$(DEB_BUILD_DIR)/debian/*.substvars \
		$(DEB_PACKAGE_DIR) \
		$(RPM_BUILD_DIR) \
		$(APT_RPM_BUILD_DIR) \
		$(URPMI_BUILD_DIR) \
		$(TAR_BUILD_DIR) \
		$(DEB_REPO)\
		$(RPM_REPO)\
		$(APT_RPM_REPO)\
		$(URPMI_REPO)\
		$(INSTALLER)\
		*.tar.gz\
		index.html

desktopeditor: $(LINUX_DEPS)
	mkdir -p common/opt
	cp -rft common/opt $(SRC)

	cp -rf $(BRANDING_DIR)/* $(PWD)/
	cp -rf ../../../common/package/license/agpl-3.0.htm common/opt/desktopeditors/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE common/opt/desktopeditors

	chmod 755 common/opt/desktopeditors/DesktopEditors
ifeq ($(COMPANY_NAME), R7-Office)
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

$(RPM): desktopeditor
	cd rpm && setarch $(RPM_ARCH) rpmbuild \
		-bb \
		--define "_topdir $(RPM_BUILD_DIR)" \
		--define "_company_name $(COMPANY_NAME)" \
		--define "_product_name $(PRODUCT_NAME)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(RPM_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		--define "_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)" \
		--define "_publisher_name $(PUBLISHER_NAME)" \
		--define "_publisher_url $(PUBLISHER_URL)" \
		--define "_support_mail $(SUPPORT_MAIL)" \
		package.spec

$(DEB): desktopeditor $(DEB_DEPS)
	cd deb && dpkg-buildpackage -b --no-sign
	mkdir -p $(DEB_PACKAGE_DIR)
	mv -f *.deb *.ddeb *.buildinfo *.changes -t $(DEB_PACKAGE_DIR)

$(APT_RPM): desktopeditor
	cd apt-rpm && setarch $(APT_RPM_ARCH) rpmbuild \
		-bb \
		--define "_topdir $(APT_RPM_BUILD_DIR)" \
		--define "_company_name $(COMPANY_NAME)" \
		--define "_product_name $(PRODUCT_NAME)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(APT_RPM_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		--define "_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)" \
		--define "_publisher_name $(PUBLISHER_NAME)" \
		--define "_publisher_url $(PUBLISHER_URL)" \
		--define "_support_mail $(SUPPORT_MAIL)" \
		package.spec

$(URPMI): desktopeditor
	cd urpmi && setarch $(URPMI_ARCH) rpmbuild \
		-bb \
		--define "_topdir $(URPMI_BUILD_DIR)" \
		--define "_company_name $(COMPANY_NAME)" \
		--define "_product_name $(PRODUCT_NAME)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(URPMI_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		--define "_desktopeditors_prefix $(DESKTOPEDITORS_PREFIX)" \
		--define "_publisher_name $(PUBLISHER_NAME)" \
		--define "_publisher_url $(PUBLISHER_URL)" \
		--define "_support_mail $(SUPPORT_MAIL)" \
		package.spec

$(TAR): desktopeditor
	cd tar && tar -czf $@ \
		--owner=root --group=root \
		--transform="s|home|"$(PACKAGE_NAME)"|" \
		-C ../$(DESKTOPEDITORS) home
	mkdir -p $(TAR_PACKAGE_DIR)
	mv -f tar/$@ -t $(TAR_PACKAGE_DIR)

$(INSTALLER): $(INSTALLER_DEPENDENCY)
	cd qt-installer && ./make-install.sh

deploy-rpm: $(RPM)
	rm -rfv $(RPM_REPO)
	mkdir -p $(RPM_REPO)

	cp -rv $(RPM) $(RPM_REPO);
	createrepo -v $(RPM_REPO);

	aws s3 sync $(RPM_REPO) s3://$(S3_BUCKET)/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(RPM_ARCH)/ --acl public-read --delete
	aws s3 sync $(RPM_REPO) s3://$(S3_BUCKET)/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(RPM_ARCH)/ --acl public-read --delete

deploy-deb: $(DEB)
	rm -rfv $(DEB_REPO)
	mkdir -p $(DEB_REPO)

	cp -rv $(DEB) $(DEB_REPO);
	dpkg-scanpackages -m $(REPO_NAME) /dev/null | gzip -9c > $(DEB_REPO)/Packages.gz

	aws s3 sync $(DEB_REPO) s3://$(S3_BUCKET)/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(DEB_ARCH)/$(REPO_NAME) --acl public-read --delete
	aws s3 sync $(DEB_REPO) s3://$(S3_BUCKET)/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(DEB_ARCH)/$(REPO_NAME) --acl public-read --delete

deploy-apt-rpm: $(APT_RPM)
	rm -rfv $(APT_RPM_REPO)
	mkdir -p $(APT_RPM_REPO)
	
	cp -rv $(APT_RPM) $(APT_RPM_REPO);
	#createrepo -v $(APT_RPM_REPO);
	
	#for i in i586 i686 x86_64 noarch; do \
	#	mkdir -p "$(URPMI_REPO)/$$i/base" "$(REPO)/$$i/RPMS.$(REPO_NAME)" ; \
	#done

	#cp -rv $(APT_RPM) $(URPMI_REPO)/$(APT_RPM_ARCH)/RPMS.$(REPO_NAME);

	#genbasedir --topdir=$(URPMI_REPO) $(APT_RPM_ARCH) $(REPO_NAME)

	aws s3 sync $(APT_RPM_REPO) s3://$(S3_BUCKET)/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(APT_RPM_ARCH)/ --acl public-read --delete
	aws s3 sync $(APT_RPM_REPO) s3://$(S3_BUCKET)/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(APT_RPM_ARCH)/ --acl public-read --delete

deploy-urpmi: $(URPMI)
	rm -rfv $(URPMI_REPO)
	mkdir -p $(URPMI_REPO)

	
	cp -rv $(URPMI) $(URPMI_REPO);
	#createrepo -v $(URPMI_REPO);

	#for i in i586 i686 x86_64 noarch; do \
	#	mkdir -p "$(URPMI_REPO)/$$i/base" "$(REPO)/$$i/RPMS.$(REPO_NAME)" ; \
	#done

	#cp -rv $(URPMI) $(URPMI_REPO)/$(URPMI_ARCH)/RPMS.$(REPO_NAME);

	#genbasedir --topdir=$(URPMI_REPO) $(URPMI_ARCH) $(REPO_NAME)

	aws s3 sync $(URPMI_REPO) s3://$(S3_BUCKET)/$(ROSA_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(URPMI_ARCH)/ --acl public-read --delete
	aws s3 sync $(URPMI_REPO) s3://$(S3_BUCKET)/$(ROSA_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(URPMI_ARCH)/ --acl public-read --delete

deploy-tar: $(TAR)
	aws s3 cp $(TAR) s3://$(S3_BUCKET)/$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/ --acl public-read

deploy-installer: $(INSTALLER)
	aws s3 cp $(INSTALLER) s3://$(S3_BUCKET)/$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/ --acl public-read       

M4_PARAMS += -D M4_S3_BUCKET=$(S3_BUCKET)
M4_PARAMS += -D M4_APT_RPM_URI="$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(APT_RPM_ARCH)/$(notdir $(APT_RPM))"
M4_PARAMS += -D M4_DEB_URI="$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(DEB_ARCH)/$(REPO_NAME)/$(notdir $(DEB))"
M4_PARAMS += -D M4_RPM_URI="$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(RPM_ARCH)/$(notdir $(RPM))"
M4_PARAMS += -D M4_TAR_URI="$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/$(notdir $(TAR))"
M4_PARAMS += -D M4_URPMI_URI="$(ROSA_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(URPMI_ARCH)/$(notdir $(URPMI))"

common/usr/bin/$(DESKTOPEDITORS_EXEC) : common/usr/bin/desktopeditors.m4
common/usr/bin/$(IMAGEVIEWER_EXEC) : common/usr/bin/imageviewer.m4
common/usr/bin/$(VIDEOPLAYER_EXEC) : common/usr/bin/videoplayer.m4
common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop : common/usr/share/applications/desktopeditors.desktop.m4
common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop : common/usr/share/applications/imageviewer.desktop.m4
common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop : common/usr/share/applications/videoplayer.desktop.m4
deb/debian/$(PACKAGE_NAME).install : deb/debian/package.install

common/usr/bin/$(DESKTOPEDITORS_EXEC) common/usr/bin/$(IMAGEVIEWER_EXEC) common/usr/bin/$(VIDEOPLAYER_EXEC) common/usr/share/applications/$(DESKTOPEDITORS_EXEC).desktop common/usr/share/applications/$(IMAGEVIEWER_EXEC).desktop common/usr/share/applications/$(VIDEOPLAYER_EXEC).desktop deb/debian/$(PACKAGE_NAME).install :
	m4 $(M4_PARAMS)	$< > $@

% : %.m4
	m4 $(M4_PARAMS)	$< > $@

common/usr/bin/$(PRODUCT_NAME_LOW) : common/usr/bin/$(DESKTOPEDITORS_EXEC)
common/usr/bin/$(COMPANY_NAME_LOW) : common/usr/bin/$(DESKTOPEDITORS_EXEC)

common/usr/bin/$(PRODUCT_NAME_LOW) common/usr/bin/$(COMPANY_NAME_LOW) :
	ln -srf $< $@

deploy: deploy-rpm deploy-deb deploy-apt-rpm deploy-urpmi deploy-tar index.html
#deploy-installer