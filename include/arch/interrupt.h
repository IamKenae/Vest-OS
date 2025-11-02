/**
 * @file interrupt.h
 * @brief 中断处理头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _ARCH_INTERRUPT_H
#define _ARCH_INTERRUPT_H

#include <stdint.h>

/* 中断向量 */
#define IRQ_BASE        0x20
#define IRQ_TIMER       (IRQ_BASE + 0)
#define IRQ_KEYBOARD    (IRQ_BASE + 1)
#define IRQ_CASCADE     (IRQ_BASE + 2)
#define IRQ_COM2        (IRQ_BASE + 3)
#define IRQ_COM1        (IRQ_BASE + 4)
#define IRQ_LPT2        (IRQ_BASE + 5)
#define IRQ_FLOPPY      (IRQ_BASE + 6)
#define IRQ_LPT1        (IRQ_BASE + 7)
#define IRQ_RTC         (IRQ_BASE + 8)
#define IRQ_MOUSE       (IRQ_BASE + 12)
#define IRQ_FPU         (IRQ_BASE + 13)
#define IRQ_ATA_PRIMARY (IRQ_BASE + 14)
#define IRQ_ATA_SECONDARY (IRQ_BASE + 15)

/* 中断处理函数类型 */
typedef void (*interrupt_handler_t)(void);

/* 函数声明 */

/**
 * @brief 初始化中断系统
 * @return 0成功，-1失败
 */
int interrupt_init(void);

/**
 * @brief 设置中断处理函数
 * @param vector 中断向量
 * @param handler 处理函数
 * @return 0成功，-1失败
 */
int interrupt_set_handler(uint8_t vector, interrupt_handler_t handler);

/**
 * @brief 移除中断处理函数
 * @param vector 中断向量
 */
void interrupt_remove_handler(uint8_t vector);

/**
 * @brief 启用中断
 * @param vector 中断向量
 */
void interrupt_enable(uint8_t vector);

/**
 * @brief 禁用中断
 * @param vector 中断向量
 */
void interrupt_disable(uint8_t vector);

/**
 * @brief 启用全局中断
 */
static inline void interrupt_enable_global(void) {
    __asm__ volatile("sti");
}

/**
 * @brief 禁用全局中断
 */
static inline void interrupt_disable_global(void) {
    __asm__ volatile("cli");
}

/**
 * @brief 保存中断状态并禁用中断
 * @return 中断状态
 */
static inline uint32_t interrupt_save_and_disable(void) {
    uint32_t flags;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(flags));
    return flags;
}

/**
 * @brief 恢复中断状态
 * @param flags 中断状态
 */
static inline void interrupt_restore(uint32_t flags) {
    __asm__ volatile("push %0; popf" : : "r"(flags));
}

/**
 * @brief 检查是否在中断上下文中
 * @return 1在中断上下文中，0不在
 */
int interrupt_in_context(void);

/**
 * @brief 中断返回
 */
static inline void interrupt_return(void) {
    __asm__ volatile("iret");
}

/**
 * @brief 暂停CPU等待中断
 */
static inline void interrupt_halt(void) {
    __asm__ volatile("hlt");
}

#endif /* _ARCH_INTERRUPT_H */