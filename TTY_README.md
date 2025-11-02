# Vest-OS TTY终端系统

## 概述

Vest-OS TTY终端系统是一个完整的终端输入输出子系统，为操作系统提供文本模式显示、键盘输入处理和多虚拟终端支持。该系统采用模块化设计，提供从底层硬件到用户空间的完整接口。

## 系统架构

### 核心组件

1. **VGA文本模式驱动** (`drivers/tty/vga.c`)
   - 80x25文本模式显示
   - 16色前景/背景色支持
   - 硬件光标控制
   - 屏幕滚动和区域操作

2. **键盘输入驱动** (`drivers/tty/keyboard.c`)
   - 美国键盘布局支持
   - 修饰键处理(Shift, Ctrl, Alt)
   - LED指示灯控制
   - 键盘缓冲区管理

3. **TTY驱动核心** (`drivers/tty/tty.c`)
   - TTY设备管理
   - 行规程处理
   - 输入输出缓冲区
   - 多终端支持

4. **终端管理器** (`kernel/terminal.c`)
   - 虚拟终端切换
   - ANSI转义序列解析
   - 终端会话管理
   - 历史记录支持

5. **用户空间接口** (`lib/tty.c`)
   - POSIX兼容的termios接口
   - 系统调用包装
   - 伪终端支持
   - 终端控制工具

## 目录结构

```
Vest-OS/
├── drivers/tty/          # TTY驱动程序
│   ├── vga.c            # VGA显示驱动
│   ├── keyboard.c       # 键盘输入驱动
│   └── tty.c            # TTY核心驱动
├── kernel/              # 内核支持
│   ├── terminal.c       # 终端管理器
│   ├── string.c         # 字符串函数
│   ├── memory.c         # 内存管理
│   └── spinlock.c       # 自旋锁
├── arch/x86/            # 架构支持
│   ├── io.c             # I/O端口操作
│   └── interrupt.c      # 中断处理
├── lib/                 # 用户空间库
│   └── tty.c            # TTY用户接口
├── include/             # 头文件
│   ├── drivers/         # 驱动头文件
│   ├── kernel/          # 内核头文件
│   ├── sys/             # 系统调用头文件
│   └── arch/            # 架构头文件
├── examples/            # 示例程序
│   └── tty_test.c       # TTY测试程序
└── Makefile             # 构建系统
```

## 功能特性

### 显示功能
- ✅ 80x25标准文本模式
- ✅ 16色前景/背景色支持
- ✅ 硬件光标控制
- ✅ 屏幕滚动和清除
- ✅ 区域复制和填充
- ✅ ANSI颜色序列支持

### 输入功能
- ✅ 标准QWERTY键盘支持
- ✅ 修饰键处理(Shift, Ctrl, Alt)
- ✅ LED指示灯控制(NumLock, CapsLock, ScrollLock)
- ✅ 键盘重复率设置
- ✅ 输入缓冲区管理

### 终端功能
- ✅ 多虚拟终端支持(最多8个)
- ✅ 终端切换
- ✅ 行规程处理
- ✅ 规范模式和原始模式
- ✅ 回显控制
- ✅ 信号处理

### 用户接口
- ✅ POSIX termios兼容
- ✅ 标准I/O重定向
- ✅ 伪终端支持
- ✅ 终端属性控制
- ✅ 窗口大小控制

## 构建和安装

### 构建系统

```bash
# 创建目录结构
make dirs

# 构建所有目标
make all

# 查看帮助
make help

# 清理构建文件
make clean

# 深度清理
make distclean
```

### 安装

```bash
# 安装头文件
make install-headers

# 安装库文件
make install

# 完整安装
make install-all
```

### 构建目标

- `libtty.a` - TTY用户空间库
- `tty_driver.o` - TTY驱动对象文件
- `kernel_tty.o` - 内核TTY支持

## 使用示例

### 基本TTY操作

```c
#include <sys/tty.h>
#include <stdio.h>

int main() {
    // 打开TTY设备
    int fd = tty_open("/dev/tty0", O_RDWR);

    // 写入数据
    tty_write(fd, "Hello TTY!\n", 12);

    // 读取数据
    char buffer[256];
    ssize_t bytes = tty_read(fd, buffer, sizeof(buffer));

    // 关闭设备
    tty_close(fd);

    return 0;
}
```

### 终端属性控制

```c
#include <sys/termios.h>
#include <sys/tty.h>

// 设置原始模式
struct termios original, raw;
int fd = tty_open("/dev/tty0", O_RDWR);

tcgetattr(fd, &original);
raw = original;
raw.c_lflag &= ~(ECHO | ICANON);
tcsetattr(fd, TCSANOW, &raw);

// 恢复原始模式
tcsetattr(fd, TCSANOW, &original);
```

