param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir,
    [string]$BrandingDir,
    [switch]$Sign
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $BuildDir) {
    $BuildDir = ".build.$Arch"
}
if (-not (Test-Path "$BuildDir")) {
    Write-Error "Path `"$BuildDir`" does not exist"
}
$InnoFile = switch ($Target) {
    "standalone" { "$CompanyName-$ProductName-Standalone-$Version-$Arch.exe" }
    "xp"         { "$CompanyName-$ProductName-$Version-$Arch-xp.exe" }
    "update"     { "$CompanyName-$ProductName-Update-$Version-$Arch.exe" }
    "xp_update"  { "$CompanyName-$ProductName-Update-$Version-$Arch-xp.exe" }
    default      { "$CompanyName-$ProductName-$Version-$Arch.exe" }
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
BuildDir    = $BuildDir
BrandingDir = $BrandingDir
Sign        = $Sign
InnoFile    = $InnoFile
"@

####

Write-Host "`n[ Get Inno Setup path ]"

if ($env:INNOPATH) {
    $InnoPath = $env:INNOPATH
}
elseif ($Target -notlike "xp*") {
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1"
    $InnoPath = (Get-ItemProperty $RegPath)."Inno Setup: App Path"
}
elseif ($Target -like "xp*") {
    $RegPath = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 5_is1"
    $InnoPath = (Get-ItemProperty $RegPath)."Inno Setup: App Path"
}
$InnoPath
$env:Path = "$InnoPath;$env:Path"

####

if ($Target -notlike "*update") {
    Write-Host "`n[ Download VCRedist $Arch ]"

    $VCRedist = "data\vcredist_$Arch.exe"
    $VCRedistUrl = switch -Wildcard ("$Arch-$Target") {
        # Microsoft Visual C++ 2015-2019 Redistributable - 14.27.29114
        "x64-xp" { "https://download.visualstudio.microsoft.com/download/pr/722d59e4-0671-477e-b9b1-b8da7d4bd60b/591CBE3A269AFBCC025681B968A29CD191DF3C6204712CBDC9BA1CB632BA6068/VC_redist.x64.exe"; Break }
        "x86-xp" { "https://download.visualstudio.microsoft.com/download/pr/c168313d-1754-40d4-8928-18632c2e2a71/D305BAA965C9CD1B44EBCD53635EE9ECC6D85B54210E2764C8836F4E9DEFA345/VC_redist.x86.exe"; Break }
        # Microsoft Visual C++ 2015-2022 Redistributable
        default  { "https://aka.ms/vs/17/release/vc_redist.$Arch.exe" }
    }
    if ((-not (Test-Path "$VCRedist")) -or `
        (-not (Get-Item "$VCRedist").VersionInfo.ProductVersion)) {
        Write-Host "DOWNLOAD: $VCRedistUrl"
        $WebClient = New-Object System.Net.WebClient
        $WebClient.DownloadFile($VCRedistUrl, $VCRedist)
    }

    Write-Host "Version: $((Get-Item $VCRedist).VersionInfo.ProductVersion)"
    if (-not (Get-Item $VCRedist).VersionInfo.ProductVersion) { throw }
}

####

if ($Target -notlike "*update") {
    Write-Host "`n[ Create package.config ]"

    Write-Host "WRITE: $BuildDir\desktop\converter\package.config"
    Write-Output "package=exe" `
        | Out-File -Encoding ASCII "$BuildDir\desktop\converter\package.config"
}

####

Write-Host "`n[ Build Inno Setup project ]"

$IssFile = "common.iss"
$InnoArgs = "/Qp",
            "/DVERSION=$Version",
            "/DARCH=$Arch",
            "/DBUILD_DIR=$BuildDir"
if ($BrandingDir) {
    $InnoArgs += "/DBRANDING_DIR=$BrandingDir"
}
if ($CompanyName -eq "onlyoffice") {
    $InnoArgs += "/D_ONLYOFFICE"
}
switch ($Target) {
    "standalone" {
        $InnoArgs += "/DEMBED_HELP", "/DPACKAGE_EDITION=Standalone"
    }
    "xp" {
        $InnoArgs += "/D_WIN_XP"
    }
    "update" {
        $InnoArgs += "/DTARGET_NAME=$CompanyName-$ProductName-$Version-$Arch"
        $IssFile = "update_common.iss"
    }
    "xp_update" {
        $InnoArgs += "/D_WIN_XP",
            "/DTARGET_NAME=$CompanyName-$ProductName-$Version-$Arch-xp"
        $IssFile = "update_common.iss"
    }
}
if ($Sign) {
    $CertFile = $env:WINDOWS_CERTIFICATE
    $CertPass = $env:WINDOWS_CERTIFICATE_PASSWORD
    $TimestampServer = "http://timestamp.digicert.com"
    $InnoArgs += "/DSIGN", "/Sbyparam=signtool sign /f `$q$CertFile`$q /p `$q$CertPass`$q /t $TimestampServer `$f"
}

Write-Host "iscc $InnoArgs $IssFile"
& iscc $InnoArgs $IssFile
if ($LastExitCode -ne 0) { throw }
