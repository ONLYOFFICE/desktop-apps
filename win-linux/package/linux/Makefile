COMPANY_NAME := onlyoffice
PRODUCT_NAME := desktopeditors
PACKAGE_NAME := $(COMPANY_NAME)-$(PRODUCT_NAME)
PRODUCT_VERSION := 0.2.3
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

DOCUMENTSERVER = common/documentserver

ifeq ($(COMPANY_NAME), onlyoffice)
INDEX_HTML := index.html
else
INDEX_HTML := index.$(COMPANY_NAME).html
endif

.PHONY: all clean rpm deb apt-rpm deploy deploy-rpm deploy-deb deploy-apt-rpm desktopeditor rpm-version deb-version apt-rpm-version

all: rpm apt-rpm deb

rpm: desktopeditor rpm-version $(RPM)

deb: desktopeditor deb-version $(DEB)

apt-rpm: desktopeditor apt-rpm-version $(APT_RPM)

clean:
	rm -rfv $(DEB_PACKAGE_DIR)/*.deb\
		$(DEB_PACKAGE_DIR)/*.changes\
		$(RPM_BUILD_DIR)\
		$(REPO)

desktopeditor:
	mkdir -p common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter
	cp -rf ../../../common/converter/linux/* common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter

	cp -rf ../../../common/package/dictionaries common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/
	cp -rf ../../../common/editors common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/

	#rm -f ../../../win-linux/install/data/libs/qt/linux64/libascdocumentscore.so 
	#cp -rf ../../../win-linux/install/data/libs/qt/linux64/* common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/


	mkdir -p common/$(COMPANY_NAME)/var/lib/$(COMPANY_NAME)/$(PRODUCT_NAME)/webdata/local
	cp -f ../../../common/loginpage/deploy/$(INDEX_HTML) common/$(COMPANY_NAME)/var/lib/$(COMPANY_NAME)/$(PRODUCT_NAME)/webdata/local/index.html

	mkdir -p common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local/fonts
	cp  ../../../common/package/fonts/OpenSans-*.ttf common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local/fonts

	mkdir -p common/$(COMPANY_NAME)/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)
	cp  ../../../common/package/fonts/Carlito-*.ttf common/$(COMPANY_NAME)/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)
	cp  ../../../common/package/fonts/Asana-Math.ttf common/$(COMPANY_NAME)/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)

	chmod 777 common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/DesktopEditors
	chmod 777 common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter/HtmlFileInternal
	chmod 777 common/$(COMPANY_NAME)/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter/x2t

	chmod 777 common/$(COMPANY_NAME)/usr/bin/desktopeditors

rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i rpm/$(PACKAGE_NAME).spec

deb-version:
	cp -rfp common/$(COMPANY_NAME)/* deb/

	chmod 755 deb/DEBIAN/post*
	chmod 755 deb/DEBIAN/pre*
	#sed 's/{{PACKAGE_VERSION}}/'$(PACKAGE_VERSION)'/'  -i deb/$(PACKAGE_NAME)/debian/changelog

apt-rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i apt-rpm/$(PACKAGE_NAME).spec

$(RPM):
	cd rpm && rpmbuild -bb --define "_topdir $(RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

$(DEB):
	fakeroot dpkg-deb --build deb/$(PACKAGE_NAME) && mv $(PACKAGE_NAME).deb $(DEB)

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