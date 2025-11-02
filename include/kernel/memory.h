/**
 * @file memory.h
 * @brief 内存管理函数
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

/* 内存分配标志 */
#define GFP_KERNEL      0x01    /* 内核内存 */
#define GFP_USER        0x02    /* 用户内存 */
#define GFP_ATOMIC      0x04    /* 原子分配 */
#define GFP_DMA         0x08    /* DMA内存 */

/* 函数声明 */

/**
 * @brief 分配内核内存
 * @param size 分配大小
 * @return 内存指针，NULL失败
 */
void *kmalloc(size_t size);

/**
 * @brief 分配内核内存（带标志）
 * @param size 分配大小
 * @param flags 分配标志
 * @return 内存指针，NULL失败
 */
void *kmalloc_flags(size_t size, int flags);

/**
 * @brief 释放内核内存
 * @param ptr 内存指针
 */
void kfree(void *ptr);

/**
 * @brief 重新分配内核内存
 * @param ptr 原内存指针
 * @param size 新大小
 * @return 新内存指针，NULL失败
 */
void *krealloc(void *ptr, size_t size);

/**
 * @brief 分配页对齐内存
 * @param order 页面数量(2^order)
 * @return 内存指针，NULL失败
 */
void *get_free_pages(unsigned int order);

/**
 * @brief 释放页对齐内存
 * @param addr 内存地址
 * @param order 页面数量(2^order)
 */
void free_pages(void *addr, unsigned int order);

/**
 * @brief 分配单个页面
 * @return 页面指针，NULL失败
 */
static inline void *get_free_page(void) {
    return get_free_pages(0);
}

/**
 * @brief 释放单个页面
 * @param addr 页面地址
 */
static inline void free_page(void *addr) {
    free_pages(addr, 0);
}

/**
 * @brief 内存拷贝（用户到内核）
 * @param to 目标地址
 * @param from 源地址
 * @param n 拷贝字节数
 * @return 0成功，-1失败
 */
int copy_from_user(void *to, const void *from, size_t n);

/**
 * @brief 内存拷贝（内核到用户）
 * @param to 目标地址
 * @param from 源地址
 * @param n 拷贝字节数
 * @return 0成功，-1失败
 */
int copy_to_user(void *to, const void *from, size_t n);

/**
 * @brief 验证用户内存指针
 * @param ptr 用户指针
 * @param size 内存大小
 * @return 1有效，0无效
 */
int access_ok(const void *ptr, size_t size);

/**
 * @brief 内存清零
 * @param s 内存地址
 * @param n 清零字节数
 */
void *kzalloc(size_t size);

/**
 * @brief 初始化内存管理
 * @return 0成功，-1失败
 */
int memory_init(void);

#endif /* _KERNEL_MEMORY_H */