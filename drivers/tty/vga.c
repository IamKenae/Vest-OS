/**
 * @file vga.c
 * @brief VGA文本模式显示驱动实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <drivers/vga.h>
#include <arch/io.h>
#include <stdarg.h>
#include <string.h>

/* VGA寄存器端口 */
#define VGA_CRTC_ADDR    0x3D4      /* CRTC地址寄存器 */
#define VGA_CRTC_DATA    0x3D5      /* CRTC数据寄存器 */

/* CRTC寄存器索引 */
#define VGA_CURSOR_HIGH  0x0E       /* 光标位置高字节 */
#define VGA_CURSOR_LOW   0x0F       /* 光标位置低字节 */
#define VGA_START_ADDR_H 0x0C       /* 显存起始地址高字节 */
#define VGA_START_ADDR_L 0x0D       /* 显存起始地址低字节 */

/* 全局VGA状态 */
static vga_state_t vga_state;

/**
 * @brief 初始化VGA显示系统
 */
int vga_init(void) {
    /* 初始化VGA状态 */
    vga_state.buffer = (vga_cell_t*)VGA_MEMORY;
    vga_state.cursor.x = 0;
    vga_state.cursor.y = 0;
    vga_state.foreground_color = VGA_COLOR_LIGHT_GREY;
    vga_state.background_color = VGA_COLOR_BLACK;
    vga_state.current_color = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_state.tab_size = 8;
    vga_state.auto_scroll = 1;

    /* 清屏 */
    vga_clear();

    /* 显示光标 */
    vga_show_cursor();

    return 0;
}

/**
 * @brief 清屏
 */
void vga_clear(void) {
    vga_cell_t blank = {
        .character = ' ',
        .color = vga_state.current_color
    };

    /* 填充整个屏幕 */
    for (size_t i = 0; i < VGA_SIZE; i++) {
        vga_state.buffer[i] = blank;
    }

    /* 重置光标位置 */
    vga_set_cursor(0, 0);
}

/**
 * @brief 设置颜色
 */
void vga_set_color(vga_color_t foreground, vga_color_t background) {
    vga_state.foreground_color = foreground;
    vga_state.background_color = background;
    vga_state.current_color = vga_make_color(foreground, background);
}

/**
 * @brief 获取当前颜色
 */
uint8_t vga_get_color(void) {
    return vga_state.current_color;
}

/**
 * @brief 设置光标位置
 */
void vga_set_cursor(uint8_t x, uint8_t y) {
    /* 边界检查 */
    if (x >= VGA_WIDTH) {
        x = VGA_WIDTH - 1;
    }
    if (y >= VGA_HEIGHT) {
        y = VGA_HEIGHT - 1;
    }

    vga_state.cursor.x = x;
    vga_state.cursor.y = y;

    /* 更新硬件光标位置 */
    uint16_t pos = y * VGA_WIDTH + x;

    /* 输出光标位置高字节 */
    outb(VGA_CRTC_ADDR, VGA_CURSOR_HIGH);
    outb(VGA_CRTC_DATA, (pos >> 8) & 0xFF);

    /* 输出光标位置低字节 */
    outb(VGA_CRTC_ADDR, VGA_CURSOR_LOW);
    outb(VGA_CRTC_DATA, pos & 0xFF);
}

/**
 * @brief 获取光标位置
 */
vga_cursor_t vga_get_cursor(void) {
    return vga_state.cursor;
}

/**
 * @brief 显示硬件光标
 */
void vga_show_cursor(void) {
    uint16_t pos = vga_state.cursor.y * VGA_WIDTH + vga_state.cursor.x;

    /* 启用光标 */
    outb(VGA_CRTC_ADDR, 0x0A);
    uint8_t cursor_start = inb(VGA_CRTC_DATA) & 0xC0;
    outb(VGA_CRTC_DATA, cursor_start | 0x0E);

    outb(VGA_CRTC_ADDR, 0x0B);
    uint8_t cursor_end = inb(VGA_CRTC_DATA) & 0xE0;
    outb(VGA_CRTC_DATA, cursor_end | 0x0F);

    /* 设置光标位置 */
    outb(VGA_CRTC_ADDR, VGA_CURSOR_HIGH);
    outb(VGA_CRTC_DATA, (pos >> 8) & 0xFF);

    outb(VGA_CRTC_ADDR, VGA_CURSOR_LOW);
    outb(VGA_CRTC_DATA, pos & 0xFF);
}

/**
 * @brief 隐藏硬件光标
 */
void vga_hide_cursor(void) {
    /* 禁用光标 */
    outb(VGA_CRTC_ADDR, 0x0A);
    outb(VGA_CRTC_DATA, 0x20);
}

/**
 * @brief 在指定位置写字符
 */
void vga_put_char_at(uint8_t x, uint8_t y, char ch, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }

    size_t index = y * VGA_WIDTH + x;
    vga_state.buffer[index].character = ch;
    vga_state.buffer[index].color = color;
}

/**
 * @brief 在当前位置写字符
 */
void vga_put_char(char ch) {
    switch (ch) {
        case '\n':
            vga_new_line();
            break;
        case '\t':
            vga_tab();
            break;
        case '\b':
            vga_backspace();
            break;
        case '\r':
            vga_set_cursor(0, vga_state.cursor.y);
            break;
        default:
            if (ch >= ' ' && ch <= '~') {  /* 可打印字符 */
                vga_put_char_at(vga_state.cursor.x, vga_state.cursor.y,
                               ch, vga_state.current_color);

                /* 移动光标 */
                vga_state.cursor.x++;
                if (vga_state.cursor.x >= VGA_WIDTH) {
                    vga_new_line();
                }
            }
            break;
    }

    /* 更新硬件光标 */
    vga_show_cursor();
}

