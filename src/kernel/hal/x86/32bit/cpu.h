#ifndef CPU_H
#define CPU_H

#include <kernel.h>
#include <stdint.h>
#include <stdbool.h>

// GDT结构
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT结构
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// TSS结构
struct tss_struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

// CPU特性结构
struct cpu_features {
    bool has_fpu;
    bool has_mmx;
    bool has_sse;
    bool has_sse2;
    bool has_nx;
    bool has_pae;
    bool has_pse;
    bool has_pge;
    bool has_apic;
    bool has_mtrr;
    bool has_pat;
    bool has_cmov;
    bool has_clflush;
    bool has_acpi;
    bool has_mmx_ext;
    bool has_fxsr;
    bool has_ffxsr;
    bool has_xmm;
    bool has_xmm2;
    bool has_3dnow;
    bool has_3dnow_ext;
};

// 段选择子
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x18
#define USER_DS   0x20

// 外部函数声明
extern void gdt_flush(uint32_t gdt_ptr);
extern void tss_flush(uint16_t tss_selector);
extern void idt_flush(uint32_t idt_ptr);
extern void exception_handlers[32];
extern void irq_handlers[16];
extern void syscall_handler(void);

// CPU相关函数
void cpu_init(void);
void enable_interrupts(void);
void disable_interrupts(void);
bool are_interrupts_enabled(void);
void cpu_halt(void);
uint32_t get_cpu_id(void);
const struct cpu_features* get_cpu_features(void);

// 控制寄存器操作
uint32_t read_cr0(void);
void write_cr0(uint32_t cr0);
uint32_t read_cr2(void);
uint32_t read_cr3(void);
void write_cr3(uint32_t cr3);
uint32_t read_cr4(void);
void write_cr4(uint32_t cr4);

// 内存管理
void flush_tlb(void);
void flush_tlb_page(void* addr);
void invalidate_cache(void);

// 同步和屏障
void cpu_idle(void);
void memory_barrier(void);
void serialize(void);

// GDT操作
static inline void set_gdt_entry(int num, uint32_t base, uint32_t limit,
                                uint8_t access, uint8_t gran) {
    struct gdt_entry *entry = &gdt[num];

    entry->limit_low = (limit & 0xFFFF);
    entry->base_low = (base & 0xFFFF);
    entry->base_mid = ((base >> 16) & 0xFF);
    entry->access = access;
    entry->granularity = ((limit >> 16) & 0x0F);
    entry->granularity |= (gran & 0xF0);
    entry->base_high = ((base >> 24) & 0xFF);
}

// IDT操作
static inline void set_idt_entry(int num, uint32_t base, uint16_t sel, uint8_t flags) {
    struct idt_entry *entry = &idt[num];

    entry->base_low = (base & 0xFFFF);
    entry->selector = sel;
    entry->zero = 0;
    entry->type_attr = flags;
    entry->base_high = ((base >> 16) & 0xFFFF);
}

// CPUID支持检测
static inline bool check_cpuid_support(void) {
    uint32_t flags;
    asm volatile ("pushf; popl %0" : "=r"(flags));
    uint32_t flags2 = flags;
    flags2 ^= 0x200000;  // 翻转ID位
    asm volatile ("pushl %0; popf" : : "r"(flags2));
    asm volatile ("pushf; popl %0" : "=r"(flags2));
    return (flags2 != flags);
}

// CPUID指令
static inline void cpuid(uint32_t code, uint32_t *eax, uint32_t *ebx,
                        uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid"
                  : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                  : "a"(code));
}

// I/O端口操作
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    asm volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outl(uint16_t port, uint32_t value) {
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 读/写模型特定寄存器
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = (value >> 32) & 0xFFFFFFFF;
    asm volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

// 控制台I/O端口定义
#define VGA_BASE       0xB8000
#define VGA_WIDTH      80
#define VGA_HEIGHT     25
#define VGA_COLOR      0x0F

// PIC I/O端口
#define PIC1_COMMAND   0x20
#define PIC1_DATA      0x21
#define PIC2_COMMAND   0xA0
#define PIC2_DATA      0xA1

// PIC命令
#define PIC_EOI        0x20

// 键盘控制器端口
#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64

// 串口端口
#define COM1_BASE      0x3F8
#define COM2_BASE      0x2F8
#define COM3_BASE      0x3E8
#define COM4_BASE      0x2E8

// 串口寄存器偏移
#define COM_RBR        0  // 接收缓冲寄存器
#define COM_THR        0  // 发送保持寄存器
#define COM_DLL        0  // 除数锁存器低字节
#define COM_DLH        1  // 除数锁存器高字节
#define COM_IER        1  // 中断使能寄存器
#define COM_IIR        2  // 中断标识寄存器
#define COM_FCR        2  // FIFO控制寄存器
#define COM_LCR        3  // 线路控制寄存器
#define COM_MCR        4  // 调制解调器控制寄存器
#define COM_LSR        5  // 线路状态寄存器
#define COM_MSR        6  // 调制解调器状态寄存器
#define COM_SCR        7  // 暂存寄存器

#endif // CPU_H