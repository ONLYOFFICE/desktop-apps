COMPANY_NAME ?= onlyoffice
PRODUCT_NAME ?= desktopeditors
PRODUCT_VERSION ?= 0.0.0
BUILD_NUMBER ?= 0
PACKAGE_EDITION ?= cool

ifeq ($(COMPANY_NAME), onlyoffice)
PACKAGE_NAME := $(COMPANY_NAME)-$(PRODUCT_NAME)
else
PACKAGE_NAME := $(COMPANY_NAME)
endif

PACKAGE_VERSION := $(PRODUCT_VERSION)-$(BUILD_NUMBER)

UNAME_M ?= $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
	RPM_ARCH = x86_64
	DEB_ARCH = amd64
	TAR_ARCH = x64
	DESKTOP_SUFFIX = _64
endif
ifneq ($(filter %86,$(UNAME_M)),)
	RPM_ARCH = i686
	DEB_ARCH = i386
	TAR_ARCH = x86
	DESKTOP_SUFFIX = _32
endif

RPM_BUILD_DIR = $(PWD)/rpm/builddir
DEB_BUILD_DIR = $(PWD)/
APT_RPM_BUILD_DIR = $(PWD)/apt-rpm/builddir
URPMI_RPM_BUILD_DIR = $(PWD)/urpmi-rpm/builddir

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
DEB_PACKAGE_DIR = $(DEB_BUILD_DIR)
APT_RPM_PACKAGE_DIR = $(APT_RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
URPMI_RPM_PACKAGE_DIR = $(URPMI_RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)

REPO_NAME := repo

DEB_REPO := $(PWD)/$(REPO_NAME)
RPM_REPO := $(PWD)/rpm-$(REPO_NAME)
APT_RPM_REPO := $(PWD)/apt-rpm-$(REPO_NAME)
URPMI_RPM_REPO := $(PWD)/urpmi-rpm-$(REPO_NAME)

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
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
URPMI_RPM = $(URPMI_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
DEB = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb

PROGRAM_PATH := common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)

ifeq ($(COMPANY_NAME), onlyoffice)
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/desktopeditors
else
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/$(COMPANY_NAME)
endif

DEB_DEPENDENCY += deb-version

TAR_DEPENDENCY += tar-desktopeditor

INSTALLER_DEPENDENCY += desktopeditor

USR_LIB_X64 := /usr/lib/x86_64-linux-gnu

ifeq ($(PACKAGE_EDITION), cool)
BILD_PATH := ../../../../core/build/linux_desktop/desktop$(DESKTOP_SUFFIX)/*
PORTABLE_BILD_PATH := ../../../../core/build/linux_desktop/desktop_full_modern$(DESKTOP_SUFFIX)/*
else
BILD_PATH := ../../../../core/build/linux_desktop/desktop_full$(DESKTOP_SUFFIX)/*
PORTABLE_BILD_PATH := $(BILD_PATH)
endif

TAR_PROGRAM_PATH := desktopeditors
TAR := $(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(TAR_ARCH).tar.gz

INSTALLER := qt-installer/$(PACKAGE_NAME)-installer-$(TAR_ARCH)
#QT_LIBS := $(shell qmake -query QT_INSTALL_LIBS)
#QT_PLUGINS := $(shell qmake -query QT_INSTALL_PLUGINS)

PLATFORM := linux_64_static_stdlib

.PHONY: all clean rpm deb apt-rpm tar deploy deploy-rpm deploy-deb deploy-apt-rpm deploy-tar desktopeditor tar-desktopeditor rpm-version deb-version apt-rpm-version urpmi-rpm-version qt-redist libstdc++-addon libc-addon

all: rpm deb apt-rpm urpmi-rpm tar 
#installer

rpm: $(RPM)

deb: $(DEB)

tar: $(TAR)

apt-rpm: $(APT_RPM)

urpmi-rpm: $(URPMI_RPM)

installer: $(INSTALLER)

clean:
	rm -rfv $(DEB_PACKAGE_DIR)/*.deb\
		$(DEB_PACKAGE_DIR)/*.changes\
		$(RPM_BUILD_DIR)\
		$(APT_RPM_BUILD_DIR)\
		$(URPMI_RPM_BUILD_DIR)\
		$(DEB_REPO)\
		$(RPM_REPO)\
		$(APT_RPM_REPO)\
		$(URPMI_RPM_REPO)\
		$(TAR_PROGRAM_PATH)\
		$(INSTALLER)\
		*.tar.gz

desktopeditor:
	mkdir -p $(PROGRAM_PATH)
	cp -rf $(BILD_PATH) $(PROGRAM_PATH)

	cp -rf ../../../common/package/license/agpl-3.0.htm $(PROGRAM_PATH)/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE $(PROGRAM_PATH)
	
	chmod 755 $(START_SCRIPT)
	
tar-desktopeditor:
	mkdir -p $(TAR_PROGRAM_PATH)
	cp -rf $(PORTABLE_BILD_PATH) $(TAR_PROGRAM_PATH)

	cp -rf ../../../common/package/license/agpl-3.0.htm $(TAR_PROGRAM_PATH)/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE $(TAR_PROGRAM_PATH)

qt-redist:
	cp -r -L -t  $(PROGRAM_PATH)/ \
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

deb-version: desktopeditor
	cp -rfp common/$(COMPANY_NAME)/* deb/$(PACKAGE_NAME)/

	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/post*
	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/pre*
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
	sed 's/{{PACKAGE_VERSION}}/'$(PACKAGE_VERSION)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control

ifeq ($(PACKAGE_EDITION), full)
	sed '/^Depends.*/d'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
	sed 's/^#Full\(Depends.*\)/\1/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
endif

$(RPM): desktopeditor
	cd rpm && rpmbuild \
		-bb \
		--define "_topdir $(RPM_BUILD_DIR)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(RPM_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		$(PACKAGE_NAME).spec

$(DEB): $(DEB_DEPENDENCY)
	fakeroot dpkg-deb --build deb/$(PACKAGE_NAME) && mv deb/$(PACKAGE_NAME).deb $(DEB)

$(APT_RPM): desktopeditor
	cd apt-rpm && rpmbuild \
		-bb \
		--define "_topdir $(APT_RPM_BUILD_DIR)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(RPM_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		$(PACKAGE_NAME).spec

$(URPMI_RPM): desktopeditor
	cd urpmi-rpm && rpmbuild \
		-bb \
		--define "_topdir $(URPMI_RPM_BUILD_DIR)" \
		--define "_package_name $(PACKAGE_NAME)" \
		--define "_product_version $(PRODUCT_VERSION)" \
		--define "_build_number $(BUILD_NUMBER)" \
		--define "_package_arch $(RPM_ARCH)" \
		--define "_package_edition $(PACKAGE_EDITION)" \
		$(PACKAGE_NAME).spec

$(TAR): $(TAR_DEPENDENCY)
	tar -czf $@ --owner=root --group=root  -C $(TAR_PROGRAM_PATH)/.. desktopeditors

$(INSTALLER): $(INSTALLER_DEPENDENCY)
	cd qt-installer && ./make-install.sh

deploy-rpm: $(RPM)
	rm -rfv $(RPM_REPO)
	mkdir -p $(RPM_REPO)

	cp -rv $(RPM) $(RPM_REPO);
	createrepo -v $(RPM_REPO);

	aws s3 sync $(RPM_REPO) s3://repo-doc-onlyoffice-com/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/ --acl public-read --delete
	aws s3 sync $(RPM_REPO) s3://repo-doc-onlyoffice-com/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/ --acl public-read --delete

deploy-deb: $(DEB)
	rm -rfv $(DEB_REPO)
	mkdir -p $(DEB_REPO)

	cp -rv $(DEB) $(DEB_REPO);
	dpkg-scanpackages -m $(REPO_NAME) /dev/null | gzip -9c > $(DEB_REPO)/Packages.gz

	aws s3 sync $(DEB_REPO) s3://repo-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(DEB_ARCH)/$(REPO_NAME) --acl public-read --delete
	aws s3 sync $(DEB_REPO) s3://repo-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(DEB_ARCH)/$(REPO_NAME) --acl public-read --delete

deploy-apt-rpm: $(APT_RPM)
	rm -rfv $(APT_RPM_REPO)
	mkdir -p $(APT_RPM_REPO)
	
	cp -rv $(APT_RPM) $(APT_RPM_REPO);
	#createrepo -v $(APT_RPM_REPO);
	
	#for i in i586 i686 x86_64 noarch; do \
	#	mkdir -p "$(URPMI_RPM_REPO)/$$i/base" "$(REPO)/$$i/RPMS.$(REPO_NAME)" ; \
	#done

	#cp -rv $(APT_RPM) $(URPMI_RPM_REPO)/$(RPM_ARCH)/RPMS.$(REPO_NAME);

	#genbasedir --topdir=$(URPMI_RPM_REPO) $(RPM_ARCH) $(REPO_NAME)

	aws s3 sync $(APT_RPM_REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(RPM_ARCH)/ --acl public-read --delete
	aws s3 sync $(APT_RPM_REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(RPM_ARCH)/ --acl public-read --delete

deploy-urpmi-rpm: $(URPMI_RPM)
	rm -rfv $(URPMI_RPM_REPO)
	mkdir -p $(URPMI_RPM_REPO)

	
	cp -rv $(URPMI_RPM) $(URPMI_RPM_REPO);
	#createrepo -v $(URPMI_RPM_REPO);

	#for i in i586 i686 x86_64 noarch; do \
	#	mkdir -p "$(URPMI_RPM_REPO)/$$i/base" "$(REPO)/$$i/RPMS.$(REPO_NAME)" ; \
	#done

	#cp -rv $(URPMI_RPM) $(URPMI_RPM_REPO)/$(RPM_ARCH)/RPMS.$(REPO_NAME);

	#genbasedir --topdir=$(URPMI_RPM_REPO) $(RPM_ARCH) $(REPO_NAME)

	aws s3 sync $(URPMI_RPM_REPO) s3://repo-doc-onlyoffice-com/$(ROSA_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(RPM_ARCH)/ --acl public-read --delete
	aws s3 sync $(URPMI_RPM_REPO) s3://repo-doc-onlyoffice-com/$(ROSA_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(RPM_ARCH)/ --acl public-read --delete

deploy-tar: $(TAR)
	aws s3 cp $(TAR) s3://repo-doc-onlyoffice-com/$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/ --acl public-read

deploy-installer: $(INSTALLER)
	aws s3 cp $(INSTALLER) s3://repo-doc-onlyoffice-com/$(OTHER_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(TAR_ARCH)/ --acl public-read       

deploy: deploy-rpm deploy-deb deploy-apt-rpm deploy-urpmi-rpm deploy-tar 
#deploy-installer
