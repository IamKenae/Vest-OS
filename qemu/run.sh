#!/bin/bash
# Vest-OS QEMU Test Script
# Simple script to test Vest-OS in QEMU

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
KERNEL_IMG="kernel.bin"
MEMORY="512M"
SMP="2"
ARCH="i386"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--arch)
            ARCH="$2"
            shift 2
            ;;
        -m|--memory)
            MEMORY="$2"
            shift 2
            ;;
        -s|--smp)
            SMP="$2"
            shift 2
            ;;
        -d|--debug)
            DEBUG=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -a, --arch ARCH     Architecture (i386, x86_64) [default: i386]"
            echo "  -m, --memory SIZE   Memory size [default: 512M]"
            echo "  -s, --smp COUNT     CPU count [default: 2]"
            echo "  -d, --debug         Enable debug mode"
            echo "  -h, --help          Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}Vest-OS QEMU Test Script${NC}"
echo "========================"
echo "Architecture: $ARCH"
echo "Memory: $MEMORY"
echo "CPUs: $SMP"
echo ""

# Check if QEMU is available
QEMU_FOUND=false

# Try different QEMU commands
if command -v qemu-system-i386 &> /dev/null; then
    QEMU_CMD="qemu-system-i386"
    QEMU_FOUND=true
elif command -v qemu-system-x86_64 &> /dev/null; then
    QEMU_CMD="qemu-system-x86_64"
    QEMU_FOUND=true
elif [ -f "/usr/bin/qemu-system-i386" ]; then
    QEMU_CMD="/usr/bin/qemu-system-i386"
    QEMU_FOUND=true
elif [ -f "C:/Program Files/qemu/qemu-system-i386.exe" ]; then
    QEMU_CMD="C:/Program Files/qemu/qemu-system-i386.exe"
    QEMU_FOUND=true
elif [ -f "C:/qemu/qemu-system-i386.exe" ]; then
    QEMU_CMD="C:/qemu/qemu-system-i386.exe"
    QEMU_FOUND=true
fi

if [ "$QEMU_FOUND" = false ]; then
    echo -e "${RED}Error: QEMU is not installed or not found${NC}"
    echo ""
    echo "Please install QEMU:"
    echo "  Ubuntu/Debian: sudo apt-get install qemu-system"
    echo "  Fedora: sudo dnf install qemu-system"
    echo "  Arch: sudo pacman -S qemu"
    echo "  macOS: brew install qemu"
    echo "  Windows: Download from https://qemu.weilnetz.de/"
    echo ""
    echo "For Windows, download and extract to:"
    echo "  C:/qemu/ or C:/Program Files/qemu/"
    echo ""
    echo "Current kernel file: $KERNEL_IMG ($(stat -f%z "$KERNEL_IMG" 2>/dev/null || echo "unknown size") bytes)"
    echo ""
    echo "Alternative testing methods:"
    echo "1. Use VirtualBox with custom Linux VM"
    echo "2. Use Windows Subsystem for Linux (WSL)"
    echo "3. Test on real hardware with bootable USB"
    echo "4. Use online emulator (if available)"
    exit 1
fi

# QEMU command already determined above

# Check if kernel image exists
if [ ! -f "$KERNEL_IMG" ]; then
    echo -e "${YELLOW}Warning: Kernel image $KERNEL_IMG not found${NC}"
    echo "Attempting to build kernel..."

    if [ -f "Makefile" ]; then
        echo "Running make..."
        if command -v make &> /dev/null; then
            make all || {
                echo -e "${RED}Error: Failed to build kernel${NC}"
                exit 1
            }
        else
            echo -e "${RED}Error: 'make' command not found${NC}"
            echo "Please install build tools:"
            echo "  Ubuntu/Debian: sudo apt-get install build-essential"
            echo "  Fedora: sudo dnf groupinstall \"Development Tools\""
            echo "  macOS: xcode-select --install"
            exit 1
        fi
    else
        echo -e "${RED}Error: No Makefile found${NC}"
        exit 1
    fi
fi

# Check again for kernel image
if [ ! -f "$KERNEL_IMG" ]; then
    echo -e "${RED}Error: Kernel image $KERNEL_IMG not found after build${NC}"
    exit 1
fi

echo -e "${GREEN}Found kernel image: $KERNEL_IMG${NC}"

# Build QEMU command
QEMU_ARGS=(
    "-machine" "pc,accel=kvm:tcg"
    "-cpu" "qemu32"
    "-m" "$MEMORY"
    "-smp" "$SMP"
    "-kernel" "$KERNEL_IMG"
    "-serial" "stdio"
    "-vga" "std"
    "-no-reboot"
    "-display" "gtk"
)

if [ "$DEBUG" = true ]; then
    QEMU_ARGS+=("-s" "-S")
    echo -e "${YELLOW}Debug mode enabled - GDB will listen on port 1234${NC}"
fi

echo ""
echo -e "${BLUE}Starting QEMU with command:${NC}"
echo "$QEMU_CMD ${QEMU_ARGS[*]}"
echo ""
echo -e "${YELLOW}Press Ctrl+A, X to exit QEMU${NC}"
echo ""

# Launch QEMU
"$QEMU_CMD" "${QEMU_ARGS[@]}"