#!/bin/bash
# Vest-OS Disk Image Builder
# Creates bootable disk images for Vest-OS

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
KERNEL_IMG="kernel.bin"
IMG_SIZE="100M"
IMG_NAME="vestos.img"
ISO_NAME="vestos.iso"

echo -e "${BLUE}Vest-OS Disk Image Builder${NC}"
echo "==========================="

# Check dependencies
check_deps() {
    local deps=("dd" "mkfs.fat" "mtools" "xorriso")
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            echo -e "${RED}Error: $dep not found${NC}"
            echo "Please install required tools:"
            echo "  Ubuntu/Debian: sudo apt-get install dosfstools mtools xorriso"
            echo "  Fedora: sudo dnf install dosfstools mtools xorriso"
            echo "  Arch: sudo pacman -S dosfstools mtools libisoburn"
            exit 1
        fi
    done
    echo -e "${GREEN}All dependencies found${NC}"
}

# Build kernel if needed
build_kernel() {
    if [ ! -f "$KERNEL_IMG" ]; then
        echo -e "${YELLOW}Kernel image not found, building...${NC}"
        if [ -f "Makefile" ] && command -v make &> /dev/null; then
            make all
        else
            echo -e "${RED}Error: Cannot build kernel - make or Makefile not found${NC}"
            exit 1
        fi
    fi

    if [ -f "$KERNEL_IMG" ]; then
        echo -e "${GREEN}Kernel image ready: $KERNEL_IMG${NC}"
    else
        echo -e "${RED}Error: Failed to build kernel${NC}"
        exit 1
    fi
}

# Create disk image
create_disk_image() {
    echo -e "${BLUE}Creating disk image...${NC}"

    # Create blank image
    dd if=/dev/zero of="$IMG_NAME" bs="$IMG_SIZE" count=1 status=progress

    # Create FAT32 filesystem
    mkfs.fat -F 32 -n "VESTOS" "$IMG_NAME"

    echo -e "${GREEN}Disk image created: $IMG_NAME${NC}"
}

# Copy kernel to image
copy_kernel() {
    echo -e "${BLUE}Copying kernel to image...${NC}"

    # Create mount point
    mkdir -p mnt

    # Mount image
    sudo mount -o loop "$IMG_NAME" mnt

    # Copy kernel
    sudo cp "$KERNEL_IMG" mnt/kernel.bin

    # Create basic boot info
    echo "Vest-OS v0.1.0" | sudo tee mnt/version.txt
    echo "Build: $(date)" | sudo tee -a mnt/version.txt

    # Unmount
    sudo umount mnt
    rmdir mnt

    echo -e "${GREEN}Kernel copied to disk image${NC}"
}

# Create ISO image
create_iso() {
    echo -e "${BLUE}Creating ISO image...${NC}"

    # Create temporary directory
    mkdir -p iso

    # Copy kernel
    cp "$KERNEL_IMG" iso/kernel.bin

    # Create boot catalog
    mkdir -p iso/boot/grub
    cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=10
set default=0

menuentry "Vest-OS" {
    multiboot /kernel.bin
    boot
}

menuentry "Vest-OS (Debug)" {
    multiboot /kernel.bin
    boot
}

menuentry "Reboot" {
    reboot
}

menuentry "Shutdown" {
    halt
}
EOF

    # Create ISO with xorriso
    xorriso -as mkisofs \
        -R -J \
        -b boot/grub/i386-pc/eltorito.img \
        -no-emul-boot \
        -boot-load-size 4 \
        -boot-info-table \
        -input-charset utf-8 \
        -output-charset utf-8 \
        -o "$ISO_NAME" \
        iso

    # Cleanup
    rm -rf iso

    echo -e "${GREEN}ISO image created: $ISO_NAME${NC}"
}

# Test image in QEMU
test_in_qemu() {
    echo -e "${BLUE}Testing in QEMU...${NC}"

    if [ -f "qemu/run.sh" ]; then
        # Use existing QEMU script
        qemu/run.sh
    else
        echo -e "${YELLOW}QEMU script not found, using direct qemu command${NC}"
        if command -v qemu-system-i386 &> /dev/null; then
            qemu-system-i386 -m 512M -kernel "$KERNEL_IMG" -serial stdio
        else
            echo -e "${RED}QEMU not found${NC}"
        fi
    fi
}

# Show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -d, --disk-only     Create only disk image"
    echo "  -i, --iso-only      Create only ISO image"
    echo "  -t, --test          Test after creation"
    echo "  -c, --check-deps    Only check dependencies"
    echo "  -h, --help          Show this help"
    echo ""
    echo "Examples:"
    echo "  $0                 Create both disk and ISO images"
    echo "  $0 -d              Create only disk image"
    echo "  $0 -i              Create only ISO image"
    echo "  $0 -t              Create images and test in QEMU"
}

# Main execution
main() {
    local DISK_ONLY=false
    local ISO_ONLY=false
    local TEST_AFTER=false
    local CHECK_DEPS_ONLY=false

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--disk-only)
                DISK_ONLY=true
                shift
                ;;
            -i|--iso-only)
                ISO_ONLY=true
                shift
                ;;
            -t|--test)
                TEST_AFTER=true
                shift
                ;;
            -c|--check-deps)
                CHECK_DEPS_ONLY=true
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done

    # Check dependencies only
    if [ "$CHECK_DEPS_ONLY" = true ]; then
        check_deps
        exit 0
    fi

    # Check dependencies
    check_deps

    # Build kernel
    build_kernel

    # Create images
    if [ "$ISO_ONLY" = false ]; then
        create_disk_image
        copy_kernel
    fi

    if [ "$DISK_ONLY" = false ]; then
        create_iso
    fi

    echo ""
    echo -e "${GREEN}Build completed successfully!${NC}"
    echo "Files created:"
    if [ "$ISO_ONLY" = false ]; then
        echo "  - $IMG_NAME (Disk image)"
    fi
    if [ "$DISK_ONLY" = false ]; then
        echo "  - $ISO_NAME (ISO image)"
    fi

    # Test in QEMU
    if [ "$TEST_AFTER" = true ]; then
        echo ""
        test_in_qemu
    fi
}

# Run main function
main "$@"