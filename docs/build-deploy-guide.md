# Vest-OS Build and Deployment Guide

## Table of Contents
- [Environment Preparation](#environment-preparation)
- [Source Code Acquisition](#source-code-acquisition)
- [Build System](#build-system)
- [Build Process](#build-process)
- [Configuration Options](#configuration-options)
- [Packaging](#packaging)
- [Deployment Methods](#deployment-methods)
- [Continuous Integration](#continuous-integration)
- [Troubleshooting](#troubleshooting)

## Environment Preparation

### System Requirements

#### Build Environment
- **Operating System**: Linux (Ubuntu 20.04+, CentOS 8+, Fedora 33+)
- **CPU**: x86_64 architecture, 4 cores or more
- **Memory**: 8GB or more
- **Storage**: 100GB available space
- **Network**: Stable internet connection

#### Required Software Packages

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    bc \
    bison \
    flex \
    libelf-dev \
    libssl-dev \
    pkg-config \
    git \
    python3 \
    python3-pip \
    qemu-kvm \
    genisoimage \
    grub-pc-bin \
    grub-common \
    mtools \
    xorriso
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    bc \
    bison \
    flex \
    elfutils-libelf-devel \
    openssl-devel \
    pkgconfig \
    git \
    python3 \
    python3-pip \
    qemu-kvm \
    genisoimage \
    grub2-tools \
    mtools \
    xorriso
```

### Cross-Compilation Environment (Optional)

#### ARM64 Cross-Compilation
```bash
sudo apt install -y gcc-aarch64-linux-gnu \
                       binutils-aarch64-linux-gnu \
                       libc6-dev-arm64-cross
```

#### RISC-V Cross-Compilation
```bash
sudo apt install -y gcc-riscv64-linux-gnu \
                       binutils-riscv64-linux-gnu \
                       libc6-dev-riscv64-cross
```

## Source Code Acquisition

### Clone from Git Repository
```bash
# Clone main repository
git clone https://github.com/vest-os/vest-os.git
cd vest-os

# Clone submodules
git submodule update --init --recursive
```

### Checkout Specific Version
```bash
# View all tags
git tag -l

# Checkout stable version
git checkout v1.0.0

# Checkout development branch
git checkout develop
```

### Verify Source Code
```bash
# Verify GPG signature (if available)
git verify-tag v1.0.0

# Calculate SHA256 hash
sha256sum vest-os.tar.gz
```

## Build System

Vest-OS uses a GNU Make-based build system that supports parallel builds and incremental compilation.

### Build System Structure
```
vest-os/
├── Makefile              # Main Makefile
├── config/               # Configuration files
│   ├── defconfig         # Default configuration
│   └── custom.config     # Custom configuration
├── scripts/              # Build scripts
│   ├── build.sh         # Build script
│   └── package.sh       # Packaging script
├── tools/                # Build tools
├── kernel/               # Kernel source code
├── userspace/            # User space programs
└── docs/                 # Documentation
```

### Makefile Targets

```bash
# Show all targets
make help

# Clean build directory
make clean

# Complete clean (including downloaded files)
make distclean

# Configuration
make config               # Interactive configuration
make menuconfig           # Menu configuration
make oldconfig            # Use old configuration
make defconfig            # Default configuration

# Build
make all                  # Build all components
make kernel               # Build kernel only
make userspace            # Build user space only
make tools                # Build tools only

# Test
make test                 # Run tests
make test-kernel          # Test kernel
make test-userspace       # Test user space

# Package
make iso                  # Create ISO image
make img                  # Create disk image
make tar                  # Create tar package

# Install
make install              # Install to system
make install-headers      # Install header files
```

## Build Process

### 1. Configuration

#### Interactive Configuration
```bash
make menuconfig
```

Configuration menu includes:
- **Target Architecture**: x86_64, ARM64, RISC-V
- **CPU Features**: SMP, 64-bit, virtualization support
- **Device Drivers**: Network, storage, input devices
- **File Systems**: EXT4, BTRFS, NFS
- **Network Protocols**: TCP/IP, IPv6, wireless
- **Security Features**: SELinux, encryption support
- **Debug Options**: Kernel debugging, symbol tables

#### Use Predefined Configurations
```bash
# Use server configuration
make vest-os_server_defconfig

# Use desktop configuration
make vest-os_desktop_defconfig

# Use embedded configuration
make vest-os_embedded_defconfig
```

#### Custom Configuration File
```bash
# Create configuration file
cp config/defconfig .config

# Edit configuration
vim .config

# Verify configuration
make olddefconfig
```

### 2. Build

#### Complete Build
```bash
# Use all CPU cores
make -j$(nproc)

# Specify parallelism
make -j8

# Build with verbose information
make V=1

# Silent build
make -s
```

#### Step-by-Step Build
```bash
# 1. Build toolchain
make toolchain

# 2. Build kernel
make kernel

# 3. Build initramfs
make initramfs

# 4. Build user space
make userspace

# 5. Create system image
make image
```

### 3. Configuration Examples

#### Minimal System Configuration
```config
CONFIG_ARCH_X86_64=y
CONFIG_SMP=y
CONFIG_EARLY_PRINTK=y
CONFIG_BLK_DEV_INITRD=y
CONFIG_VFS=y
CONFIG_EXT4_FS=y
CONFIG_SERIAL_CONSOLE=y
CONFIG_TCP=y
CONFIG_E1000=y
```

#### Server Configuration
```config
CONFIG_ARCH_X86_64=y
CONFIG_SMP=y
CONFIG_NR_CPUS=64
CONFIG_X86_HZ=y
CONFIG_HZ_1000=y
CONFIG_TCP=y
CONFIG_TCP_ADVANCED_WINDOW_SCALE=y
CONFIG_IPV6=y
CONFIG_NETFILTER=y
CONFIG_EXT4_FS=y
CONFIG_XFS_FS=y
CONFIG_BTRFS_FS=y
CONFIG_RAID=y
CONFIG_LVM2=y
CONFIG_DM_MIRROR=y
CONFIG_DM_ZERO=y
```

#### Desktop Configuration
```config
CONFIG_ARCH_X86_64=y
CONFIG_SMP=y
CONFIG_PREEMPT=y
CONFIG_HZ_250=y
CONFIG_DESKTOP=y
CONFIG_FRAMEBUFFER=y
CONFIG_DRM=y
CONFIG_DRM_I915=y
CONFIG_INPUT=y
CONFIG_INPUT_MOUSE=y
CONFIG_INPUT_KEYBOARD=y
CONFIG_USB=y
CONFIG_USB_EHCI_HCD=y
CONFIG_SND=y
CONFIG_SND_HDA=y
CONFIG_ACPI=y
CONFIG_ACPI_BATTERY=y
CONFIG_ACPI_AC=y
```

## Configuration Options Details

### Kernel Configuration

#### Basic Configuration
```config
# Version information
CONFIG_VERSION="1.0.0"
CONFIG_LOCALVERSION="-custom"

# Compiler options
CONFIG_CC_VERSION_TEXT="gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE=y
CONFIG_CC_OPTIMIZE_FOR_SIZE=n

# Linker options
CONFIG_LD_VERSION="2.34"
CONFIG_LD_SCRIPT_KERNEL=y
CONFIG_LD_SCRIPT_BUILTIN=y

# Debug information
CONFIG_DEBUG_KERNEL=y
CONFIG_DEBUG_INFO=y
CONFIG_DEBUG_INFO_REDUCED=n
CONFIG_STRIP_ASM_SYMS=y
CONFIG_READABLE_ASM=y
```

#### Memory Management
```config
# Memory model
CONFIG_FLATMEM=y
CONFIG_SPARSEMEM=n
CONFIG_MEMORY_HOTPLUG=n

# SLAB allocator
CONFIG_SLUB=y
CONFIG_SLUB_DEBUG=y
CONFIG_SLUB_CPU_PARTIAL=y

# Virtual memory
CONFIG_MMU=y
CONFIG_PAGE_OFFSET=0xffff880000000000
CONFIG_PHYSICAL_START=0x1000000
CONFIG_PHYSICAL_ALIGN=0x200000
```

#### Process Scheduling
```config
# Scheduler
CONFIG_SCHED_AUTOGROUP=y
CONFIG_SCHED_DEBUG=y
CONFIG_SCHEDSTATS=y

# Preemption mode
CONFIG_PREEMPT_NONE=y
CONFIG_PREEMPT_VOLUNTARY=n
CONFIG_PREEMPT=y
CONFIG_PREEMPT_RT=n
```

### User Space Configuration

#### Initialization System
```config
# Init system
CONFIG_INIT_SYSTEM=y
CONFIG_INIT_SYSTEMD=y

# Shell
CONFIG_BASH=y
CONFIG_ZSH=y

# Core utilities
CONFIG_COREUTILS=y
CONFIG_UTIL_LINUX=y
CONFIG_PROCPS=y
CONFIG_E2FSPROGS=y
```

#### Network Tools
```config
# Basic network tools
CONFIG_IPROUTE2=y
CONFIG_IPTABLES=y
CONFIG_NFTABLES=y
CONFIG_WPA_SUPPLICANT=y

# Network services
CONFIG_SSHD=y
CONFIG_NTPD=y
CONFIG_CRON=y
```

## Packaging

### Create ISO Image
```bash
# Create bootable ISO
make iso

# ISO with UEFI support
make iso UEFI=y

# Create hybrid ISO (BIOS+UEFI)
make iso HYBRID=y
```

### Create Disk Image
```bash
# Create virtual disk image
make img

# Specify image size (GB)
make img SIZE=8

# Create cloud image (qcow2 format)
make img FORMAT=qcow2

# Create raw image
make img FORMAT=raw
```

### Create tar Package
```bash
# Create source tar package
make tar

# Create binary tar package
make tar-binary

# Create SDK tar package
make tar-sdk
```

## Deployment Methods

### 1. Bare Metal Deployment

#### Create Bootable USB
```bash
# Write ISO to USB
sudo dd if=vest-os-1.0.0.iso of=/dev/sdX bs=4M status=progress
sync

# Or use ddrescue (safer)
sudo ddrescue vest-os-1.0.0.iso /dev/sdX
```

#### Network Boot (PXE)
```bash
# Setup TFTP server
mkdir -p /tftpboot
cp vest-os-1.0.0/vmlinuz /tftpboot/
cp vest-os-1.0.0/initramfs /tftpboot/

# DHCP configuration (/etc/dhcp/dhcpd.conf)
subnet 192.168.1.0 netmask 255.255.255.0 {
    range 192.168.1.100 192.168.1.200;
    next-server 192.168.1.1;
    filename "pxelinux.0";
}

# TFTP configuration (/etc/xinetd.d/tftp)
service tftp {
    protocol        = udp
    port            = 69
    socket_type     = dgram
    wait            = yes
    user            = root
    server          = /usr/sbin/in.tftpd
    server_args     = -s /tftpboot
    disable         = no
}
```

### 2. Virtualization Deployment

#### KVM/QEMU
```bash
# Create virtual disk
qemu-img create -f qcow2 vest-os.qcow2 20G

# Start virtual machine
qemu-system-x86_64 \
    -m 4096 \
    -cpu host \
    -enable-kvm \
    -hda vest-os.qcow2 \
    -cdrom vest-os-1.0.0.iso \
    -boot d

# Or use virt-install
virt-install \
    --name vest-os \
    --memory 4096 \
    --vcpus 4 \
    --disk vest-os.qcow2,size=20 \
    --cdrom vest-os-1.0.0.iso \
    --os-type linux \
    --os-variant generic \
    --graphics vnc
```

#### VirtualBox
```bash
# Create virtual machine
VBoxManage createvm --name "Vest-OS" --register
VBoxManage modifyvm "Vest-OS" --memory 4096 --cpus 4
VBoxManage storagectl "Vest-OS" --name "SATA" --add sata
VBoxManage createhd --filename "vest-os.vdi" --size 20000
VBoxManage storageattach "Vest-OS" --storagectl "SATA" --port 0 --device 0 --type hdd --medium "vest-os.vdi"
VBoxManage storageattach "Vest-OS" --storagectl "SATA" --port 1 --device 0 --type dvddrive --medium "vest-os-1.0.0.iso"
```

#### VMware
```bash
# Create OVF template
ovftool \
    --compress=9 \
    vest-os.vmx \
    vest-os.ovf

# Deploy to vSphere
govc import.ovf \
    -ds datastore1 \
    -pool Resources \
    -host esxi-host \
    vest-os.ovf
```

### 3. Cloud Platform Deployment

#### OpenStack
```bash
# Upload image
openstack image create \
    --disk-format qcow2 \
    --container-format bare \
    --file vest-os-1.0.0.qcow2 \
    vest-os-1.0.0

# Create instance
openstack server create \
    --image vest-os-1.0.0 \
    --flavor m1.medium \
    --key-name mykey \
    vest-os-vm
```

#### AWS EC2
```bash
# Install AWS CLI
pip install awscli

# Upload image to S3
aws s3 cp vest-os-1.0.0.raw s3://my-bucket/

# Import snapshot
aws ec2 import-snapshot \
    --description "Vest-OS Snapshot" \
    --disk-container "Description=Vest-OS,Format=raw,UserBucket={S3Bucket=my-bucket,S3Key=vest-os-1.0.0.raw}"

# Create AMI
aws ec2 register-image \
    --name "Vest-OS 1.0.0" \
    --architecture x86_64 \
    --root-device-name /dev/sda1 \
    --virtualization-type hvm \
    --block-device-mappings "DeviceName=/dev/sda1,Ebs={SnapshotId=snap-xxxxxxxx}"
```

#### Azure
```bash
# Install Azure CLI
curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

# Create storage account
az storage account create \
    --name mystorageaccount \
    --resource-group myResourceGroup \
    --location westus

# Upload VHD
az storage blob upload \
    --account-name mystorageaccount \
    --container-name images \
    --file vest-os-1.0.0.vhd \
    --name vest-os-1.0.0.vhd

# Create image
az image create \
    --resource-group myResourceGroup \
    --name vest-os-image \
    --source https://mystorageaccount.blob.core.windows.net/images/vest-os-1.0.0.vhd
```

### 4. Containerized Deployment

#### Docker
```dockerfile
# Dockerfile
FROM scratch
COPY vest-os-rootfs.tar.gz /
CMD ["/sbin/init"]
```

```bash
# Build image
docker build -t vest-os:1.0.0 .

# Run container
docker run -it --privileged vest-os:1.0.0
```

#### Kubernetes
```yaml
# vest-os-pod.yaml
apiVersion: v1
kind: Pod
metadata:
  name: vest-os-pod
spec:
  containers:
  - name: vest-os
    image: vest-os:1.0.0
    command: ["/sbin/init"]
    securityContext:
      privileged: true
    volumeMounts:
    - name: sysfs
      mountPath: /sys
    - name: proc
      mountPath: /proc
  volumes:
  - name: sysfs
    hostPath:
      path: /sys
  - name: proc
    hostPath:
      path: /proc
```

## Continuous Integration

### GitHub Actions Configuration
```yaml
# .github/workflows/build.yml
name: Build Vest-OS

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        arch: [x86_64, arm64, riscv64]

    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive

    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential bc bison flex libelf-dev
        if [ "${{ matrix.arch }}" != "x86_64" ]; then
          sudo apt install -y gcc-${{ matrix.arch }}-linux-gnu
        fi

    - name: Configure
      run: |
        make vest-os_${{ matrix.arch }}_defconfig

    - name: Build
      run: |
        make -j$(nproc) ARCH=${{ matrix.arch }}

    - name: Test
      run: |
        make test ARCH=${{ matrix.arch }}

    - name: Package
      run: |
        make iso ARCH=${{ matrix.arch }}

    - name: Upload artifacts
      uses: actions/upload-artifact@v2
      with:
        name: vest-os-${{ matrix.arch }}.iso
        path: vest-os-*.iso
```

### Jenkins Pipeline
```groovy
// Jenkinsfile
pipeline {
    agent any

    environment {
        CC = 'gcc'
        ARCH = 'x86_64'
    }

    stages {
        stage('Checkout') {
            steps {
                git url: 'https://github.com/vest-os/vest-os.git',
                    submodules: true
            }
        }

        stage('Configure') {
            steps {
                sh 'make vest-os_defconfig'
            }
        }

        stage('Build') {
            steps {
                sh 'make -j$(nproc)'
            }
        }

        stage('Test') {
            steps {
                sh 'make test'
            }
        }

        stage('Package') {
            steps {
                sh 'make iso'
                archiveArtifacts artifacts: '*.iso', fingerprint: true
            }
        }

        stage('Deploy') {
            when {
                branch 'main'
            }
            steps {
                sh 'aws s3 cp *.iso s3://vest-os-releases/'
            }
        }
    }

    post {
        always {
            cleanWs()
        }
        success {
            emailext subject: "Build Successful: ${env.JOB_NAME}",
                body: "Build succeeded in ${env.JOB_URL}"
        }
        failure {
            emailext subject: "Build Failed: ${env.JOB_NAME}",
                body: "Build failed in ${env.JOB_URL}"
        }
    }
}
```

## Troubleshooting

### Common Build Errors

#### 1. Compilation Error
```
error: 'CONFIG_X86_64' undeclared
```
**Solution:**
```bash
make clean
make defconfig
make menuconfig  # Ensure correct architecture
make -j$(nproc)
```

#### 2. Linking Error
```
undefined reference to `symbol_name'
```
**Solution:**
```bash
# Check configuration
grep -R "symbol_name" .config

# Enable related configuration
make menuconfig
# Enable module containing the symbol
```

#### 3. Insufficient Memory
```
g++: internal compiler error: Killed (program cc1plus)
```
**Solution:**
```bash
# Reduce parallel tasks
make -j2

# Or increase swap space
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

#### 4. Missing Dependencies
```
fatal error: openssl/ssl.h: No such file or directory
```
**Solution:**
```bash
# Install development packages
sudo apt install libssl-dev

# Or specify library path
make CFLAGS=-I/usr/include/openssl LDFLAGS=-L/usr/lib/ssl
```

### Performance Optimization

#### Accelerate Build
```bash
# Use ccache
export CC="ccache gcc"
export CXX="ccache g++"

# Use tmpfs
sudo mount -t tmpfs -o size=4G tmpfs /tmp

# Use distcc (distributed compilation)
export CC="distcc gcc"
make -j$(distcc -j)
```

#### Incremental Build
```bash
# Enable ccache
export CCACHE_DIR=~/.ccache
export CCACHE_MAXSIZE=10G
export CCACHE_COMPRESS=1

# Clean ccache
ccache -C
```

### Debugging Tips

#### View Build Logs
```bash
# Verbose logging
make V=1 > build.log 2>&1

# Show only errors and warnings
make 2>&1 | grep -E "(error|warning)"
```

#### Analyze Build Time
```bash
# Use make's time analysis
make --debug=basic

# Or use buildtime script
./scripts/buildtime.sh
```

#### Track Dependencies
```bash
# View dependency relationships
make -p | grep -E "^[^:#=-].*:" | head

# View dependencies for specific target
make -qp | grep -A5 "target_name:"
```

---

*Document Version: 1.0*
*Last updated: January 2024*