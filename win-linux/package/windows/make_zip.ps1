param (
    [Parameter(Mandatory)][string]$Target,
    [string]$BuildDir = "build",
    [string]$DesktopDir = "DesktopEditors",
    [string]$MultimediaDir,
    [string]$BrandingDir = ".",
    [switch]$ExcludeHelp,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$Suffix = switch ( $Target )
{
    "windows_x64"    { "x64" }
    "windows_x86"    { "x86" }
    "windows_x64_xp" { "x64-xp" }
    "windows_x86_xp" { "x86-xp" }
}
$DesktopHelpDir = "$DesktopDir-Help"

# Check directory
if ( -Not (Test-Path $BuildDir) ) {
    Write-Error "Path $BuildDir does not exist"
}

# Copy VisualElements
Write-Host "Copy: $BrandingDir\data\VisualElementsManifest.xml > $BuildDir\$DesktopDir\DesktopEditors.VisualElementsManifest.xml" -ForegroundColor Yellow
Copy-Item -Path "$BrandingDir\data\VisualElementsManifest.xml" `
    -Destination "$BuildDir\$DesktopDir\DesktopEditors.VisualElementsManifest.xml" `
    -Force
Write-Host "Copy: $BrandingDir\data\visual_elements_icon_* > $BuildDir\$DesktopDir\browser" -ForegroundColor Yellow
New-Item "$BuildDir\$DesktopDir\browser" -ItemType Directory -Force | Out-Null
Copy-Item -Path "$BrandingDir\data\visual_elements_icon_*" `
    -Destination "$BuildDir\$DesktopDir\browser" -Force

# Move Help
Get-ChildItem "$BuildDir\$DesktopDir\editors\web-apps\apps\*\main\resources\help" -Directory `
    | ForEach-Object {
        $src = $(Split-Path $_.FullName -Parent | Resolve-Path -Relative)
        $dst = $src.Replace("\$DesktopDir\","\$DesktopHelpDir\")
        Write-Host "Move: $src\help > $dst" -ForegroundColor Yellow
        New-Item "$dst" -ItemType Directory | Out-Null
        Move-Item -Path "$src\help" -Destination "$dst"
    }

if ( $DesktopDir ) {
    Set-Location "$BuildDir\$DesktopDir"
    if ( $Sign ) {
        $SignFiles = Get-ChildItem `
            *.exe, *.dll, converter\*.exe, converter\*.dll, plugins\*\*.dll `
            | Resolve-Path -Relative
        # Sign
        Write-Host "signtool sign /a /n $CertName /t $TimestampServer $SignFiles" -ForegroundColor Yellow
        & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # Verify
        Write-Host "signtool verify /pa /all $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # VLC plugin cache
        Write-Host ".\vlc-cache-gen $PWD\plugins" -ForegroundColor Yellow
        & .\vlc-cache-gen "$PWD\plugins"
        if ( $LastExitCode -ne 0 ) { throw }
    }
    if ( Test-Path "vlc-cache-gen.exe" ) {
        Write-Host "Delete: vlc-cache-gen.exe" -ForegroundColor Yellow
        Remove-Item "vlc-cache-gen.exe" -Force
    }
    Set-Location $PSScriptRoot
}

if ( $MultimediaDir ) {
    Set-Location "$BuildDir\$MultimediaDir"
    if ( $Sign ) {
        $SignFiles = Get-ChildItem *.exe, *.dll, plugins\*\*.dll `
            | Resolve-Path -Relative
        # Sign
        Write-Host "signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles" -ForegroundColor Yellow
        & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # Verify
        Write-Host "signtool verify /pa /all $SignFiles" -ForegroundColor Yellow
        & signtool verify /pa /all $SignFiles
        if ( $LastExitCode -ne 0 ) { throw }
        # VLC plugin cache
        Write-Host ".\vlc-cache-gen $PWD\plugins" -ForegroundColor Yellow
        & .\vlc-cache-gen "$PWD\plugins"
        if ( $LastExitCode -ne 0 ) { throw }
    }
    if ( Test-Path "vlc-cache-gen.exe" ) {
        Write-Host "Delete: vlc-cache-gen.exe" -ForegroundColor Yellow
        Remove-Item "vlc-cache-gen.exe" -Force
    }
    Set-Location $PSScriptRoot
}

# Create archives
$OutFile = "$Env:COMPANY_NAME-$DesktopDir-$Env:PRODUCT_VERSION.$Env:BUILD_NUMBER-$Suffix.zip"
Write-Host "7z a -y $OutFile .\$BuildDir\$DesktopDir\*" -ForegroundColor Yellow
& 7z a -y $OutFile .\$BuildDir\$DesktopDir\*
if ( $LastExitCode -ne 0 ) { throw }
if ( !$ExcludeHelp ) {
    Write-Host "7z a -y $OutFile .\$BuildDir\$DesktopHelpDir\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\$DesktopHelpDir\*
    if ( $LastExitCode -ne 0 ) { throw }
}
if ( $MultimediaDir ) {
    $OutFile = "$Env:COMPANY_NAME-$MultimediaDir-$Env:PRODUCT_VERSION.$Env:BUILD_NUMBER-$Suffix.zip"
    Write-Host "7z a -y $OutFile .\$BuildDir\$MultimediaDir\*" -ForegroundColor Yellow
    & 7z a -y $OutFile .\$BuildDir\$MultimediaDir\*
    if ( $LastExitCode -ne 0 ) { throw }
}
