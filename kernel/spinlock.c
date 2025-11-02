/**
 * @file spinlock.c
 * @brief 自旋锁实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <kernel/spinlock.h>
#include <arch/interrupt.h>

/**
 * @brief 初始化自旋锁
 */
void spinlock_init(spinlock_t *lock, const char *name) {
    lock->lock = 0;
    lock->owner = 0;
    lock->name = name ? name : "unknown";
}

/**
 * @brief 获取自旋锁
 */
void spinlock_lock(spinlock_t *lock) {
    /* 简单的test-and-set实现 */
    while (__sync_lock_test_and_set(&lock->lock, 1)) {
        /* 自旋等待 */
        __asm__ volatile("pause");
    }
    lock->owner = 0;  /* 这里可以记录当前CPU */
}

/**
 * @brief 释放自旋锁
 */
void spinlock_unlock(spinlock_t *lock) {
    lock->owner = 0;
    __sync_lock_release(&lock->lock);
}

/**
 * @brief 尝试获取自旋锁
 */
int spinlock_trylock(spinlock_t *lock) {
    if (__sync_lock_test_and_set(&lock->lock, 1)) {
        return 0;  /* 获取失败 */
    }
    lock->owner = 0;
    return 1;  /* 获取成功 */
}

/**
 * @brief 检查自旋锁状态
 */
int spinlock_is_locked(spinlock_t *lock) {
    return lock->lock != 0;
}

/**
 * @brief 获取自旋锁并禁用中断
 */
uint32_t spinlock_lock_irqsave(spinlock_t *lock) {
    uint32_t flags = interrupt_save_and_disable();
    spinlock_lock(lock);
    return flags;
}

/**
 * @brief 释放自旋锁并恢复中断
 */
void spinlock_unlock_irqrestore(spinlock_t *lock, uint32_t flags) {
    spinlock_unlock(lock);
    interrupt_restore(flags);
}

/**
 * @brief 获取自旋锁并禁用中断（不保存状态）
 */
void spinlock_lock_irq(spinlock_t *lock) {
    interrupt_disable_global();
    spinlock_lock(lock);
}

/**
 * @brief 释放自旋锁并启用中断
 */
void spinlock_unlock_irq(spinlock_t *lock) {
    spinlock_unlock(lock);
    interrupt_enable_global();
}