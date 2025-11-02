/*
 * Vest-OS x86 32位CPU抽象层
 * 提供CPU初始化和基本操作
 */

#include <kernel.h>
#include <hal/cpu.h>

// CPU特性检测
static struct cpu_features cpu_features = {0};

// GDT和IDT
static struct gdt_entry gdt[5];
static struct idt_entry idt[256];
static struct gdt_ptr gdt_ptr;
static struct idt_ptr idt_ptr;

// TSS结构
static struct tss_struct tss;

/**
 * 初始化GDT
 */
static void init_gdt(void)
{
    // 设置GDT指针
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    // 清空GDT
    memset(&gdt, 0, sizeof(gdt));

    // 内核代码段 (0x08)
    set_gdt_entry(0, 0, 0, 0, 0);                           // 空描述符
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);           // 内核代码段
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);           // 内核数据段
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);           // 用户代码段
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);           // 用户数据段

    // 加载GDT
    gdt_flush((uint32_t)&gdt_ptr);

    // 设置TSS
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;  // 内核栈段选择子
    tss.esp0 = 0;    // 内核栈指针（在切换时设置）

    // 设置TSS描述符
    set_gdt_entry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

    // 加载TSS
    tss_flush(0x28);
}

/**
 * 初始化IDT
 */
static void init_idt(void)
{
    // 设置IDT指针
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    // 清空IDT
    memset(&idt, 0, sizeof(idt));

    // 设置异常处理程序
    for (int i = 0; i < 32; i++) {
        set_idt_entry(i, (uint32_t)exception_handlers[i], 0x08, 0x8E);
    }

    // 设置IRQ处理程序
    for (int i = 32; i < 48; i++) {
        set_idt_entry(i, (uint32_t)irq_handlers[i - 32], 0x08, 0x8E);
    }

    // 设置系统调用处理程序 (int 0x80)
    set_idt_entry(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);

    // 加载IDT
    idt_flush((uint32_t)&idt_ptr);
}

/**
 * 检测CPU特性
 */
static void detect_cpu_features(void)
{
    uint32_t eax, ebx, ecx, edx;

    // 检查是否支持CPUID
    if (!check_cpuid_support()) {
        return;
    }

    // 获取基本CPU信息
    cpuid(1, &eax, &ebx, &ecx, &edx);

    cpu_features.has_fpu  = (edx & (1 << 0)) != 0;
    cpu_features.has_mmx  = (edx & (1 << 23)) != 0;
    cpu_features.has_sse  = (edx & (1 << 25)) != 0;
    cpu_features.has_sse2 = (edx & (1 << 26)) != 0;

    // 获取扩展特性
    cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    cpu_features.has_nx = (edx & (1 << 20)) != 0;
}

/**
 * 初始化中断控制器 (8259A PIC)
 */
static void init_pic(void)
{
    // 重新映射PIC中断向量
    outb(0x20, 0x11);  // 开始初始化主PIC
    outb(0xA0, 0x11);  // 开始初始化从PIC
    outb(0x21, 0x20);  // 主PIC中断向量偏移
    outb(0xA1, 0x28);  // 从PIC中断向量偏移
    outb(0x21, 0x04);  // 告诉主PIC从PIC在IRQ2
    outb(0xA1, 0x02);  // 告诉从PIC级联标识
    outb(0x21, 0x01);  // 8086模式
    outb(0xA1, 0x01);  // 8086模式

    // 屏蔽所有中断
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

/**
 * 启用中断
 */
void enable_interrupts(void)
{
    asm volatile ("sti");
}

/**
 * 禁用中断
 */
void disable_interrupts(void)
{
    asm volatile ("cli");
}

/**
 * 检查中断是否启用
 */
bool are_interrupts_enabled(void)
{
    uint32_t flags;
    asm volatile ("pushf; popl %0" : "=r"(flags));
    return (flags & (1 << 9)) != 0;
}

/**
 * 暂停CPU
 */
void cpu_halt(void)
{
    asm volatile ("hlt");
}

/**
 * 获取当前CPU ID (多核支持预留)
 */
uint32_t get_cpu_id(void)
{
    // 单核系统返回0
    return 0;
}

/**
 * 获取CPU特性
 */
const struct cpu_features* get_cpu_features(void)
{
    return &cpu_features;
}

/**
 * 读取CR0寄存器
 */
uint32_t read_cr0(void)
{
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

/**
 * 写入CR0寄存器
 */
void write_cr0(uint32_t cr0)
{
    asm volatile ("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * 读取CR2寄存器 (页故障地址)
 */
uint32_t read_cr2(void)
{
    uint32_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

/**
 * 读取CR3寄存器 (页目录基址)
 */
uint32_t read_cr3(void)
{
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

/**
 * 写入CR3寄存器
 */
void write_cr3(uint32_t cr3)
{
    asm volatile ("mov %0, %%cr3" : : "r"(cr3));
}

/**
 * 读取CR4寄存器
 */
uint32_t read_cr4(void)
{
    uint32_t cr4;
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

/**
 * 写入CR4寄存器
 */
void write_cr4(uint32_t cr4)
{
    asm volatile ("mov %0, %%cr4" : : "r"(cr4));
}

/**
 * 刷新TLB
 */
void flush_tlb(void)
{
    asm volatile ("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "%eax");
}

/**
 * 刷新特定页面的TLB
 */
void flush_tlb_page(void* addr)
{
    asm volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}

/**
 * 无效化缓存
 */
void invalidate_cache(void)
{
    asm volatile ("wbinvd");
}

/**
 * CPU空闲等待
 */
void cpu_idle(void)
{
    asm volatile ("hlt");
}

/**
 * 内存屏障
 */
void memory_barrier(void)
{
    asm volatile ("mfence");
}

/**
 * 串行化指令
 */
void serialize(void)
{
    asm volatile ("cpuid" ::: "eax", "ebx", "ecx", "edx");
}

/**
 * 初始化CPU
 */
void cpu_init(void)
{
    // 检测CPU特性
    detect_cpu_features();

    // 初始化GDT
    init_gdt();

    // 初始化IDT
    init_idt();

    // 初始化PIC
    init_pic();

    // 启用必要的CPU特性
    uint32_t cr0 = read_cr0();
    cr0 |= (1 << 16);  // 设置写保护
    write_cr0(cr0);

    uint32_t cr4 = read_cr4();
    if (cpu_features.has_nx) {
        cr4 |= (1 << 14);  // 启用NX位
    }
    write_cr4(cr4);

    kernel_printk("CPU初始化完成\n");
    kernel_printk("CPU特性: FPU=%d MMX=%d SSE=%d SSE2=%d NX=%d\n",
                  cpu_features.has_fpu, cpu_features.has_mmx,
                  cpu_features.has_sse, cpu_features.has_sse2,
                  cpu_features.has_nx);
}