# Vest-OS Development Environment Setup Guide

## Overview

This guide provides complete instructions for downloading and setting up the development environment needed to build and test Vest-OS on Windows.

## Required Tools

### 1. QEMU Emulator
**Purpose**: Run and test Vest-OS in a virtual machine

**Download Options:**
- **Official Windows Binaries**: https://qemu.weilnetz.de/w64/
- **Latest Version**: Check https://qemu.weilnetz.de/w64/2024/ for the newest release
- **Direct Download**: Look for `qemu-w64-setup-<version>.exe`

**Installation:**
1. Download the 64-bit Windows installer
2. Run the installer as Administrator
3. Install to `C:\qemu\` (recommended)
4. Add `C:\qemu\` to system PATH

**Alternative Downloads:**
- GitHub Releases: https://github.com/qemu/qemu/releases
- Chocolatey: `choco install qemu`
- Scoop: `scoop install qemu`

### 2. NASM Assembler
**Purpose**: Compile assembly code for Vest-OS kernel

**Download Options:**
- **Official Site**: https://nasm.us/
- **Windows Installer**: Look for `nasm-<version>-win64.zip` or installer
- **Latest Version**: Check https://nasm.us/doc/nasmdocc.html for current version

**Installation:**
1. Download the Windows 64-bit package
2. Extract to `C:\nasm\` or install via installer
3. Add `C:\nasm\` to system PATH
4. Verify installation: `nasm --version`

**Alternative Downloads:**
- Winget: `winget install nasm`
- Chocolatey: `choco install nasm`

### 3. C Compiler (MinGW-w64 or MSYS2)

#### Option A: MinGW-w64 (Recommended)
**Purpose**: Compile C code for Vest-OS

**Download Options:**
- **SourceForge**: https://sourceforge.net/projects/mingw-w64/
- **Direct Download**: Look for `x86_64-posix-seh-rev<number>.7z`

**Installation:**
1. Download the 64-bit POSIX version
2. Extract to `C:\mingw64\`
3. Add `C:\mingw64\bin` to system PATH
4. Verify installation: `gcc --version`

#### Option B: MSYS2 (Alternative)
**Purpose**: Complete Linux-like development environment

**Download and Installation:**
1. **Download**: https://www.msys2.org/
2. **Run installer**: Choose installation path (e.g., `C:\msys64`)
3. **Update packages**: Run MSYS2 and execute:
   ```bash
   pacman -Syu
   pacman -Su
   ```
4. **Install development tools**:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-nasm
   pacman -S mingw-w64-x86_64-make
   ```

### 4. Additional Tools (Optional but Recommended)

#### Git
**Purpose**: Version control for Vest-OS source code

**Download**: https://git-scm.com/download/win

#### Visual Studio Code
**Purpose**: Code editor with excellent development features

**Download**: https://code.visualstudio.com/download

**Recommended Extensions:**
- C/C++ Extension Pack
- Makefile Tools
- GitLens

## Quick Installation Script

I've created automated installation scripts for you:

### Windows Batch Script
```batch
:: Download and install Vest-OS development environment
:: Save as install_vest_os_env.bat

echo Vest-OS Development Environment Installer
echo =========================================

:: Create directories
mkdir C:\vestos_tools 2>nul
cd /d C:\vestos_tools

:: Download URLs (update these with latest versions)
set QEMU_URL=https://qemu.weilnetz.de/w64/2024/qemu-w64-setup-2024-08-20.exe
set NASM_URL=https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip
set MINGW_URL=https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z

echo Downloading QEMU...
powershell -Command "Invoke-WebRequest -Uri '%QEMU_URL%' -OutFile 'qemu-installer.exe'"
echo Installing QEMU...
qemu-installer.exe /S /D=C:\qemu

echo Downloading NASM...
powershell -Command "Invoke-WebRequest -Uri '%NASM_URL%' -OutFile 'nasm.zip'"
echo Extracting NASM...
powershell -Command "Expand-Archive -Path 'nasm.zip' -DestinationPath 'C:\nasm'"

echo Downloading MinGW-w64...
powershell -Command "Invoke-WebRequest -Uri '%MINGW_URL%' -OutFile 'mingw64.7z'"
echo Extracting MinGW-w64...
powershell -Command "Expand-Archive -Path 'mingw64.7z' -DestinationPath 'C:\mingw64'"

echo Setting up PATH...
setx PATH "%PATH%;C:\qemu;C:\nasm;C:\mingw64\bin" /M

echo Installation complete!
echo Please restart your terminal and run: test_vest_os_env.bat
```

