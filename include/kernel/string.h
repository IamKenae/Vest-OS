/**
 * @file string.h
 * @brief 字符串操作函数
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stdint.h>
#include <stddef.h>

/* 字符串函数 */

/**
 * @brief 计算字符串长度
 * @param str 字符串
 * @return 字符串长度
 */
size_t strlen(const char *str);

/**
 * @brief 复制字符串
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @return 目标字符串指针
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief 复制指定长度的字符串
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param n 最大复制长度
 * @return 目标字符串指针
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * @brief 连接字符串
 * @param dest 目标字符串
 * @param src 源字符串
 * @return 目标字符串指针
 */
char *strcat(char *dest, const char *src);

/**
 * @brief 连接指定长度的字符串
 * @param dest 目标字符串
 * @param src 源字符串
 * @param n 最大连接长度
 * @return 目标字符串指针
 */
char *strncat(char *dest, const char *src, size_t n);

/**
 * @brief 比较字符串
 * @param str1 字符串1
 * @param str2 字符串2
 * @return 比较结果
 */
int strcmp(const char *str1, const char *str2);

/**
 * @brief 比较指定长度的字符串
 * @param str1 字符串1
 * @param str2 字符串2
 * @param n 比较长度
 * @return 比较结果
 */
int strncmp(const char *str1, const char *str2, size_t n);

/**
 * @brief 查找字符
 * @param str 字符串
 * @param c 字符
 * @return 字符位置指针，NULL未找到
 */
char *strchr(const char *str, int c);

/**
 * @brief 查找字符（从字符串末尾）
 * @param str 字符串
 * @param c 字符
 * @return 字符位置指针，NULL未找到
 */
char *strrchr(const char *str, int c);

/**
 * @brief 查找子字符串
 * @param haystack 字符串
 * @param needle 子字符串
 * @return 子字符串位置指针，NULL未找到
 */
char *strstr(const char *haystack, const char *needle);

/**
 * @brief 内存复制
 * @param dest 目标地址
 * @param src 源地址
 * @param n 复制字节数
 * @return 目标地址指针
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * @brief 内存移动
 * @param dest 目标地址
 * @param src 源地址
 * @param n 移动字节数
 * @return 目标地址指针
 */
void *memmove(void *dest, const void *src, size_t n);

/**
 * @brief 内存填充
 * @param s 内存地址
 * @param c 填充字符
 * @param n 填充字节数
 * @return 内存地址指针
 */
void *memset(void *s, int c, size_t n);

/**
 * @brief 内存比较
 * @param s1 内存1
 * @param s2 内存2
 * @param n 比较字节数
 * @return 比较结果
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * @brief 内存扫描
 * @param s 内存地址
 * @param c 字符
 * @param n 扫描字节数
 * @return 字符位置指针，NULL未找到
 */
void *memchr(const void *s, int c, size_t n);

/**
 * @brief 格式化字符串
 * @param str 目标缓冲区
 * @param size 缓冲区大小
 * @param format 格式字符串
 * @return 输出字符数
 */
int snprintf(char *str, size_t size, const char *format, ...);

/**
 * @brief 变参格式化字符串
 * @param str 目标缓冲区
 * @param size 缓冲区大小
 * @param format 格式字符串
 * @param args 变参列表
 * @return 输出字符数
 */
int vsnprintf(char *str, size_t size, const char *format, va_list args);

#endif /* _KERNEL_STRING_H */