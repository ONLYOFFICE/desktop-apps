COMPANY_NAME := onlyoffice
PRODUCT_NAME := desktopeditors

ifeq ($(COMPANY_NAME), onlyoffice)
PACKAGE_NAME := $(COMPANY_NAME)-$(PRODUCT_NAME)
else
PACKAGE_NAME := $(COMPANY_NAME)
endif

PRODUCT_VERSION := 1.1.2
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

RPM_REPO_OS_NAME = centos
RPM_REPO_OS_VER = 7
RPM_REPO_DIR = $(RPM_REPO_OS_NAME)/$(RPM_REPO_OS_VER)

DEB_REPO_OS_NAME = ubuntu
DEB_REPO_OS_VER = trusty
DEB_REPO_DIR = $(DEB_REPO_OS_NAME)/$(DEB_REPO_OS_VER)

ALT_REPO_OS_NAME = altlinux
ALT_REPO_OS_VER = centaurus
ALT_REPO_DIR = $(ALT_REPO_OS_NAME)/$(ALT_REPO_OS_VER)

RPM = $(RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
APT_RPM = $(APT_RPM_PACKAGE_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION).$(RPM_ARCH).rpm
DEB = $(DEB_PACKAGE_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb

PROGRAM_PATH := common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)
FONTS_PATH := common/$(COMPANY_NAME)/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)
LOCAL_PATH := common/$(COMPANY_NAME)/var/lib/$(COMPANY_NAME)/$(PRODUCT_NAME)/webdata/local

ifeq ($(COMPANY_NAME), onlyoffice)
INDEX_HTML := index.html
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/desktopeditors
else
INDEX_HTML := index.$(COMPANY_NAME).html
START_SCRIPT := common/$(COMPANY_NAME)/usr/bin/$(COMPANY_NAME)
endif

RPM_DEPENDENCY += desktopeditor
RPM_DEPENDENCY += rpm-version
ifeq ($(RPM_REPO_OS_VER), 6)
RPM_DEPENDENCY += libstdc++-addon
RPM_DEPENDENCY += libc-addon
endif
RPM_DEPENDENCY += $(RPM)

DEB_DEPENDENCY += desktopeditor
DEB_DEPENDENCY += deb-version
ifeq ($(DEB_REPO_OS_VER), precise)
DEB_DEPENDENCY += libstdc++-addon
endif
DEB_DEPENDENCY += $(DEB)

LIB_X64 := /lib/x86_64-linux-gnu
USR_LIB_X64 := /usr/lib/x86_64-linux-gnu

QT_LIBS := $(shell qmake -query QT_INSTALL_LIBS)
QT_PLUGINS := $(shell qmake -query QT_INSTALL_PLUGINS)

PLATFORM := linux_64_static_stdlib

.PHONY: all clean rpm deb apt-rpm deploy deploy-rpm deploy-deb deploy-apt-rpm desktopeditor rpm-version deb-version apt-rpm-version qt-redist libstdc++-addon libc-addon

all: rpm deb apt-rpm

rpm: $(RPM_DEPENDENCY)

deb: $(DEB_DEPENDENCY)

apt-rpm: desktopeditor apt-rpm-version $(APT_RPM)

clean:
	rm -rfv $(DEB_PACKAGE_DIR)/*.deb\
		$(DEB_PACKAGE_DIR)/*.changes\
		$(RPM_BUILD_DIR)\
		$(REPO)

desktopeditor:
	cp -rf ../../../../core/core/linux_desktop/desktop/* $(PROGRAM_PATH)	

	cp -rf ../../../common/package/license/eula_$(COMPANY_NAME).htm $(PROGRAM_PATH)/LICENSE.htm
	cp -rf ../../../common/package/license/3dparty/3DPARTYLICENSE $(PROGRAM_PATH)

	mkdir -p $(LOCAL_PATH)
	cp -f ../../../common/loginpage/deploy/$(INDEX_HTML) $(LOCAL_PATH)/index.html

	mkdir -p $(LOCAL_PATH)/fonts
	cp  ../../../common/package/fonts/LICENSE.txt $(LOCAL_PATH)/fonts
	cp  ../../../common/package/fonts/OpenSans-*.ttf $(LOCAL_PATH)/fonts

	mkdir -p $(FONTS_PATH)
	cp  ../../../common/package/fonts/Carlito-*.ttf $(FONTS_PATH)
	cp  ../../../common/package/fonts/Asana-Math.ttf $(FONTS_PATH)

	chmod 777 $(START_SCRIPT)

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

libstdc++-addon:
	#cp -t $(PROGRAM_PATH)/ \
		#$(USR_LIB_X64)/libffi.so.6 \
		#$(USR_LIB_X64)/libgio-2.0.so.0 \
		#$(LIB_X64)/libglib-2.0.so.0 \
		#$(USR_LIB_X64)/libgmodule-2.0.so.0 \
		#$(USR_LIB_X64)/libgobject-2.0.so.0 \
		#$(LIB_X64)/libpcre.so.3 \
		#$(USR_LIB_X64)/libstdc++.so.6

	sed 's/libstdc++6 (>=4.8),//'  -i deb/$(PACKAGE_NAME)/DEBIAN/control  
	sed 's/libstdc++ >= 4.8.0,//'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/libstdc++ >= 4.8.0,//'  -i apt-rpm/$(PACKAGE_NAME).spec

libc-addon:
	#cp -t $(PROGRAM_PATH)/ \
		#$(LIB_X64)/ld-linux-x86-64.so.2 \
		#$(LIB_X64)/libc.so.6

rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i rpm/$(PACKAGE_NAME).spec

ifeq ($(RPM_REPO_OS_VER), 6)
	rm -rf $(PROGRAM_PATH)/platformthemes/
endif

deb-version:
	cp -rfp common/$(COMPANY_NAME)/* deb/$(PACKAGE_NAME)/

	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/post*
	chmod 755 deb/$(PACKAGE_NAME)/DEBIAN/pre*
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control
	sed 's/{{PACKAGE_VERSION}}/'$(PACKAGE_VERSION)'/'  -i deb/$(PACKAGE_NAME)/DEBIAN/control

apt-rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i apt-rpm/$(PACKAGE_NAME).spec

$(RPM):
	cd rpm && rpmbuild -bb --define "_topdir $(RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

$(DEB):
	fakeroot dpkg-deb --build deb/$(PACKAGE_NAME) && mv deb/$(PACKAGE_NAME).deb $(DEB)

$(APT_RPM):
	cd apt-rpm && rpmbuild -bb --define "_topdir $(APT_RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

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