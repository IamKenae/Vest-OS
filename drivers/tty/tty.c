/**
 * @file tty.c
 * @brief TTY终端驱动核心实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <drivers/tty.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <kernel/string.h>
#include <kernel/memory.h>
#include <stdarg.h>

/* TTY管理器全局实例 */
static tty_manager_t tty_manager;

/* 默认TTY配置 */
static const tty_config_t default_tty_config = {
    .echo = 1,
    .canonical = 1,
    .signal_en = 1,
    .crlf = 1,
    .tab_expand = 1,
    .flow_control = 0
};

/* 内部函数声明 */
static int tty_process_input_char(tty_device_t *tty, char ch);
static int tty_process_output_char(tty_device_t *tty, char ch);
static int tty_line_discipline_input(tty_device_t *tty, char ch);
static int tty_line_discipline_output(tty_device_t *tty, char ch);
static void tty_update_display(tty_device_t *tty);
static void tty_save_cursor_state(tty_device_t *tty);
static void tty_restore_cursor_state(tty_device_t *tty);
static int tty_buffer_putchar(char *buffer, uint16_t *head, uint16_t *tail,
                             uint16_t *count, uint16_t size, char ch);

/**
 * @brief 初始化TTY系统
 */
int tty_init(void) {
    if (tty_manager.initialized) {
        return 0;  /* 已经初始化 */
    }

    /* 清零管理器 */
    memset(&tty_manager, 0, sizeof(tty_manager));

    /* 初始化VGA显示 */
    if (vga_init() != 0) {
        return -1;
    }

    /* 初始化键盘 */
    if (keyboard_init() != 0) {
        return -1;
    }

    /* 设置键盘处理器 */
    keyboard_set_handler(tty_keyboard_handler);

    /* 注册控制台TTY (tty0) */
    tty_device_t *console = tty_register("tty0", 0);
    if (!console) {
        return -1;
    }

    /* 设置为当前TTY */
    tty_manager.current_tty = 0;
    tty_manager.initialized = 1;

    return 0;
}

/**
 * @brief 注册TTY设备
 */
tty_device_t *tty_register(const char *name, int minor) {
    if (!name || !tty_is_valid_minor(minor)) {
        return NULL;
    }

    /* 检查是否已注册 */
    if (tty_manager.ttys[minor].state != TTY_STATE_CLOSED) {
        return NULL;
    }

    tty_device_t *tty = &tty_manager.ttys[minor];

    /* 初始化TTY设备 */
    memset(tty, 0, sizeof(tty_device_t));
    strncpy(tty->name, name, TTY_NAME_MAX - 1);
    tty->minor = minor;
    tty->state = TTY_STATE_CLOSED;
    tty->mode = TTY_MODE_COOKED;
    tty->config = default_tty_config;

    /* 设置默认颜色 */
    tty->foreground_color = VGA_COLOR_LIGHT_GREY;
    tty->background_color = VGA_COLOR_BLACK;
    tty->current_color = vga_make_color(tty->foreground_color, tty->background_color);

    /* 初始化光标 */
    tty->cursor.x = 0;
    tty->cursor.y = 0;
    tty->cursor_visible = 1;

    /* 设置输入超时 */
    tty->input_timeout = 0;  /* 无限等待 */

    /* 初始化统计信息 */
    tty->bytes_read = 0;
    tty->bytes_written = 0;
    tty->lines_processed = 0;
    tty->characters_processed = 0;

    tty_manager.tty_count++;

    return tty;
}

/**
 * @brief 注销TTY设备
 */
int tty_unregister(int minor) {
    if (!tty_is_valid_minor(minor)) {
        return -1;
    }

    tty_device_t *tty = &tty_manager.ttys[minor];
    if (tty->state != TTY_STATE_CLOSED) {
        return -1;  /* 设备正在使用 */
    }

    memset(tty, 0, sizeof(tty_device_t));
    tty_manager.tty_count--;

    return 0;
}

/**
 * @brief 打开TTY设备
 */
int tty_open(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    if (tty->state != TTY_STATE_CLOSED) {
        return -1;  /* 设备已经打开 */
    }

    /* 清空缓冲区 */
    memset(&tty->line, 0, sizeof(tty->line));

    /* 设置为打开状态 */
    tty->state = TTY_STATE_OPEN;

    /* 如果是当前TTY，显示欢迎信息 */
    if (minor == tty_manager.current_tty) {
        tty_printf(minor, "\r\nVest-OS TTY %s ready\r\n", tty->name);
        tty->state = TTY_STATE_READY;
    }

    return 0;
}

/**
 * @brief 关闭TTY设备
 */
int tty_close(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    if (tty->state == TTY_STATE_CLOSED) {
        return 0;  /* 已经关闭 */
    }

    /* 清空缓冲区 */
    tty_flush_input(minor);
    tty_flush_output(minor);

    /* 设置为关闭状态 */
    tty->state = TTY_STATE_CLOSED;

    return 0;
}

