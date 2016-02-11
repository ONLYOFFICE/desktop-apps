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

REPO = $(PWD)/repo

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
	mkdir -p common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter
	cp -rf ../../../common/converter/linux/* common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/converter

	cp -rf ../../../common/package/dictionaries common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/
	cp -rf ../../../common/package/editors common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/
	cp -rf ../../../win-linux/install/data/libs/qt/linux64/platforms common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/

	mkdir -p common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local
	cp -rf ../../../common/loginpage/deploy/* common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local

	mkdir -p common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local/fonts
	cp  ../../../common/package/fonts/OpenSans-*.ttf common/opt/$(COMPANY_NAME)/$(PRODUCT_NAME)/local/fonts

	mkdir -p common/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)
	cp  ../../../common/package/fonts/Carlito-*.ttf common/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)
	cp  ../../../common/package/fonts/Asana-Math.ttf common/usr/share/fonts/truetype/$(COMPANY_NAME)/$(PRODUCT_NAME)

	#locales

	#chmod u+x $(DOCUMENTSERVER)/NodeJsProjects/FileConverter/Bin/x2t
	#chmod u+x $(DOCUMENTSERVER)/NodeJsProjects/FileConverter/Bin/HtmlFileInternal/HtmlFileInternal
	#chmod u+x $(DOCUMENTSERVER)/Tools/AllFontsGen

rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i rpm/$(PACKAGE_NAME).spec

deb-version:
	mkdir -p deb/opt
	cp -rf common/opt/$(COMPANY_NAME) deb/opt/
	cp -rf common/usr deb/
	#sed 's/{{PACKAGE_VERSION}}/'$(PACKAGE_VERSION)'/'  -i deb/$(PACKAGE_NAME)/debian/changelog

apt-rpm-version:
	sed 's/{{PACKAGE_NAME}}/'$(PACKAGE_NAME)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{PRODUCT_VERSION}}/'$(PRODUCT_VERSION)'/'  -i apt-rpm/$(PACKAGE_NAME).spec
	sed 's/{{BUILD_NUMBER}}/'${BUILD_NUMBER}'/'  -i apt-rpm/$(PACKAGE_NAME).spec

$(RPM):
	cd rpm && rpmbuild -bb --define "_topdir $(RPM_BUILD_DIR)" $(PACKAGE_NAME).spec

$(DEB):
	fakeroot dpkg-deb --build deb && mv deb.deb $(DEB)

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
	dpkg-scanpackages -m repo /dev/null | gzip -9c > $(REPO)/Packages.gz

	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/repo --acl public-read --delete
	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(DEB_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/repo --acl public-read --delete

deploy-apt-rpm: $(APT_RPM)
	rm -rfv $(REPO)
	mkdir -p $(REPO)

	for i in i586 i686 x86_64 noarch; do \
		mkdir -p "$(REPO)/$$i/base" "$(REPO)/$$i/RPMS.repo" ; \
	done

	cp -rv $(APT_RPM) $(REPO)/$(RPM_ARCH)/RPMS.repo;

	genbasedir --topdir=$(REPO) $(RPM_ARCH) repo

	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/$(PACKAGE_VERSION)/ --acl public-read --delete
	aws s3 sync $(REPO) s3://repo-doc-onlyoffice-com/$(ALT_REPO_DIR)/$(PACKAGE_NAME)/$(SVN_TAG)/latest/ --acl public-read --delete

deploy: deploy-rpm deploy-deb deploy-apt-rpm