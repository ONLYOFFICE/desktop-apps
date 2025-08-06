param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "DesktopEditors",
    [string]$SourceDir,
    [string]$BuildDir,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $SourceDir) {
    $BuildPrefix = switch ($Arch) {
        "x64" { "win_64" }
        "x86" { "win_32" }
    }
    if ($Target -eq "xp") {
        $BuildPrefix += "_xp"
    }
    $SourceDir = "$PSScriptRoot\..\..\..\..\build_tools\out\" `
        + "$BuildPrefix\$CompanyName\$ProductName" | Resolve-Path
}
if (-not (Test-Path "$SourceDir")) {
    Write-Error "Path `"$SourceDir`" does not exist"
}
if (-not $BuildDir) {
    $BuildDir = ".build.$Arch"
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
SourceDir   = $SourceDir
BuildDir    = $BuildDir
Sign        = $Sign
"@

####

Write-Host "`n[ Prepare build directory ]"

if (Test-Path "$BuildDir") {
    Write-Host "REMOVE DIR: $BuildDir"
    Remove-Item -Force -Recurse -LiteralPath "$BuildDir"
}

Write-Host "CREATE DIR: $BuildDir\desktop"
New-Item -ItemType Directory -Force -Path "$BuildDir\desktop" | Out-Null

Write-Host "CREATE DIR: $BuildDir\help"
New-Item -ItemType Directory -Force -Path "$BuildDir\help" | Out-Null

Write-Host "COPY: $SourceDir\* > $BuildDir\desktop\"
Copy-Item -Force -Recurse `
    -Path "$SourceDir\*" `
    -Destination "$BuildDir\desktop\"

Write-Host "MOVE: $BuildDir\desktop\editors\web-apps\apps\*\main\resources\help\* > $BuildDir\help\"
Get-ChildItem -Directory `
    -Path "$BuildDir\desktop\editors\web-apps\apps\*\main\resources\help" `
    | ForEach-Object {
        $src = $_.FullName | Resolve-Path -Relative
        $dst = $src.Replace("$BuildDir\desktop", "$BuildDir\help")

        Write-Host "MOVE: $src > $dst"
        New-Item -ItemType Directory -Force -Path "$dst\.." | Out-Null
        Move-Item -Path "$src" -Destination "$dst"
    }

Write-Host "COPY: ..\..\..\common\package\license\* > $BuildDir\desktop\"
$LicenseDir = switch ($Target) {
    "commercial" { "commercial" }
    default      { "opensource" }
}
Copy-Item -Force `
    -Path "..\..\..\common\package\license\3dparty\3DPARTYLICENSE", `
          "..\..\..\common\package\license\$LicenseDir\LICENSE.txt" `
    -Destination "$BuildDir\desktop\"

# "$BuildDir\desktop: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\desktop" | Measure-Object -Property Length -Sum).Sum / 1MB)
# "$BuildDir\help: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\help" | Measure-Object -Property Length -Sum).Sum / 1MB)

####

Write-Host "`n[ Sign files ]"

if ($Sign) {
    Set-Location "$BuildDir\desktop"
    $SignFiles = Get-ChildItem `
        *.exe, *.dll, converter\*.exe, converter\*.dll, plugins\*\*.dll `
        | Resolve-Path -Relative

    # Sign
    Write-Host "signtool sign /a /n $CertName /t $TimestampServer ..."
    & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
    if ($LastExitCode -ne 0) { throw }

    # Verify
    Write-Host "signtool verify /q /pa /all ..."
    & signtool verify /q /pa /all $SignFiles | Out-Null
    if ($LastExitCode -ne 0) { throw }

    # VLC plugin cache
    Write-Host ".\vlc-cache-gen $PWD\plugins"
    & .\vlc-cache-gen "$PWD\plugins"
    if ($LastExitCode -ne 0) { throw }

    Set-Location $PSScriptRoot
}

if (Test-Path "$BuildDir\desktop\vlc-cache-gen.exe") {
    Write-Host "DELETE: $BuildDir\desktop\vlc-cache-gen.exe"
    Remove-Item -Force -LiteralPath "$BuildDir\desktop\vlc-cache-gen.exe"
}

if (Test-Path "$BuildDir\desktop\online-installer.exe") {
    Write-Host "MOVE: $BuildDir\desktop\online-installer.exe > OnlineInstaller-$Version-$Arch.exe"
    Move-Item `
        -Path "$BuildDir\desktop\online-installer.exe" `
        -Destination "OnlineInstaller-$Version-$Arch.exe"
}
