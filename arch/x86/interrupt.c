/**
 * @file interrupt.c
 * @brief 中断处理实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <arch/interrupt.h>
#include <kernel/string.h>

/* 中断处理函数表 */
static interrupt_handler_t interrupt_handlers[256];

/**
 * @brief 初始化中断系统
 */
int interrupt_init(void) {
    /* 清零中断处理函数表 */
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    /* 这里应该设置IDT和其他中断硬件 */
    /* 简化实现 */

    return 0;
}

/**
 * @brief 设置中断处理函数
 */
int interrupt_set_handler(uint8_t vector, interrupt_handler_t handler) {
    if (vector >= 256) {
        return -1;
    }

    interrupt_handlers[vector] = handler;
    return 0;
}

/**
 * @brief 移除中断处理函数
 */
void interrupt_remove_handler(uint8_t vector) {
    if (vector < 256) {
        interrupt_handlers[vector] = NULL;
    }
}

/**
 * @brief 启用中断
 */
void interrupt_enable(uint8_t vector) {
    /* 这里应该操作PIC来启用特定中断 */
    /* 简化实现 */
}

/**
 * @brief 禁用中断
 */
void interrupt_disable(uint8_t vector) {
    /* 这里应该操作PIC来禁用特定中断 */
    /* 简化实现 */
}

/**
 * @brief 检查是否在中断上下文中
 */
int interrupt_in_context(void) {
    /* 这里应该检查当前是否在中断处理中 */
    /* 简化实现 */
    return 0;
}