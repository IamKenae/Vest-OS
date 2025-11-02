#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

// 版本信息
#define VESTOS_VERSION_MAJOR  0
#define VESTOS_VERSION_MINOR  1
#define VESTOS_VERSION_PATCH  0

// 架构检测
#if defined(__x86_64__)
    #define ARCH_X86_64 1
    #define ARCH_BITS 64
    typedef uint64_t ptr_t;
    typedef uint64_t size_t;
    typedef int64_t ssize_t;
    #define PTR_FMT "0x%016lx"
#elif defined(__i386__)
    #define ARCH_I386 1
    #define ARCH_BITS 32
    typedef uint32_t ptr_t;
    typedef uint32_t size_t;
    typedef int32_t ssize_t;
    #define PTR_FMT "0x%08x"
#else
    #error "不支持的架构"
#endif

// 页面大小
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

// 内存布局
#if ARCH_BITS == 32
    #define KERNEL_BASE_ADDR    0xC0000000
    #define USER_SPACE_END      0xBFFFFFFF
    #define KERNEL_SPACE_START  0xC0000000
#else
    #define KERNEL_BASE_ADDR    0xFFFF800000000000
    #define USER_SPACE_END      0x00007FFFFFFFFFFF
    #define KERNEL_SPACE_START  0xFFFF800000000000
#endif

// 最大进程和线程数
#define MAX_PROCESSES    256
#define MAX_THREADS      1024
#define MAX_OPEN_FILES   256

// 系统调用号
#define SYS_READ         0
#define SYS_WRITE        1
#define SYS_OPEN         2
#define SYS_CLOSE        3
#define SYS_EXIT         4
#define SYS_FORK         5
#define SYS_EXEC         6
#define SYS_WAIT         7
#define SYS_MMAP         8
#define SYS_MUNMAP       9
#define SYS_IOCTL        10

// 错误码
#define ERROR_NONE       0
#define ERROR_INVALID    -1
#define ERROR_NOMEM      -2
#define ERROR_NOENT      -3
#define ERROR_BUSY       -4
#define ERROR_PERM       -5
#define ERROR_IO         -6

// 前向声明
struct process;
struct thread;
struct tty_device;

// 内核启动函数
void kernel_main(void);

// 初始化函数
void kernel_init(void);
void hal_init(void);
void memory_init(void);
void scheduler_init(void);
void interrupt_init(void);
void tty_init(void);

// 调试输出
void kernel_printk(const char *format, ...);
void kernel_panic(const char *message);

// 内存管理
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kmap_page(ptr_t physical);
void kunmap_page(void *virtual);

// 进程管理
struct process *create_process(void);
void destroy_process(struct process *proc);
struct thread *create_thread(struct process *proc);
void destroy_thread(struct thread *thread);
void schedule(void);

// 中断处理
void enable_interrupts(void);
void disable_interrupts(void);
bool are_interrupts_enabled(void);

// TTY相关
int tty_write(struct tty_device *tty, const char *data, size_t len);
int tty_read(struct tty_device *tty, char *data, size_t len);
struct tty_device *get_tty_device(int tty_id);

// 系统调用处理
void syscall_handler(uint32_t syscall_number, uint32_t arg1, uint32_t arg2,
                     uint32_t arg3, uint32_t arg4, uint32_t arg5);

// 工具函数
void memcpy(void *dest, const void *src, size_t n);
void memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);

#endif // KERNEL_H