/**
 * @brief 写字符串
 */
void vga_put_string(const char *str) {
    if (!str) {
        return;
    }

    while (*str) {
        vga_put_char(*str++);
    }
}

/**
 * @brief 滚动屏幕一行
 */
void vga_scroll_line(void) {
    /* 向上移动一行 */
    for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_state.buffer[i] = vga_state.buffer[i + VGA_WIDTH];
    }

    /* 清空最后一行 */
    size_t last_line = (VGA_HEIGHT - 1) * VGA_WIDTH;
    vga_cell_t blank = {
        .character = ' ',
        .color = vga_state.current_color
    };

    for (size_t i = 0; i < VGA_WIDTH; i++) {
        vga_state.buffer[last_line + i] = blank;
    }
}

/**
 * @brief 滚动屏幕多行
 */
void vga_scroll_lines(uint8_t lines) {
    if (lines == 0) {
        return;
    }

    if (lines >= VGA_HEIGHT) {
        vga_clear();
        return;
    }

    /* 向上移动多行 */
    size_t move_count = (VGA_HEIGHT - lines) * VGA_WIDTH;
    for (size_t i = 0; i < move_count; i++) {
        vga_state.buffer[i] = vga_state.buffer[i + lines * VGA_WIDTH];
    }

    /* 清空底部几行 */
    size_t start_clear = (VGA_HEIGHT - lines) * VGA_WIDTH;
    size_t clear_count = lines * VGA_WIDTH;
    vga_cell_t blank = {
        .character = ' ',
        .color = vga_state.current_color
    };

    for (size_t i = 0; i < clear_count; i++) {
        vga_state.buffer[start_clear + i] = blank;
    }
}

/**
 * @brief 插入新行
 */
void vga_new_line(void) {
    vga_state.cursor.x = 0;
    vga_state.cursor.y++;

    /* 检查是否需要滚动 */
    if (vga_state.cursor.y >= VGA_HEIGHT) {
        if (vga_state.auto_scroll) {
            vga_scroll_line();
            vga_state.cursor.y = VGA_HEIGHT - 1;
        } else {
            vga_state.cursor.y = VGA_HEIGHT - 1;
        }
    }
}

/**
 * @brief 制表符处理
 */
void vga_tab(void) {
    uint8_t next_tab = (vga_state.cursor.x / vga_state.tab_size + 1) * vga_state.tab_size;

    while (vga_state.cursor.x < next_tab && vga_state.cursor.x < VGA_WIDTH) {
        vga_put_char(' ');
    }
}

/**
 * @brief 删除光标处字符
 */
void vga_backspace(void) {
    if (vga_state.cursor.x > 0) {
        vga_state.cursor.x--;
        vga_put_char_at(vga_state.cursor.x, vga_state.cursor.y, ' ', vga_state.current_color);
    } else if (vga_state.cursor.y > 0) {
        /* 移动到上一行末尾 */
        vga_state.cursor.y--;
        vga_state.cursor.x = VGA_WIDTH - 1;
        vga_put_char_at(vga_state.cursor.x, vga_state.cursor.y, ' ', vga_state.current_color);
    }
}

/**
 * @brief 删除当前行
 */
void vga_delete_line(void) {
    size_t line_start = vga_state.cursor.y * VGA_WIDTH;
    vga_cell_t blank = {
        .character = ' ',
        .color = vga_state.current_color
    };

    for (size_t i = 0; i < VGA_WIDTH; i++) {
        vga_state.buffer[line_start + i] = blank;
    }

    vga_set_cursor(0, vga_state.cursor.y);
}

/**
 * @brief 获取VGA缓冲区
 */
vga_cell_t *vga_get_buffer(void) {
    return vga_state.buffer;
}

/**
 * @brief 复制屏幕区域
 */
void vga_copy_region(uint8_t src_x, uint8_t src_y, uint8_t dst_x, uint8_t dst_y,
                     uint8_t width, uint8_t height) {
    /* 边界检查 */
    if (src_x + width > VGA_WIDTH || src_y + height > VGA_HEIGHT ||
        dst_x + width > VGA_WIDTH || dst_y + height > VGA_HEIGHT) {
        return;
    }

    for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = 0; x < width; x++) {
            size_t src_index = (src_y + y) * VGA_WIDTH + (src_x + x);
            size_t dst_index = (dst_y + y) * VGA_WIDTH + (dst_x + x);
            vga_state.buffer[dst_index] = vga_state.buffer[src_index];
        }
    }
}

/**
 * @brief 填充屏幕区域
 */
void vga_fill_region(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                     char ch, uint8_t color) {
    /* 边界检查 */
    if (x + width > VGA_WIDTH || y + height > VGA_HEIGHT) {
        return;
    }

    for (uint8_t dy = 0; dy < height; dy++) {
        for (uint8_t dx = 0; dx < width; dx++) {
            vga_put_char_at(x + dx, y + dy, ch, color);
        }
    }
}

/* 简单的printf实现 */
int vga_printf(const char *format, ...) {
    va_list args;
    char buffer[256];
    int len;

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        vga_put_string(buffer);
    }

    return len;
}