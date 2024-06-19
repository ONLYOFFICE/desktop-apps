param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$BuildDir
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if ($Target) {
    $Suffix = "-$Target"
}
if (-not $BuildDir) {
    $BuildDir = ".build.$Arch"
}
if (-not (Test-Path "$BuildDir")) {
    Write-Error "Path `"$BuildDir`" does not exist"
}
$ZipFile = "$CompanyName-$ProductName-$Version-$Arch$Suffix.zip"

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
BuildDir    = $BuildDir
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
