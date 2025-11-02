# Vest-OS 开发者指南

## 目录
- [开发环境搭建](#开发环境搭建)
- [代码结构](#代码结构)
- [开发流程](#开发流程)
- [编码规范](#编码规范)
- [内核开发](#内核开发)
- [驱动开发](#驱动开发)
- [应用程序开发](#应用程序开发)
- [调试技巧](#调试技巧)
- [测试框架](#测试框架)
- [贡献指南](#贡献指南)

## 开发环境搭建

### 1. 环境准备

#### 基础开发环境
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
    build-essential \
    git \
    gitk \
    cscope \
    ctags \
    vim \
    emacs \
    vscode \
    gdb \
    cgdb \
    valgrind \
    strace \
    ltrace \
    perf \
    trace-cmd

# 安装内核开发工具
sudo apt install -y \
    linux-headers-$(uname -r) \
    libelf-dev \
    libncurses-dev \
    bc \
    bison \
    flex
```

#### 配置开发工具

##### Vim配置 (.vimrc)
```vim
" 语法高亮
syntax on

" 显示行号
set number

" 高亮当前行
set cursorline

" 设置制表符
set tabstop=4
set shiftwidth=4
set expandtab

" 代码折叠
set foldmethod=syntax
set foldcolumn=4

" 搜索选项
set hlsearch
set incsearch

" 显示匹配括号
set showmatch

" 自动补全
set omnifunc=syntaxcomplete#Complete

" Ctags
set tags=tags;
set autochdir

" 快捷键
map <C-\> :cstag <C-R>=expand("<cword>")<CR><CR>
map <A-\> :tag <C-R>=expand("<cword>")<CR><CR>
```

##### Emacs配置 (.emacs)
```elisp
;; 启用语法高亮
(global-font-lock-mode 1)

;; 显示行号
(global-linum-mode 1)

;; C++模式配置
(add-hook 'c++-mode-hook
          (lambda ()
            (c-set-style "linux")
            (setq tab-width 4)
            (setq c-basic-offset 4)
            (setq indent-tabs-mode nil)))

;; 自动补全
(require 'auto-complete-config)
(ac-config-default)

;; CEDET
(require 'cedet)
(require 'semantic)
(global-semantic-mode 1)

;; GDB模式
(setq gdb-many-windows t)
```

##### VS Code配置 (.vscode/settings.json)
```json
{
    "editor.tabSize": 4,
    "editor.insertSpaces": true,
    "editor.formatOnSave": true,
    "files.associations": {
        "*.h": "c",
        "*.c": "c",
        "*.S": "c",
        "Makefile": "makefile",
        "*.mk": "makefile"
    },
    "C_Cpp.intelliSenseMode": "gcc-x64",
    "C_Cpp.default.cppStandard": "c11",
    "C_Cpp.default.cStandard": "c11"
}
```

### 2. 源码管理

#### Git配置
```bash
# 配置用户信息
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"

# 配置编辑器
git config --global core.editor vim

# 配置合并工具
git config --global merge.tool meld

# 配置别名
git config --global alias.st status
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.unstage 'reset HEAD --'
git config --global alias.last 'log -1 HEAD'
git config --global alias.visual '!gitk'

# 配置颜色
git config --global color.ui auto

# 配置换行符
git config --global core.autocrlf input
```

#### 工作流设置
```bash
# 克隆仓库
git clone https://github.com/vest-os/vest-os.git
cd vest-os

# 创建开发分支
git checkout -b feature/my-feature

# 查看状态
git status

# 查看改动
git diff
git diff --cached

# 查看历史
git log --oneline --graph
git log -p -1  # 查看最近一次提交的详情
```

## 代码结构

### 项目布局
```
vest-os/
├── arch/                    # 架构相关代码
│   ├── x86/                # x86架构
│   │   ├── kernel/         # 内核架构相关
│   │   └── boot/           # 启动代码
│   ├── arm/                # ARM架构
│   └── riscv/              # RISC-V架构
├── kernel/                 # 内核核心代码
│   ├── init/               # 初始化
│   ├── sched/              # 调度器
│   ├── mm/                 # 内存管理
│   ├── fs/                 # 文件系统
│   ├── ipc/                # 进程间通信
│   ├── net/                # 网络协议栈
│   └── drivers/            # 设备驱动
├── userspace/              # 用户空间程序
│   ├── init/               # init系统
│   ├── coreutils/          # 核心工具
│   ├── shell/              # Shell
│   └── apps/               # 应用程序
├── tools/                  # 开发工具
│   ├── build/              # 构建工具
│   ├── debug/              # 调试工具
│   └── testing/            # 测试工具
├── docs/                   # 文档
│   ├── api/                # API文档
│   ├── design/             # 设计文档
│   └── guides/             # 指南
├── scripts/                # 脚本
├── tests/                  # 测试代码
├── include/                # 头文件
│   ├── kernel/             # 内核头文件
│   ├── lib/                # 库头文件
│   └── uapi/               # 用户空间API头文件
├── Makefile                # 主Makefile
├── Kconfig                 # 配置菜单
└── README.md               # 项目说明
```

### 核心子系统

#### 1. 进程管理 (kernel/sched/)
- `sched.c`: 主调度器
- `fair.c`: CFS调度器
- `rt.c`: 实时调度器
- `idle.c`: 空闲任务

#### 2. 内存管理 (kernel/mm/)
- `page_alloc.c`: 页面分配器
- `slab.c`: SLAB分配器
- `vmalloc.c`: 虚拟内存分配
- `mmap.c`: 内存映射

#### 3. 文件系统 (kernel/fs/)
- `namei.c`: 路径名解析
- `inode.c`: inode管理
- `file_table.c`: 文件表
- `super.c`: 超级块管理

#### 4. 网络协议栈 (kernel/net/)
- `socket.c`: Socket接口
- `ipv4/`: IPv4协议
- `ipv6/`: IPv6协议
- `core/`: 网络核心

## 开发流程

### 1. 功能开发流程

#### 分析需求
1. 阅读设计文档
2. 理解现有代码
3. 制定实现方案
4. 评估工作量

#### 编码流程
```bash
# 1. 创建功能分支
git checkout -b feature/new-feature

# 2. 编写代码
vim kernel/sched/new_feature.c

# 3. 编写测试
vim tests/sched/new_feature_test.c

# 4. 编译验证
make -j$(nproc)

# 5. 运行测试
make test

# 6. 代码审查
make check

# 7. 提交代码
git add .
git commit -m "sched: add new feature"

# 8. 推送分支
git push origin feature/new-feature

# 9. 创建PR
# 使用GitHub UI创建Pull Request
```

### 2. 代码审查流程

#### 提交前检查
```bash
# 运行静态分析工具
make checkpatch
make sparse
make cppcheck

# 运行格式化工具
make clang-format
make checkstyle

# 运行测试
make test
make test-all
```

#### 审查清单
- [ ] 代码符合编码规范
- [ ] 函数和变量命名合理
- [ ] 注释充分且准确
- [ ] 错误处理完善
- [ ] 无内存泄漏
- [ ] 无竞态条件
- [ ] 测试覆盖充分
- [ ] 文档更新

### 3. 发布流程

#### 版本管理
```bash
# 创建发布分支
git checkout -b release/v1.1.0

# 更新版本号
echo "1.1.0" > VERSION
git commit -m "Release v1.1.0"

# 标记版本
git tag -a v1.1.0 -m "Release version 1.1.0"

# 合并到主分支
git checkout main
git merge release/v1.1.0

# 推送标签
git push origin v1.1.0
```

#### 发布准备
```bash
# 创建发布包
make distclean
make -j$(nproc)
make test-all
make package

# 生成变更日志
git log --pretty=format:"* %s (%h)" v1.0.0..v1.1.0 > CHANGELOG.md

# 更新文档
vim docs/release-notes/v1.1.0.md
```

## 编码规范

### C语言编码规范

#### 1. 命名规范

**函数命名：**
```c
// 小写字母，下划线分隔
int create_process(void);
void update_memory_stats(void);
bool is_valid_inode(struct inode *inode);
```

**变量命名：**
```c
// 局部变量：小写，下划线分隔
int task_count;
struct task_struct *current_task;

// 全局变量：g_前缀
static int g_system_state;
extern struct list_head g_process_list;

// 常量：大写，下划线分隔
#define MAX_PID 32768
#define PAGE_SIZE 4096
```

**结构体命名：**
```c
struct task_struct {
    pid_t pid;
    char comm[TASK_COMM_LEN];
    struct list_head list;
};

// 类型定义使用小写
typedef struct task_struct task_t;
```

#### 2. 格式规范

**缩进：** 4个空格，不使用Tab

```c
if (condition) {
    do_something();
    do_another();
} else {
    do_other();
}
```

**大括号：** K&R风格

```c
// 单语句可选
if (condition)
    do_something();

// 多语句必须
if (condition) {
    do_something();
    do_another();
}
```

**长行处理：** 80字符限制

```c
int very_long_function_name(int parameter_one, int parameter_two,
                           int parameter_three, int parameter_four) {
    // 函数体
}
```

#### 3. 注释规范

**文件头注释：**
```c
/*
 * Vest-OS Process Scheduler
 *
 * Copyright (C) 2024 Vest-OS Project
 *
 * This file implements the Completely Fair Scheduler (CFS) algorithm.
 *
 * Author: Developer Name <email@example.com>
 */
```

**函数注释：**
```c
/**
 * create_process - Create a new process
 * @parent: Parent process
 * @name: Process name
 *
 * Return: Pointer to new process on success, NULL on failure
 *
 * This function allocates and initializes a new process structure.
 * The new process will be added to the scheduler's run queue.
 */
struct task_struct *create_process(struct task_struct *parent,
                                  const char *name);
```

**行内注释：**
```c
// Calculate the virtual runtime
vruntime = delta_exec * (NICE_0_LOAD / load.weight);

/* Update the min_vruntime to ensure fairness */
if (vruntime < cfs_rq->min_vruntime)
    cfs_rq->min_vruntime = vruntime;
```

### Makefile规范

```makefile
# 注释使用#
# 目标：依赖
#	命令

# 变量定义
CC = gcc
CFLAGS = -Wall -Werror -O2
LDFLAGS = -lm

# 默认目标
all: program

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 链接规则
program: main.o util.o
	$(CC) $(LDFLAGS) $^ -o $@

# 清理
clean:
	rm -f *.o program

.PHONY: all clean
```

## 内核开发

### 1. 内核模块开发

#### 基础模块模板
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name <your.email@example.com>");
MODULE_DESCRIPTION("A simple kernel module");
MODULE_VERSION("1.0");

static int __init my_module_init(void) {
    printk(KERN_INFO "My module loaded\n");
    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "My module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
```

#### Makefile
```makefile
obj-m += my_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 2. 字符设备驱动

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "my_char_dev"
#define CLASS_NAME  "my_class"

static int major_number;
static char message[256] = {0};
static short size_of_message;
static struct class* char_class = NULL;
static struct device* char_device = NULL;

// 设备文件操作
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init char_driver_init(void) {
    // 动态分配主设备号
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return major_number;
    }

    // 创建设备类
    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(char_class);
    }

    // 创建设备文件
    char_device = device_create(char_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_device)) {
        class_destroy(char_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(char_device);
    }

    printk(KERN_INFO "Device registered successfully\n");
    return 0;
}

static void __exit char_driver_exit(void) {
    device_destroy(char_class, MKDEV(major_number, 0));
    class_unregister(char_class);
    class_destroy(char_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Device unregistered\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer,
                        size_t len, loff_t *offset) {
    int error_count = 0;

    // 拷贝数据到用户空间
    error_count = copy_to_user(buffer, message, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "Sent %d characters to user\n", size_of_message);
        return (size_of_message = 0);
    } else {
        printk(KERN_INFO "Failed to send %d characters\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char __user *buffer,
                         size_t len, loff_t *offset) {
    // 从用户空间拷贝数据
    if (copy_from_user(message, buffer, len) != 0) {
        return -EFAULT;
    }

    size_of_message = strlen(message);
    printk(KERN_INFO "Received %zu characters from user\n", len);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

module_init(char_driver_init);
module_exit(char_driver_exit);
```

### 3. 网络设备驱动

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

struct my_net_priv {
    struct net_device_stats stats;
    struct napi_struct napi;
};

static int my_net_open(struct net_device *dev) {
    netif_start_queue(dev);
    return 0;
}

static int my_net_stop(struct net_device *dev) {
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t my_net_start_xmit(struct sk_buff *skb,
                                    struct net_device *dev) {
    struct my_net_priv *priv = netdev_priv(dev);

    // 更新统计
    priv->stats.tx_packets++;
    priv->stats.tx_bytes += skb->len;

    // 释放skb
    dev_kfree_skb(skb);

    return NETDEV_TX_OK;
}

static const struct net_device_ops my_net_ops = {
    .ndo_open = my_net_open,
    .ndo_stop = my_net_stop,
    .ndo_start_xmit = my_net_start_xmit,
};

static void my_net_setup(struct net_device *dev) {
    // 设置设备属性
    dev->netdev_ops = &my_net_ops;
    dev->type = ARPHRD_ETHER;
    dev->hard_header_len = ETH_HLEN;
    dev->mtu = 1500;
    dev->tx_queue_len = 1000;
    dev->flags = IFF_BROADCAST | IFF_MULTICAST;

    // 设置MAC地址
    eth_random_addr(dev->dev_addr);
}

static int __init my_net_init(void) {
    struct net_device *dev;
    int ret;

    // 分配网络设备
    dev = alloc_etherdev(sizeof(struct my_net_priv));
    if (!dev)
        return -ENOMEM;

    // 设置设备
    my_net_setup(dev);

    // 注册设备
    ret = register_netdev(dev);
    if (ret) {
        free_netdev(dev);
        return ret;
    }

    printk(KERN_INFO "Network device registered\n");
    return 0;
}

static void __exit my_net_exit(void) {
    unregister_netdev(dev);
    free_netdev(dev);
    printk(KERN_INFO "Network device unregistered\n");
}

module_init(my_net_init);
module_exit(my_net_exit);
```

## 调试技巧

### 1. 内核调试

#### printk调试
```c
// 日志级别
printk(KERN_EMERG   "Emergency message\n");
printk(KERN_ALERT   "Alert message\n");
printk(KERN_CRIT    "Critical message\n");
printk(KERN_ERR     "Error message\n");
printk(KERN_WARNING "Warning message\n");
printk(KERN_NOTICE  "Notice message\n");
printk(KERN_INFO    "Info message\n");
printk(KERN_DEBUG   "Debug message\n");

// 动态调试
pr_debug("Debug info: %d\n", value);
dev_dbg(dev, "Device debug: %s\n", msg);
```

#### 使用ftrace
```bash
# 查看可用函数
cat /sys/kernel/debug/tracing/available_filter_functions

# 启用函数跟踪
echo function > /sys/kernel/debug/tracing/current_tracer
echo schedule > /sys/kernel/debug/tracing/set_ftrace_filter
echo 1 > /sys/kernel/debug/tracing/tracing_on

# 查看输出
cat /sys/kernel/debug/tracing/trace
```

#### 使用perf
```bash
# 性能分析
perf record -e cycles -a
perf report

# 分析特定函数
perf record -g -e sched:sched_switch
perf report

# 热点分析
perf top
```

### 2. 用户空间调试

#### GDB调试
```bash
# 启动GDB
gdb ./program

# 断点
break main
break function_name
break file.c:100

# 运行
run
run arg1 arg2

# 查看变量
print variable
print *pointer

# 单步执行
next
step
stepi

# 查看栈
backtrace
frame 2
```

#### Valgrind内存检查
```bash
# 内存泄漏检测
valgrind --leak-check=full --show-leak-kinds=all ./program

# 缓冲区溢出检测
valgrind --tool=memcheck ./program

# 性能分析
valgrind --tool=callgrind ./program
kcachegrind callgrind.out.*
```

### 3. 调试工具脚本

#### 内核模块调试脚本 (debug-module.sh)
```bash
#!/bin/bash

MODULE_NAME=$1

if [ -z "$MODULE_NAME" ]; then
    echo "Usage: $0 <module_name>"
    exit 1
fi

# 加载模块
insmod $MODULE_NAME.ko

# 查看输出
dmesg | tail

# 查看模块信息
modinfo $MODULE_NAME.ko

# 卸载模块
rmmod $MODULE_NAME
```

#### 性能分析脚本 (perf-analyze.sh)
```bash
#!/bin/bash

PROGRAM=$1
DURATION=30

if [ -z "$PROGRAM" ]; then
    echo "Usage: $0 <program>"
    exit 1
fi

# 开始记录
perf record -F 99 -g -p $(pidof $PROGRAM) -- sleep $DURATION

# 生成报告
perf report > perf-report.txt
perf annotate > perf-annotate.txt

# 生成火焰图
perf script | stackcollapse-perf.pl | flamegraph.pl > perf-flamegraph.svg

echo "Analysis complete. Check perf-report.txt and perf-flamegraph.svg"
```

## 测试框架

### 1. 内核单元测试

#### KUnit测试示例
```c
#include <kunit/test.h>

/* 测试加法函数 */
static int add(int a, int b) {
    return a + b;
}

/* 测试用例 */
static void test_add(struct kunit *test) {
    KUNIT_EXPECT_EQ(test, add(1, 1), 2);
    KUNIT_EXPECT_EQ(test, add(-1, 1), 0);
    KUNIT_EXPECT_NE(test, add(1, 1), 3);
}

/* 测试套件 */
static struct kunit_case math_tests[] = {
    KUNIT_CASE(test_add),
    {}
};

static struct kunit_suite math_suite = {
    .name = "math",
    .test_cases = math_tests,
};

kunit_test_suite(math_suite);
```

### 2. 用户空间测试

#### 使用check框架
```c
#include <check.h>
#include "mylib.h"

START_TEST(test_add) {
    ck_assert_int_eq(add(2, 3), 5);
    ck_assert_int_eq(add(-1, 1), 0);
}
END_TEST

Suite *math_suite(void) {
    Suite *s = suite_create("Math");
    TCase *tc = tcase_create("Core");

    tcase_add_test(tc, test_add);
    suite_add_tcase(s, tc);

    return s;
}

int main(void) {
    Suite *s = math_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? 0 : 1;
}
```

### 3. 集成测试

#### 自动化测试脚本 (run-tests.sh)
```bash
#!/bin/bash

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 测试结果统计
TOTAL=0
PASSED=0
FAILED=0

# 运行测试函数
run_test() {
    local test_name=$1
    local test_cmd=$2

    echo -e "${YELLOW}Running $test_name...${NC}"

    if eval $test_cmd > /dev/null 2>&1; then
        echo -e "${GREEN}✓ $test_name passed${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ $test_name failed${NC}"
        ((FAILED++))
    fi
    ((TOTAL++))
}

# 单元测试
run_test "Kernel Unit Tests" "make kunit"
run_test "Library Tests" "make check-lib"
run_test "API Tests" "make check-api"

# 集成测试
run_test "System Integration" "make test-integration"
run_test "Performance Tests" "make test-perf"

# 压力测试
run_test "Stress Test" "make test-stress"

# 输出结果
echo
echo "================================"
echo "Test Summary:"
echo "Total: $TOTAL"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo "================================"

# 退出码
if [ $FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi
```

## 贡献指南

### 1. 提交规范

#### 提交消息格式
```
<type>(<scope>): <subject>

<body>

<footer>
```

**类型（type）：**
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 重构
- `test`: 测试相关
- `chore`: 构建过程或辅助工具的变动

**示例：**
```
sched: implement CFS bandwidth control

Add support for CFS bandwidth control to limit the CPU
bandwidth for each task group. This is useful for
container scenarios where we need to enforce CPU quotas.

Fixes #1234
Signed-off-by: Developer Name <email@example.com>
```

### 2. Pull Request流程

#### 创建PR清单
- [ ] 代码通过所有测试
- [ ] 代码符合项目规范
- [ ] 添加了必要的测试
- [ ] 更新了相关文档
- [ ] 提交信息清晰
- [ ] 没有合并冲突

#### PR模板
```markdown
## Description
Brief description of the changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed

## Checklist
- [ ] My code follows the project's coding style
- [ ] I have performed a self-review
- [ ] I have commented my code as necessary
- [ ] I have updated the documentation

## Additional Notes
Any additional information that reviewers should know.
```

### 3. 代码审查

#### 审查要点
1. **正确性**：代码是否正确实现了功能
2. **可读性**：代码是否易于理解
3. **性能**：是否有性能问题
4. **安全性**：是否存在安全漏洞
5. **可维护性**：代码是否易于维护

#### 审查流程
1. 自动检查通过（CI/CD）
2. 至少一个maintainer审查
3. 所有意见被处理
4. 测试通过
5. 合并到目标分支

---

*文档版本：1.0*
*最后更新：2024年1月*