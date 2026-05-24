param(
    [string]$InstallDir = "$env:LOCALAPPDATA\WPStools"
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host "WPStools Installer"
Write-Host "================="
Write-Host "Install target: $InstallDir"

# Stop running instances
Get-Process -Name "WPPTouchHelper", "WPStoolsPanel" -ErrorAction SilentlyContinue | Stop-Process -Force

# Create install directory
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# Copy source files (the package directory content)
$SourceDir = Join-Path $ScriptDir "..\package"
if (-not (Test-Path $SourceDir)) {
    Write-Error "Package directory not found: $SourceDir"
    Write-Host "Run 'build.bat' first to create the package."
    exit 1
}

Write-Host "Copying files..."
Get-ChildItem -Path $SourceDir -Recurse | ForEach-Object {
    $Target = $_.FullName.Replace($SourceDir, $InstallDir)
    if ($_.PSIsContainer) {
        New-Item -ItemType Directory -Path $Target -Force | Out-Null
    } else {
        Copy-Item -LiteralPath $_.FullName -Destination $Target -Force
    }
}
Write-Host "Files copied successfully."

# Create Start Menu shortcuts
$StartMenu = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\WPStools"
if (-not (Test-Path $StartMenu)) {
    New-Item -ItemType Directory -Path $StartMenu -Force | Out-Null
}

$ws = New-Object -ComObject WScript.Shell

$helperLnk = Join-Path $StartMenu "WPPTouchHelper.lnk"
$s = $ws.CreateShortcut($helperLnk)
$s.TargetPath = Join-Path $InstallDir "WPPTouchHelper.exe"
$s.Description = "WPS PPT Touch Helper - Floating controls for presentations"
$s.Save()

$panelLnk = Join-Path $StartMenu "WPStools 配置面板.lnk"
$s = $ws.CreateShortcut($panelLnk)
$s.TargetPath = Join-Path $InstallDir "WPStoolsPanel.exe"
$s.Description = "WPStools Configuration Panel"
$s.Save()

$uninstallLnk = Join-Path $StartMenu "卸载 WPStools.lnk"
$s = $ws.CreateShortcut($uninstallLnk)
$s.TargetPath = Join-Path $InstallDir "uninstaller.exe"
$s.Description = "Uninstall WPStools"
$s.Save()

Write-Host "Start Menu shortcuts created."

# Register uninstall in Add/Remove Programs
$UninstallKey = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\WPStools"
if (-not (Test-Path $UninstallKey)) {
    New-Item -Path $UninstallKey -Force | Out-Null
}

$uninstallerPath = Join-Path $InstallDir "uninstaller.exe"
Set-ItemProperty -Path $UninstallKey -Name "DisplayName" -Value "WPStools - WPS PPT Touch Helper"
Set-ItemProperty -Path $UninstallKey -Name "UninstallString" -Value "`"$uninstallerPath`""
Set-ItemProperty -Path $UninstallKey -Name "DisplayIcon" -Value (Join-Path $InstallDir "WPStoolsPanel.exe")
Set-ItemProperty -Path $UninstallKey -Name "Publisher" -Value "Wayne"
Set-ItemProperty -Path $UninstallKey -Name "InstallLocation" -Value "$InstallDir"
Set-ItemProperty -Path $UninstallKey -Name "DisplayVersion" -Value "1.0.0"
Set-ItemProperty -Path $UninstallKey -Name "NoModify" -Value 1
Set-ItemProperty -Path $UninstallKey -Name "NoRepair" -Value 1

Write-Host "Uninstall registry entry created."
Write-Host ""
Write-Host "Installation complete!"
Write-Host "You can launch WPPTouchHelper from the Start Menu."
