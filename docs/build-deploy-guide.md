# Vest-OS 构建和部署指南

## 目录
- [环境准备](#环境准备)
- [源码获取](#源码获取)
- [构建系统](#构建系统)
- [构建流程](#构建流程)
- [配置选项](#配置选项)
- [打包](#打包)
- [部署方法](#部署方法)
- [持续集成](#持续集成)
- [故障排除](#故障排除)

## 环境准备

### 系统要求

#### 构建环境
- **操作系统**: Linux (Ubuntu 20.04+, CentOS 8+, Fedora 33+)
- **CPU**: x86_64 架构，4核心以上
- **内存**: 8GB 以上
- **存储**: 100GB 可用空间
- **网络**: 稳定的互联网连接

#### 必需软件包

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

### 交叉编译环境（可选）

#### ARM64交叉编译
```bash
sudo apt install -y gcc-aarch64-linux-gnu \
                       binutils-aarch64-linux-gnu \
                       libc6-dev-arm64-cross
```

#### RISC-V交叉编译
```bash
sudo apt install -y gcc-riscv64-linux-gnu \
                       binutils-riscv64-linux-gnu \
                       libc6-dev-riscv64-cross
```

## 源码获取

### 从Git仓库克隆
```bash
# 克隆主仓库
git clone https://github.com/vest-os/vest-os.git
cd vest-os

# 克隆子模块
git submodule update --init --recursive
```

### 检出特定版本
```bash
# 查看所有标签
git tag -l

# 检出稳定版本
git checkout v1.0.0

# 检出开发分支
git checkout develop
```

### 验证源码
```bash
# 验证GPG签名（如果有）
git verify-tag v1.0.0

# 计算SHA256哈希
sha256sum vest-os.tar.gz
```

## 构建系统

Vest-OS使用基于GNU Make的构建系统，支持并行构建和增量编译。

### 构建系统结构
```
vest-os/
├── Makefile              # 主Makefile
├── config/               # 配置文件
│   ├── defconfig         # 默认配置
│   └── custom.config     # 自定义配置
├── scripts/              # 构建脚本
│   ├── build.sh         # 构建脚本
│   └── package.sh       # 打包脚本
├── tools/                # 构建工具
├── kernel/               # 内核源码
├── userspace/            # 用户空间程序
└── docs/                 # 文档
```

### Makefile目标

```bash
# 显示所有目标
make help

# 清理构建目录
make clean

# 完全清理（包括下载的文件）
make distclean

# 配置
make config               # 交互式配置
make menuconfig           # 菜单配置
make oldconfig            # 使用旧配置
make defconfig            # 默认配置

# 构建
make all                  # 构建所有组件
make kernel               # 只构建内核
make userspace            # 只构建用户空间
make tools                # 只构建工具

# 测试
make test                 # 运行测试
make test-kernel          # 测试内核
make test-userspace       # 测试用户空间

# 打包
make iso                  # 创建ISO镜像
make img                  # 创建磁盘镜像
make tar                  # 创建tar包

# 安装
make install              # 安装到系统
make install-headers      # 安装头文件
```

## 构建流程

### 1. 配置

#### 交互式配置
```bash
make menuconfig
```

配置菜单包括：
- **目标架构**: x86_64, ARM64, RISC-V
- **CPU特性**: SMP, 64位, 虚拟化支持
- **设备驱动**: 网络, 存储, 输入设备
- **文件系统**: EXT4, BTRFS, NFS
- **网络协议**: TCP/IP, IPv6, 无线
- **安全特性**: SELinux, 加密支持
- **调试选项**: 内核调试, 符号表

#### 使用预定义配置
```bash
# 使用服务器配置
make vest-os_server_defconfig

# 使用桌面配置
make vest-os_desktop_defconfig

# 使用嵌入式配置
make vest-os_embedded_defconfig
```

#### 自定义配置文件
```bash
# 创建配置文件
cp config/defconfig .config

# 编辑配置
vim .config

# 验证配置
make olddefconfig
```

### 2. 构建

#### 完整构建
```bash
# 使用所有CPU核心
make -j$(nproc)

# 指定并行度
make -j8

# 构建时显示详细信息
make V=1

# 静默构建
make -s
```

#### 分步构建
```bash
# 1. 构建工具链
make toolchain

# 2. 构建内核
make kernel

# 3. 构建初始化RAMFS
make initramfs

# 4. 构建用户空间
make userspace

# 5. 创建系统镜像
make image
```

### 3. 配置示例

#### 最小系统配置
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

#### 服务器配置
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

#### 桌面配置
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

## 配置选项详解

### 内核配置

#### 基本配置
```config
# 版本信息
CONFIG_VERSION="1.0.0"
CONFIG_LOCALVERSION="-custom"

# 编译器选项
CONFIG_CC_VERSION_TEXT="gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE=y
CONFIG_CC_OPTIMIZE_FOR_SIZE=n

# 链接器选项
CONFIG_LD_VERSION="2.34"
CONFIG_LD_SCRIPT_KERNEL=y
CONFIG_LD_SCRIPT_BUILTIN=y

# 调试信息
CONFIG_DEBUG_KERNEL=y
CONFIG_DEBUG_INFO=y
CONFIG_DEBUG_INFO_REDUCED=n
CONFIG_STRIP_ASM_SYMS=y
CONFIG_READABLE_ASM=y
```

#### 内存管理
```config
# 内存模型
CONFIG_FLATMEM=y
CONFIG_SPARSEMEM=n
CONFIG_MEMORY_HOTPLUG=n

# SLAB分配器
CONFIG_SLUB=y
CONFIG_SLUB_DEBUG=y
CONFIG_SLUB_CPU_PARTIAL=y

# 虚拟内存
CONFIG_MMU=y
CONFIG_PAGE_OFFSET=0xffff880000000000
CONFIG_PHYSICAL_START=0x1000000
CONFIG_PHYSICAL_ALIGN=0x200000
```

#### 进程调度
```config
# 调度器
CONFIG_SCHED_AUTOGROUP=y
CONFIG_SCHED_DEBUG=y
CONFIG_SCHEDSTATS=y

# 抢占模式
CONFIG_PREEMPT_NONE=y
CONFIG_PREEMPT_VOLUNTARY=n
CONFIG_PREEMPT=y
CONFIG_PREEMPT_RT=n
```

### 用户空间配置

#### 初始化系统
```config
# Init系统
CONFIG_INIT_SYSTEM=y
CONFIG_INIT_SYSTEMD=y

# Shell
CONFIG_BASH=y
CONFIG_ZSH=y

# 核心工具
CONFIG_COREUTILS=y
CONFIG_UTIL_LINUX=y
CONFIG_PROCPS=y
CONFIG_E2FSPROGS=y
```

#### 网络工具
```config
# 基本网络工具
CONFIG_IPROUTE2=y
CONFIG_IPTABLES=y
CONFIG_NFTABLES=y
CONFIG_WPA_SUPPLICANT=y

# 网络服务
CONFIG_SSHD=y
CONFIG_NTPD=y
CONFIG_CRON=y
```

## 打包

### 创建ISO镜像
```bash
# 创建可启动ISO
make iso

# 带UEFI支持的ISO
make iso UEFI=y

# 创建混合ISO（BIOS+UEFI）
make iso HYBRID=y
```

### 创建磁盘镜像
```bash
# 创建虚拟磁盘镜像
make img

# 指定镜像大小（GB）
make img SIZE=8

# 创建云镜像（qcow2格式）
make img FORMAT=qcow2

# 创建原始镜像
make img FORMAT=raw
```

### 创建tar包
```bash
# 创建源码tar包
make tar

# 创建二进制tar包
make tar-binary

# 创建SDK tar包
make tar-sdk
```

## 部署方法

### 1. 裸机部署

#### 创建启动USB
```bash
# 写入ISO到USB
sudo dd if=vest-os-1.0.0.iso of=/dev/sdX bs=4M status=progress
sync

# 或使用ddrescue（更安全）
sudo ddrescue vest-os-1.0.0.iso /dev/sdX
```

#### 网络启动（PXE）
```bash
# 设置TFTP服务器
mkdir -p /tftpboot
cp vest-os-1.0.0/vmlinuz /tftpboot/
cp vest-os-1.0.0/initramfs /tftpboot/

# DHCP配置（/etc/dhcp/dhcpd.conf）
subnet 192.168.1.0 netmask 255.255.255.0 {
    range 192.168.1.100 192.168.1.200;
    next-server 192.168.1.1;
    filename "pxelinux.0";
}

# TFTP配置（/etc/xinetd.d/tftp）
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

### 2. 虚拟化部署

#### KVM/QEMU
```bash
# 创建虚拟磁盘
qemu-img create -f qcow2 vest-os.qcow2 20G

# 启动虚拟机
qemu-system-x86_64 \
    -m 4096 \
    -cpu host \
    -enable-kvm \
    -hda vest-os.qcow2 \
    -cdrom vest-os-1.0.0.iso \
    -boot d

# 或使用virt-install
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
# 创建虚拟机
VBoxManage createvm --name "Vest-OS" --register
VBoxManage modifyvm "Vest-OS" --memory 4096 --cpus 4
VBoxManage storagectl "Vest-OS" --name "SATA" --add sata
VBoxManage createhd --filename "vest-os.vdi" --size 20000
VBoxManage storageattach "Vest-OS" --storagectl "SATA" --port 0 --device 0 --type hdd --medium "vest-os.vdi"
VBoxManage storageattach "Vest-OS" --storagectl "SATA" --port 1 --device 0 --type dvddrive --medium "vest-os-1.0.0.iso"
```

#### VMware
```bash
# 创建OVF模板
ovftool \
    --compress=9 \
    vest-os.vmx \
    vest-os.ovf

# 部署到vSphere
govc import.ovf \
    -ds datastore1 \
    -pool Resources \
    -host esxi-host \
    vest-os.ovf
```

### 3. 云平台部署

#### OpenStack
```bash
# 上传镜像
openstack image create \
    --disk-format qcow2 \
    --container-format bare \
    --file vest-os-1.0.0.qcow2 \
    vest-os-1.0.0

# 创建实例
openstack server create \
    --image vest-os-1.0.0 \
    --flavor m1.medium \
    --key-name mykey \
    vest-os-vm
```

#### AWS EC2
```bash
# 安装AWS CLI
pip install awscli

# 上传镜像到S3
aws s3 cp vest-os-1.0.0.raw s3://my-bucket/

# 导入快照
aws ec2 import-snapshot \
    --description "Vest-OS Snapshot" \
    --disk-container "Description=Vest-OS,Format=raw,UserBucket={S3Bucket=my-bucket,S3Key=vest-os-1.0.0.raw}"

# 创建AMI
aws ec2 register-image \
    --name "Vest-OS 1.0.0" \
    --architecture x86_64 \
    --root-device-name /dev/sda1 \
    --virtualization-type hvm \
    --block-device-mappings "DeviceName=/dev/sda1,Ebs={SnapshotId=snap-xxxxxxxx}"
```

#### Azure
```bash
# 安装Azure CLI
curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

# 创建存储账户
az storage account create \
    --name mystorageaccount \
    --resource-group myResourceGroup \
    --location westus

# 上传VHD
az storage blob upload \
    --account-name mystorageaccount \
    --container-name images \
    --file vest-os-1.0.0.vhd \
    --name vest-os-1.0.0.vhd

# 创建镜像
az image create \
    --resource-group myResourceGroup \
    --name vest-os-image \
    --source https://mystorageaccount.blob.core.windows.net/images/vest-os-1.0.0.vhd
```

### 4. 容器化部署

#### Docker
```dockerfile
# Dockerfile
FROM scratch
COPY vest-os-rootfs.tar.gz /
CMD ["/sbin/init"]
```

```bash
# 构建镜像
docker build -t vest-os:1.0.0 .

# 运行容器
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

## 持续集成

### GitHub Actions配置
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

## 故障排除

### 常见构建错误

#### 1. 编译错误
```
error: 'CONFIG_X86_64' undeclared
```
**解决方案：**
```bash
make clean
make defconfig
make menuconfig  # 确保架构正确
make -j$(nproc)
```

#### 2. 链接错误
```
undefined reference to `symbol_name'
```
**解决方案：**
```bash
# 检查配置
grep -R "symbol_name" .config

# 启用相关配置
make menuconfig
# 启用包含该符号的模块
```

#### 3. 内存不足
```
g++: internal compiler error: Killed (program cc1plus)
```
**解决方案：**
```bash
# 减少并行任务数
make -j2

# 或增加交换空间
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

#### 4. 依赖缺失
```
fatal error: openssl/ssl.h: No such file or directory
```
**解决方案：**
```bash
# 安装开发包
sudo apt install libssl-dev

# 或指定库路径
make CFLAGS=-I/usr/include/openssl LDFLAGS=-L/usr/lib/ssl
```

### 性能优化

#### 加速构建
```bash
# 使用ccache
export CC="ccache gcc"
export CXX="ccache g++"

# 使用tmpfs
sudo mount -t tmpfs -o size=4G tmpfs /tmp

# 使用distcc（分布式编译）
export CC="distcc gcc"
make -j$(distcc -j)
```

#### 增量构建
```bash
# 启用ccache
export CCACHE_DIR=~/.ccache
export CCACHE_MAXSIZE=10G
export CCACHE_COMPRESS=1

# 清理ccache
ccache -C
```

### 调试技巧

#### 查看构建日志
```bash
# 详细日志
make V=1 > build.log 2>&1

# 只显示错误和警告
make 2>&1 | grep -E "(error|warning)"
```

#### 分析构建时间
```bash
# 使用make的时间分析
make --debug=basic

# 或使用buildtime脚本
./scripts/buildtime.sh
```

#### 跟踪依赖
```bash
# 查看依赖关系
make -p | grep -E "^[^:#=-].*:" | head

# 查看特定目标的依赖
make -qp | grep -A5 "target_name:"
```

---

*文档版本：1.0*
*最后更新：2024年1月*