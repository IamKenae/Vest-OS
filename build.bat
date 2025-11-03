@echo off
:: Vest-OS Build Script for Windows
:: Uses available tools to build a simple kernel

echo Vest-OS Build Script for Windows
echo ================================

:: Check if files exist
if not exist "simple_kernel.c" (
    echo ERROR: simple_kernel.c not found
    exit /b 1
)

if not exist "boot.asm" (
    echo ERROR: boot.asm not found
    exit /b 1
)

:: Try different approaches to build
echo Attempting to build Vest-OS kernel...

:: Method 1: Try with 32-bit GCC if available
echo.
echo Method 1: Trying 32-bit GCC...
gcc --version | findstr "i686" >nul
if %errorlevel% == 0 (
    echo Found 32-bit GCC
    gcc -m32 -nostdlib -ffreestanding -c simple_kernel.c -o simple_kernel.o
    if %errorlevel% == 0 (
        nasm -f elf32 boot.asm -o boot.o
        if %errorlevel% == 0 (
            ld -m elf_i386 -T linker.ld -o kernel.bin boot.o simple_kernel.o
            if %errorlevel% == 0 (
                echo SUCCESS: kernel.bin created
                goto :success
            )
        )
    )
)

:: Method 2: Try with objcopy to convert format
echo.
echo Method 2: Trying with format conversion...
gcc -c simple_kernel.c -o simple_kernel_mingw.o
if %errorlevel% == 0 (
    nasm -f win32 boot.asm -o boot_mingw.o
    if %errorlevel% == 0 (
        echo Files compiled successfully
        echo Note: Cannot create ELF kernel on MinGW without cross-compiler
        echo But we have object files that could be linked with proper tools
        goto :partial_success
    )
)

:: Method 3: Create a simple flat binary
echo.
echo Method 3: Creating flat binary kernel...
nasm -f bin boot.asm -o kernel.bin
if %errorlevel% == 0 (
    echo Created basic kernel binary (C code not included)
    goto :basic_success
)

echo ERROR: All build methods failed
exit /b 1

:success
echo.
echo Build completed successfully!
echo File created: kernel.bin
echo Size:
dir kernel.bin | findstr kernel.bin
echo.
echo You can now test this kernel with QEMU:
echo   qemu-system-i386 -kernel kernel.bin
goto :end

:partial_success
echo.
echo Partial success - Object files created:
echo   simple_kernel_mingw.o
echo   boot_mingw.o
echo.
echo Note: Full kernel linking requires cross-compiler or Linux environment
echo However, the components are ready for integration
goto :end

:basic_success
echo.
echo Basic kernel binary created: kernel.bin
echo This contains only the assembly boot code
echo Full C kernel compilation requires proper cross-compiler
goto :end

:end
echo.
echo Build script completed.