@echo off
:: Quick test for Vest-OS development environment
:: Run this after you've installed the tools

echo Vest-OS Quick Environment Test
echo ===============================

:: Test QEMU
echo [1/4] Testing QEMU...
qemu-system-i386 --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] QEMU is working
    qemu-system-i386 --version | findstr /R "qemu-[0-9]"
) else (
    echo [✗] QEMU not found or not working
    echo   Please ensure C:\qemu is in your PATH
)

echo.

:: Test NASM
echo [2/4] Testing NASM...
nasm --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] NASM is working
    nasm --version | findstr /R "NASM version"
) else (
    echo [✗] NASM not found or not working
    echo   Please ensure C:\nasm is in your PATH
)

echo.

:: Test GCC
echo [3/4] Testing GCC...
gcc --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] GCC is working
    gcc --version | findstr /R "gcc.*gcc"
) else (
    echo [✗] GCC not found or not working
    echo   Please ensure C:\mingw64\bin is in your PATH
)

echo.

:: Test Git
echo [4/4] Testing Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [✓] Git is working
    git --version
) else (
    echo [✗] Git not found or not working
    echo   Please ensure Git is in your PATH
)

echo.
echo ========================================
echo Test Summary:
echo Check above results. All tools should show [✓]
echo.

:: Now test Vest-OS build if tools are working
echo Testing Vest-OS build...
echo.

if exist "kernel.bin" (
    echo [INFO] kernel.bin already exists, rebuilding...
    del kernel.bin
)

if exist "minimal_kernel.asm" (
    echo Assembling Vest-OS kernel...
    nasm -f bin minimal_kernel.asm -o kernel.bin
    if %errorlevel% equ 0 (
        echo [✓] Kernel assembled successfully
        for %%F in (kernel.bin) do echo   File size: %%~zF bytes

        echo.
        echo Testing kernel in QEMU...
        echo Starting QEMU (close the QEMU window to continue)...
        qemu-system-i386 -kernel kernel.bin
        if %errorlevel% equ 0 (
            echo [✓] QEMU test completed
        ) else (
            echo [!] QEMU was closed by user or encountered an error
        )
    ) else (
        echo [✗] Kernel assembly failed
    )
) else (
    echo [ERROR] minimal_kernel.asm not found
    echo Please ensure you're in the Vest-OS directory
)

echo.
echo ===============================
echo Test completed!
echo.

:: Provide instructions for next steps
echo Next steps:
echo 1. If any tool shows [✗], run: complete_setup.bat
echo 2. If build failed, check the error messages above
echo 3. To manually build: nasm -f bin minimal_kernel.asm -o kernel.bin
echo 4. To manually test: qemu-system-i386 -kernel kernel.bin
echo.

pause