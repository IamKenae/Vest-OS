/**
 * @file vga.h
 * @brief VGA文本模式显示驱动头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _VGA_H
#define _VGA_H

#include <stdint.h>

/* VGA显示参数 */
#define VGA_WIDTH        80
#define VGA_HEIGHT       25
#define VGA_SIZE         (VGA_WIDTH * VGA_HEIGHT)
#define VGA_MEMORY       0xB8000
#define VGA_MEMORY_SIZE  (VGA_SIZE * 2)

/* VGA颜色定义 */
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15
} vga_color_t;

/* VGA字符单元结构 */
typedef struct {
    uint8_t character;    /* ASCII字符 */
    uint8_t color;        /* 前景和背景颜色 */
} __attribute__((packed)) vga_cell_t;

/* VGA光标位置 */
typedef struct {
    uint8_t x;           /* 列位置 */
    uint8_t y;           /* 行位置 */
} vga_cursor_t;

/* VGA显示状态 */
typedef struct {
    vga_cell_t *buffer;              /* 显存缓冲区 */
    vga_cursor_t cursor;             /* 光标位置 */
    uint8_t foreground_color;        /* 前景色 */
    uint8_t background_color;        /* 背景色 */
    uint8_t current_color;           /* 当前颜色组合 */
    uint8_t tab_size;                /* 制表符大小 */
    uint8_t auto_scroll;             /* 自动滚动 */
} vga_state_t;

/* 函数声明 */

/**
 * @brief 初始化VGA显示系统
 * @return 0成功，-1失败
 */
int vga_init(void);

/**
 * @brief 清屏
 */
void vga_clear(void);

/**
 * @brief 设置颜色
 * @param foreground 前景色
 * @param background 背景色
 */
void vga_set_color(vga_color_t foreground, vga_color_t background);

/**
 * @brief 获取当前颜色
 * @return 当前颜色组合
 */
uint8_t vga_get_color(void);

/**
 * @brief 设置光标位置
 * @param x 列位置
 * @param y 行位置
 */
void vga_set_cursor(uint8_t x, uint8_t y);

/**
 * @brief 获取光标位置
 * @return 光标位置结构
 */
vga_cursor_t vga_get_cursor(void);

/**
 * @brief 显示硬件光标
 */
void vga_show_cursor(void);

/**
 * @brief 隐藏硬件光标
 */
void vga_hide_cursor(void);

/**
 * @brief 在指定位置写字符
 * @param x 列位置
 * @param y 行位置
 * @param ch 字符
 * @param color 颜色
 */
void vga_put_char_at(uint8_t x, uint8_t y, char ch, uint8_t color);

/**
 * @brief 在当前位置写字符
 * @param ch 字符
 */
void vga_put_char(char ch);

/**
 * @brief 写字符串
 * @param str 字符串
 */
void vga_put_string(const char *str);

/**
 * @brief 格式化输出
 * @param format 格式字符串
 * @return 输出字符数
 */
int vga_printf(const char *format, ...);

/**
 * @brief 滚动屏幕一行
 */
void vga_scroll_line(void);

/**
 * @brief 滚动屏幕多行
 * @param lines 行数
 */
void vga_scroll_lines(uint8_t lines);

/**
 * @brief 删除光标处字符
 */
void vga_backspace(void);

/**
 * @brief 删除当前行
 */
void vga_delete_line(void);

/**
 * @line 插入新行
 */
void vga_new_line(void);

/**
 * @brief 制表符处理
 */
void vga_tab(void);

/**
 * @brief 获取VGA缓冲区
 * @return 显存缓冲区指针
 */
vga_cell_t *vga_get_buffer(void);

/**
 * @brief 复制屏幕区域
 * @param src_x 源列
 * @param src_y 源行
 * @param dst_x 目标列
 * @param dst_y 目标行
 * @param width 宽度
 * @param height 高度
 */
void vga_copy_region(uint8_t src_x, uint8_t src_y, uint8_t dst_x, uint8_t dst_y,
                     uint8_t width, uint8_t height);

/**
 * @brief 填充屏幕区域
 * @param x 起始列
 * @param y 起始行
 * @param width 宽度
 * @param height 高度
 * @param ch 填充字符
 * @param color 颜色
 */
void vga_fill_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                     char ch, uint8_t color);

/* 内联函数 */
static inline uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_make_entry(char ch, uint8_t color) {
    return ((uint16_t)color << 8) | (uint16_t)ch;
}

static inline void vga_write_entry(uint8_t x, uint8_t y, uint16_t entry) {
    const size_t index = y * VGA_WIDTH + x;
    *((volatile uint16_t*)VGA_MEMORY + index) = entry;
}

#endif /* _VGA_H */