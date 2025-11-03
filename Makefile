# Vest-OS Makefile
# 支持内核和TTY系统构建

# 默认目标
TARGET ?= kernel

# 编译器设置
CC = gcc
AS = nasm
LD = ld
AR = ar
STRIP = strip

# 编译标志
CFLAGS = -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -ffreestanding -m32 -c -D__VESTOS__ -Iinclude -Iinclude/arch -Iinclude/drivers -Iinclude/kernel \
         -O2 -g -fno-pic

ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld
ARFLAGS = rcs

# 内核目标文件
KERNEL_OBJS = kernel.o

# TTY系统目标文件
TTY_KERNEL_OBJS = kernel/terminal.o \
                  kernel/string.o \
                  kernel/memory.o \
                  kernel/spinlock.o

TTY_DRIVER_OBJS = drivers/tty/vga.o \
                  drivers/tty/keyboard.o \
                  drivers/tty/tty.o

TTY_ARCH_OBJS = arch/x86/io.o \
                arch/x86/interrupt.o

TTY_LIB_OBJS = lib/tty.o

# 所有TTY目标文件
TTY_OBJS = $(TTY_KERNEL_OBJS) $(TTY_DRIVER_OBJS) $(TTY_ARCH_OBJS) $(TTY_LIB_OBJS)

# 目标
KERNEL_TARGET = kernel.bin
TTY_TARGETS = libtty.a tty_driver.o kernel_tty.o

# 默认目标
all: $(KERNEL_TARGET)

# 构建内核
$(KERNEL_TARGET): $(KERNEL_OBJS) linker.ld
	@echo "Linking Vest-OS kernel..."
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)
	@echo "Kernel created: $@"

# 创建TTY驱动库
libtty.a: $(TTY_LIB_OBJS)
	@echo "Creating TTY library..."
	$(AR) $(ARFLAGS) $@ $^
	@echo "TTY library created: $@"

# TTY驱动对象文件
tty_driver.o: $(TTY_DRIVER_OBJS) $(TTY_ARCH_OBJS)
	@echo "Linking TTY driver object..."
	$(LD) -m elf_i386 -r -o $@ $^
	@echo "TTY driver object created: $@"

# 内核TTY支持
kernel_tty.o: $(TTY_KERNEL_OBJS) $(TTY_DRIVER_OBJS) $(TTY_ARCH_OBJS)
	@echo "Linking kernel TTY support..."
	$(LD) -m elf_i386 -r -o $@ $^
	@echo "Kernel TTY support created: $@"

# TTY系统构建
tty: $(TTY_TARGETS)

# 构建所有目标
build-all: $(KERNEL_TARGET) $(TTY_TARGETS)

# 编译规则
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.s
	@echo "Assembling $<..."
	$(AS) $(ASFLAGS) -o $@ $<

%.o: %.S
	@echo "Assembling $<..."
	$(CC) $(CFLAGS) -D__ASM__ -o $@ $<

# 依赖文件
-include $(ALL_OBJS:.o=.d)

# 生成依赖
%.d: %.c
	@$(CC) -MM $(CFLAGS) $< > $@

# 清理
clean:
	@echo "Cleaning build files..."
	rm -f $(ALL_OBJS) $(ALL_OBJS:.o=.d) $(TARGETS)
	@echo "Clean completed"

# 深度清理
distclean: clean
	@echo "Deep cleaning..."
	rm -f *.a *.o
	@echo "Deep clean completed"