/**
 * @brief 从TTY读取数据
 */
int tty_read(int minor, char *buffer, size_t size) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty || !buffer || size == 0) {
        return -1;
    }

    if (tty->state != TTY_STATE_READY) {
        return -1;
    }

    size_t bytes_read = 0;

    while (bytes_read < size) {
        /* 检查输入缓冲区 */
        if (tty->line.input_count > 0) {
            /* 从输入缓冲区读取数据 */
            char ch = tty->line.input_buffer[tty->line.input_head];
            tty->line.input_head = (tty->line.input_head + 1) % TTY_BUFFER_SIZE;
            tty->line.input_count--;

            buffer[bytes_read++] = ch;
            tty->bytes_read++;
        } else {
            /* 没有数据可读 */
            if (tty->mode == TTY_MODE_COOKED) {
                /* 熟模式：等待完整的行 */
                break;
            } else {
                /* 生模式：没有数据就直接返回 */
                break;
            }
        }
    }

    return bytes_read;
}

/**
 * @brief 向TTY写入数据
 */
int tty_write(int minor, const char *data, size_t size) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty || !data || size == 0) {
        return -1;
    }

    if (tty->state != TTY_STATE_READY) {
        return -1;
    }

    size_t bytes_written = 0;

    for (size_t i = 0; i < size; i++) {
        if (tty_process_output_char(tty, data[i]) == 0) {
            bytes_written++;
            tty->bytes_written++;
        } else {
            break;  /* 写入失败 */
        }
    }

    /* 刷新输出缓冲区 */
    tty_flush(minor);

    return bytes_written;
}

/**
 * @brief TTY输出字符
 */
int tty_putchar(int minor, char ch) {
    return tty_write(minor, &ch, 1);
}

/**
 * @brief TTY输入字符
 */
int tty_input_char(int minor, char ch) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    if (tty->state != TTY_STATE_READY) {
        return -1;
    }

    return tty_process_input_char(tty, ch);
}

/**
 * @brief TTY格式化输出
 */
int tty_printf(int minor, const char *format, ...) {
    va_list args;
    char buffer[512];
    int len;

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        return tty_write(minor, buffer, len);
    }

    return 0;
}

/**
 * @brief 切换TTY
 */
int tty_switch(int minor) {
    if (!tty_is_valid_minor(minor)) {
        return -1;
    }

    tty_device_t *old_tty = &tty_manager.ttys[tty_manager.current_tty];
    tty_device_t *new_tty = &tty_manager.ttys[minor];

    /* 保存当前TTY状态 */
    if (old_tty->state == TTY_STATE_READY) {
        tty_save_cursor_state(old_tty);
    }

    /* 切换TTY */
    tty_manager.current_tty = minor;

    /* 如果新TTY未打开，自动打开 */
    if (new_tty->state == TTY_STATE_CLOSED) {
        tty_open(minor);
    }

    /* 恢复新TTY状态 */
    if (new_tty->state == TTY_STATE_READY) {
        tty_restore_cursor_state(new_tty);
    }

    return 0;
}

/**
 * @brief 获取当前TTY
 */
int tty_get_current(void) {
    return tty_manager.current_tty;
}

/**
 * @brief 获取TTY设备
 */
tty_device_t *tty_get_device(int minor) {
    if (!tty_is_valid_minor(minor)) {
        return NULL;
    }

    tty_device_t *tty = &tty_manager.ttys[minor];
    if (tty->state == TTY_STATE_CLOSED) {
        return NULL;
    }

    return tty;
}

/**
 * @brief TTY键盘事件处理器
 */
void tty_keyboard_handler(const keyboard_event_t *event) {
    int current_tty = tty_get_current();
    if (event->state == KEY_PRESSED && event->ascii != 0) {
        tty_input_char(current_tty, event->ascii);
    }
}

/**
 * @brief 处理输入字符
 */
static int tty_process_input_char(tty_device_t *tty, char ch) {
    if (tty->mode == TTY_MODE_COOKED) {
        return tty_line_discipline_input(tty, ch);
    } else {
        /* 生模式：直接将字符放入输入缓冲区 */
        return tty_buffer_putchar(tty->line.input_buffer,
                                 &tty->line.input_head,
                                 &tty->line.input_tail,
                                 &tty->line.input_count,
                                 TTY_BUFFER_SIZE, ch);
    }
}

/**
 * @brief 处理输出字符
 */
static int tty_process_output_char(tty_device_t *tty, char ch) {
    if (tty->mode == TTY_MODE_COOKED) {
        return tty_line_discipline_output(tty, ch);
    } else {
        /* 生模式：直接输出到VGA */
        if (tty->minor == tty_manager.current_tty) {
            vga_set_color(tty->foreground_color, tty->background_color);
            vga_put_char(ch);
            tty->cursor = vga_get_cursor();
        }
        return 0;
    }
}

/**
 * @brief 行规程输入处理
 */
