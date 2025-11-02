/**
 * @file string.c
 * @brief 字符串操作函数实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <kernel/string.h>
#include <stdarg.h>

/**
 * @brief 计算字符串长度
 */
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/**
 * @brief 复制字符串
 */
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++)) {
        /* 空循环体 */
    }
    return dest;
}

/**
 * @brief 复制指定长度的字符串
 */
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

/**
 * @brief 连接字符串
 */
char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) {
        d++;
    }
    while ((*d++ = *src++)) {
        /* 空循环体 */
    }
    return dest;
}

/**
 * @brief 连接指定长度的字符串
 */
char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) {
        d++;
    }
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        d[i] = src[i];
    }
    d[i] = '\0';
    return dest;
}

/**
 * @brief 比较字符串
 */
int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

/**
 * @brief 比较指定长度的字符串
 */
int strncmp(const char *str1, const char *str2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return *(unsigned char*)&str1[i] - *(unsigned char*)&str2[i];
        }
        if (str1[i] == '\0') {
            break;
        }
    }
    return 0;
}

/**
 * @brief 查找字符
 */
char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

/**
 * @brief 查找字符（从字符串末尾）
 */
char *strrchr(const char *str, int c) {
    const char *last = NULL;
    while (*str) {
        if (*str == c) {
            last = str;
        }
        str++;
    }
    return (char*)last;
}

/**
 * @brief 查找子字符串
 */
char *strstr(const char *haystack, const char *needle) {
    if (!*needle) {
        return (char*)haystack;
    }

    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);

    if (needle_len > haystack_len) {
        return NULL;
    }

    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (strncmp(haystack + i, needle, needle_len) == 0) {
            return (char*)(haystack + i);
        }
    }

    return NULL;
}

/**
 * @brief 内存复制
 */
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

/**
 * @brief 内存移动
 */
void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d <= s || d >= s + n) {
        /* 没有重叠，可以直接复制 */
        while (n--) {
            *d++ = *s++;
        }
    } else {
        /* 有重叠，从后向前复制 */
        d += n - 1;
        s += n - 1;
        while (n--) {
            *d-- = *s--;
        }
    }

    return dest;
}

/**
 * @brief 内存填充
 */
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) {
        *p++ = c;
    }
    return s;
}

/**
 * @brief 内存比较
 */
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/**
 * @brief 内存扫描
 */
void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = s;
    while (n--) {
        if (*p == c) {
            return (void*)p;
        }
        p++;
    }
    return NULL;
}

/**
 * @brief 简单的格式化字符串实现
 */
int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    int result;

    va_start(args, format);
    result = vsnprintf(str, size, format, args);
    va_end(args);

    return result;
}

/**
 * @brief 变参格式化字符串
 */
int vsnprintf(char *str, size_t size, const char *format, va_list args) {
    if (!str || size == 0) {
        return 0;
    }

    char *s = str;
    size_t remaining = size - 1;  /* 保留空间给null终止符 */
    int written = 0;

    while (*format && remaining > 0) {
        if (*format != '%') {
            *s++ = *format++;
            written++;
            remaining--;
            continue;
        }

        format++;  /* 跳过 '%' */

        /* 处理格式说明符 */
        switch (*format) {
            case 'c': {
                char c = (char)va_arg(args, int);
                *s++ = c;
                written++;
                remaining--;
                format++;
                break;
            }
            case 's': {
                char *str_arg = va_arg(args, char*);
                size_t len = strlen(str_arg);
                size_t copy_len = (len < remaining) ? len : remaining;
                memcpy(s, str_arg, copy_len);
                s += copy_len;
                written += copy_len;
                remaining -= copy_len;
                format++;
                break;
            }
            case 'd': {
                int num = va_arg(args, int);
                /* 简单的整数转字符串 */
                char temp[32];
                int i = 0;
                if (num == 0) {
                    temp[i++] = '0';
                } else {
                    int n = num;
                    if (n < 0) {
                        n = -n;
                    }
                    while (n > 0) {
                        temp[i++] = '0' + (n % 10);
                        n /= 10;
                    }
                    if (num < 0) {
                        temp[i++] = '-';
                    }
                    /* 反转字符串 */
                    int j;
                    for (j = 0; j < i/2; j++) {
                        char t = temp[j];
                        temp[j] = temp[i - 1 - j];
                        temp[i - 1 - j] = t;
                    }
                }
                temp[i] = '\0';
                size_t len = strlen(temp);
                size_t copy_len = (len < remaining) ? len : remaining;
                memcpy(s, temp, copy_len);
                s += copy_len;
                written += copy_len;
                remaining -= copy_len;
                format++;
                break;
            }
            case 'x': {
                unsigned int num = va_arg(args, unsigned int);
                char temp[32];
                int i = 0;
                if (num == 0) {
                    temp[i++] = '0';
                } else {
                    while (num > 0) {
                        int digit = num % 16;
                        temp[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                        num /= 16;
                    }
                    /* 反转字符串 */
                    int j;
                    for (j = 0; j < i/2; j++) {
                        char t = temp[j];
                        temp[j] = temp[i - 1 - j];
                        temp[i - 1 - j] = t;
                    }
                }
                temp[i] = '\0';
                size_t len = strlen(temp);
                size_t copy_len = (len < remaining) ? len : remaining;
                memcpy(s, temp, copy_len);
                s += copy_len;
                written += copy_len;
                remaining -= copy_len;
                format++;
                break;
            }
            case '%': {
                *s++ = '%';
                written++;
                remaining--;
                format++;
                break;
            }
            default:
                /* 未知格式，原样输出 */
                *s++ = '%';
                written++;
                remaining--;
                if (remaining > 0 && *format) {
                    *s++ = *format++;
                    written++;
                    remaining--;
                }
                break;
        }
    }

    *s = '\0';
    return written;
}