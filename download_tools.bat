@echo off
:: Vest-OS Development Environment Downloader
:: This script downloads all necessary tools for Vest-OS development

echo Vest-OS Development Environment Downloader
echo ============================================
echo.

:: Check if running as administrator
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Please run this script as Administrator
    echo Right-click the script and select "Run as administrator"
    pause
    exit /b 1
)

:: Create installation directory
set INSTALL_DIR=C:\vestos_dev_tools
echo Creating installation directory: %INSTALL_DIR%
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
cd /d "%INSTALL_DIR%"

echo.
echo This script will download:
echo 1. QEMU (Virtual Machine Emulator)
echo 2. NASM (Assembler)
echo 3. MinGW-w64 (C Compiler)
echo 4. Git (Version Control)
echo.

:: Ask user for confirmation
set /p confirm="Continue with download? (y/n): "
if /i not "%confirm%"=="y" (
    echo Installation cancelled.
    pause
    exit /b 0
)

echo.
echo Starting downloads...
echo.

:: Function to check if PowerShell is available
powershell -Command "Write-Host 'PowerShell available'" >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] PowerShell is not available
    echo Please install PowerShell or download tools manually
    pause
    exit /b 1
)

:: Download QEMU
echo [1/4] Downloading QEMU...
echo This may take a few minutes...
powershell -Command "& {try { Invoke-WebRequest -Uri 'https://qemu.weilnetz.de/w64/2024/qemu-w64-setup-2024-08-20.exe' -OutFile 'qemu-installer.exe' -UseBasicParsing; Write-Host 'QEMU downloaded successfully'} catch { Write-Host 'QEMU download failed. Please download manually from: https://qemu.weilnetz.de/w64/' }}"

if exist "qemu-installer.exe" (
    echo Installing QEMU...
    start /wait qemu-installer.exe /S /D=C:\qemu
    if exist "C:\qemu\qemu-system-i386.exe" (
        echo [SUCCESS] QEMU installed to C:\qemu\
    ) else (
        echo [WARNING] QEMU installation may have failed
    )
    del qemu-installer.exe
) else (
    echo [ERROR] QEMU download failed
)

echo.

:: Download NASM
echo [2/4] Downloading NASM...
powershell -Command "& {try { Invoke-WebRequest -Uri 'https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-win64.zip' -OutFile 'nasm.zip' -UseBasicParsing; Write-Host 'NASM downloaded successfully'} catch { Write-Host 'NASM download failed. Please download manually from: https://nasm.us/' }}"

if exist "nasm.zip" (
    echo Extracting NASM...
    powershell -Command "Expand-Archive -Path 'nasm.zip' -DestinationPath 'C:\nasm' -Force"
    if exist "C:\nasm\nasm.exe" (
        echo [SUCCESS] NASM installed to C:\nasm\
    ) else (
        echo [WARNING] NASM extraction may have failed
    )
    del nasm.zip
) else (
    echo [ERROR] NASM download failed
)

echo.

:: Download MinGW-w64
echo [3/4] Downloading MinGW-w64...
echo This is a large file (100MB+), please be patient...
powershell -Command "& {try { Invoke-WebRequest -Uri 'https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z' -OutFile 'mingw64.7z' -UseBasicParsing; Write-Host 'MinGW-w64 downloaded successfully'} catch { Write-Host 'MinGW-w64 download failed. Please download manually from: https://sourceforge.net/projects/mingw-w64/' }}"

if exist "mingw64.7z" (
    echo Extracting MinGW-w64...
    powershell -Command "Expand-Archive -Path 'mingw64.7z' -DestinationPath 'C:\mingw64' -Force"
    if exist "C:\mingw64\mingw64\bin\gcc.exe" (
        echo Moving MinGW-w64 to correct location...
        move "C:\mingw64\mingw64\*" "C:\mingw64\" >nul 2>&1
        rmdir "C:\mingw64\mingw64" >nul 2>&1
        if exist "C:\mingw64\bin\gcc.exe" (
            echo [SUCCESS] MinGW-w64 installed to C:\mingw64\
        ) else (
            echo [WARNING] MinGW-w64 installation may have failed
        )
    ) else (
        echo [WARNING] MinGW-w64 extraction may have failed
    )
    del mingw64.7z
) else (
    echo [ERROR] MinGW-w64 download failed
)

echo.