static int tty_line_discipline_input(tty_device_t *tty, char ch) {
    switch (ch) {
        case '\n':
        case '\r':
            /* 回车处理 */
            if (tty->config.echo) {
                tty_putchar(tty->minor, '\r');
                tty_putchar(tty->minor, '\n');
            }

            /* 将行缓冲区内容复制到输入缓冲区 */
            for (int i = 0; i < tty->line.line_count; i++) {
                tty_buffer_putchar(tty->line.input_buffer,
                                 &tty->line.input_head,
                                 &tty->line.input_tail,
                                 &tty->line.input_count,
                                 TTY_BUFFER_SIZE,
                                 tty->line.line_buffer[i]);
            }

            /* 添加换行符 */
            tty_buffer_putchar(tty->line.input_buffer,
                             &tty->line.input_head,
                             &tty->line.input_tail,
                             &tty->line.input_count,
                             TTY_BUFFER_SIZE, '\n');

            tty->line.line_count = 0;
            tty->line.line_pos = 0;
            tty->lines_processed++;
            break;

        case '\b':
            /* 退格处理 */
            if (tty->line.line_pos > 0) {
                tty->line.line_pos--;
                tty->line.line_count--;

                if (tty->config.echo) {
                    tty_putchar(tty->minor, '\b');
                    tty_putchar(tty->minor, ' ');
                    tty_putchar(tty->minor, '\b');
                }
            }
            break;

        default:
            /* 普通字符处理 */
            if (tty->line.line_count < TTY_MAX_LINE_LENGTH - 1) {
                tty->line.line_buffer[tty->line.line_pos++] = ch;
                tty->line.line_count++;

                if (tty->config.echo) {
                    tty_putchar(tty->minor, ch);
                }
            }
            break;
    }

    tty->characters_processed++;
    return 0;
}

/**
 * @brief 行规程输出处理
 */
static int tty_line_discipline_output(tty_device_t *tty, char ch) {
    /* CR-LF转换 */
    if (tty->config.crlf && ch == '\n') {
        tty_putchar(tty->minor, '\r');
    }

    /* Tab扩展 */
    if (tty->config.tab_expand && ch == '\t') {
        int spaces = 8 - (tty->cursor.x % 8);
        for (int i = 0; i < spaces; i++) {
            tty_putchar(tty->minor, ' ');
        }
        return 0;
    }

    /* 输出到VGA */
    if (tty->minor == tty_manager.current_tty) {
        vga_set_color(tty->foreground_color, tty->background_color);
        vga_put_char(ch);
        tty->cursor = vga_get_cursor();
    }

    return 0;
}

/**
 * @brief 刷新TTY输出缓冲区
 */
int tty_flush(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    /* 这里可以添加更多刷新逻辑 */
    return 0;
}

/**
 * @brief 设置TTY颜色
 */
int tty_set_color(int minor, vga_color_t foreground, vga_color_t background) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    tty->foreground_color = foreground;
    tty->background_color = background;
    tty->current_color = vga_make_color(foreground, background);

    return 0;
}

/**
 * @brief 清除TTY屏幕
 */
int tty_clear(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    if (tty->minor == tty_manager.current_tty) {
        vga_clear();
        tty->cursor.x = 0;
        tty->cursor.y = 0;
    }

    return 0;
}

/**
 * @brief 缓冲区字符入队
 */
static int tty_buffer_putchar(char *buffer, uint16_t *head, uint16_t *tail,
                             uint16_t *count, uint16_t size, char ch) {
    if (*count >= size) {
        return -1;  /* 缓冲区满 */
    }

    buffer[*tail] = ch;
    *tail = (*tail + 1) % size;
    (*count)++;

    return 0;
}

/**
 * @brief 保存光标状态
 */
static void tty_save_cursor_state(tty_device_t *tty) {
    if (tty->minor == tty_manager.current_tty) {
        tty->cursor = vga_get_cursor();
    }
}

/**
 * @brief 恢复光标状态
 */
static void tty_restore_cursor_state(tty_device_t *tty) {
    if (tty->minor == tty_manager.current_tty) {
        vga_set_cursor(tty->cursor.x, tty->cursor.y);
        vga_set_color(tty->foreground_color, tty->background_color);
    }
}

/**
 * @brief 清空TTY输入缓冲区
 */
int tty_flush_input(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    tty->line.input_head = 0;
    tty->line.input_tail = 0;
    tty->line.input_count = 0;
    tty->line.line_pos = 0;
    tty->line.line_count = 0;

    return 0;
}

/**
 * @brief 清空TTY输出缓冲区
 */
int tty_flush_output(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return -1;
    }

    tty->line.output_head = 0;
    tty->line.output_tail = 0;
    tty->line.output_count = 0;

    return 0;
}

/**
 * @brief 检查TTY是否有数据可读
 */
int tty_has_data(int minor) {
    tty_device_t *tty = tty_get_device(minor);
    if (!tty) {
        return 0;
    }

    return (tty->line.input_count > 0);
}