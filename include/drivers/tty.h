/**
 * @file tty.h
 * @brief TTY终端驱动头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _TTY_H
#define _TTY_H

#include <stdint.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>

/* TTY配置 */
#define MAX_TTYS            8       /* 最大TTY数量 */
#define TTY_BUFFER_SIZE     4096    /* TTY缓冲区大小 */
#define TTY_NAME_MAX        16      /* TTY名称最大长度 */
#define TTY_MAX_LINE_LENGTH 1024    /* 最大行长度 */

/* TTY编号 */
#define TTY_CONSOLE_MAJOR   4       /* 主设备号 */
#define TTY_CONSOLE_MINOR   0       /* 次设备号起始 */

/* TTY状态 */
typedef enum {
    TTY_STATE_CLOSED = 0,
    TTY_STATE_OPEN,
    TTY_STATE_READY,
    TTY_STATE_ERROR
} tty_state_t;

/* TTY模式 */
typedef enum {
    TTY_MODE_COOKED = 0,    /* 熟模式 - 行缓冲 */
    TTY_MODE_RAW            /* 生模式 - 字符缓冲 */
} tty_mode_t;

/* TTY配置 */
typedef struct {
    uint8_t echo;           /* 回显 */
    uint8_t canonical;      /* 规范模式 */
    uint8_t signal_en;      /* 信号使能 */
    uint8_t crlf;           /* CR-LF转换 */
    uint8_t tab_expand;     /* Tab扩展 */
    uint8_t flow_control;   /* 流控制 */
} tty_config_t;

/* TTY行规程 */
typedef struct {
    char input_buffer[TTY_BUFFER_SIZE];      /* 输入缓冲区 */
    char line_buffer[TTY_MAX_LINE_LENGTH];   /* 行缓冲区 */
    char output_buffer[TTY_BUFFER_SIZE];     /* 输出缓冲区 */

    uint16_t input_head;     /* 输入缓冲区头 */
    uint16_t input_tail;     /* 输入缓冲区尾 */
    uint16_t input_count;    /* 输入缓冲区计数 */

    uint16_t line_pos;       /* 行缓冲区位置 */
    uint16_t line_count;     /* 行缓冲区计数 */

    uint16_t output_head;    /* 输出缓冲区头 */
    uint16_t output_tail;    /* 输出缓冲区尾 */
    uint16_t output_count;   /* 输出缓冲区计数 */
} tty_line_discipline_t;

/* TTY设备结构 */
typedef struct tty_device {
    char name[TTY_NAME_MAX];         /* TTY名称 */
    int minor;                       /* 次设备号 */
    tty_state_t state;               /* 状态 */
    tty_mode_t mode;                 /* 模式 */
    tty_config_t config;             /* 配置 */
    tty_line_discipline_t line;      /* 行规程 */

    /* 显示相关 */
    uint8_t foreground_color;        /* 前景色 */
    uint8_t background_color;        /* 背景色 */
    uint8_t current_color;           /* 当前颜色 */
    vga_cursor_t cursor;             /* 光标位置 */
    uint8_t cursor_visible;          /* 光标可见性 */

    /* 输入相关 */
    keyboard_handler_t keyboard_handler;  /* 键盘处理器 */
    uint32_t input_timeout;               /* 输入超时 */

    /* 统计信息 */
    uint32_t bytes_read;              /* 读取字节数 */
    uint32_t bytes_written;           /* 写入字节数 */
    uint32_t lines_processed;         /* 处理行数 */
    uint32_t characters_processed;    /* 处理字符数 */

    /* 回调函数 */
    void (*write_callback)(struct tty_device *tty, const char *data, size_t size);
    void (*read_callback)(struct tty_device *tty, char *data, size_t size);
} tty_device_t;

/* TTY管理器结构 */
typedef struct {
    tty_device_t ttys[MAX_TTYS];     /* TTY设备数组 */
    int current_tty;                  /* 当前活动TTY */
    int tty_count;                    /* TTY数量 */
    uint8_t initialized;              /* 初始化标志 */
} tty_manager_t;

/* 函数声明 */

/**
 * @brief 初始化TTY系统
 * @return 0成功，-1失败
 */
int tty_init(void);

