ISCC := iscc
ISCC_S_PARAM := //S
SIGN_STR := "byparam=signtool.exe sign /v /n $(word 1, $(PUBLISHER_NAME)) /t http://timestamp.verisign.com/scripts/timstamp.dll \$$f"

S3_BUCKET ?= repo-doc-onlyoffice-com
WIN_REPO_DIR := windows

DESKTOP_EDITORS_EXE += win-linux/package/windows/$(PACKAGE_NAME)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).exe
DESKTOP_EDITORS_ZIP += win-linux/package/windows/$(PACKAGE_NAME)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).zip

PACKAGES += $(DESKTOP_EDITORS_EXE)
PACKAGES += $(DESKTOP_EDITORS_ZIP)

VCREDIST := win-linux/package/windows/data/vcredist/vcredist_$(WIN_ARCH).exe

WINSPARKLE := win-linux/windows/data/winsparkle/WinSparkle.dll
WINSPARKLE_URL := https://github.com/ONLYOFFICE/winsparkle/releases/download/v0.7.0b/WinSparkle-0.7.0.zip
WINSPARKLE_ARCH := WinSparkle-0.7.0.zip

ifeq ($(WIN_ARCH),x64)
	VCREDIST_URL := http://download.microsoft.com/download/2/c/6/2c675af0-2155-4961-b32e-289d7addfcec/vc_redist.x64.exe
	WINSPARKLE_DLL := WinSparkle-0.7.0/x64/Release/WinSparkle.dll
else ifeq ($(WIN_ARCH),x86)
	VCREDIST_URL := http://download.microsoft.com/download/d/e/c/dec58546-c2f5-40a7-b38e-4df8d60b9764/vc_redist.x86.exe
	WINSPARKLE_DLL := WinSparkle-0.7.0/Release/WinSparkle.dll
endif

INDEX_HTML := win-linux/package/windows/index.html

M4_PARAMS += -D M4_COMPANY_NAME='$(COMPANY_NAME)'
M4_PARAMS += -D M4_PRODUCT_NAME='$(PRODUCT_NAME)'
M4_PARAMS += -D M4_PRODUCT_NAME_SHORT='$(PRODUCT_NAME_SHORT)'
M4_PARAMS += -D M4_PUBLISHER_NAME='$(PUBLISHER_NAME)'
M4_PARAMS += -D M4_PUBLISHER_URL='$(PUBLISHER_URL)'
M4_PARAMS += -D M4_SUPPORT_MAIL='$(SUPPORT_MAIL)'
M4_PARAMS += -D M4_SUPPORT_URL='$(SUPPORT_URL)'
M4_PARAMS += -D M4_PACKAGE_NAME=$(PACKAGE_NAME)
M4_PARAMS += -D M4_PACKAGE_VERSION=$(PACKAGE_VERSION)
M4_PARAMS += -D M4_WIN_ARCH=$(WIN_ARCH)
M4_PARAMS += -D M4_WIN_XP_FLAG=$(_WIN_XP)
M4_PARAMS += -D M4_BRANDING_DIR='$(abspath $(BRANDING_DIR))'

$(DESKTOP_EDITORS_EXE): $(DEST_DIR) $(VCREDIST) $(WINSPARKLE)
$(DESKTOP_EDITORS_ZIP): $(DEST_DIR)

$(VCREDIST):
	mkdir -p $(dir $(VCREDIST))
	$(CURL) $(VCREDIST) $(VCREDIST_URL)


$(WINSPARKLE): $(WINSPARKLE_ARCH)
	mkdir -p $(dir $(WINSPARKLE))
	7z e $(WINSPARKLE_ARCH) -y -o$(dir $(WINSPARKLE)) $(WINSPARKLE_DLL)
	
$(WINSPARKLE_ARCH):
	$(CURL) $(WINSPARKLE_ARCH) $(WINSPARKLE_URL)

$(DEST_DIR): install

win-linux/package/windows/%.exe:
	cd $(dir $@) && $(ISCC) \
		//DSCRIPT_CUSTOM_FILES=1 \
		//DENABLE_SIGNING=1 \
		//Qp \
		$(ISCC_S_PARAM)$(SIGN_STR) \
		$(PACKAGE_NAME)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).iss
	
win-linux/package/windows/%.zip:
	7z a -y $@ $(DEST_DIR)/*
	
package: $(PACKAGES)
#zip: $(DESKTOP_EDITORS_ZIP)

clean-package:
	rm -f $(PACKAGES) $(WINSPARKLE_ARCH) $(WINSPARKLE) $(VCREDIST) $(INDEX_HTML)

deploy: $(PACKAGES) $(INDEX_HTML)
	aws s3 cp \
	$(DESKTOP_EDITORS_EXE) \
	s3://$(S3_BUCKET)/$(WIN_REPO_DIR)/$(PACKAGE_NAME)/$(PACKAGE_VERSION)/ \
	--acl public-read 

	aws s3 cp \
	$(DESKTOP_EDITORS_ZIP) \
	s3://$(S3_BUCKET)/$(WIN_REPO_DIR)/$(PACKAGE_NAME)/$(PACKAGE_VERSION)/ \
	--acl public-read 

#	aws s3 sync \
#	s3://$(S3_BUCKET)/$(WIN_REPO_DIR)/$(PACKAGE_NAME)/$(PACKAGE_VERSION)/ \
#	s3://$(S3_BUCKET)/$(WIN_REPO_DIR)/$(PACKAGE_NAME)/latest/ \
#	--acl public-read \
#	--delete

M4_PARAMS += -D M4_S3_BUCKET=$(S3_BUCKET)
M4_PARAMS += -D M4_EXE_URI="$(WIN_REPO_DIR)/$(PACKAGE_NAME)/$(PACKAGE_VERSION)/$(notdir $(DESKTOP_EDITORS_EXE))"
M4_PARAMS += -D M4_ZIP_URI="$(WIN_REPO_DIR)/$(PACKAGE_NAME)/$(PACKAGE_VERSION)/$(notdir $(DESKTOP_EDITORS_ZIP))"

% : %.m4
	m4 $(M4_PARAMS)	$< > $@
