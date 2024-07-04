param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [switch]$Sign,
    [string]$BuildDir = ".build.$Arch",
    [string]$SourceDir,
    [string]$BrandingDir = "."
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

Import-Module "$BrandingDir\branding.ps1"

if (-not $SourceDir) {
    $BuildPrefix = switch ($Arch) {
        "x64" { "win_64" }
        "x86" { "win_32" }
    }
    if ($Target -eq "xp") {
        $BuildPrefix += "_xp"
    }
    $SourceDir = "$PSScriptRoot\..\..\..\..\build_tools\out\$BuildPrefix\" `
        + "$CompanyName\$($ProductName -replace '\s','')" | Resolve-Path
}
if (-not (Test-Path "$SourceDir")) {
    Write-Error "Path `"$SourceDir`" does not exist"
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
Sign        = $Sign
BuildDir    = $BuildDir
SourceDir   = $SourceDir
BrandingDir = $BrandingDir
CompanyName = $CompanyName
ProductName = $ProductName
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

# "$BuildDir\desktop: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\desktop" | Measure-Object -Property Length -Sum).Sum / 1MB)
# "$BuildDir\help: {0:0.00} MB" -f ((Get-ChildItem -Recurse `
#     -Path "$BuildDir\help" | Measure-Object -Property Length -Sum).Sum / 1MB)

####

Write-Host "`n[ Add visual elements ]"

Write-Host "COPY: $BrandingDir\data\VisualElementsManifest.xml > $BuildDir\desktop\DesktopEditors.VisualElementsManifest.xml"
Copy-Item -Force `
    -Path "$BrandingDir\data\VisualElementsManifest.xml" `
    -Destination "$BuildDir\desktop\DesktopEditors.VisualElementsManifest.xml"

Write-Host "CREATE DIR: $BuildDir\desktop\browser"
New-Item -ItemType Directory -Force -Path "$BuildDir\desktop\browser" | Out-Null

Write-Host "COPY: $BrandingDir\data\visual_elements_icon_* > $BuildDir\desktop\browser"
Copy-Item -Force `
    -Path "$BrandingDir\data\visual_elements_icon_*" `
    -Destination "$BuildDir\desktop\browser"

####

Write-Host "`n[ Sign files ]"

if ($Sign) {
    if (-not $CertName) {
        $CertName = $PublisherName
    }
    if (-not $TimestampServer) {
        $TimestampServer = "http://timestamp.digicert.com"
    }

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