/**
 * @brief 注册TTY设备
 * @param name TTY名称
 * @param minor 次设备号
 * @return TTY设备指针，NULL失败
 */
tty_device_t *tty_register(const char *name, int minor);

/**
 * @brief 注销TTY设备
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_unregister(int minor);

/**
 * @brief 打开TTY设备
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_open(int minor);

/**
 * @brief 关闭TTY设备
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_close(int minor);

/**
 * @brief 从TTY读取数据
 * @param minor 次设备号
 * @param buffer 缓冲区
 * @param size 要读取的大小
 * @return 实际读取的字节数，-1失败
 */
int tty_read(int minor, char *buffer, size_t size);

/**
 * @brief 向TTY写入数据
 * @param minor 次设备号
 * @param data 数据
 * @param size 数据大小
 * @return 实际写入的字节数，-1失败
 */
int tty_write(int minor, const char *data, size_t size);

/**
 * @brief TTY输出字符
 * @param minor 次设备号
 * @param ch 字符
 * @return 0成功，-1失败
 */
int tty_putchar(int minor, char ch);

/**
 * @brief TTY输入字符
 * @param minor 次设备号
 * @param ch 字符
 * @return 0成功，-1失败
 */
int tty_input_char(int minor, char ch);

/**
 * @brief TTY格式化输出
 * @param minor 次设备号
 * @param format 格式字符串
 * @return 输出字符数
 */
int tty_printf(int minor, const char *format, ...);

/**
 * @brief 切换TTY
 * @param minor 目标TTY次设备号
 * @return 0成功，-1失败
 */
int tty_switch(int minor);

/**
 * @brief 获取当前TTY
 * @return 当前TTY次设备号
 */
int tty_get_current(void);

/**
 * @brief 设置TTY模式
 * @param minor 次设备号
 * @param mode 模式
 * @return 0成功，-1失败
 */
int tty_set_mode(int minor, tty_mode_t mode);

/**
 * @brief 获取TTY模式
 * @param minor 次设备号
 * @return 模式
 */
tty_mode_t tty_get_mode(int minor);

/**
 * @brief 设置TTY配置
 * @param minor 次设备号
 * @param config 配置
 * @return 0成功，-1失败
 */
int tty_set_config(int minor, const tty_config_t *config);

/**
 * @brief 获取TTY配置
 * @param minor 次设备号
 * @param config 配置存储位置
 * @return 0成功，-1失败
 */
int tty_get_config(int minor, tty_config_t *config);

/**
 * @brief 设置TTY颜色
 * @param minor 次设备号
 * @param foreground 前景色
 * @param background 背景色
 * @return 0成功，-1失败
 */
int tty_set_color(int minor, vga_color_t foreground, vga_color_t background);

/**
 * @brief 清除TTY屏幕
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_clear(int minor);

/**
 * @brief 设置TTY光标位置
 * @param minor 次设备号
 * @param x 列位置
 * @param y 行位置
 * @return 0成功，-1失败
 */
int tty_set_cursor(int minor, uint8_t x, uint8_t y);

/**
 * @brief 获取TTY设备
 * @param minor 次设备号
 * @return TTY设备指针，NULL失败
 */
tty_device_t *tty_get_device(int minor);

/**
 * @brief TTY键盘事件处理器
 * @param event 键盘事件
 */
void tty_keyboard_handler(const keyboard_event_t *event);

/**
 * @brief 刷新TTY输出缓冲区
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_flush(int minor);

/**
 * @brief 检查TTY是否有数据可读
 * @param minor 次设备号
 * @return 1有数据，0无数据
 */
int tty_has_data(int minor);

/**
 * @brief 清空TTY输入缓冲区
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_flush_input(int minor);

/**
 * @brief 清空TTY输出缓冲区
 * @param minor 次设备号
 * @return 0成功，-1失败
 */
int tty_flush_output(int minor);

/* 内联函数 */
static inline int tty_is_valid_minor(int minor) {
    return (minor >= 0 && minor < MAX_TTYS);
}

static inline int tty_is_open(tty_device_t *tty) {
    return (tty && tty->state == TTY_STATE_OPEN);
}

static inline int tty_is_ready(tty_device_t *tty) {
    return (tty && tty->state == TTY_STATE_READY);
}

#endif /* _TTY_H */