:: Download Git
echo [4/4] Downloading Git...
powershell -Command "& {try { Invoke-WebRequest -Uri 'https://github.com/git-for-windows/git/releases/download/v2.47.0.windows.2/Git-2.47.0.2-64-bit.exe' -OutFile 'git-installer.exe' -UseBasicParsing; Write-Host 'Git downloaded successfully'} catch { Write-Host 'Git download failed. Please download manually from: https://git-scm.com/download/win' }}"

if exist "git-installer.exe" (
    echo Installing Git...
    start /wait git-installer.exe /VERYSILENT /NORESTART /DIR="C:\Program Files\Git"
    if exist "C:\Program Files\Git\bin\git.exe" (
        echo [SUCCESS] Git installed
    ) else (
        echo [WARNING] Git installation may have failed
    )
    del git-installer.exe
) else (
    echo [ERROR] Git download failed
)

echo.
echo Download and installation process completed!
echo.

:: Update system PATH
echo Updating system PATH...
set NEW_PATH=C:\qemu;C:\nasm;C:\mingw64\bin;C:\Program Files\Git\bin

:: Add to system PATH (requires admin rights)
setx PATH "%PATH%;%NEW_PATH%" /M >nul 2>&1
if %errorlevel% equ 0 (
    echo [SUCCESS] System PATH updated
) else (
    echo [WARNING] Could not update system PATH automatically
    echo Please add these directories to your PATH manually:
    echo   C:\qemu
    echo   C:\nasm
    echo   C:\mingw64\bin
    echo   C:\Program Files\Git\bin
)

echo.
echo Creating test script...
echo @echo off > test_environment.bat
echo echo Testing Vest-OS Development Environment >> test_environment.bat
echo echo ======================================= >> test_environment.bat
echo echo. >> test_environment.bat
echo echo Testing QEMU... >> test_environment.bat
echo qemu-system-i386 --version ^>^nul 2^>^&1 >> test_environment.bat
echo if %%errorlevel%% equ 0 ^( >> test_environment.bat
echo     echo [SUCCESS] QEMU is available >> test_environment.bat
echo ^) else ^( >> test_environment.bat
echo     echo [ERROR] QEMU not found >> test_environment.bat
echo ^) >> test_environment.bat
echo echo. >> test_environment.bat
echo echo Testing NASM... >> test_environment.bat
echo nasm --version ^>^nul 2^>^&1 >> test_environment.bat
echo if %%errorlevel%% equ 0 ^( >> test_environment.bat
echo     echo [SUCCESS] NASM is available >> test_environment.bat
echo ^) else ^( >> test_environment.bat
echo     echo [ERROR] NASM not found >> test_environment.bat
echo ^) >> test_environment.bat
echo echo. >> test_environment.bat
echo echo Testing GCC... >> test_environment.bat
echo gcc --version ^>^nul 2^>^&1 >> test_environment.bat
echo if %%errorlevel%% equ 0 ^( >> test_environment.bat
echo     echo [SUCCESS] GCC is available >> test_environment.bat
echo ^) else ^( >> test_environment.bat
echo     echo [ERROR] GCC not found >> test_environment.bat
echo ^) >> test_environment.bat
echo echo. >> test_environment.bat
echo echo Testing Git... >> test_environment.bat
echo git --version ^>^nul 2^>^&1 >> test_environment.bat
echo if %%errorlevel%% equ 0 ^( >> test_environment.bat
echo     echo [SUCCESS] Git is available >> test_environment.bat
echo ^) else ^( >> test_environment.bat
echo     echo [ERROR] Git not found >> test_environment.bat
echo ^) >> test_environment.bat
echo echo. >> test_environment.bat
echo echo Test completed! >> test_environment.bat
echo pause >> test_environment.bat

move test_environment.bat "%INSTALL_DIR%\test_environment.bat" >nul 2>&1

echo.
echo Installation completed!
echo.
echo Summary:
echo - QEMU: C:\qemu\
echo - NASM: C:\nasm\
echo - MinGW-w64: C:\mingw64\
echo - Git: C:\Program Files\Git\
echo.
echo IMPORTANT: Please restart Command Prompt or PowerShell before using the tools.
echo.
echo To test your installation, run:
echo   %INSTALL_DIR%\test_environment.bat
echo.
echo To build Vest-OS:
echo   cd K:\vestos\Vest-OS
echo   nasm -f bin minimal_kernel.asm -o kernel.bin
echo   qemu-system-i386 -kernel kernel.bin
echo.
echo If any downloads failed, please download them manually:
echo - QEMU: https://qemu.weilnetz.de/w64/
echo - NASM: https://nasm.us/
echo - MinGW-w64: https://sourceforge.net/projects/mingw-w64/
echo - Git: https://git-scm.com/download/win
echo.
pause