### Environment Test Script
```batch
:: Test Vest-OS development environment
:: Save as test_vest_os_env.bat

echo Testing Vest-OS Development Environment
echo ========================================

echo Testing QEMU...
qemu-system-i386 --version
if %errorlevel% neq 0 (
    echo [ERROR] QEMU not found in PATH
) else (
    echo [SUCCESS] QEMU is available
)

echo.
echo Testing NASM...
nasm --version
if %errorlevel% neq 0 (
    echo [ERROR] NASM not found in PATH
) else (
    echo [SUCCESS] NASM is available
)

echo.
echo Testing GCC...
gcc --version
if %errorlevel% neq 0 (
    echo [ERROR] GCC not found in PATH
) else (
    echo [SUCCESS] GCC is available
)

echo.
echo Testing MAKE...
make --version
if %errorlevel% neq 0 (
    echo [WARNING] MAKE not found (optional)
) else (
    echo [SUCCESS] MAKE is available
)

echo.
echo Environment test completed!
```

## Step-by-Step Manual Installation

### Step 1: Install QEMU

1. **Visit**: https://qemu.weilnetz.de/w64/
2. **Download**: Latest `qemu-w64-setup-<version>.exe`
3. **Run installer**: Right-click → "Run as administrator"
4. **Installation directory**: `C:\qemu\` (recommended)
5. **Add to PATH**: Control Panel → System → Advanced → Environment Variables

### Step 2: Install NASM

1. **Visit**: https://nasm.us/
2. **Download**: Windows 64-bit version (`.zip` or `.exe`)
3. **Extract**: To `C:\nasm\`
4. **Add to PATH**: Add `C:\nasm\` to system PATH
5. **Verify**: Open Command Prompt and run `nasm --version`

### Step 3: Install MinGW-w64

#### Option A: Manual Installation
1. **Visit**: https://sourceforge.net/projects/mingw-w64/
2. **Download**: `x86_64-posix-seh-rev<number>.7z`
3. **Extract**: To `C:\mingw64\`
4. **Add to PATH**: Add `C:\mingw64\bin` to system PATH
5. **Verify**: `gcc --version`

#### Option B: MSYS2 Installation
1. **Visit**: https://www.msys2.org/
2. **Download**: `msys2-x86_64-<date>.exe`
3. **Install**: Run installer, choose path like `C:\msys64`
4. **Update**: Open MSYS2 and run:
   ```bash
   pacman -Syu
   pacman -Su
   ```
5. **Install tools**:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-nasm
   pacman -S mingw-w64-x86_64-make
   ```

### Step 4: Configure Environment Variables

1. **Open**: Control Panel → System → Advanced system settings
2. **Click**: Environment Variables
3. **Edit**: PATH variable under System variables
4. **Add**: `C:\qemu;C:\nasm;C:\mingw64\bin` (adjust paths as needed)
5. **Restart**: Command Prompt to apply changes

## Verification

After installation, verify everything works:

```bash
# Test QEMU
qemu-system-i386 --version

# Test NASM
nasm --version

# Test GCC
gcc --version

# Test MAKE (optional)
make --version
```

## Building Vest-OS

Once environment is set up:

```bash
# Navigate to Vest-OS directory
cd K:\vestos\Vest-OS

# Build kernel
nasm -f bin minimal_kernel.asm -o kernel.bin

# Test with QEMU
qemu-system-i386 -kernel kernel.bin
```

## Troubleshooting

### Common Issues

1. **"Command not found" errors**:
   - Ensure paths are added to system PATH
   - Restart Command Prompt after changing PATH
   - Check if tools are installed in correct directories

2. **QEMU installation issues**:
   - Run installer as administrator
   - Try different QEMU versions if latest doesn't work
   - Check Windows version compatibility

3. **NASM issues**:
   - Download correct version (64-bit for 64-bit Windows)
   - Ensure NASM is in PATH
   - Try different download sources if official site is down

4. **MinGW issues**:
   - Use MSYS2 as alternative to manual MinGW
   - Ensure using 64-bit version on 64-bit Windows
   - Check antivirus software isn't blocking installation

### Getting Help

If you encounter issues:

1. **Check**: All tools are in system PATH
2. **Restart**: Command Prompt after PATH changes
3. **Verify**: Downloaded files are not corrupted
4. **Try**: Alternative installation methods
5. **Consult**: Tool-specific documentation

## Alternative: Using WSL (Windows Subsystem for Linux)

If Windows tools cause issues, consider WSL:

1. **Enable WSL**: In PowerShell (as admin):
   ```powershell
   wsl --install
   ```

2. **Install Ubuntu** (or preferred distribution)

3. **Install tools in WSL**:
   ```bash
   sudo apt update
   sudo apt install qemu-system nasm gcc make
   ```

4. **Access Windows files**: `/mnt/c/vestos/`

## Recommended Installation Order

1. **QEMU** (virtualization platform)
2. **NASM** (assembler)
3. **MinGW-w64 or MSYS2** (C compiler)
4. **Git** (version control)
5. **VS Code** (code editor)

## File Locations Summary

```
C:\qemu\                    - QEMU installation
C:\nasm\                    - NASM installation
C:\mingw64\                 - MinGW-w64 installation
C:\msys64\                  - MSYS2 installation (if used)
K:\vestos\Vest-OS\         - Vest-OS source code
```

---

**Last Updated**: November 3, 2024
**Version**: 1.0
**Compatible**: Windows 10/11 (64-bit)