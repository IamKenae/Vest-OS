# Vest-OS 源码结构说明

## 目录结构

```
src/
├── kernel/                   # 内核源码
│   ├── core/                # 内核核心模块
│   │   ├── scheduler.c/h   # 进程调度器
│   │   ├── ipc.c/h         # 进程间通信
│   │   ├── memory.c/h      # 内存管理
│   │   ├── interrupt.c/h   # 中断处理
│   │   ├── syscall.c/h     # 系统调用
│   │   ├── exception.c/h   # 异常处理
│   │   └── timer.c/h       # 定时器管理
│   ├── drivers/             # 设备驱动
│   │   ├── tty/           # TTY终端驱动
│   │   ├── storage/       # 存储设备驱动
│   │   ├── network/       # 网络设备驱动
│   │   ├── input/         # 输入设备驱动
│   │   └── graphics/      # 图形设备驱动
│   ├── hal/                # 硬件抽象层
│   │   ├── x86/           # x86架构支持
│   │   │   ├── 32bit/    # 32位支持
│   │   │   └── 64bit/    # 64位支持
│   │   ├── arm/           # ARM架构支持(预留)
│   │   └── common/        # 通用HAL组件
│   ├── include/            # 内核头文件
│   ├── lib/               # 内核库
│   └── Makefile           # 内核构建脚本
├── userspace/              # 用户空间程序
│   ├── services/          # 系统服务
│   │   ├── filesystem/   # 文件系统服务
│   │   ├── network/      # 网络服务
│   │   ├── process/      # 进程管理服务
│   │   ├── memory/       # 内存管理服务
│   │   └── device/       # 设备管理服务
│   ├── lib/              # 系统库
│   │   ├── libc/         # C标准库
│   │   ├── libposix/     # POSIX兼容库
│   │   ├── libvestos/    # VestOS专用库
│   │   └── libgui/       # GUI库
│   ├── utils/            # 系统工具
│   │   ├── shell/        # Shell程序
│   │   ├── init/         # 初始化程序
│   │   ├── coreutils/    # 核心工具集
│   │   └── system/       # 系统管理工具
│   ├── apps/             # 应用程序
│   │   ├── gui/          # GUI应用程序
│   │   ├── cli/          # CLI应用程序
│   │   └── system/       # 系统应用程序
│   └── Makefile          # 用户空间构建脚本
├── tools/                # 开发工具
│   ├── compiler/         # 编译器工具链
│   ├── linker/           # 链接器
│   ├── debugger/         # 调试器
│   └── bootloader/       # 引导加载程序
├── tests/                # 测试代码
│   ├── unit/             # 单元测试
│   ├── integration/      # 集成测试
│   └── system/           # 系统测试
└── docs/                 # 文档
    ├── design/           # 设计文档
    ├── api/              # API文档
    └── tutorials/        # 教程文档
```

## 编译说明

### 内核编译
```bash
# 编译32位内核
make ARCH=i386

# 编译64位内核
make ARCH=x86_64
```

### 用户空间编译
```bash
# 编译所有用户空间程序
make userspace

# 编译特定服务
make services/filesystem
```

### 完整系统编译
```bash
# 编译完整系统
make all

# 清理构建文件
make clean
```

## 开发规范

### 代码风格
- 使用4空格缩进
- 函数名使用下划线命名法
- 变量名使用驼峰命名法
- 常量使用大写字母和下划线

### 提交规范
- 使用描述性的提交信息
- 每个提交只包含一个功能或修复
- 代码必须通过测试才能提交

### 文档要求
- 所有公共API必须有文档
- 复杂算法必须有注释说明
- 重要的设计决策需要记录ADR