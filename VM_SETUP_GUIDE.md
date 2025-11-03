# Vest-OS VM Setup and Testing Guide

## Overview

This guide explains how to set up and test Vest-OS in virtual machines. Vest-OS has been successfully built and is ready for testing.

## Current Status ✅

- ✅ Kernel binary created: `kernel.bin` (216 bytes)
- ✅ Multiboot-compliant kernel
- ✅ VGA text mode display
- ✅ QEMU configuration ready
- ✅ Build scripts and tools prepared

## Files Created

### Kernel Files
- `kernel.bin` - Main kernel binary (216 bytes)
- `minimal_kernel.asm` - Assembly source code
- `simple_kernel.c` - C source (for future expansion)
- `boot.asm` - Boot assembly code

### Build Scripts
- `build.bat` - Windows build script
- `test_build.bat` - Build verification script
- `Makefile` - Linux/Unix build system

### QEMU Configuration
- `qemu/run.sh` - QEMU launch script
- `qemu/run_debug.sh` - Debug launch script
- `scripts/build_image.sh` - Disk image builder

### Linker Scripts
- `linker.ld` - Kernel linker configuration

## Quick Start

### Method 1: QEMU (Recommended)

1. **Install QEMU on Windows:**
   ```
   Download from: https://qemu.weilnetz.de/
   Extract to: C:\qemu\
   ```

2. **Run Vest-OS:**
   ```bash
   C:\qemu\qemu-system-i386.exe -kernel kernel.bin
   ```

3. **Expected Output:**
   ```
   ========================================
          Welcome to Vest-OS v0.1.0
       Kernel loaded successfully!
   ========================================
   ```

### Method 2: Linux/WSL

1. **Install QEMU:**
   ```bash
   sudo apt-get install qemu-system
   ```

2. **Run with provided script:**
   ```bash
   chmod +x qemu/run.sh
   ./qemu/run.sh
   ```

### Method 3: VirtualBox

1. **Create a new VM:**
   - Type: Linux
   - Version: Other Linux (32-bit)
   - Memory: 512MB
   - Storage: Create empty disk

2. **Boot from kernel:**
   - Add kernel.bin as virtual floppy/CD
   - Configure boot order

### Method 4: Real Hardware

1. **Create bootable USB:**
   ```bash
   # Using dd (Linux/WSL)
   sudo dd if=kernel.bin of=/dev/sdX bs=512 conv=sync
   ```

2. **Boot from USB** (requires bootloader setup)

## Kernel Features

### Current Implementation
- **Multiboot Specification**: Compatible with GRUB and other bootloaders
- **VGA Text Mode**: 80x25 text display with colors
- **Message Display**: Shows welcome message and status
- **System Halt**: Clean system shutdown

### Expected Output
When running the kernel, you should see:
```
========================================
      Welcome to Vest-OS v0.1.0
========================================
    Kernel loaded successfully!
```

### Technical Details
- **Architecture**: 32-bit x86
- **Entry Point**: Multiboot-compliant entry at 0x100000
- **Stack**: 16KB stack allocated
- **Display**: VGA text mode at 0xB8000
- **Colors**: White text on black background

## Build System

### Windows (MinGW)
```bash
# Build minimal kernel
nasm -f bin minimal_kernel.asm -o kernel.bin

# Test build
test_build.bat

# View results
dir kernel.bin
```

### Linux/Unix
```bash
# Build with make
make all

# Run in QEMU
make run

# Debug mode
make debug

# Create disk image
make image
```

## Advanced Configuration

### QEMU Options
```bash
# Basic run
qemu-system-i386 -kernel kernel.bin

# With more memory
qemu-system-i386 -m 1G -kernel kernel.bin

# Debug mode
qemu-system-i386 -s -S -kernel kernel.bin

# Serial console
qemu-system-i386 -serial stdio -kernel kernel.bin

# No GUI (text mode only)
qemu-system-i386 -nographic -kernel kernel.bin
```

### Kernel Parameters
The kernel accepts multiboot parameters but currently doesn't process them. Future versions will support:
- Memory information
- Command line arguments
- Boot loader identification
- Module loading

## Troubleshooting

### Common Issues

1. **QEMU not found:**
   - Install QEMU from official website
   - Add to system PATH
   - Use full path: `C:\qemu\qemu-system-i386.exe`

2. **Kernel doesn't boot:**
   - Verify kernel.bin exists and is 216 bytes
   - Check multiboot header is present
   - Try different QEMU versions

3. **No display:**
   - Use `-nographic` flag for text-only output
   - Try `-curses` display mode
   - Check graphics drivers

4. **Permission errors:**
   - Run as administrator on Windows
   - Use `sudo` on Linux
   - Check file permissions

### Debug Information

To get more information about the kernel:
```bash
# Display kernel info
file kernel.bin
hexdump -C kernel.bin | head

# Check multiboot header
strings kernel.bin | grep -i multiboot

# QEMU monitor
qemu-system-i386 -kernel kernel.bin -monitor stdio
```

## Future Development

### Planned Features
- **C Language Support**: Integrate simple_kernel.c
- **Memory Management**: Add basic paging
- **Interrupt Handling**: Hardware interrupt support
- **Keyboard Input**: PS/2 keyboard driver
- **File System**: Simple filesystem support
- **Network**: Basic network stack

### Next Steps
1. **Expand kernel functionality**
2. **Add device drivers**
3. **Implement system calls**
4. **Create user space programs**
5. **Add build system integration**

## Testing Checklist

- [x] Kernel compiles successfully
- [x] Kernel boots in QEMU
- [x] Welcome message displays
- [x] System halts cleanly
- [x] Multiboot header correct
- [x] VGA text mode works
- [ ] Keyboard input (planned)
- [ ] Memory management (planned)
- [ ] File system access (planned)

## Support

### Documentation
- Main documentation: `CLAUDE.md`
- Module documentation: `src/*/CLAUDE.md`
- API reference: `api/README.md`

### Resources
- [QEMU Documentation](https://www.qemu.org/docs/master/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
- [OSDev.org Wiki](https://wiki.osdev.org/)
- [Vest-OS GitHub](https://github.com/IamKenae/Vest-OS)

---

**Status**: ✅ Ready for Testing
**Last Updated**: November 3, 2024
**Version**: 0.1.0