### 颜色输出

```c
// ANSI颜色序列
printf("\033[31m红色文本\033[0m\n");
printf("\033[32;44m绿字蓝背景\033[0m\n");
```

## API参考

### TTY设备操作

| 函数 | 描述 |
|------|------|
| `tty_open()` | 打开TTY设备 |
| `tty_close()` | 关闭TTY设备 |
| `tty_read()` | 读取TTY数据 |
| `tty_write()` | 写入TTY数据 |
| `tty_isatty()` | 检查是否为TTY设备 |

### 终端属性控制

| 函数 | 描述 |
|------|------|
| `tcgetattr()` | 获取终端属性 |
| `tcsetattr()` | 设置终端属性 |
| `tcflush()` | 刷新输入输出队列 |
| `tcflow()` | 控制数据流 |
| `tcdrain()` | 等待输出完成 |

### 终端信息

| 函数 | 描述 |
|------|------|
| `tty_get_info()` | 获取TTY信息 |
| `tty_get_winsize()` | 获取窗口大小 |
| `tty_set_winsize()` | 设置窗口大小 |
| `tty_get_name()` | 获取TTY名称 |

## 配置选项

### 编译时配置

- `MAX_TTYS` - 最大TTY数量(默认8)
- `TTY_BUFFER_SIZE` - TTY缓冲区大小(默认4096)
- `VGA_WIDTH` - VGA宽度(默认80)
- `VGA_HEIGHT` - VGA高度(默认25)

### 运行时配置

```c
// 设置TTY模式
tty_set_mode(fd, TTY_MODE_RAW);     // 原始模式
tty_set_mode(fd, TTY_MODE_COOKED);  // 熟模式

// 设置颜色
tty_set_color(fd, VGA_COLOR_GREEN, VGA_COLOR_BLACK);

// 设置回显
tty_set_echo(fd, 1);  // 启用回显
tty_set_echo(fd, 0);  // 禁用回显
```

## 测试程序

项目包含完整的测试程序 `examples/tty_test.c`，测试以下功能：

- TTY基本操作
- 终端属性控制
- 输入输出操作
- 伪终端支持
- 颜色显示

运行测试：

```bash
# 编译测试程序
gcc -o tty_test examples/tty_test.c -L. -ltty

# 运行测试
./tty_test
```

## 技术规格

### 硬件要求

- x86架构处理器
- VGA兼容显卡
- 标准AT/PS2键盘
- 最小1MB内存

### 性能指标

- 显示响应时间: <1ms
- 键盘响应时间: <1ms
- 终端切换时间: <10ms
- 支持并发终端: 最多8个

### 内存使用

- VGA缓冲区: 4KB
- 键盘缓冲区: 2KB
- TTY缓冲区: 每终端4KB
- 总内存占用: <64KB

## 兼容性

### 标准兼容

- IEEE Std 1003.1 (POSIX.1)
- ANSI X3.64-1979 (转义序列)
- ISO 6429 (控制序列)

### 平台支持

- ✅ x86/i386
- ⏳ x86_64 (计划中)
- ⏳ ARM (计划中)

## 故障排除

### 常见问题

**Q: TTY设备无法打开**
A: 检查设备节点是否存在，确认权限设置正确

**Q: 键盘输入无响应**
A: 检查键盘驱动初始化，确认中断处理正确

**Q: 显示异常**
A: 检查VGA初始化，确认显存映射正确

**Q: 终端切换失败**
A: 检查终端管理器状态，确认目标终端可用

### 调试选项

```c
// 启用调试输出
#define DEBUG_TTY 1

// 检查TTY状态
struct tty_info info;
tty_get_info(fd, &info);
printf("TTY状态: %d\n", info.state);
```

## 贡献指南

### 开发环境

- GCC 7.0+
- NASM 2.13+
- Make 4.0+

### 代码规范

- 遵循Linux内核编码风格
- 使用Doxygen注释格式
- 函数命名采用下划线分隔
- 变量命名使用描述性名称

### 提交流程

1. Fork项目仓库
2. 创建功能分支
3. 提交代码更改
4. 运行测试验证
5. 提交Pull Request

## 许可证

本项目采用GPL v2许可证，详见LICENSE文件。

## 联系方式

- 项目主页: https://github.com/vestos/tty
- 问题报告: https://github.com/vestos/tty/issues
- 邮箱: vestos@example.com

---

**Vest-OS TTY终端系统** - 为现代操作系统提供可靠的终端支持