param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [switch]$Sign,
    [string]$BrandingDir = "."
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if ( Test-Path "$BrandingDir\branding.ps1" ) {
    Import-Module "$BrandingDir\branding.ps1"
}
$VersionShort = "$($Version.Major).$($Version.Minor).$($Version.Build)"
switch ( $Arch ) {
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
BrandingDir = $BrandingDir
MsiBuild    = $MsiBuild
ProductCode = $ProductCode
"@



Write-Host "`n[ Get Advanced Installer path ]"
if ( $ENV:ADVINSTPATH ) {
    $AdvInstPath = $ENV:ADVINSTPATH
}
else
{
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Caphyon\Advanced Installer"
    $AdvInstPath = (Get-ItemProperty $RegPath)."InstallRoot" + "bin\x86"
}
$AdvInstPath
$ENV:Path = "$AdvInstPath;$ENV:Path"



Write-Host "`n[ Create package.config ]"
Write-Output "package=msi" `
    | Out-File -Encoding ASCII "$BuildDir\$DesktopDir\converter\package.config"
Write-Host "$BuildDir\$DesktopDir\converter\package.config"



Write-Host "`n[ Create Advanced Installer config ]"
$AdvInstConfig = @()
$AdvInstConfig += BrandingAdvInstConfig
if ( $Arch -eq "x86" ) {
    $AdvInstConfig += `
        "SetComponentAttribute -feature_name MainFeature -unset -64bit_component", `
        "SetComponentAttribute -feature_name Files -unset -64bit_component", `
        "SetComponentAttribute -feature_name Registry -unset -64bit_component", `
        "SetComponentAttribute -feature_name UpdateService -unset -64bit_component", `
        "SetComponentAttribute -feature_name RegFileTypeAssociations -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__csv -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__djvu -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__doc -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__docx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__docxf -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__dotx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__epub -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__fb2 -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__html -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__odp -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__ods -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__odt -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__oform -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__otp -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__ots -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__ott -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__pdf -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__potx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__pps -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__ppsx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__ppt -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__pptx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__rtf -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__txt -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__xls -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__xlsx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__xltx -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__xml -unset -64bit_component", `
        "SetComponentAttribute -feature_name ft_fa_ext__xps -unset -64bit_component"
}
if ( ! $Sign ) {
    $AdvInstConfig += "ResetSig"
}
$AdvInstConfig += `
    "SetProperty Version=$VersionShort", `
    "SetVersion $Version -noprodcode", `
    "SetCurrentFeature MainFeature", `
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
