#!/bin/bash
#
# Copyright (c) Ascensio System SIA, 2016
#
#

INSTALL_PATH=$1

# This script is running as root (not from sudo), so we need to know actual user for some operations
IFW_CURRENT_USER=$(who | awk '{print $1}' | head -1)

# Linux distrubution name (f.e. "ubuntu", "astra", ...)
IFW_LINUX_DISTRIBUTION=$(cat /etc/*-release | grep "^ID=" | sed "s|ID=||g")

createAppEntryPoint()
{
    sed -i "s|__INSTALL_PATH__|${INSTALL_PATH}|g" "${INSTALL_PATH}/$1.sh"
    rm -f "/usr/bin/$1"
    ln -s "${INSTALL_PATH}/$1.sh" "/usr/bin/$1"
}

# Also it allows us to launch apps from terminal like onlyoffice
createAppEntryPoint onlyoffice

createAppEntryPointAliase()
{
    rm -f "/usr/bin/$2"
    ln -s "${INSTALL_PATH}/$1.sh" "/usr/bin/$2"
}

# Also it allows us to launch apps from terminal like desktopeditors and onlyoffice-desktopeditors
createAppEntryPointAliase onlyoffice onlyoffice-desktopeditors
createAppEntryPointAliase onlyoffice desktopeditors

# Workaround for bug in xdg. See https://bugs.archlinux.org/task/33316 for details
mkdir -p /usr/share/desktop-directories/

# register in desktop environment
xdg-desktop-menu install --novendor --mode system "${INSTALL_PATH}/onlyoffice.desktop"

IFW_TEXT_MIMES=( application/vnd.openxmlformats-officedocument.wordprocessingml.document application/vnd.oasis.opendocument.text application/msword application/vnd.ms-word application/x-doc application/x-extension-txt text/plain )
IFW_TABLE_MIMES=( application/vnd.oasis.opendocument.spreadsheet application/vnd.sun.xml.calc application/msexcel application/vnd.ms-excel application/csv application/excel application/x-excel application/x-msexcel application/x-ms-excel text/comma-separated-values text/tab-separated-values text/x-comma-separated-values text/x-csv text/csv application/vnd.openxmlformats-officedocument.spreadsheetml.sheet)
IFW_PRESENTATION_MIMES=( application/vnd.oasis.opendocument.presentation application/mspowerpoint application/vnd.ms-powerpoint application/vnd.openxmlformats-officedocument.presentationml.presentation )
IFW_ALL_MIMES=( ${IFW_TEXT_MIMES[@]} ${IFW_TABLE_MIMES[@]} ${IFW_PRESENTATION_MIMES[@]} )

setDefaultAppForMimes()
{
    declare -a mimesList=("${!2}")
    xdg-mime default /usr/share/applications/$1.desktop "${mimesList[@]}"
}

# Astra Linux does not work with system-wide default apps, so registering application for current user only
if [[ "$IFW_LINUX_DISTRIBUTION" == "astra" ]]; then
    # ... and in Astra Linux command 'su - $IFW_CURRENT_USER -c "xdg-mime-default ...' is failing by unknown reason,
    # so registering ONLYOFFICE for known mimetypes manually
    DEFULT_USER_APPS_DIR="/home/${IFW_CURRENT_USER}/.local/share/applications"
    MIMEAPPS_FILE="${DEFULT_USER_APPS_DIR}/mimeapps.list"

    su - $IFW_CURRENT_USER -c "mkdir -p $DEFULT_USER_APPS_DIR"

    if [ -f $MIMEAPPS_FILE ]; then
        echo "File $MIMEAPPS_FILE exists"
        # Removing already registered mimes associations
        for mime in "${IFW_ALL_MIMES[@]}"; do
            sed -i "s|${mime}=.*||g" $MIMEAPPS_FILE
        done
    else
        echo "File $MIMEAPPS_FILE does not exist"
        echo "Creating new one"
        su - $IFW_CURRENT_USER -c "echo $'[Added Associations]' > ${MIMEAPPS_FILE}"
    fi

    # Updating defaults for mimes with onlyoffice, but saving libreoffice fallback
    for mime in "${IFW_TEXT_MIMES[@]}"; do
        sed -i "s|\[Added Associations\]|\[Added Associations\]\n${mime}=onlyoffice.desktop;libreoffice-writer.desktop|g" $MIMEAPPS_FILE
    done

    for mime in "${IFW_TABLE_MIMES[@]}"; do
        sed -i "s|\[Added Associations\]|\[Added Associations\]\n${mime}=onlyoffice.desktop;libreoffice-calc.desktop|g" $MIMEAPPS_FILE
    done

    for mime in "${IFW_PRESENTATION_MIMES[@]}"; do
        sed -i "s|\[Added Associations\]|\[Added Associations\]\n${mime}=onlyoffice.desktop;libreoffice-impress.desktop|g" $MIMEAPPS_FILE
    done
else
    setDefaultAppForMimes onlyoffice IFW_TEXT_MIMES[@]
    setDefaultAppForMimes onlyoffice IFW_TABLE_MIMES[@]
    setDefaultAppForMimes onlyoffice IFW_PRESENTATION_MIMES[@]
fi

registerAppIcon()
{
    xdg-icon-resource install --novendor --noupdate --theme $1 --mode system --size $3 "${INSTALL_PATH}/$2-$3.png" $4
}

IFW_ICON_SIZE=(256 128 64 48 32 24 16)

for size in ${IFW_ICON_SIZE[@]}; do
    registerAppIcon hicolor asc-de ${size} onlyoffice
done

# Updating document icons
registerDocumentIcon()
{
    xdg-icon-resource install --noupdate --context mimetypes --theme $1 --mode system --size $3 "${INSTALL_PATH}/$2-$3.png" $4
}

registerDocumentIconForMimes()
{
    declare -a mimesList=("${!4}")

    for mime in "${mimesList[@]}"; do
        # We should replace all '/' characters with '-' according to Open Desktop Standard
        local modifiedMime=`echo ${mime} | sed "s|\/|-|g"`
        registerDocumentIcon $1 $2 $3 ${modifiedMime}
    done
}

#for size in "${IFW_ICON_SIZE[@]}"; do
#    registerDocumentIconForMimes hicolor editor-docx-icon ${size} IFW_TEXT_MIMES[@]
#    registerDocumentIconForMimes hicolor editor-xlsx-icon ${size} IFW_TABLE_MIMES[@]
#    registerDocumentIconForMimes hicolor editor-pptx-icon ${size} IFW_PRESENTATION_MIMES[@]
#done

xdg-icon-resource forceupdate --theme hicolor --mode system

# TODO: register own mime types if Libre was not installed
update-mime-database /usr/share/mime

# Not all the Linux platforms are using standard icons cache (f.e. Astra Linux)
if which gtk-update-icon-cache >/dev/null; then
    gtk-update-icon-cache -f --include-image-data /usr/share/icons/hicolor
fi

# Fonts
IFW_FONTS_EST_DIR="/usr/share/fonts/truetype/"
if [ ! -d "$IFW_FONTS_EST_DIR" ]; then
    # On some Linuxes true type fonts have folder with this name
    IFW_FONTS_EST_DIR="/usr/share/fonts/ttf/"
fi

cp "${INSTALL_PATH}/fonts"/* ${IFW_FONTS_EST_DIR}
fc-cache

# Creating desktop shortcut for current user
su - "${IFW_CURRENT_USER}" -c "xdg-desktop-icon install --novendor /usr/share/applications/onlyoffice.desktop"

# We need to turn on antialiasing in Astra Linux to make fonts rendering better in our app.
# It has been agreed with the developers of Astra. These changes are restored during the uninstallation.
backupFontConfig()
{
    local fontConfigFolder="/etc/fonts/conf.avail"

    if [ -f "${fontConfigFolder}/${1}" ]; then
        mv "${fontConfigFolder}/${1}" "${fontConfigFolder}/${1}.backup"
    fi
}

if [[ "$IFW_LINUX_DISTRIBUTION" == "astra" ]]; then
    backupFontConfig "19-fly-no-antialiasing.conf" # Used in version 1.4
    backupFontConfig "29-fly-rendering.conf" # Used in version 1.5
fi

${INSTALL_PATH}/post_install.sh
