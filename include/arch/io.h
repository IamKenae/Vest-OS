/**
 * @file io.h
 * @brief I/O端口操作头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _ARCH_IO_H
#define _ARCH_IO_H

#include <stdint.h>

/* I/O端口操作内联函数 */

/**
 * @brief 从I/O端口读取8位数据
 * @param port 端口号
 * @return 读取的数据
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * @brief 向I/O端口写入8位数据
 * @param port 端口号
 * @param data 要写入的数据
 */
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * @brief 从I/O端口读取16位数据
 * @param port 端口号
 * @return 读取的数据
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * @brief 向I/O端口写入16位数据
 * @param port 端口号
 * @param data 要写入的数据
 */
static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * @brief 从I/O端口读取32位数据
 * @param port 端口号
 * @return 读取的数据
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/**
 * @brief 向I/O端口写入32位数据
 * @param port 端口号
 * @param data 要写入的数据
 */
static inline void outl(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * @brief 等待I/O操作完成
 */
static inline void io_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

#endif /* _ARCH_IO_H */