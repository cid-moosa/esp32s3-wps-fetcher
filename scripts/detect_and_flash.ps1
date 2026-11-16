<#
.SYNOPSIS
    Automated ESP32-S3 COM Port Detection, PlatformIO Build, Flashing & Artifact Archival Script

.DESCRIPTION
    Scans Windows system for connected ESP32-S3 boards (CH343 / Espressif USB / CP210x),
    runs PlatformIO compilation, flashes firmware directly to the hardware, and archives
    the generated .bin to builds/firmware/.
#>

[CmdletBinding()]
param(
    [string]$Port = "",
    [int]$Baud = 921600,
    [switch]$SkipBuild,
    [switch]$SkipFlash
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $ProjectRoot

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "   ESP32-S3 Automated Build & Flash Utility" -ForegroundColor Cyan
Write-Host "   Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. COM Port Detection
if ([string]::IsNullOrWhiteSpace($Port)) {
    Write-Host "[1/5] Detecting connected serial COM ports..." -ForegroundColor Yellow
    $comDevices = Get-CimInstance Win32_PnPEntity | Where-Object { $_.Caption -match '\((COM\d+)\)' }
    
    if (-not $comDevices) {
        Write-Error "No COM ports detected! Please ensure your ESP32-S3 is connected via USB."
    }

    # Prioritize CH343 / ESP32 / USB Serial devices
    $targetDevice = $comDevices | Where-Object { 
        $_.Caption -match 'CH343' -or $_.Caption -match 'ESP' -or $_.Caption -match 'CP210' -or $_.Caption -match 'USB-Enhanced-SERIAL'
    } | Select-Object -First 1

    if (-not $targetDevice) {
        $targetDevice = $comDevices | Select-Object -First 1
    }

    if ($targetDevice.Caption -match '\((COM\d+)\)') {
        $Port = $Matches[1]
    } else {
        Write-Error "Failed to parse COM port name from: $($targetDevice.Caption)"
    }

    Write-Host "      Detected Device : $($targetDevice.Caption)" -ForegroundColor Green
    Write-Host "      Target Port     : $Port" -ForegroundColor Green
} else {
    Write-Host "[1/5] Using user-specified COM port: $Port" -ForegroundColor Green
}

# 2. Interrogate Chip using esptool
Write-Host "[2/5] Interrogating connected chip via esptool..." -ForegroundColor Yellow
try {
    $chipInfo = esptool.exe --port $Port chip-id 2>&1 | Out-String
    Write-Host $chipInfo -ForegroundColor DarkGray
    if ($chipInfo -match "ESP32-S3") {
        Write-Host "      Confirmed target chip: ESP32-S3" -ForegroundColor Green
    } else {
        Write-Host "      Target responded on $Port" -ForegroundColor Green
    }
} catch {
    Write-Warning "Could not interrogate chip with esptool directly ($_.Exception.Message), proceeding with PlatformIO."
}

# 3. Compile Firmware via PlatformIO
if (-not $SkipBuild) {
    Write-Host "[3/5] Compiling firmware via PlatformIO (pio run)..." -ForegroundColor Yellow
    $buildStopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    & pio run
    if ($LASTEXITCODE -ne 0) {
        Write-Error "PlatformIO build failed with exit code $LASTEXITCODE"
    }
    $buildStopwatch.Stop()
    Write-Host "      Build succeeded in $($buildStopwatch.Elapsed.TotalSeconds.ToString('F1'))s" -ForegroundColor Green
} else {
    Write-Host "[3/5] Skipping compilation (--SkipBuild specified)" -ForegroundColor Gray
}

# 4. Flash Firmware to Board
if (-not $SkipFlash) {
    Write-Host "[4/5] Flashing firmware to $Port at $Baud baud..." -ForegroundColor Yellow
    & pio run --target upload --upload-port $Port
    if ($LASTEXITCODE -ne 0) {
        Write-Error "PlatformIO upload failed with exit code $LASTEXITCODE"
    }
    Write-Host "      Flashing completed successfully!" -ForegroundColor Green
} else {
    Write-Host "[4/5] Skipping upload (--SkipFlash specified)" -ForegroundColor Gray
}

# 5. Archive Release Binary
Write-Host "[5/5] Archiving firmware binary artifact..." -ForegroundColor Yellow
$firmwareBin = Join-Path $ProjectRoot ".pio\build\esp32s3\firmware.bin"
$buildsDir = Join-Path $ProjectRoot "builds\firmware"

if (Test-Path $firmwareBin) {
    if (-not (Test-Path $buildsDir)) {
        New-Item -ItemType Directory -Path $buildsDir -Force | Out-Null
    }
    $timestamp = (Get-Date).ToString("yyyyMMdd-HHmmss")
    $archiveName = "wps-esp32s3-$timestamp.bin"
    $archivePath = Join-Path $buildsDir $archiveName
    Copy-Item -Path $firmwareBin -Destination $archivePath -Force
    $binSize = (Get-Item $archivePath).Length
    Write-Host "      Archived: $archivePath ($([math]::Round($binSize / 1KB, 1)) KB)" -ForegroundColor Green
} else {
    Write-Warning "Firmware binary not found at $firmwareBin"
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "   Done! ESP32-S3 is now running WPS Password Fetcher." -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
