@echo off
:: Vest-OS Build and Test Script
:: This script tests the environment and builds Vest-OS

echo Vest-OS Build and Test Script
echo ==============================

:: Change to Vest-OS directory
cd /d "%~dp0"
echo Current directory: %CD%
echo.

:: Test all tools
echo Testing development tools...
echo.

:: Test QEMU
echo [1/5] Testing QEMU...
qemu-system-i386 --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] QEMU is available
    for /f "tokens=*" %%i in ('qemu-system-i386 --version 2^>nul ^| findstr /R "qemu-[0-9]"') do echo   %%i
) else (
    echo [✗] QEMU not found - please install QEMU to C:\qemu\
    goto :error
)

:: Test NASM
echo [2/5] Testing NASM...
nasm --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] NASM is available
    for /f "tokens=*" %%i in ('nasm -v 2^>nul ^| findstr /R "NASM version"') do echo   %%i
) else (
    echo [✗] NASM not found - please install NASM to C:\nasm\
    goto :error
)

:: Test GCC
echo [3/5] Testing GCC...
gcc --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] GCC is available
    for /f "tokens=*" %%i in ('gcc --version 2^>nul ^| findstr /R "gcc.*gcc"') do echo   %%i
) else (
    echo [✗] GCC not found - please setup MinGW-w64
    echo   Run: setup_mingw_now.bat
    goto :error
)

:: Test Git
echo [4/5] Testing Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] Git is available
    git --version
) else (
    echo [!] Git not found - optional for basic usage
)

:: Check kernel source
echo [5/5] Checking Vest-OS source files...
if exist "minimal_kernel.asm" (
    echo [✓] Found kernel source: minimal_kernel.asm
) else (
    echo [✗] Kernel source not found
    goto :error
)

echo.
echo All tools are available! Starting build process...
echo.

:: Clean previous build
if exist "kernel.bin" (
    echo Removing previous build...
    del kernel.bin
)

:: Build the kernel
echo Building Vest-OS kernel...
echo Command: nasm -f bin minimal_kernel.asm -o kernel.bin
nasm -f bin minimal_kernel.asm -o kernel.bin

if %errorlevel% equ 0 (
    echo [SUCCESS] Kernel built successfully!

    if exist "kernel.bin" (
        for %%F in (kernel.bin) do echo   Kernel file: %%F (%%~zF bytes)

        echo.
        echo Kernel build details:
        echo - Architecture: 32-bit x86
        echo - Format: Flat binary
        echo - Multiboot: Yes
        echo - Entry point: 0x100000
        echo.

        :ask_test
        echo Ready to test the kernel in QEMU!
        set /p test_choice="Test kernel in QEMU now? (y/n): "

        if /i "%test_choice%"=="y" (
            echo.
            echo Starting QEMU...
            echo The kernel will display:
            echo "========================================"
            echo "      Welcome to Vest-OS v0.1.0"
            echo "========================================"
            echo "    Kernel loaded successfully!"
            echo "========================================"
            echo.
            echo Close the QEMU window to continue...
            echo.

            qemu-system-i386 -kernel kernel.bin

            echo.
            echo QEMU test completed.
        ) else (
            echo.
            echo To test the kernel later, run:
            echo   qemu-system-i386 -kernel kernel.bin
        )

    ) else (
        echo [ERROR] kernel.bin file not found after build
        goto :error
    )

) else (
    echo [ERROR] Kernel build failed
    echo Check the error messages above
    goto :error
)

echo.
echo ========================================
echo Build and Test Completed!
echo ========================================
echo.
echo Summary:
echo - Development environment: Working
echo - Kernel build: Successful
echo - File created: kernel.bin
echo.
echo To rebuild the kernel:
echo   nasm -f bin minimal_kernel.asm -o kernel.bin
echo.
echo To test the kernel:
echo   qemu-system-i386 -kernel kernel.bin
echo.
goto :end

:error
echo.
echo ========================================
echo Setup Required
echo ========================================
echo.
echo To fix the issues above:
echo.
echo 1. For QEMU issues:
echo    - Install QEMU to C:\qemu\
echo    - Add C:\qemu to system PATH
echo.
echo 2. For NASM issues:
echo    - Install NASM to C:\nasm\
echo    - Add C:\nasm to system PATH
echo.
echo 3. For GCC issues:
echo    - Run: setup_mingw_now.bat
echo    - Or follow the manual steps in MINGW_SETUP.md
echo.
echo 4. After fixing issues, restart Command Prompt and run this script again.
echo.

:end
pause