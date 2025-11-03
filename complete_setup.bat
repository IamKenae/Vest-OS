@echo off
:: Vest-OS Development Environment Completion Script
:: This script helps complete the setup when tools are already partially installed

echo Vest-OS Development Environment Completion
echo ==========================================
echo.

:: Check current installation status
echo Checking current installation status...
echo.

echo [1/4] Checking QEMU...
qemu-system-i386 --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] QEMU is available
    qemu-system-i386 --version | findstr /R "qemu"
) else (
    echo [ERROR] QEMU not found in PATH
)

echo.
echo [2/4] Checking NASM...
nasm --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] NASM is available
    nasm --version | findstr /R "NASM"
) else (
    echo [ERROR] NASM not found in PATH
)

echo.
echo [3/4] Checking Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] Git is available
    git --version
) else (
    echo [ERROR] Git not found in PATH
)

echo.
echo [4/4] Setting up MinGW-w64...

:: Look for the downloaded MinGW-w64 file
set MINGW_FILE=x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z
set MINGW_FOUND=0

:: Check common download locations
if exist "%USERPROFILE%\Downloads\%MINGW_FILE%" (
    set MINGW_PATH=%USERPROFILE%\Downloads
    set MINGW_FOUND=1
    echo Found MinGW-w64 in Downloads folder
) else if exist "C:\Users\%USERNAME%\Downloads\%MINGW_FILE%" (
    set MINGW_PATH=C:\Users\%USERNAME%\Downloads
    set MINGW_FOUND=1
    echo Found MinGW-w64 in Downloads folder
) else if exist ".\%MINGW_FILE%" (
    set MINGW_PATH=%CD%
    set MINGW_FOUND=1
    echo Found MinGW-w64 in current directory
) else if exist "C:\vestos_dev_tools\%MINGW_FILE%" (
    set MINGW_PATH=C:\vestos_dev_tools
    set MINGW_FOUND=1
    echo Found MinGW-w64 in vestos_dev_tools folder
) else (
    echo MinGW-w64 file not found in common locations
    echo Please locate the file: %MINGW_FILE%
    echo.
    set /p MINGW_PATH="Enter the full path to the directory containing the .7z file: "
    if exist "%MINGW_PATH%\%MINGW_FILE%" (
        set MINGW_FOUND=1
        echo Found MinGW-w64 at specified location
    ) else (
        echo [ERROR] MinGW-w64 file not found at specified location
        goto :manual_setup
    )
)

if %MINGW_FOUND% equ 1 (
    echo Extracting MinGW-w64...
    echo This may take a minute as it's a large file...

    :: Create installation directory
    if not exist "C:\mingw64_temp" mkdir "C:\mingw64_temp"

    :: Extract using PowerShell (more reliable than built-in extraction)
    powershell -Command "try { Expand-Archive -Path '%MINGW_PATH%\%MINGW_FILE%' -DestinationPath 'C:\mingw64_temp' -Force; Write-Host 'Extraction successful' } catch { Write-Host 'Extraction failed - trying 7-Zip method' }"

    :: Check if extraction worked
    if exist "C:\mingw64_temp\mingw64\bin\gcc.exe" (
        echo Moving MinGW-w64 to final location...
        :: Remove old installation if exists
        if exist "C:\mingw64" rmdir /s /q "C:\mingw64"
        :: Move to final location
        move "C:\mingw64_temp\mingw64" "C:\mingw64"
        :: Clean up temp directory
        rmdir /s /q "C:\mingw64_temp"

        if exist "C:\mingw64\bin\gcc.exe" (
            echo [SUCCESS] MinGW-w64 installed to C:\mingw64\
        ) else (
            echo [ERROR] Failed to move MinGW-w64 files
            goto :manual_setup
        )
    ) else (
        echo [ERROR] MinGW-w64 extraction failed
        echo Trying alternative method...
        goto :manual_setup
    )
)

:check_gcc
echo.
echo Testing MinGW-w64 GCC...
if exist "C:\mingw64\bin\gcc.exe" (
    echo [SUCCESS] MinGW-w64 GCC found
    "C:\mingw64\bin\gcc.exe" --version | findstr /R "gcc"
) else (
    echo [ERROR] GCC not found at C:\mingw64\bin\gcc.exe
    goto :manual_setup
)

:update_path
echo.
echo Updating system PATH...
set PATH_TO_ADD=C:\qemu;C:\nasm;C:\mingw64\bin;C:\Program Files\Git\bin

:: Check if paths are already in PATH
echo %PATH% | findstr /C:"C:\mingw64\bin" >nul
if %errorlevel% neq 0 (
    echo Adding MinGW-w64 to PATH...
    setx PATH "%PATH%;%PATH_TO_ADD%" /M
    echo [SUCCESS] System PATH updated
) else (
    echo MinGW-w64 already in PATH
)

:final_test
echo.
echo Running final environment test...
echo ========================================

echo Testing QEMU...
qemu-system-i386 --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] QEMU is working
) else (
    echo [ERROR] QEMU test failed
)

echo.
echo Testing NASM...
nasm --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] NASM is working
) else (
    echo [ERROR] NASM test failed
)

echo.
echo Testing GCC...
gcc --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] GCC is working
) else (
    echo [ERROR] GCC test failed
)

echo.
echo Testing Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] Git is working
) else (
    echo [ERROR] Git test failed
)

echo.
echo Environment setup completed!
echo.
echo Now you can build Vest-OS:
echo   cd K:\vestos\Vest-OS
echo   nasm -f bin minimal_kernel.asm -o kernel.bin
echo   qemu-system-i386 -kernel kernel.bin
echo.
echo IMPORTANT: Please restart Command Prompt to use the updated PATH.
echo.
goto :end

:manual_setup
echo.
echo Manual setup instructions:
echo =========================
echo.
echo 1. Extract the MinGW-w64 .7z file manually:
echo    - Right-click on x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z
echo    - Select "Extract All..." or "Extract Here"
echo    - Extract to: C:\mingw64\
echo    - You should see C:\mingw64\bin\gcc.exe after extraction
echo.
echo 2. Add to system PATH:
echo    - Press Win+R, type "sysdm.cpl", press Enter
echo    - Go to Advanced -> Environment Variables
echo    - Find "Path" in System variables, click Edit
echo    - Add: C:\qemu
echo    - Add: C:\nasm
echo    - Add: C:\mingw64\bin
echo    - Add: C:\Program Files\Git\bin
echo    - Click OK on all windows
echo.
echo 3. Restart Command Prompt
echo.
echo 4. Test the installation by running this script again
echo.

:end
pause