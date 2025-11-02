# Vest-OS 内核 Makefile
# 支持32位和64位x86架构

# 默认架构
ARCH ?= x86_64

# 编译器设置
CC = gcc
AS = nasm
LD = ld

# 编译标志
CFLAGS = -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-stack-protector
CFLAGS += -nodefaultlibs -ffreestanding -c

# 架构相关设置
ifeq ($(ARCH), x86_64)
    CFLAGS += -m64 -D__x86_64__
    ASFLAGS = -f elf64
    LDFLAGS = -m elf_x86_64
    LINKER_SCRIPT = linker_64.ld
    KERNEL_IMAGE = kernel64.bin
else ifeq ($(ARCH), i386)
    CFLAGS += -m32 -D__i386__
    ASFLAGS = -f elf32
    LDFLAGS = -m elf_i386
    LINKER_SCRIPT = linker_32.ld
    KERNEL_IMAGE = kernel32.bin
else
    $(error Unsupported architecture: $(ARCH))
endif

# 调试信息
ifdef DEBUG
    CFLAGS += -g -DDEBUG
endif

# 优化设置
CFLAGS += -O2

# 包含目录
INCLUDES = -Iinclude -Ihal/common -Ihal/$(ARCH)

# 源文件目录
CORE_DIR = core
DRIVERS_DIR = drivers
HAL_DIR = hal
LIB_DIR = lib

# 核心源文件
CORE_SOURCES = $(CORE_DIR)/scheduler.c \
               $(CORE_DIR)/ipc.c \
               $(CORE_DIR)/memory.c \
               $(CORE_DIR)/interrupt.c \
               $(CORE_DIR)/syscall.c \
               $(CORE_DIR)/exception.c \
               $(CORE_DIR)/timer.c

# 驱动源文件
DRIVER_SOURCES = $(DRIVERS_DIR)/tty/tty.c \
                 $(DRIVERS_DIR)/tty/console.c \
                 $(DRIVERS_DIR)/storage/ata.c \
                 $(DRIVERS_DIR)/network/rtl8139.c

# HAL源文件
HAL_SOURCES = $(HAL_DIR)/common/hal_common.c

# 架构相关源文件
ifeq ($(ARCH), x86_64)
    HAL_SOURCES += $(HAL_DIR)/x86/64bit/cpu.c \
                   $(HAL_DIR)/x86/64bit/memory.c \
                   $(HAL_DIR)/x86/64bit/interrupt.c
    ASM_SOURCES = $(HAL_DIR)/x86/64bit/boot.asm \
                  $(HAL_DIR)/x86/64bit/interrupt_asm.asm
else ifeq ($(ARCH), i386)
    HAL_SOURCES += $(HAL_DIR)/x86/32bit/cpu.c \
                   $(HAL_DIR)/x86/32bit/memory.c \
                   $(HAL_DIR)/x86/32bit/interrupt.c
    ASM_SOURCES = $(HAL_DIR)/x86/32bit/boot.asm \
                  $(HAL_DIR)/x86/32bit/interrupt_asm.asm
endif

# 库源文件
LIB_SOURCES = $(LIB_DIR)/string.c \
              $(LIB_DIR)/stdio.c \
              $(LIB_DIR)/stdlib.c

# 所有源文件
ALL_SOURCES = $(CORE_SOURCES) $(DRIVER_SOURCES) $(HAL_SOURCES) $(LIB_SOURCES)

# 生成目标文件
OBJECTS = $(ALL_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

# 目标
all: $(KERNEL_IMAGE)

# 链接内核
$(KERNEL_IMAGE): $(OBJECTS) $(LINKER_SCRIPT)
	@echo "链接内核: $(KERNEL_IMAGE)"
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE) $(OBJECTS)
	@echo "内核大小: $$(stat -c%s $(KERNEL_IMAGE)) 字节"

# 编译C文件
%.o: %.c
	@echo "编译: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# 编译汇编文件
%.o: %.asm
	@echo "汇编: $<"
	$(AS) $(ASFLAGS) -o $@ $<

# 清理
clean:
	@echo "清理构建文件..."
	find . -name "*.o" -delete
	rm -f $(KERNEL_IMAGE)

# 安装（复制到boot目录）
install: $(KERNEL_IMAGE)
	@echo "安装内核..."
	mkdir -p ../boot
	cp $(KERNEL_IMAGE) ../boot/

# 查看信息
info:
	@echo "Vest-OS 内核构建信息"
	@echo "架构: $(ARCH)"
	@echo "编译器: $(CC)"
	@echo "汇编器: $(AS)"
	@echo "链接器: $(LD)"
	@echo "编译标志: $(CFLAGS)"
	@echo "源文件数: $(words $(ALL_SOURCES))"
	@echo "目标文件: $(KERNEL_IMAGE)"

# 帮助
help:
	@echo "可用目标:"
	@echo "  all      - 编译内核"
	@echo "  clean    - 清理构建文件"
	@echo "  install  - 安装内核到boot目录"
	@echo "  info     - 显示构建信息"
	@echo "  help     - 显示此帮助信息"
	@echo ""
	@echo "架构选项:"
	@echo "  ARCH=i386     - 编译32位内核"
	@echo "  ARCH=x86_64   - 编译64位内核"
	@echo ""
	@echo "调试选项:"
	@echo "  DEBUG=1       - 启用调试信息"

# 依赖关系
-include $(OBJECTS:.o=.d)

# 生成依赖文件
%.d: %.c
	@$(CC) $(CFLAGS) $(INCLUDES) -MM $< > $@

.PHONY: all clean install info help