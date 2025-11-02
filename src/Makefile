# Vest-OS 主 Makefile
# 构建整个操作系统

# 默认目标
.DEFAULT_GOAL := all

# 架构设置
ARCH ?= x86_64

# 构建配置
BUILD_TYPE ?= debug
CROSS_COMPILE ?=

# 工具链
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)nasm
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP = $(CROSS_COMPILE)strip

# 基本目录
SRC_DIR = src
BUILD_DIR = build
DIST_DIR = dist
TOOLS_DIR = tools

# 构建目录
KERNEL_BUILD = $(BUILD_DIR)/kernel
USERSPACE_BUILD = $(BUILD_DIR)/userspace
BOOTLOADER_BUILD = $(BUILD_DIR)/bootloader

# 源码目录
KERNEL_DIR = $(SRC_DIR)/kernel
USERSPACE_DIR = $(SRC_DIR)/userspace
BOOTLOADER_DIR = $(SRC_DIR)/tools/bootloader

# 编译标志
COMMON_CFLAGS = -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-stack-protector
COMMON_CFLAGS += -nodefaultlibs -ffreestanding

# 架构相关标志
ifeq ($(ARCH), x86_64)
    COMMON_CFLAGS += -m64 -D__x86_64__
    COMMON_ASFLAGS = -f elf64
    COMMON_LDFLAGS = -m elf_x86_64
    KERNEL_IMAGE = kernel64.bin
    BOOTLOADER_IMAGE = boot64.bin
else ifeq ($(ARCH), i386)
    COMMON_CFLAGS += -m32 -D__i386__
    COMMON_ASFLAGS = -f elf32
    COMMON_LDFLAGS = -m elf_i386
    KERNEL_IMAGE = kernel32.bin
    BOOTLOADER_IMAGE = boot32.bin
else
    $(error 不支持的架构: $(ARCH))
endif

# 构建类型相关标志
ifeq ($(BUILD_TYPE), release)
    COMMON_CFLAGS += -O2 -DNDEBUG
else
    COMMON_CFLAGS += -g -O0 -DDEBUG
endif

# 包含目录
INCLUDES = -I$(SRC_DIR)/kernel/include -I$(SRC_DIR)/kernel/hal/common

# 内核标志
KERNEL_CFLAGS = $(COMMON_CFLAGS) $(INCLUDES)
KERNEL_LDFLAGS = $(COMMON_LDFLAGS) -T $(KERNEL_DIR)/linker_$(ARCH).ld

# 用户空间标志
USERSPACE_CFLAGS = $(COMMON_CFLAGS) -DUSERSPACE
USERSPACE_LDFLAGS = $(COMMON_LDFLAGS)

# 引导加载程序标志
BOOTLOADER_ASFLAGS = $(COMMON_ASFLAGS)

# 目标文件
KERNEL_TARGET = $(DIST_DIR)/$(KERNEL_IMAGE)
BOOTLOADER_TARGET = $(DIST_DIR)/$(BOOTLOADER_IMAGE)
SYSTEM_IMAGE = $(DIST_DIR)/vestos_$(ARCH).img

# 源文件
KERNEL_SOURCES = $(shell find $(KERNEL_DIR) -name "*.c")
USERSPACE_SOURCES = $(shell find $(USERSPACE_DIR) -name "*.c")
BOOTLOADER_SOURCES = $(BOOTLOADER_DIR)/boot.asm

# 主要目标
.PHONY: all clean install qemu bochs debug info help

all: $(SYSTEM_IMAGE)

