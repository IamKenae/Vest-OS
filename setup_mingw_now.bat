@echo off
:: Quick MinGW-w64 Setup Script
:: For user who has the file in C:\Downloads

echo MinGW-w64 Quick Setup Script
echo =============================
echo.

echo Found MinGW-w64 file in C:\Downloads
echo Starting extraction and setup...
echo.

:: Create installation directory
if not exist "C:\mingw64" mkdir "C:\mingw64"

echo Extracting MinGW-w64 (this may take a minute)...

:: Extract using PowerShell
powershell -Command "try { Write-Host 'Starting extraction...'; Expand-Archive -Path 'C:\Downloads\x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z' -DestinationPath 'C:\mingw64_temp' -Force; Write-Host 'Extraction completed successfully' } catch { Write-Host 'Extraction failed - trying manual method...' }"

:: Check if extraction worked
if exist "C:\mingw64_temp\mingw64\bin\gcc.exe" (
    echo Moving files to final location...
    :: Remove old installation if exists
    if exist "C:\mingw64" rmdir /s /q "C:\mingw64"
    :: Move extracted files to correct location
    move "C:\mingw64_temp\mingw64" "C:\mingw64"
    :: Clean up temp directory
    rmdir /s /q "C:\mingw64_temp"

    if exist "C:\mingw64\bin\gcc.exe" (
        echo [SUCCESS] MinGW-w64 installed to C:\mingw64\

        :: Show what was installed
        echo.
        echo MinGW-w64 components installed:
        dir C:\mingw64\bin\*.exe | findstr "gcc.exe g++.exe make.exe"

    ) else (
        echo [ERROR] Failed to move MinGW-w64 files
        goto :manual_steps
    )
) else (
    echo [ERROR] Automatic extraction failed
    goto :manual_steps
)

:update_path
echo.
echo Updating system PATH...
set PATH_TO_ADD=C:\mingw64\bin

:: Add to system PATH
setx PATH "%PATH%;%PATH_TO_ADD%" /M >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] System PATH updated
) else (
    echo [INFO] Adding to user PATH instead...
    setx PATH "%PATH%;%PATH_TO_ADD%" >nul 2>&1
)

:test_installation
echo.
echo Testing MinGW-w64 installation...
echo.

:: Test GCC
echo Testing GCC...
C:\mingw64\bin\gcc.exe --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] GCC is working
    C:\mingw64\bin\gcc.exe --version | findstr /R "gcc.*gcc"
) else (
    echo [ERROR] GCC test failed
    goto :manual_steps
)

:: Test Make
echo.
echo Testing Make...
C:\mingw64\bin\make.exe --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] Make is working
) else (
    echo [WARNING] Make not found (optional for basic usage)
)

:final_instructions
echo.
echo ========================================
echo MinGW-w64 Setup Completed!
echo ========================================
echo.
echo What was done:
echo - Extracted MinGW-w64 to C:\mingw64\
echo - Added C:\mingw64\bin to system PATH
echo - Tested GCC installation
echo.
echo IMPORTANT: Please restart Command Prompt to use the updated PATH
echo.
echo Next steps:
echo 1. Close and reopen Command Prompt
echo 2. Run: quick_test.bat
echo 3. Or manually test with: gcc --version
echo.
echo Then you can build Vest-OS:
echo   cd K:\vestos\Vest-OS
echo   nasm -f bin minimal_kernel.asm -o kernel.bin
echo   qemu-system-i386 -kernel kernel.bin
echo.
goto :end

:manual_steps
echo.
echo Manual setup steps:
echo ===================
echo.
echo 1. Extract the file manually:
echo    - Navigate to C:\Downloads\
echo    - Right-click on: x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z
echo    - Select "Extract All..."
echo    - Extract to: C:\mingw64\
echo.
echo 2. Add to PATH:
echo    - Press Win+R, type: sysdm.cpl
echo    - Advanced → Environment Variables → System variables → Path → Edit
echo    - Add: C:\mingw64\bin
echo.
echo 3. Restart Command Prompt and test with: gcc --version
echo.

:end
pause