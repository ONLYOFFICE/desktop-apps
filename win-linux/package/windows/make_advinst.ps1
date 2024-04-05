param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [switch]$Sign,
    [string]$BuildDir = "build",
    [string]$BrandingDir = "."
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (Test-Path "$BrandingDir\branding.ps1") {
    Import-Module "$BrandingDir\branding.ps1"
}
$VersionShort = "$($Version.Major).$($Version.Minor).$($Version.Build)"
switch ($Arch) {
    "x64" { $MsiBuild = 'MsiBuild64' }
    "x86" { $MsiBuild = 'MsiBuild32' }
}
$MD5 = New-Object -TypeName System.Security.Cryptography.MD5CryptoServiceProvider
$ASCII = New-Object -TypeName System.Text.ASCIIEncoding
[guid]$GUID = $MD5.ComputeHash($ASCII.GetBytes("$PackageName $VersionShort $Arch"))
$ProductCode = "{" + $GUID.ToString().ToUpper() + "}"

Write-Host @"
PackageName = $PackageName
Version     = $Version
Arch        = $Arch
Sign        = $Sign
BuildDir    = $BuildDir
BrandingDir = $BrandingDir
MsiBuild    = $MsiBuild
ProductCode = $ProductCode
"@



Write-Host "`n[ Get Advanced Installer path ]"
if ($ENV:ADVINSTPATH) {
    $AdvInstPath = $ENV:ADVINSTPATH
} else {
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Caphyon\Advanced Installer"
    $AdvInstPath = (Get-ItemProperty $RegPath)."InstallRoot" + "bin\x86"
}
$AdvInstPath
$ENV:Path = "$AdvInstPath;$ENV:Path"



Write-Host "`n[ Download VCRedist 2015+ $Arch ]"
$VCRedist = "data\vcredist_$Arch.exe", "https://aka.ms/vs/17/release/vc_redist.$Arch.exe"
if (-not (Get-Item $VCRedist[0]).VersionInfo.ProductVersion) {
    Write-Host "Downloading: $($VCRedist[1])"
    $WebClient = New-Object System.Net.WebClient
    $WebClient.DownloadFile($VCRedist[1], $VCRedist[0])
}
Write-Host "Version: $((Get-Item $VCRedist[0]).VersionInfo.ProductVersion)"
if (-not (Get-Item $VCRedist[0]).VersionInfo.ProductVersion) { throw }



Write-Host "`n[ Create package.config ]"
Write-Output "package=msi" `
    | Out-File -Encoding ASCII "$BuildDir\$DesktopDir\converter\package.config"
Write-Host "$BuildDir\$DesktopDir\converter\package.config"



Write-Host "`n[ Create Advanced Installer config ]"
$AdvInstConfig = @()
$AdvInstConfig += BrandingAdvInstConfig
if ($Arch -eq "x86") {
    $AdvInstConfig += `
        "SetComponentAttribute -feature_name MainFeature -unset -64bit_component", `
        "SetComponentAttribute -feature_name Files -unset -64bit_component", `
        "SetComponentAttribute -feature_name Registry -unset -64bit_component", `
        "SetComponentAttribute -feature_name UpdateService -unset -64bit_component", `
        "SetComponentAttribute -feature_name RegFileTypeAssociations -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_CSV -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DJVU -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOC -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOCX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOCXF -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_DOTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_EPUB -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_FB2 -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_HTML -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODP -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_ODT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OFORM -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTP -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_OTT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PDF -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_POTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPSX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_PPTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_RTF -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_TXT -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLS -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLSX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XLTX -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XML -unset -64bit_component", `
        "SetComponentAttribute -feature_name FA_XPS -unset -64bit_component"
}
if (-not $Sign) {
    $AdvInstConfig += "ResetSig"
}
$AdvInstConfig += `
    "SetProperty Version=$VersionShort", `
    "SetVersion $Version -noprodcode", `
    "SetCurrentFeature MainFeature", `
    "UpdateFile APPDIR\DesktopEditors.exe $BuildDir\$DesktopDir\DesktopEditors.exe", `
    "UpdateFile APPDIR\updatesvc.exe $BuildDir\$DesktopDir\updatesvc.exe", `
    "NewSync APPDIR $BuildDir\$DesktopDir -existingfiles keep -feature Files", `
    # "GenerateReport -buildname $MsiBuild -output_path .\report.pdf", `
    "Rebuild -buildslist $MsiBuild"
$AdvInstConfig = ";aic", $AdvInstConfig
$AdvInstConfig
Write-Output $AdvInstConfig | Out-File -Encoding UTF8 "DesktopEditors.aic"



Write-Host "`n[ Build Advanced Installer project ]"
AdvancedInstaller.com /? | Select-Object -First 1
Write-Host "AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic"
AdvancedInstaller.com /execute DesktopEditors.aip DesktopEditors.aic
if ($LastExitCode -ne 0) { throw }
