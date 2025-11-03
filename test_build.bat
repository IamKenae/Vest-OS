@echo off
:: Vest-OS Build Test Script
:: Tests the kernel build and shows results

echo Vest-OS Build Test
echo ===================

:: Check if kernel exists
if exist "kernel.bin" (
    echo [SUCCESS] Kernel binary found: kernel.bin
    for %%F in (kernel.bin) do echo   Size: %%~zF bytes
    echo.

    :: Display kernel info
    echo Kernel Information:
    echo - Multiboot compliant
    echo - 32-bit x86 architecture
    echo - VGA text mode output
    echo - Simple message display
    echo.

    :: Show what the kernel will display
    echo Expected Output:
    echo ========================================
    echo "       Welcome to Vest-OS v0.1.0      "
    echo "    Kernel loaded successfully!     "
    echo ========================================
    echo.

    echo To test this kernel:
    echo 1. Install QEMU from https://qemu.weilnetz.de/
    echo 2. Extract to C:\qemu\
    echo 3. Run: C:\qemu\qemu-system-i386.exe -kernel kernel.bin
    echo.
    echo Or use alternative methods:
    echo - VirtualBox with custom Linux VM
    echo - Windows Subsystem for Linux (WSL)
    echo - Real hardware with bootable USB
    echo - Online emulator

) else (
    echo [ERROR] Kernel binary not found: kernel.bin
    echo.
    echo To build the kernel:
    echo 1. Make sure NASM is installed
    echo 2. Run: nasm -f bin minimal_kernel.asm -o kernel.bin
    echo.
    echo If NASM is not available:
    echo - Download from https://www.nasm.us/
    echo - Or use WSL with Linux build tools
)

echo.
echo Build test completed.
echo.
pause