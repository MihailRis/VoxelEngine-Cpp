param (
    [switch]$clean
)

if ($clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Blue
    Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "Clean completed" -ForegroundColor Green
    exit 0
}

Write-Host "Checking for Git..." -ForegroundColor Blue
$gitVersion = git --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Git not found! Please install Git" -ForegroundColor Red
    exit 1
}
Write-Host "Found $gitVersion" -ForegroundColor Green

Write-Host "Checking for Python installation..." -ForegroundColor Blue
$pythonVersion = python --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Python not found! Please install Python from https://www.python.org/" -ForegroundColor Red
    Write-Host "Minimum required version: Python 3.6+" -ForegroundColor Red
    exit 1
}

try {
    $minVersion = [Version]"3.6"
    $version = [Version]($pythonVersion -replace "Python ")
    if ($version -lt $minVersion) {
        Write-Host "Python version $version is too old. Minimum required version: $minVersion" -ForegroundColor Red
        exit 1
    }
    Write-Host "Found Python $version" -ForegroundColor Green
} catch {
    Write-Host "Failed to parse Python version!" -ForegroundColor Red
    exit 1
}

Write-Host "Checking for Visual Studio..." -ForegroundColor Blue
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        $vsVersion = & $vsWhere -latest -property catalog_productDisplayVersion
        Write-Host "Found Visual Studio $vsVersion at: $vsPath" -ForegroundColor Green
    } else {
        Write-Host "Visual Studio installation found but unable to determine path" -ForegroundColor Yellow
        Write-Host "Please make sure you have C++ development tools installed" -ForegroundColor Yellow
    }
} else {
    Write-Host "Warning: Visual Studio installation not found. Build may fail." -ForegroundColor Yellow
    Write-Host "Please install Visual Studio with C++ development tools" -ForegroundColor Yellow
}

Write-Host "Checking for pip..." -ForegroundColor Blue
$pipVersion = python -m pip --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "pip not found! Attempting to install pip..." -ForegroundColor Yellow
    python -m ensurepip --default-pip
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to install pip!" -ForegroundColor Red
        exit 1
    }
}
Write-Host "pip is installed" -ForegroundColor Green

Write-Host "Checking for SCons..." -ForegroundColor Blue
$sconsVersion = scons --version 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "SCons not found! Installing SCons via pip..." -ForegroundColor Yellow
    python -m pip install scons
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to install SCons!" -ForegroundColor Red
        exit 1
    }
}
Write-Host "SCons is ready" -ForegroundColor Green

Write-Host "Checking system architecture..." -ForegroundColor Blue
if ([Environment]::Is64BitOperatingSystem) {
    Write-Host "64-bit system detected" -ForegroundColor Green
} else {
    Write-Host "Warning: 32-bit system detected - this is not recommended" -ForegroundColor Yellow
}

Write-Host "Starting build process..." -ForegroundColor Blue
$buildStart = Get-Date

$cpuCount = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
Write-Host "Using $cpuCount CPU cores for parallel build" -ForegroundColor Blue

scons -j $cpuCount
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

$buildTime = (Get-Date) - $buildStart
Write-Host "Build completed in $($buildTime.TotalSeconds.ToString('0.00')) seconds!" -ForegroundColor Green

$exePath = "build\release\VoxelEngine.exe"
if (Test-Path $exePath) {
    $fileVersion = (Get-Item $exePath).VersionInfo.FileVersion
    Write-Host "Build artifact found at: $exePath" -ForegroundColor Green
    Write-Host "Build version: $fileVersion" -ForegroundColor Green
    
    Write-Host "Checking required DLLs..." -ForegroundColor Blue
    $requiredDlls = @(
        "glew32.dll",
        "glfw3.dll",
        "lua51.dll",
        "libpng16.dll",
        "zlib1.dll",
        "OpenAL32.dll",
        "vorbis.dll",
        "vorbisfile.dll"
    )
    
    $missingDlls = @()
    foreach ($dll in $requiredDlls) {
        if (!(Test-Path "build\release\$dll")) {
            $missingDlls += $dll
        }
    }
    
    if ($missingDlls.Count -gt 0) {
        Write-Host "Warning: Some required DLLs are missing:" -ForegroundColor Yellow
        foreach ($dll in $missingDlls) {
            Write-Host "  - $dll" -ForegroundColor Yellow
        }
    } else {
        Write-Host "All required DLLs are present" -ForegroundColor Green
    }

    $launch = Read-Host "Do you want to run the program? (y/n)"
    if ($launch -eq 'y') {
        Write-Host "Launching VoxelEngine..." -ForegroundColor Blue
        Push-Location "build\release"
        try {
            Start-Process ".\VoxelEngine.exe"
        } catch {
            Write-Host "Failed to launch program: $_" -ForegroundColor Red
        }
        Pop-Location
    }
} else {
    Write-Host "Warning: Build artifact not found at expected location!" -ForegroundColor Yellow
}

Write-Host "`nBuild Summary:" -ForegroundColor Blue
Write-Host "=============" -ForegroundColor Blue
Write-Host "Build Time: $($buildTime.TotalSeconds.ToString('0.00')) seconds" -ForegroundColor White
Write-Host "Target: $exePath" -ForegroundColor White
if ($missingDlls.Count -gt 0) {
    Write-Host "Status: Complete with warnings (missing DLLs)" -ForegroundColor Yellow
} else {
    Write-Host "Status: Success" -ForegroundColor Green
}