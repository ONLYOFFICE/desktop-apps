param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir,
    [switch]$Sign
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
$LanguageCodes = @(1029, 1031, 1033, 1036, 1041, 1046, 1049, 1060, 2070, 3082)
$AssociationList = @(
    "doc", "dot", "docm", "dotm", "docx", "dotx",
    "xls", <#"xlt",#> "xlsm", "xltm", "xlsb", "xlsx", "xltx",
    "ppt", "pot", "pps", "pptm", "potm", "ppsm", "pptx", "potx", "ppsx",
    "odt", "ott", "ods", "ots", "odp", "otp",
    "djvu", "fb2", "pdf", "rtf", "xps", "oxps",
    "epub", "html", "xml",
    "csv", "txt",
    "docxf"
)
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

$AdvInstConfig = @()
if ($Sign) {
    $AdvInstConfig += "SetDigitalCertificateFile -file %WINDOWS_CERTIFICATE% -password %WINDOWS_CERTIFICATE_PASSWORD% -enable_signing"
}
if ($Arch -eq "x86") {
    $AdvInstConfig += `
        "SetComponentAttribute -feature_name MainFeature -unset -64bit_component", `
        "SetComponentAttribute -feature_name Files -unset -64bit_component", `
        "SetComponentAttribute -feature_name Registry -unset -64bit_component", `
        "SetComponentAttribute -feature_name UpdateService -unset -64bit_component", `
        "SetComponentAttribute -feature_name RegFileTypeAssociations -unset -64bit_component"
    $AssociationList | % {$AdvInstConfig += `
            "SetComponentAttribute -feature_name FA_$($_.ToUpper()) -unset -64bit_component"}
}
$LanguageCodes | % {$AdvInstConfig += "SetProductCode -langid $_ -guid $ProductCode"}
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
$Template = switch ($Arch) {
    "x64" { "x64" + $Template }
    "x86" { "Intel" + $Template }
}

Write-Host "MsiInfo $MsiFile /p $Template"
& MsiInfo $MsiFile /p $Template
if ($LastExitCode -ne 0) { throw }

if ($Sign) {
    $CertFile = $env:WINDOWS_CERTIFICATE
    $CertPass = $env:WINDOWS_CERTIFICATE_PASSWORD
    $TimestampServer = "http://timestamp.digicert.com"

    Write-Host "signtool sign /f $CertFile /p $CertPass /t $TimestampServer $MsiFile"
    & signtool sign /f $CertFile /p $CertPass /t $TimestampServer $MsiFile
    if ($LastExitCode -ne 0) { throw }
}
