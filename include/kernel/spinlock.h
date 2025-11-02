/**
 * @file spinlock.h
 * @brief 自旋锁实现
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _KERNEL_SPINLOCK_H
#define _KERNEL_SPINLOCK_H

#include <stdint.h>

/* 自旋锁结构 */
typedef struct {
    volatile uint32_t lock;     /* 锁状态 */
    uint32_t owner;             /* 锁持有者 */
    const char *name;           /* 锁名称 */
} spinlock_t;

/* 定义并初始化自旋锁 */
#define SPINLOCK_INITIALIZER(name) { .lock = 0, .owner = 0, .name = #name }

#define DEFINE_SPINLOCK(name) \
    spinlock_t name = SPINLOCK_INITIALIZER(name)

/* 函数声明 */

/**
 * @brief 初始化自旋锁
 * @param lock 自旋锁指针
 * @param name 锁名称
 */
void spinlock_init(spinlock_t *lock, const char *name);

/**
 * @brief 获取自旋锁
 * @param lock 自旋锁指针
 */
void spinlock_lock(spinlock_t *lock);

/**
 * @brief 释放自旋锁
 * @param lock 自旋锁指针
 */
void spinlock_unlock(spinlock_t *lock);

/**
 * @brief 尝试获取自旋锁
 * @param lock 自旋锁指针
 * @return 1成功获取，0失败
 */
int spinlock_trylock(spinlock_t *lock);

/**
 * @brief 检查自旋锁状态
 * @param lock 自旋锁指针
 * @return 1已锁定，0未锁定
 */
int spinlock_is_locked(spinlock_t *lock);

/**
 * @brief 获取自旋锁并禁用中断
 * @param lock 自旋锁指针
 * @return 中断状态
 */
uint32_t spinlock_lock_irqsave(spinlock_t *lock);

/**
 * @brief 释放自旋锁并恢复中断
 * @param lock 自旋锁指针
 * @param flags 中断状态
 */
void spinlock_unlock_irqrestore(spinlock_t *lock, uint32_t flags);

/**
 * @brief 获取自旋锁并禁用中断（不保存状态）
 * @param lock 自旋锁指针
 */
void spinlock_lock_irq(spinlock_t *lock);

/**
 * @brief 释放自旋锁并启用中断
 * @param lock 自旋锁指针
 */
void spinlock_unlock_irq(spinlock_t *lock);

/**
 * @brief 自旋锁断言（调试用）
 * @param lock 自旋锁指针
 */
#ifdef DEBUG
#define spinlock_assert_held(lock) \
    do { \
        if (!spinlock_is_locked(lock)) { \
            panic("Spinlock %s not held!\n", (lock)->name); \
        } \
    } while(0)
#else
#define spinlock_assert_held(lock) do {} while(0)
#endif

#endif /* _KERNEL_SPINLOCK_H */