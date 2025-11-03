# MinGW-w64 Setup Guide for Vest-OS

## Current Status

You have successfully downloaded:
- ✅ QEMU (installed)
- ✅ NASM (installed)
- ✅ Git (installed)
- ⏳ MinGW-w64 (downloaded, needs extraction and setup)

## MinGW-w64 Setup Instructions

### Method 1: Automated Setup (Recommended)

1. **Run the completion script:**
   ```cmd
   complete_setup.bat
   ```

2. **Follow the prompts** - the script will:
   - Locate your downloaded MinGW-w64 file
   - Extract it to the correct location
   - Update your system PATH
   - Test the installation

### Method 2: Manual Setup

#### Step 1: Extract MinGW-w64

1. **Locate the file:** `x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z`
   - Usually in: `%USERPROFILE%\Downloads\`
   - Or: `C:\Users\%USERNAME%\Downloads\`

2. **Extract the file:**
   - Right-click on the `.7z` file
   - Select "Extract All..." or "Extract Here"
   - **Important:** Extract to `C:\mingw64\`
   - After extraction, you should have: `C:\mingw64\bin\gcc.exe`

#### Step 2: Update System PATH

1. **Open System Properties:**
   - Press `Win + R`
   - Type `sysdm.cpl`
   - Press Enter

2. **Environment Variables:**
   - Click "Advanced" tab
   - Click "Environment Variables..."
   - Find "Path" under "System variables"
   - Click "Edit..."

3. **Add these paths:**
   ```
   C:\qemu
   C:\nasm
   C:\mingw64\bin
   C:\Program Files\Git\bin
   ```

4. **Restart Command Prompt** to apply changes

#### Step 3: Verify Installation

Open a **new** Command Prompt and test:
```cmd
gcc --version
nasm --version
qemu-system-i386 --version
git --version
```

### Method 3: Using 7-Zip (if extraction fails)

1. **Download 7-Zip:** https://www.7-zip.org/

2. **Install 7-Zip**

3. **Extract MinGW-w64:**
   - Right-click on the `.7z` file
   - Select "7-Zip" → "Extract Here"
   - Move the extracted folder to `C:\mingw64\`

## Expected Directory Structure

After successful setup, you should have:
```
C:\qemu\
├── qemu-system-i386.exe
├── qemu-system-x86_64.exe
└── [other QEMU files]

C:\nasm\
├── nasm.exe
├── ndisasm.exe
└── [other NASM files]

C:\mingw64\
├── bin\
│   ├── gcc.exe
│   ├── g++.exe
│   ├── make.exe
│   └── [other compiler tools]
├── include\
├── lib\
└── [other MinGW files]

C:\Program Files\Git\
├── bin\
│   ├── git.exe
│   └── [other Git tools]
└── [other Git files]
```

## Common Issues and Solutions

### Issue 1: "gcc is not recognized"
**Solution:**
- Verify `C:\mingw64\bin\gcc.exe` exists
- Add `C:\mingw64\bin` to system PATH
- Restart Command Prompt

### Issue 2: "Access denied" during extraction
**Solution:**
- Run Command Prompt as Administrator
- Extract to a different directory first
- Check if the file is corrupted

### Issue 3: PATH not updating
**Solution:**
- Use System Properties (Win + R → sysdm.cpl)
- Restart your computer
- Try adding to User PATH instead of System PATH

### Issue 4: 7-Zip extraction fails
**Solution:**
- Download 7-Zip from official site
- Try a different extraction tool
- Verify the downloaded file isn't corrupted

## Testing Your Setup

After setup, run the test script:
```cmd
quick_test.bat
```

This will:
1. Test all tools are working
2. Build the Vest-OS kernel
3. Test it in QEMU

## Expected Output

When all tools are working, you should see:
```
[✓] QEMU is working
qemu-system-i386 8.2.0 (or newer)

[✓] NASM is working
NASM version 2.16.01 (or newer)

[✓] GCC is working
gcc.exe (x86_64-posix-seh-rev1, Built by MinGW-W64 project) 13.2.0

[✓] Git is working
git version 2.47.0.windows.2 (or newer)
```

## Building Vest-OS

Once everything is set up:

```cmd
# Navigate to Vest-OS directory
cd K:\vestos\Vest-OS

# Build the kernel
nasm -f bin minimal_kernel.asm -o kernel.bin

# Run in QEMU
qemu-system-i386 -kernel kernel.bin
```

## Alternative: MSYS2 (if MinGW-w64 causes issues)

If MinGW-w64 continues to cause problems:

1. **Download MSYS2:** https://www.msys2.org/

2. **Install and run MSYS2**

3. **Install tools in MSYS2:**
   ```bash
   pacman -Syu
   pacman -Su
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-nasm
   pacman -S mingw-w64-x86_64-make
   ```

4. **Use MSYS2 MinGW-w64:**
   - Use the "MinGW-w64 Win64 Shell" shortcut
   - It will have the correct PATH automatically

## Need Help?

If you encounter issues:

1. **Run the diagnostic script:** `complete_setup.bat`
2. **Check the troubleshooting section above**
3. **Verify file paths and permissions**
4. **Try the MSYS2 alternative**

---

**Last Updated:** November 3, 2024
**Purpose:** Complete MinGW-w64 setup for Vest-OS development