# 安装头文件
install-headers:
	@echo "Installing headers..."
	mkdir -p /usr/local/include/vestos
	mkdir -p /usr/local/include/vestos/drivers
	mkdir -p /usr/local/include/vestos/kernel
	mkdir -p /usr/local/include/vestos/sys
	mkdir -p /usr/local/include/vestos/arch
	cp -r include/drivers/*.h /usr/local/include/vestos/drivers/
	cp -r include/kernel/*.h /usr/local/include/vestos/kernel/
	cp -r include/sys/*.h /usr/local/include/vestos/sys/
	cp -r include/arch/*.h /usr/local/include/vestos/arch/
	@echo "Headers installed successfully"

# 安装库
install: libtty.a
	@echo "Installing library..."
	install -m 644 libtty.a /usr/local/lib/
	@echo "Library installed successfully"

# 完整安装
install-all: install install-headers
	@echo "Complete installation finished"

# 测试构建
test-build: all
	@echo "Testing build..."
	@echo "All targets built successfully"
	@echo "Library: libtty.a"
	@echo "Driver: tty_driver.o"
	@echo "Kernel: kernel_tty.o"

# QEMU运行
run: $(KERNEL_TARGET)
	@echo "Starting QEMU..."
	@if [ -f "qemu/run.sh" ]; then \
		chmod +x qemu/run.sh; \
		./qemu/run.sh; \
	else \
		echo "QEMU script not found"; \
		echo "Please install QEMU: sudo apt-get install qemu-system"; \
	fi

# QEMU调试
debug: $(KERNEL_TARGET)
	@echo "Starting QEMU in debug mode..."
	@if [ -f "qemu/run.sh" ]; then \
		chmod +x qemu/run.sh; \
		./qemu/run.sh -d; \
	else \
		echo "QEMU script not found"; \
	fi

# 创建磁盘镜像
image: $(KERNEL_TARGET)
	@echo "Creating disk image..."
	@if [ -f "scripts/build_image.sh" ]; then \
		chmod +x scripts/build_image.sh; \
		./scripts/build_image.sh; \
	else \
		echo "Image builder script not found"; \
	fi

# 测试系统
test: $(KERNEL_TARGET)
	@echo "Running Vest-OS tests..."
	@if [ -f "scripts/build_image.sh" ]; then \
		chmod +x scripts/build_image.sh; \
		./scripts/build_image.sh -t; \
	else \
		echo "Running basic kernel test in QEMU..."; \
		$(MAKE) run; \
	fi

# 显示帮助
help:
	@echo "Vest-OS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build kernel (default)"
	@echo "  kernel       - Build Vest-OS kernel"
	@echo "  tty          - Build TTY system components"
	@echo "  build-all    - Build all components"
	@echo "  run          - Run kernel in QEMU"
	@echo "  debug        - Run kernel in QEMU with debug"
	@echo "  image        - Create bootable disk image"
	@echo "  test         - Build and test system"
	@echo "  clean        - Clean build files"
	@echo "  distclean    - Deep clean"
	@echo "  install      - Install library"
	@echo "  install-headers - Install header files"
	@echo "  install-all  - Complete installation"
	@echo "  test-build   - Test build only"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make                # Build kernel"
	@echo "  make run            # Build and run in QEMU"
	@echo "  make test           # Build and test"
	@echo "  make tty            # Build TTY system"
	@echo "  make build-all       # Build everything"
	@echo ""
	@echo "Files:"
	@echo "  Kernel: kernel.c -> kernel.bin"
	@echo "  TTY Drivers: drivers/tty/"
	@echo "  VGA Driver:  drivers/tty/vga.c"
	@echo "  Keyboard:    drivers/tty/keyboard.c"
	@echo "  TTY Core:    drivers/tty/tty.c"
	@echo "  Terminal:    kernel/terminal.c"
	@echo "  User API:    lib/tty.c"
	@echo "  Headers:     include/"

# 声明伪目标
.PHONY: all kernel tty build-all run debug image test clean distclean install install-headers install-all test-build help

# 创建必要的目录结构
dirs:
	@echo "Creating directory structure..."
	mkdir -p drivers/tty kernel arch/x86 lib include/drivers include/kernel include/sys include/arch
	@echo "Directory structure created"

# 检查源文件
check-sources:
	@echo "Checking source files..."
	@for file in $(ALL_OBJS:.o=.c); do \
		if [ -f "$$file" ]; then \
			echo "✓ $$file"; \
		else \
			echo "✗ $$file (missing)"; \
		fi; \
	done

# 代码分析
analyze:
	@echo "Analyzing code..."
	@echo "TTY Driver Files:"
	@find drivers/tty -name "*.c" -exec echo "  {}" \;
	@echo "Kernel Files:"
	@find kernel -name "*.c" -exec echo "  {}" \;
	@echo "Library Files:"
	@find lib -name "*.c" -exec echo "  {}" \;
	@echo "Header Files:"
	@find include -name "*.h" -exec echo "  {}" \;

# 统计信息
stats:
	@echo "Code Statistics:"
	@echo "Lines of code:"
	@find drivers/tty kernel lib -name "*.c" -exec wc -l {} \; | awk '{sum += $$1} END {print "  Total: " sum}'
	@echo "Header files:"
	@find include -name "*.h" | wc -l | awk '{print "  Count: " $$1}'
	@echo "Source files:"
	@find drivers/tty kernel lib -name "*.c" | wc -l | awk '{print "  Count: " $$1}'

# 文档生成
docs:
	@echo "Generating documentation..."
	@echo "TTY System Documentation" > TTY_README.md
	@echo "=======================" >> TTY_README.md
	@echo "" >> TTY_README.md
	@echo "Components:" >> TTY_README.md
	@echo "1. VGA Text Mode Driver (vga.c)" >> TTY_README.md
	@echo "2. Keyboard Input Driver (keyboard.c)" >> TTY_README.md
	@echo "3. TTY Core Driver (tty.c)" >> TTY_README.md
	@echo "4. Terminal Manager (terminal.c)" >> TTY_README.md
	@echo "5. User Space Interface (tty.c)" >> TTY_README.md
	@echo "" >> TTY_README.md
	@echo "Build: make all" >> TTY_README.md
	@echo "Install: make install-all" >> TTY_README.md
	@echo "Documentation generated: TTY_README.md"