# 构建完整系统镜像
$(SYSTEM_IMAGE): $(BOOTLOADER_TARGET) $(KERNEL_TARGET) userspace
	@echo "创建系统镜像: $@"
	@mkdir -p $(DIST_DIR)
	@dd if=/dev/zero of=$@ bs=1M count=64 2>/dev/null
	@mkfs.vfat -F 32 $@ 2>/dev/null || true
	@dd if=$(BOOTLOADER_TARGET) of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	@mcopy -i $@ $(KERNEL_TARGET) ::/kernel.bin 2>/dev/null || true
	@mkdir -p $(BUILD_DIR)/temp_mount
	@sudo mount -o loop $@ $(BUILD_DIR)/temp_mount 2>/dev/null || true
	@mkdir -p $(BUILD_DIR)/temp_mount/boot
	@cp $(KERNEL_TARGET) $(BUILD_DIR)/temp_mount/boot/kernel.bin 2>/dev/null || true
	@cp -r $(USERSPACE_BUILD)/* $(BUILD_DIR)/temp_mount/ 2>/dev/null || true
	@sudo umount $(BUILD_DIR)/temp_mount 2>/dev/null || true
	@rm -rf $(BUILD_DIR)/temp_mount
	@echo "系统镜像构建完成: $@"

# 构建引导加载程序
$(BOOTLOADER_TARGET): $(BOOTLOADER_BUILD)/bootloader.o
	@echo "链接引导加载程序: $@"
	@mkdir -p $(DIST_DIR)
	$(LD) $(COMMON_LDFLAGS) -T $(BOOTLOADER_DIR)/linker.ld -o $@ $<

$(BOOTLOADER_BUILD)/bootloader.o: $(BOOTLOADER_SOURCES)
	@echo "汇编引导加载程序: $<"
	@mkdir -p $(BOOTLOADER_BUILD)
	$(AS) $(BOOTLOADER_ASFLAGS) -o $@ $<

# 构建内核
$(KERNEL_TARGET): $(KERNEL_BUILD)/kernel.bin
	@echo "复制内核: $@"
	@mkdir -p $(DIST_DIR)
	@cp $< $@

$(KERNEL_BUILD)/kernel.bin: $(KERNEL_BUILD)/kernel.elf
	@echo "生成内核二进制文件: $@"
	@$(OBJCOPY) -O binary $< $@

$(KERNEL_BUILD)/kernel.elf: $(KERNEL_BUILD)/kernel.o $(KERNEL_BUILD)/hal.o $(KERNEL_BUILD)/drivers.o $(KERNEL_BUILD)/lib.o
	@echo "链接内核: $@"
	@mkdir -p $(KERNEL_BUILD)
	$(LD) $(KERNEL_LDFLAGS) -o $@ $^

# 内核对象文件
$(KERNEL_BUILD)/kernel.o:
	@echo "编译内核核心"
	@mkdir -p $(KERNEL_BUILD)
	$(MAKE) -C $(KERNEL_DIR) BUILD_DIR=$(KERNEL_BUILD) ARCH=$(ARCH) CC=$(CC) AS=$(AS) LD=$(LD)

# 构建用户空间
.PHONY: userspace
userspace:
	@echo "构建用户空间程序"
	@mkdir -p $(USERSPACE_BUILD)
	$(MAKE) -C $(USERSPACE_DIR) BUILD_DIR=$(USERSPACE_BUILD) ARCH=$(ARCH) CC=$(CC) LD=$(LD)

# 安装目标
.PHONY: install
install: $(SYSTEM_IMAGE)
	@echo "安装Vest-OS到 /boot/vestos"
	@sudo mkdir -p /boot/vestos
	@sudo cp $< /boot/vestos/
	@sudo cp $(KERNEL_TARGET) /boot/vestos/kernel.bin
	@echo "安装完成"

# 调试目标
.PHONY: debug
debug: all
	@echo "启动调试会话"
	@$(OBJDUMP) -d $(KERNEL_BUILD)/kernel.elf > $(BUILD_DIR)/kernel.dump
	@echo "反汇编代码已保存到: $(BUILD_DIR)/kernel.dump"

# QEMU模拟
.PHONY: qemu
qemu: all
	@echo "启动QEMU模拟器"
ifeq ($(ARCH), x86_64)
	qemu-system-x86_64 -m 256M -drive file=$(SYSTEM_IMAGE),format=raw,index=0,media=disk -serial stdio
else
	qemu-system-i386 -m 256M -drive file=$(SYSTEM_IMAGE),format=raw,index=0,media=disk -serial stdio
endif

# QEMU调试
.PHONY: qemu-debug
qemu-debug: all
	@echo "启动QEMU调试模式"
ifeq ($(ARCH), x86_64)
	qemu-system-x86_64 -m 256M -drive file=$(SYSTEM_IMAGE),format=raw,index=0,media=disk -serial stdio -s -S &
	gdb -x gdb_debug.txt
else
	qemu-system-i386 -m 256M -drive file=$(SYSTEM_IMAGE),format=raw,index=0,media=disk -serial stdio -s -S &
	gdb -x gdb_debug.txt
endif

# Bochs模拟
.PHONY: bochs
bochs: all
	@echo "启动Bochs模拟器"
	bochs -f bochs_config.txt

# 清理
clean:
	@echo "清理构建文件"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DIST_DIR)
	@$(MAKE) -C $(KERNEL_DIR) clean 2>/dev/null || true
	@$(MAKE) -C $(USERSPACE_DIR) clean 2>/dev/null || true

# 信息
info:
	@echo "Vest-OS 构建信息"
	@echo "================="
	@echo "架构: $(ARCH)"
	@echo "构建类型: $(BUILD_TYPE)"
	@echo "编译器: $(CC)"
	@echo "汇编器: $(AS)"
	@echo "链接器: $(LD)"
	@echo "内核镜像: $(KERNEL_TARGET)"
	@echo "引导程序: $(BOOTLOADER_TARGET)"
	@echo "系统镜像: $(SYSTEM_IMAGE)"
	@echo ""
	@echo "可用目标:"
	@echo "  all         - 构建完整系统"
	@echo "  kernel      - 仅构建内核"
	@echo "  userspace   - 仅构建用户空间"
	@echo "  bootloader  - 仅构建引导程序"
	@echo "  clean       - 清理构建文件"
	@echo "  install     - 安装系统"
	@echo "  qemu        - 启动QEMU模拟器"
	@echo "  qemu-debug  - 启动QEMU调试模式"
	@echo "  bochs       - 启动Bochs模拟器"
	@echo "  debug       - 生成调试信息"
	@echo "  info        - 显示构建信息"
	@echo "  help        - 显示帮助信息"

# 帮助
help:
	@echo "Vest-OS Makefile"
	@echo "================"
	@echo ""
	@echo "基本用法:"
	@echo "  make [ARCH=x86_64|i386] [BUILD_TYPE=debug|release] [目标]"
	@echo ""
	@echo "示例:"
	@echo "  make                    # 构建默认架构(64位)"
	@echo "  make ARCH=i386          # 构建32位版本"
	@echo "  make BUILD_TYPE=release # 构建发布版本"
	@echo "  make clean              # 清理所有构建文件"
	@echo "  make qemu               # 在QEMU中运行"
	@echo ""
	@echo "架构支持:"
	@echo "  x86_64 - 64位x86架构 (默认)"
	@echo "  i386   - 32位x86架构"
	@echo ""
	@echo "构建类型:"
	@echo "  debug   - 调试版本，包含调试信息 (默认)"
	@echo "  release - 发布版本，优化编译"
	@echo ""
	@echo "依赖工具:"
	@echo "  - GCC/Clang"
	@echo "  - NASM"
	@echo "  - GNU LD"
	@echo "  - QEMU (可选，用于模拟)"
	@echo "  - Bochs (可选，用于模拟)"

# 开发者工具
.PHONY: docs test coverage

# 生成文档
docs:
	@echo "生成文档"
	@mkdir -p $(BUILD_DIR)/docs
	@doxygen Doxyfile 2>/dev/null || echo "需要安装Doxygen来生成文档"

# 运行测试
test:
	@echo "运行测试"
	@$(MAKE) -C tests BUILD_DIR=$(BUILD_DIR)/tests

# 生成覆盖率报告
coverage:
	@echo "生成覆盖率报告"
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(COMMON_CFLAGS) --coverage" all
	@gcov $(KERNEL_SOURCES) 2>/dev/null || echo "需要gcov工具"

# 持续集成目标
.PHONY: ci

ci: clean all test
	@echo "持续集成构建完成"

# 发布目标
.PHONY: release

release: clean
	@echo "构建发布版本"
	$(MAKE) all BUILD_TYPE=release
	@mkdir -p $(DIST_DIR)/release
	@cp $(SYSTEM_IMAGE) $(DIST_DIR)/release/
	@cp -r docs $(DIST_DIR)/release/ 2>/dev/null || true
	@cd $(DIST_DIR)/release && tar czf vestos_$(ARCH)_release.tar.gz *
	@echo "发布包已创建: $(DIST_DIR)/release/vestos_$(ARCH)_release.tar.gz"

# 检查代码风格
.PHONY: check-style

check-style:
	@echo "检查代码风格"
	@find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -dry-run -Werror
	@find $(SRC_DIR) -name "*.c" | xargs cppcheck --enable=all --std=c99

# 代码格式化
.PHONY: format

format:
	@echo "格式化代码"
	@find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i