ifeq ($(COMPANY_NAME), onlyoffice)
PACKAGE_NAME := $(COMPANY_NAME)-$(PRODUCT_NAME)
else
PACKAGE_NAME := $(COMPANY_NAME)
endif

PACKAGE_VERSION := $(PRODUCT_VERSION)-$(BUILD_NUMBER)

RPM_ARCH = x86_64
DEB_ARCH = amd64

RPM_BUILD_DIR = $(PWD)/rpm/builddir
DEB_BUILD_DIR = $(PWD)/
APT_RPM_BUILD_DIR = $(PWD)/apt-rpm/builddir

RPM_PACKAGE_DIR = $(RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)
DEB_PACKAGE_DIR = $(DEB_BUILD_DIR)
APT_RPM_PACKAGE_DIR = $(APT_RPM_BUILD_DIR)/RPMS/$(RPM_ARCH)

REPO_NAME := repo
REPO = $(PWD)/$(REPO_NAME)

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

RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
DEB = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb

PROGRAM_PATH := common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)

ifeq ($(COMPANY_NAME), onlyoffice)
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/desktopeditors
else
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/$(COMPANY_NAME)
endif

RPM_DEPENDENCY += desktopeditor
RPM_DEPENDENCY += rpm-version
RPM_DEPENDENCY += $(RPM)

DEB_DEPENDENCY += desktopeditor
DEB_DEPENDENCY += deb-version
DEB_DEPENDENCY += $(DEB)

TAR_DEPENDENCY += tar-desktopeditor
TAR_DEPENDENCY += $(TAR)

USR_LIB_X64 := /usr/lib/x86_64-linux-gnu

ifeq ($(PACKAGE_EDITION), cool)
BILD_PATH := ../../../../core/build/linux_desktop/desktop/*
PORTABLE_BILD_PATH := ../../../../core/build/linux_desktop/desktop_full_modern/*
else
BILD_PATH := ../../../../core/build/linux_desktop/desktop_full/*
PORTABLE_BILD_PATH := $(BILD_PATH)
endif

TAR_PROGRAM_PATH := desktopeditors
TAR := $(PACKAGE_NAME)-$(PACKAGE_VERSION)-x64.tar.gz

#QT_LIBS := $(shell qmake -query QT_INSTALL_LIBS)
#QT_PLUGINS := $(shell qmake -query QT_INSTALL_PLUGINS)

PLATFORM := linux_64_static_stdlib

.PHONY: all clean rpm deb apt-rpm tar deploy deploy-rpm deploy-deb deploy-apt-rpm desktopeditor tar-desktopeditor rpm-version deb-version apt-rpm-version qt-redist libstdc++-addon libc-addon

all: rpm deb apt-rpm tar

rpm: $(RPM_DEPENDENCY)

deb: $(DEB_DEPENDENCY)

tar: $(TAR_DEPENDENCY)

apt-rpm: desktopeditor apt-rpm-version $(APT_RPM)

clean:
	rm -rfv $(DEB_PACKAGE_DIR)/*.deb\
		$(DEB_PACKAGE_DIR)/*.changes\
		$(RPM_BUILD_DIR)\
		$(APT_RPM_BUILD_DIR)\
		$(REPO)\
		$(TAR_PROGRAM_PATH)\
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

rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i rpm/$(PACKAGE_NAME).spec

ifeq ($(PACKAGE_EDITION), full)
	sed '/^Requires.*/d'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/^#Full\(Requires.*\)/\1/'  -i rpm/$(PACKAGE_NAME).spec
endif

deb-version:
	cp -rfp common/$(COMPANY_NAME)/* deb/$(PACKAGE_NAME)/

	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/post*
	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/pre*
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
	sed 's/{{PACKAGE_VERSION}}/'$(PACKAGE_VERSION)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control

ifeq ($(PACKAGE_EDITION), full)
	sed '/^Depends.*/d'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
	sed 's/^#Full\(Depends.*\)/\1/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
endif

apt-rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i apt-rpm/$(PACKAGE_NAME).spec

ifeq ($(PACKAGE_EDITION), full)
	sed '/^Requires.*/d'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/^#Full\(Requires.*\)/\1/'  -i apt-rpm/$(PACKAGE_NAME).spec
endif

$(RPM):
	cd rpm && rpmbuild -bb --define "_topdir $(RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

$(DEB):
	fakeroot dpkg-deb --build deb/$(PACKAGE_NAME) && mv deb/$(PACKAGE_NAME).deb $(DEB)

$(APT_RPM):
	cd apt-rpm && rpmbuild -bb --define "_topdir $(APT_RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

$(TAR):
	tar -czf $@ --owner=root --group=root  -C $(TAR_PROGRAM_PATH)/.. desktopeditors

deploy-rpm: $(RPM)
	rm -rfv $(REPO)
	mkdir -p $(REPO)

	cp -rv $(RPM) $(REPO);
	createrepo -v $(REPO);

	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/ --acl public-read --delete
	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(RPM_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/ --acl public-read --delete

deploy-deb: $(DEB)
	rm -rfv $(REPO)
	mkdir -p $(REPO)

	cp -rv $(DEB) $(REPO);
	dpkg-scanpackages -m $(REPO_NAME) /dev/null | gzip -9c > $(REPO)/Packages.gz

	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/$(REPO_NAME) --acl public-read --delete
	aws s3 sync $(REPO) s3://$(REPO_NAME)-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/$(REPO_NAME) --acl public-read --delete

deploy-apt-rpm: $(APT_RPM)
	rm -rfv $(REPO)
	mkdir -p $(REPO)

	for i in i586 i686 x86_64 noarch; do \
		mkdir -p "$(REPO)/$$i/base" "$(REPO)/$$i/RPMS.$(REPO_NAME)" ; \
	done

	cp -rv $(APT_RPM) $(REPO)/$(RPM_ARCH)/RPMS.$(REPO_NAME);

	genbasedir --topdir=$(REPO) $(RPM_ARCH) $(REPO_NAME)

	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/ --acl public-read --delete
	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/ --acl public-read --delete

deploy: deploy-rpm deploy-deb deploy-apt-rpm
