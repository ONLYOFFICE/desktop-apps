param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $BuildDir) {
    $BuildDir = ".build.$Arch"
}
$MsiFile = "$CompanyName-$ProductName-$Version-$Arch.msi"
$VersionShort = "$($Version.Major).$($Version.Minor).$($Version.Build)"
$MsiBuild = switch ($Arch) {
    "x64" { "MsiBuild64" }
    "x86" { "MsiBuild32" }
}
$MD5 = New-Object -TypeName System.Security.Cryptography.MD5CryptoServiceProvider
$ASCII = New-Object -TypeName System.Text.ASCIIEncoding
[guid]$GUID = $MD5.ComputeHash($ASCII.GetBytes(`
    "$CompanyName $ProductName $VersionShort $Arch"))
$ProductCode = "{" + $GUID.ToString().ToUpper() + "}"

Write-Host @"
Version     = $Version
Arch        = $Arch
CompanyName = $CompanyName
ProductName = $ProductName
BuildDir    = $BuildDir
Sign        = $Sign
MsiFile     = $MsiFile
MsiBuild    = $MsiBuild
ProductCode = $ProductCode
"@

####

Write-Host "`n[ Get Advanced Installer path ]"

if ($env:ADVINSTPATH) {
    $AdvInstPath = $env:ADVINSTPATH
} else {
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Caphyon\Advanced Installer"
    $AdvInstPath = (Get-ItemProperty $RegPath)."InstallRoot" + "bin\x86"
}
$AdvInstPath
$env:Path = "$AdvInstPath;$env:Path"

####

Write-Host "`n[ Create package.config ]"

Write-Host "WRITE: $BuildDir\desktop\converter\package.config"
Write-Output "package=msi" `
    | Out-File -Encoding ASCII "$BuildDir\desktop\converter\package.config"

####

Write-Host "`n[ Create Advanced Installer config ]"

$AdvInstConfig = @(
    "DelFolder CUSTOM_PATH", `
    "SetProductCode -langid 1029 -guid $ProductCode", `
    "SetProductCode -langid 1031 -guid $ProductCode", `
    "SetProductCode -langid 1033 -guid $ProductCode", `
    "SetProductCode -langid 1036 -guid $ProductCode", `
    "SetProductCode -langid 1041 -guid $ProductCode", `
    "SetProductCode -langid 1046 -guid $ProductCode", `
    "SetProductCode -langid 1049 -guid $ProductCode", `
    "SetProductCode -langid 1060 -guid $ProductCode", `
    "SetProductCode -langid 2070 -guid $ProductCode", `
    "SetProductCode -langid 3082 -guid $ProductCode", `
    "SetProperty FORMS=1"
)
if ($Arch -eq "x86") {
    $AdvInstConfig += `
        "SetComponentAttribute -feature_name MainFeature -unset -64bit_component", `
        "SetComponentAttribute -feature_name Files -unset -64bit_component", `
        "SetComponentAttribute -feature_name Registry -unset -64bit_component", `
        "SetComponentAttribute -feature_name UpdateService -unset -64bit_component", `
        "SetComponentAttribute -feature_name RegFileTypeAssociations -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOC -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOCM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOTM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOCX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLS -unset -64bit_component", `
        # "SetComponentAttribute -feature_name FA_XLT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLSM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLTM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLSB -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLSX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_POT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPTM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_POTM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPSM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_POTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPSX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODP -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTP -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DJVU -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_FB2 -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PDF -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_RTF -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XPS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OXPS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_EPUB -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_HTML -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XML -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_CSV -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_TXT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOCXF -unset -64bit_component"
}
if (-not $Sign) {
    $AdvInstConfig += "ResetSig"
}
$AdvInstConfig += `
    "SetProperty Version=$VersionShort", `
    "SetVersion $Version -noprodcode", `
    "SetCurrentFeature MainFeature", `
    "UpdateFile APPDIR\DesktopEditors.exe $BuildDir\desktop\DesktopEditors.exe", `
    "UpdateFile APPDIR\updatesvc.exe $BuildDir\desktop\updatesvc.exe", `
    "NewSync APPDIR $BuildDir\desktop -existingfiles keep -feature Files", `
    # "GenerateReport -buildname $MsiBuild -output_path .\report.pdf", `
    "Rebuild -buildslist $MsiBuild"
$AdvInstConfig = ";aic", $AdvInstConfig
$AdvInstConfig
Write-Output $AdvInstConfig | Out-File -Encoding UTF8 "DesktopEditors.aic"

####

Write-Host "`n[ Build Advanced Installer project ]"

Write-Host "AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic"
& AdvancedInstaller.com /? | Select-Object -First 1
& AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic
if ($LastExitCode -ne 0) { throw }

####

Write-Host "`n[ Fix Summary Information Properties ]"

$Template = ";1033,1049,1029,1031,3082,1036,2070,1046,1060,1041,0"
switch ($Arch) {
    "x64" { $Template = "x64" + $Template }
    "x86" { $Template = "Intel" + $Template }
}

Write-Host "MsiInfo $MsiFile /p $Template"
& MsiInfo $MsiFile /p $Template
if ($LastExitCode -ne 0) { throw }

if ($Sign) {
    Write-Host "signtool sign /a /n $CertName /t $TimestampServer /v $MsiFile"
    & signtool sign /a /n $CertName /t $TimestampServer /v $MsiFile
    if ($LastExitCode -ne 0) { throw }
}
