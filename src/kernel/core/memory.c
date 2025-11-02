/*
 * Vest-OS 内存管理
 * 提供物理内存分配、虚拟内存管理和堆分配器
 */

#include <kernel.h>
#include <hal/memory.h>
#include <hal/cpu.h>

// 内存管理器结构
struct memory_manager {
    // 物理内存管理
    struct page_frame *page_frames;
    uint32_t total_pages;
    uint32_t free_pages;
    uint32_t used_pages;
    spinlock_t frame_lock;

    // 虚拟内存管理
    struct page_directory *kernel_page_dir;
    struct page_directory *current_page_dir;
    spinlock_t page_lock;

    // 堆管理器
    struct heap kernel_heap;
    spinlock_t heap_lock;
};

// 全局内存管理器
static struct memory_manager memory_manager;

// 内核堆开始和结束地址
extern uint32_t __heap_start;
extern uint32_t __heap_end;

// 页目录和页表
static struct page_directory boot_page_dir __attribute__((aligned(PAGE_SIZE)));
static struct page_table boot_page_tables[256] __attribute__((aligned(PAGE_SIZE)));

/**
 * 初始化物理内存管理器
 */
static void init_physical_memory(struct multiboot_info *mb_info)
{
    // 从Multiboot信息获取内存大小
    uint32_t mem_size = mb_info->mem_lower + mb_info->mem_upper; // KB
    uint32_t mem_bytes = mem_size * 1024;

    memory_manager.total_pages = mem_bytes / PAGE_SIZE;
    memory_manager.free_pages = memory_manager.total_pages;
    memory_manager.used_pages = 0;

    // 分配页帧数组
    size_t frame_array_size = memory_manager.total_pages * sizeof(struct page_frame);
    memory_manager.page_frames = (struct page_frame *)kmalloc(frame_array_size);

    // 初始化页帧
    memset(memory_manager.page_frames, 0, frame_array_size);

    // 标记已使用的页帧（内核代码和数据）
    uint32_t kernel_end = (uint32_t)&__heap_end;
    uint32_t used_pages = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < used_pages; i++) {
        memory_manager.page_frames[i].used = 1;
        memory_manager.page_frames[i].kernel = 1;
        memory_manager.free_pages--;
        memory_manager.used_pages++;
    }

    kernel_printk("物理内存初始化完成: %d MB, 总页数: %d\n",
                  mem_size / 1024, memory_manager.total_pages);
}

/**
 * 分配一个物理页帧
 */
uint32_t alloc_page_frame(void)
{
    spinlock_lock(&memory_manager.frame_lock);

    for (uint32_t i = 0; i < memory_manager.total_pages; i++) {
        if (!memory_manager.page_frames[i].used) {
            memory_manager.page_frames[i].used = 1;
            memory_manager.free_pages--;
            memory_manager.used_pages++;
            spinlock_unlock(&memory_manager.frame_lock);
            return i * PAGE_SIZE;
        }
    }

    spinlock_unlock(&memory_manager.frame_lock);
    kernel_panic("内存耗尽");
    return 0;
}

/**
 * 释放一个物理页帧
 */
void free_page_frame(uint32_t physical_addr)
{
    uint32_t frame_index = physical_addr / PAGE_SIZE;
    if (frame_index >= memory_manager.total_pages) {
        return;
    }

    spinlock_lock(&memory_manager.frame_lock);

    if (memory_manager.page_frames[frame_index].used) {
        memory_manager.page_frames[frame_index].used = 0;
        memory_manager.page_frames[frame_index].kernel = 0;
        memory_manager.free_pages++;
        memory_manager.used_pages--;
    }

    spinlock_unlock(&memory_manager.frame_lock);
}

/**
 * 创建新的页目录
 */
struct page_directory* create_page_directory(void)
{
    struct page_directory *page_dir = (struct page_directory*)kmalloc_aligned(sizeof(struct page_directory), PAGE_SIZE);

    if (!page_dir) {
        return NULL;
    }

    // 清空页目录
    memset(page_dir, 0, sizeof(struct page_directory));

    // 映射内核空间到新页目录
    for (int i = 768; i < 1024; i++) {
        page_dir->entries[i] = boot_page_dir.entries[i];
    }

    return page_dir;
}

/**
 * 删除页目录
 */
void destroy_page_directory(struct page_directory *page_dir)
{
    if (!page_dir || page_dir == &boot_page_dir) {
        return;
    }

    // 释放用户空间的页表
    for (int i = 0; i < 768; i++) {
        if (page_dir->entries[i] & 0x1) {  // Present位
            uint32_t page_table_addr = page_dir->entries[i] & 0xFFFFF000;
            struct page_table *page_table = (struct page_table*)page_table_addr;

            // 释放页表中的页面
            for (int j = 0; j < 1024; j++) {
                if (page_table->entries[j] & 0x1) {
                    uint32_t physical_addr = page_table->entries[j] & 0xFFFFF000;
                    free_page_frame(physical_addr);
                }
            }

            // 释放页表本身
            free_page_frame(page_table_addr);
        }
    }

    kfree(page_dir);
}

/**
 * 映射虚拟页面到物理页面
 */
