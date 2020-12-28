ISCC := iscc

S3_BUCKET ?= repo-doc-onlyoffice-com
RELEASE_BRANCH ?= unstable

DESKTOP_EDITORS_EXE += win-linux/package/windows/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).exe
DESKTOP_EDITORS_ZIP += win-linux/package/windows/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).zip
DESKTOP_EDITORS_UPDATE += win-linux/package/windows/$(PACKAGE_NAME)_update_$(PACKAGE_VERSION)_$(WIN_ARCH)$(WIN_ARCH_SUFFIX:%=_%).exe

PACKAGES += $(DESKTOP_EDITORS_EXE)
PACKAGES += $(DESKTOP_EDITORS_ZIP)
PACKAGES += $(DESKTOP_EDITORS_UPDATE)

VCREDIST13 := win-linux/package/windows/data/vcredist/vcredist_2013_$(WIN_ARCH).exe
VCREDIST15 := win-linux/package/windows/data/vcredist/vcredist_2015_$(WIN_ARCH).exe

ifeq ($(WIN_ARCH),x64)
 	VCREDIST13_URL := https://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe
	VCREDIST15_URL := http://download.microsoft.com/download/2/c/6/2c675af0-2155-4961-b32e-289d7addfcec/vc_redist.x64.exe
else ifeq ($(WIN_ARCH),x86)
 	VCREDIST13_URL := https://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe
	VCREDIST15_URL := http://download.microsoft.com/download/d/e/c/dec58546-c2f5-40a7-b38e-4df8d60b9764/vc_redist.x86.exe
endif

ifneq ($(COMPANY_NAME), ONLYOFFICE)
VCREDIST += $(VCREDIST13)
endif
VCREDIST += $(VCREDIST15)

BUILD_TIMESTAMP = $(shell date +%s)
APPCAST := win-linux/package/windows/update/appcast.xml
CHANGES_EN := win-linux/package/windows/update/changes.html
CHANGES_RU := win-linux/package/windows/update/changes_ru.html
DEPLOY_JSON := win-linux/package/windows/deploy.json

EXE_URI        := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/$(notdir $(DESKTOP_EDITORS_EXE))
ZIP_URI        := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/$(notdir $(DESKTOP_EDITORS_ZIP))
EXE_UPDATE_URI := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/$(notdir $(DESKTOP_EDITORS_UPDATE))
APPCAST_URI    := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/update/appcast_$(PACKAGE_VERSION).xml
CHANGES_EN_URI := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/update/changes_$(PACKAGE_VERSION).html
CHANGES_RU_URI := $(COMPANY_NAME_LOW)/$(RELEASE_BRANCH)/windows/update/changes_ru_$(PACKAGE_VERSION).html

ISCC_PARAMS += //Qp
ISCC_PARAMS += //D_ARCH=$(ARCHITECTURE)
ifdef _WIN_XP
	ISCC_PARAMS += //D_WIN_XP=1
endif
ifeq ($(COMPANY_NAME), ONLYOFFICE)
	ISCC_PARAMS += //D_ONLYOFFICE=1
endif
ISCC_PARAMS += //D_UPDMODULE=1
ISCC_PARAMS += //DsAppVersion=$(PACKAGE_VERSION)
ISCC_PARAMS += //DsBrandingFolder="$(shell cygpath -a -w $(BRANDING_DIR))"
ISCC_PARAMS += //DsOutputFileName=$(notdir $(basename $@))
ISCC_PARAMS += //DDEPLOY_PATH="$(shell cygpath -a -w $(DEST_DIR))"
ifdef ENABLE_SIGNING
ISCC_PARAMS += //DENABLE_SIGNING=1
endif
ISCC_PARAMS += //S"byparam=signtool.exe sign /v /n $(word 1, $(PUBLISHER_NAME)) /t http://timestamp.verisign.com/scripts/timstamp.dll \$$f"

$(DESKTOP_EDITORS_EXE): $(DEST_DIR) $(VCREDIST)
$(DESKTOP_EDITORS_ZIP): $(DEST_DIR)

$(VCREDIST13):
	mkdir -p $(dir $(VCREDIST13))
	$(CURL) $(VCREDIST13) $(VCREDIST13_URL)

$(VCREDIST15):
	mkdir -p $(dir $(VCREDIST15))
	$(CURL) $(VCREDIST15) $(VCREDIST15_URL)

$(DEST_DIR): install

$(DESKTOP_EDITORS_EXE):
	cd $(dir $@) && $(ISCC) $(ISCC_PARAMS) common.iss

$(DESKTOP_EDITORS_UPDATE): $(DESKTOP_EDITORS_EXE)
	cd $(dir $@) && $(ISCC) $(ISCC_PARAMS) //DTARGET_NAME="$(notdir $<)" update_common.iss

