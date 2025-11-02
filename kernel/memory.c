/**
 * @file memory.c
 * @brief 内存管理函数实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <kernel/memory.h>
#include <kernel/string.h>
#include <arch/interrupt.h>

/* 简单的内存分配器实现 */
#define HEAP_SIZE  (1024 * 1024)  /* 1MB堆 */
#define BLOCK_SIZE 32

typedef struct memory_block {
    size_t size;
    int free;
    struct memory_block *next;
} memory_block_t;

static char heap[HEAP_SIZE];
static memory_block_t *heap_head = NULL;
static spinlock_t memory_lock;

/**
 * @brief 初始化内存管理
 */
int memory_init(void) {
    spinlock_init(&memory_lock, "memory");

    heap_head = (memory_block_t*)heap;
    heap_head->size = HEAP_SIZE - sizeof(memory_block_t);
    heap_head->free = 1;
    heap_head->next = NULL;

    return 0;
}

/**
 * @brief 查找合适的内存块
 */
static memory_block_t *find_block(size_t size) {
    memory_block_t *block = heap_head;
    while (block) {
        if (block->free && block->size >= size) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

/**
 * @brief 分割内存块
 */
static void split_block(memory_block_t *block, size_t size) {
    if (block->size - size > sizeof(memory_block_t) + BLOCK_SIZE) {
        memory_block_t *new_block = (memory_block_t*)((char*)block + sizeof(memory_block_t) + size);
        new_block->size = block->size - size - sizeof(memory_block_t);
        new_block->free = 1;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

/**
 * @brief 合并空闲块
 */
static void merge_blocks(void) {
    memory_block_t *block = heap_head;
    while (block && block->next) {
        if (block->free && block->next->free) {
            block->size += sizeof(memory_block_t) + block->next->size;
            block->next = block->next->next;
        } else {
            block = block->next;
        }
    }
}

/**
 * @brief 分配内核内存
 */
void *kmalloc(size_t size) {
    return kmalloc_flags(size, GFP_KERNEL);
}

/**
 * @brief 分配内核内存（带标志）
 */
void *kmalloc_flags(size_t size, int flags) {
    if (size == 0) {
        return NULL;
    }

    /* 对齐大小 */
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

    uint32_t irq_state = spinlock_lock_irqsave(&memory_lock);

    memory_block_t *block = find_block(size);
    if (block) {
        split_block(block, size);
        block->free = 0;
        spinlock_unlock_irqrestore(&memory_lock, irq_state);
        return (char*)block + sizeof(memory_block_t);
    }

    spinlock_unlock_irqrestore(&memory_lock, irq_state);
    return NULL;
}

/**
 * @brief 释放内核内存
 */
void kfree(void *ptr) {
    if (!ptr) {
        return;
    }

    memory_block_t *block = (memory_block_t*)((char*)ptr - sizeof(memory_block_t));

    uint32_t irq_state = spinlock_lock_irqsave(&memory_lock);

    block->free = 1;
    merge_blocks();

    spinlock_unlock_irqrestore(&memory_lock, irq_state);
}

/**
 * @brief 重新分配内核内存
 */
void *krealloc(void *ptr, size_t size) {
    if (!ptr) {
        return kmalloc(size);
    }

    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    memory_block_t *block = (memory_block_t*)((char*)ptr - sizeof(memory_block_t));
    if (block->size >= size) {
        return ptr;
    }

    void *new_ptr = kmalloc(size);
    if (new_ptr) {
        size_t copy_size = block->size < size ? block->size : size;
        memcpy(new_ptr, ptr, copy_size);
        kfree(ptr);
    }

    return new_ptr;
}

/**
 * @brief 分配页对齐内存
 */
void *get_free_pages(unsigned int order) {
    size_t size = (1 << order) * 4096;  /* 页面大小通常为4KB */
    return kmalloc(size);
}

/**
 * @brief 释放页对齐内存
 */
void free_pages(void *addr, unsigned int order) {
    (void)order;  /* 未使用的参数 */
    kfree(addr);
}

/**
 * @brief 内存拷贝（用户到内核）
 */
int copy_from_user(void *to, const void *from, size_t n) {
    if (!access_ok(from, n)) {
        return -1;
    }

    memcpy(to, from, n);
    return 0;
}

/**
 * @brief 内存拷贝（内核到用户）
 */
int copy_to_user(void *to, const void *from, size_t n) {
    if (!access_ok(to, n)) {
        return -1;
    }

    memcpy(to, from, n);
    return 0;
}

/**
 * @brief 验证用户内存指针
 */
int access_ok(const void *ptr, size_t size) {
    /* 简化实现：假设用户空间和内核空间地址范围 */
    /* 实际实现需要检查地址是否在用户空间范围内 */
    return (ptr != NULL && size > 0);
}

/**
 * @brief 内存清零
 */
void *kzalloc(size_t size) {
    void *ptr = kmalloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}