int map_page(struct page_directory *page_dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags)
{
    // 获取页目录索引和页表索引
    uint32_t page_dir_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3FF;

    spinlock_lock(&memory_manager.page_lock);

    // 检查页表是否存在
    if (!(page_dir->entries[page_dir_index] & 0x1)) {
        // 分配新的页表
        uint32_t page_table_addr = alloc_page_frame();
        struct page_table *page_table = (struct page_table*)page_table_addr;

        // 清空页表
        memset(page_table, 0, PAGE_SIZE);

        // 设置页目录项
        page_dir->entries[page_dir_index] = page_table_addr | flags | 0x1;  // Present
    }

    // 获取页表
    uint32_t page_table_addr = page_dir->entries[page_dir_index] & 0xFFFFF000;
    struct page_table *page_table = (struct page_table*)page_table_addr;

    // 设置页表项
    page_table->entries[page_table_index] = physical_addr | flags | 0x1;  // Present

    // 刷新TLB
    flush_tlb_page((void*)virtual_addr);

    spinlock_unlock(&memory_manager.page_lock);
    return 0;
}

/**
 * 取消映射虚拟页面
 */
int unmap_page(struct page_directory *page_dir, uint32_t virtual_addr)
{
    uint32_t page_dir_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3FF;

    spinlock_lock(&memory_manager.page_lock);

    // 检查页表是否存在
    if (!(page_dir->entries[page_dir_index] & 0x1)) {
        spinlock_unlock(&memory_manager.page_lock);
        return -1;  // 页面未映射
    }

    // 获取页表
    uint32_t page_table_addr = page_dir->entries[page_dir_index] & 0xFFFFF000;
    struct page_table *page_table = (struct page_table*)page_table_addr;

    // 清除页表项
    if (page_table->entries[page_table_index] & 0x1) {
        uint32_t physical_addr = page_table->entries[page_table_index] & 0xFFFFF000;
        page_table->entries[page_table_index] = 0;

        // 释放物理页帧
        free_page_frame(physical_addr);

        // 刷新TLB
        flush_tlb_page((void*)virtual_addr);
    }

    spinlock_unlock(&memory_manager.page_lock);
    return 0;
}

/**
 * 获取虚拟地址对应的物理地址
 */
uint32_t get_physical_address(struct page_directory *page_dir, uint32_t virtual_addr)
{
    uint32_t page_dir_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t page_offset = virtual_addr & 0xFFF;

    // 检查页表是否存在
    if (!(page_dir->entries[page_dir_index] & 0x1)) {
        return 0;  // 页面未映射
    }

    // 获取页表
    uint32_t page_table_addr = page_dir->entries[page_dir_index] & 0xFFFFF000;
    struct page_table *page_table = (struct page_table*)page_table_addr;

    // 检查页面是否存在
    if (!(page_table->entries[page_table_index] & 0x1)) {
        return 0;  // 页面未映射
    }

    uint32_t physical_addr = (page_table->entries[page_table_index] & 0xFFFFF000) + page_offset;
    return physical_addr;
}

/**
 * 切换页目录
 */
void switch_page_directory(struct page_directory *page_dir)
{
    memory_manager.current_page_dir = page_dir;
    write_cr3((uint32_t)page_dir);
}

/**
 * 内核堆分配器
 */
void* kmalloc(size_t size)
{
    void *ptr = heap_alloc(&memory_manager.kernel_heap, size);
    if (!ptr) {
        kernel_panic("内核堆内存耗尽");
    }
    return ptr;
}

/**
 * 内核堆释放器
 */
void kfree(void *ptr)
{
    heap_free(&memory_manager.kernel_heap, ptr);
}

/**
 * 对齐的内核分配器
 */
void* kmalloc_aligned(size_t size, size_t alignment)
{
    return heap_alloc_aligned(&memory_manager.kernel_heap, size, alignment);
}

/**
 * 页故障处理程序
 */
void page_fault_handler(uint32_t error_code, uint32_t fault_addr)
{
    uint32_t physical_addr = get_physical_address(memory_manager.current_page_dir, fault_addr);

    kernel_printk("页故障:\n");
    kernel_printk("  故障地址: 0x%08x\n", fault_addr);
    kernel_printk("  物理地址: 0x%08x\n", physical_addr);
    kernel_printk("  错误代码: 0x%02x\n", error_code);

    if (error_code & 0x1) {
        kernel_printk("    页面保护违规\n");
    } else {
        kernel_printk("    页面不存在\n");
    }

    if (error_code & 0x2) {
        kernel_printk("    写操作\n");
    } else {
        kernel_printk("    读操作\n");
    }

    if (error_code & 0x4) {
        kernel_printk("    用户模式\n");
    } else {
        kernel_printk("    内核模式\n");
    }

    kernel_panic("未处理的页故障");
}

/**
 * 初始化内存管理
 */
void memory_init(struct multiboot_info *mb_info)
{
    kernel_printk("初始化内存管理...\n");

    // 初始化锁
    spinlock_init(&memory_manager.frame_lock);
    spinlock_init(&memory_manager.page_lock);
    spinlock_init(&memory_manager.heap_lock);

    // 初始化物理内存管理
    init_physical_memory(mb_info);

    // 设置引导页目录
    memory_manager.kernel_page_dir = &boot_page_dir;
    memory_manager.current_page_dir = &boot_page_dir;

    // 初始化内核堆
    uint32_t heap_start = (uint32_t)&__heap_end;
    uint32_t heap_size = 1024 * 1024;  // 1MB初始堆大小
    heap_init(&memory_manager.kernel_heap, (void*)heap_start, heap_size);

    // 设置页故障处理程序
    set_interrupt_handler(14, page_fault_handler);

    kernel_printk("内存管理初始化完成\n");
    kernel_printk("  可用页数: %d\n", memory_manager.free_pages);
    kernel_printk("  已用页数: %d\n", memory_manager.used_pages);
    kernel_printk("  内核堆: 0x%08x - 0x%08x\n", heap_start, heap_start + heap_size);
}