win-linux/package/windows/%.zip:
	7z a -y $@ $(DEST_DIR)/*
	
package: $(PACKAGES)
#zip: $(DESKTOP_EDITORS_ZIP)

clean-package:
	rm -fv \
		$(dir $(DESKTOP_EDITORS_EXE))*.exe \
		$(dir $(DESKTOP_EDITORS_ZIP))*.zip \
		$(dir $(DESKTOP_EDITORS_UPDATE))*.exe \
		$(VCREDIST) \
		$(APPCAST) \
		$(CHANGES_EN) \
		$(CHANGES_RU) \
		$(DEPLOY_JSON)

deploy-exe: $(DESKTOP_EDITORS_EXE)
	aws s3 cp --no-progress --acl public-read \
		$(DESKTOP_EDITORS_EXE) s3://$(S3_BUCKET)/$(EXE_URI)

deploy-exe-update: $(DESKTOP_EDITORS_UPDATE)
	aws s3 cp --no-progress --acl public-read \
		$(DESKTOP_EDITORS_UPDATE) s3://$(S3_BUCKET)/$(EXE_UPDATE_URI)

deploy-zip: $(DESKTOP_EDITORS_ZIP)
	aws s3 cp --no-progress --acl public-read \
		$(DESKTOP_EDITORS_ZIP) s3://$(S3_BUCKET)/$(ZIP_URI)

deploy-appcast: $(APPCAST) $(CHANGES_EN) $(CHANGES_RU)
	aws s3 cp --no-progress --acl public-read \
		$(APPCAST) s3://$(S3_BUCKET)/$(APPCAST_URI)

	aws s3 cp --no-progress --acl public-read \
		$(CHANGES_EN) s3://$(S3_BUCKET)/$(CHANGES_EN_URI)

	aws s3 cp --no-progress --acl public-read \
		$(CHANGES_RU) s3://$(S3_BUCKET)/$(CHANGES_RU_URI)

comma := ,
json_edit = cp -f $(1) $(1).tmp; jq $(2) $(1).tmp > $(1); rm -f $(1).tmp

$(DEPLOY_JSON):
	echo '{}' > $@
	$(call json_edit, $@, '. + { \
		product:  "$(PRODUCT_NAME_LOW)"$(comma) \
		version:  "$(PRODUCT_VERSION)"$(comma) \
		build:    "$(BUILD_NUMBER)" \
	}')
ifndef _WIN_XP
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows $(WIN_ARCH)"$(comma) \
		path:     "$(EXE_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows $(WIN_ARCH) update"$(comma) \
		path:     "$(EXE_UPDATE_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows $(WIN_ARCH) portable"$(comma) \
		path:     "$(ZIP_URI)" \
	}]')
else
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows XP $(WIN_ARCH)"$(comma) \
		path:     "$(EXE_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows XP $(WIN_ARCH) update"$(comma) \
		path:     "$(EXE_UPDATE_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "windows"$(comma) \
		title:    "Windows XP $(WIN_ARCH) portable"$(comma) \
		path:     "$(ZIP_URI)" \
	}]')
endif
ifndef _WIN_XP
ifeq ($(WIN_ARCH), x64)
	$(call json_edit, $@, '.items += [{ \
		platform: "appcast"$(comma) \
		title:    "Appcast"$(comma) \
		path:     "$(APPCAST_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "appcast"$(comma) \
		title:    "Changes EN"$(comma) \
		path:     "$(CHANGES_EN_URI)" \
	}]')
	$(call json_edit, $@, '.items += [{ \
		platform: "appcast"$(comma) \
		title:    "Changes RU"$(comma) \
		path:     "$(CHANGES_RU_URI)" \
	}]')
endif
endif

DEPLOY += deploy-exe
DEPLOY += deploy-exe-update
DEPLOY += deploy-zip
ifndef _WIN_XP
	ifeq ($(WIN_ARCH), x64)
DEPLOY += deploy-appcast
	endif
endif

deploy: $(DEPLOY) $(DEPLOY_JSON)

M4_PARAMS += -D M4_COMPANY_NAME="$(COMPANY_NAME)"
M4_PARAMS += -D M4_PRODUCT_NAME="$(PRODUCT_NAME)"
M4_PARAMS += -D M4_PACKAGE_VERSION="$(PACKAGE_VERSION)"
M4_PARAMS += -D M4_BUILD_TIMESTAMP="$(shell date +%s)"

$(APPCAST):
	m4 $(M4_PARAMS) $(BRANDING_DIR)/win-linux/package/windows/update/appcast.xml.m4 > $@

$(CHANGES_EN): L10N=en
$(CHANGES_RU): L10N=ru

$(CHANGES_EN) $(CHANGES_RU):
	m4 $(M4_PARAMS) -D L10N="$(L10N)" $(BRANDING_DIR)/win-linux/package/windows/update/changes.html.m4 > $@

% : %.m4
	m4 $(M4_PARAMS)	$< > $@
