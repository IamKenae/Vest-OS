# Vest-OS Quick Setup Script for PowerShell
# Run this script as Administrator in PowerShell

Write-Host "Vest-OS Quick Setup Script" -ForegroundColor Green
Write-Host "===========================" -ForegroundColor Green
Write-Host ""

# Check if running as Administrator
if (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "ERROR: Please run this script as Administrator" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# Create installation directory
$InstallDir = "C:\vestos_dev_tools"
Write-Host "Creating installation directory: $InstallDir"
if (!(Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}
Set-Location $InstallDir

Write-Host ""
Write-Host "This script will install:"
Write-Host "1. QEMU (Virtual Machine Emulator)"
Write-Host "2. NASM (Assembler)"
Write-Host "3. MinGW-w64 (C Compiler)"
Write-Host "4. Git (Version Control)"
Write-Host ""

# Function to download with progress
function Download-File {
    param (
        [string]$Url,
        [string]$OutputFile,
        [string]$Description
    )

    Write-Host "Downloading $Description..."
    try {
        # Use Invoke-WebRequest with progress tracking
        Invoke-WebRequest -Uri $Url -OutFile $OutputFile -UseBasicParsing
        Write-Host "[SUCCESS] $Description downloaded successfully" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "[ERROR] Failed to download $Description" -ForegroundColor Red
        Write-Host "Please download manually from: $Url" -ForegroundColor Yellow
        return $false
    }
}

# Function to extract ZIP files
function Extract-ZipFile {
    param (
        [string]$ZipFile,
        [string]$ExtractPath,
        [string]$Description
    )

    Write-Host "Extracting $Description..."
    try {
        Expand-Archive -Path $ZipFile -DestinationPath $ExtractPath -Force
        Write-Host "[SUCCESS] $Description extracted successfully" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "[ERROR] Failed to extract $Description" -ForegroundColor Red
        return $false
    }
}

# Ask for confirmation
$confirmation = Read-Host "Continue with installation? (y/n)"
if ($confirmation -ne "y") {
    Write-Host "Installation cancelled."
    Read-Host "Press Enter to exit"
    exit 0
}

Write-Host ""
Write-Host "Starting installation process..." -ForegroundColor Cyan
Write-Host ""

# Install QEMU
Write-Host "[1/4] Installing QEMU..." -ForegroundColor Yellow
$QemuUrl = "https://qemu.weilnetz.de/w64/2024/qemu-w64-setup-2024-08-20.exe"
$QemuInstaller = "qemu-installer.exe"

if (Download-File -Url $QemuUrl -OutputFile $QemuInstaller -Description "QEMU") {
    Write-Host "Installing QEMU to C:\qemu\..."
    Start-Process -FilePath $QemuInstaller -ArgumentList "/S", "/D=C:\qemu" -Wait

    if (Test-Path "C:\qemu\qemu-system-i386.exe") {
        Write-Host "[SUCCESS] QEMU installed successfully" -ForegroundColor Green
    } else {
        Write-Host "[WARNING] QEMU installation may have failed" -ForegroundColor Yellow
    }
    Remove-Item $QemuInstaller -Force
}

Write-Host ""

# Install NASM
Write-Host "[2/4] Installing NASM..." -ForegroundColor Yellow
$NasmUrl = "https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip"
$NasmZip = "nasm.zip"

if (Download-File -Url $NasmUrl -OutputFile $NasmZip -Description "NASM") {
    if (Extract-ZipFile -ZipFile $NasmZip -ExtractPath "C:\nasm" -Description "NASM") {
        if (Test-Path "C:\nasm\nasm.exe") {
            Write-Host "[SUCCESS] NASM installed successfully" -ForegroundColor Green
        } else {
            Write-Host "[WARNING] NASM installation may have failed" -ForegroundColor Yellow
        }
    }
    Remove-Item $NasmZip -Force
}

Write-Host ""

# Install MinGW-w64
Write-Host "[3/4] Installing MinGW-w64..." -ForegroundColor Yellow
Write-Host "This is a large download (100MB+), please be patient..." -ForegroundColor Cyan
$MinGWUrl = "https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z"
$MinGWZip = "mingw64.7z"

if (Download-File -Url $MinGWUrl -OutputFile $MinGWZip -Description "MinGW-w64") {
    if (Extract-ZipFile -ZipFile $MinGWZip -ExtractPath "C:\mingw64_temp" -Description "MinGW-w64") {
        # Move files to correct location
        if (Test-Path "C:\mingw64_temp\mingw64\bin\gcc.exe") {
            Write-Host "Organizing MinGW-w64 files..."
            Move-Item "C:\mingw64_temp\mingw64\*" "C:\mingw64\" -Force
            Remove-Item "C:\mingw64_temp" -Recurse -Force

            if (Test-Path "C:\mingw64\bin\gcc.exe") {
                Write-Host "[SUCCESS] MinGW-w64 installed successfully" -ForegroundColor Green
            } else {
                Write-Host "[WARNING] MinGW-w64 installation may have failed" -ForegroundColor Yellow
            }
        } else {
            Write-Host "[WARNING] MinGW-w64 extraction may have failed" -ForegroundColor Yellow
        }
    }
    Remove-Item $MinGWZip -Force
}

Write-Host ""

# Install Git
Write-Host "[4/4] Installing Git..." -ForegroundColor Yellow
$GitUrl = "https://github.com/git-for-windows/git/releases/download/v2.47.0.windows.2/Git-2.47.0.2-64-bit.exe"
$GitInstaller = "git-installer.exe"

if (Download-File -Url $GitUrl -OutputFile $GitInstaller -Description "Git") {
    Write-Host "Installing Git..."
    Start-Process -FilePath $GitInstaller -ArgumentList "/VERYSILENT", "/NORESTART", "/DIR=`"C:\Program Files\Git`"" -Wait

    if (Test-Path "C:\Program Files\Git\bin\git.exe") {
        Write-Host "[SUCCESS] Git installed successfully" -ForegroundColor Green
    } else {
        Write-Host "[WARNING] Git installation may have failed" -ForegroundColor Yellow
    }
    Remove-Item $GitInstaller -Force
}

Write-Host ""
Write-Host "Installation process completed!" -ForegroundColor Green
Write-Host ""

# Update system PATH
Write-Host "Updating system PATH..." -ForegroundColor Yellow
$NewPath = "C:\qemu;C:\nasm;C:\mingw64\bin;C:\Program Files\Git\bin"
$CurrentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
if ($CurrentPath -notlike "*$NewPath*") {
    try {
        [Environment]::SetEnvironmentVariable("PATH", "$CurrentPath;$NewPath", "Machine")
        Write-Host "[SUCCESS] System PATH updated" -ForegroundColor Green
    } catch {
        Write-Host "[WARNING] Could not update system PATH automatically" -ForegroundColor Yellow
        Write-Host "Please add these directories to your PATH manually:" -ForegroundColor Yellow
        Write-Host "  C:\qemu"
        Write-Host "  C:\nasm"
        Write-Host "  C:\mingw64\bin"
        Write-Host "  C:\Program Files\Git\bin"
    }
} else {
    Write-Host "[INFO] PATH already contains required directories" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "Creating test script..." -ForegroundColor Yellow

# Create test script
$TestScript = @"
@echo off
echo Testing Vest-OS Development Environment
echo ========================================

echo Testing QEMU...
qemu-system-i386 --version >nul 2>&1
if %%errorlevel%% equ 0 (
    echo [SUCCESS] QEMU is available
) else (
    echo [ERROR] QEMU not found
)

echo.
echo Testing NASM...
nasm --version >nul 2>&1
if %%errorlevel%% equ 0 (
    echo [SUCCESS] NASM is available
) else (
    echo [ERROR] NASM not found
)

echo.
echo Testing GCC...
gcc --version >nul 2>&1
if %%errorlevel%% equ 0 (
    echo [SUCCESS] GCC is available
) else (
    echo [ERROR] GCC not found
)

echo.
echo Testing Git...
git --version >nul 2>&1
if %%errorlevel%% equ 0 (
    echo [SUCCESS] Git is available
) else (
    echo [ERROR] Git not found
)

echo.
echo Test completed!
pause
"@

$TestScript | Out-File -FilePath "test_environment.bat" -Encoding ASCII

Write-Host ""
Write-Host "Installation Summary:" -ForegroundColor Cyan
Write-Host "- QEMU: C:\qemu\"
Write-Host "- NASM: C:\nasm\"
Write-Host "- MinGW-w64: C:\mingw64\"
Write-Host "- Git: C:\Program Files\Git\"
Write-Host ""

Write-Host "IMPORTANT: Please restart PowerShell or Command Prompt before using the tools." -ForegroundColor Yellow
Write-Host ""

Write-Host "To test your installation, run:" -ForegroundColor Cyan
Write-Host "  test_environment.bat"
Write-Host ""

Write-Host "To build Vest-OS:" -ForegroundColor Cyan
Write-Host "  cd K:\vestos\Vest-OS"
Write-Host "  nasm -f bin minimal_kernel.asm -o kernel.bin"
Write-Host "  qemu-system-i386 -kernel kernel.bin"
Write-Host ""

Write-Host "If any downloads failed, please download them manually:" -ForegroundColor Yellow
Write-Host "- QEMU: https://qemu.weilnetz.de/w64/"
Write-Host "- NASM: https://nasm.us/"
Write-Host "- MinGW-w64: https://sourceforge.net/projects/mingw-w64/"
Write-Host "- Git: https://git-scm.com/download/win"
Write-Host ""

Read-Host "Press Enter to exit"