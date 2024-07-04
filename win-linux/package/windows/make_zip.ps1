param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$BuildDir = ".build.$Arch",
    [string]$BrandingDir = "."
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

Import-Module "$BrandingDir\branding.ps1"

if ($Target) {
    $Suffix = "-$Target"
}
if (-not (Test-Path "$BuildDir")) {
    Write-Error "Path `"$BuildDir`" does not exist"
}
$ZipFile = "$PackageName-$Version-$Arch$Suffix.zip"

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
BuildDir    = $BuildDir
BrandingDir = $BrandingDir
CompanyName = $CompanyName
ProductName = $ProductName
ZipFile     = $ZipFile
"@

####

Write-Host "`n[ Create archive ]"

if (Test-Path "$ZipFile") {
    Write-Host "DELETE: $ZipFile"
    Remove-Item -Force -LiteralPath "$ZipFile"
}

Write-Host "7z a -y $ZipFile $BuildDir\desktop\*"
& 7z a -y "$ZipFile" ".\$BuildDir\desktop\*"
if ($LastExitCode -ne 0) { throw }

if ($Target -eq "standalone" -or $Target -eq "xp") {
    Write-Host "7z a -y $ZipFile $BuildDir\help\*"
    & 7z a -y "$ZipFile" ".\$BuildDir\help\*"
    if ($LastExitCode -ne 0) { throw }
}
