/**
 * @file keyboard.h
 * @brief 键盘输入驱动头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

/* 键盘控制器端口 */
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

/* 键盘状态寄存器位 */
#define KEYBOARD_STATUS_OUT_BUF_FULL  0x01  /* 输出缓冲区满 */
#define KEYBOARD_STATUS_IN_BUF_FULL   0x02  /* 输入缓冲区满 */
#define KEYBOARD_STATUS_SYS           0x04  /* 系统 */
#define KEYBOARD_STATUS_CMD_DATA      0x08  /* 命令/数据 */
#define KEYBOARD_STATUS_LOCKED        0x10  /* 锁定 */
#define KEYBOARD_STATUS_AUX_BUF       0x20  /* 辅助缓冲区 */
#define KEYBOARD_STATUS_TIMEOUT       0x40  /* 超时 */
#define KEYBOARD_STATUS_PARITY_ERR    0x80  /* 奇偶校验错误 */

/* 键盘命令 */
#define KEYBOARD_CMD_SET_LEDS        0xED
#define KEYBOARD_CMD_ECHO            0xEE
#define KEYBOARD_CMD_SET_SCANCODE    0xF0
#define KEYBOARD_CMD_SEND_ID         0xF2
#define KEYBOARD_CMD_SET_RATE        0xF3
#define KEYBOARD_CMD_ENABLE          0xF4
#define KEYBOARD_CMD_DISABLE         0xF5
#define KEYBOARD_CMD_SET_DEFAULT     0xF6
#define KEYBOARD_CMD_RESET           0xFF

/* LED状态位 */
#define KEYBOARD_LED_SCROLL_LOCK     0x01
#define KEYBOARD_LED_NUM_LOCK        0x02
#define KEYBOARD_LED_CAPS_LOCK       0x04

/* 扫描码集合 */
typedef enum {
    SCANCODE_SET_1 = 0,
    SCANCODE_SET_2,
    SCANCODE_SET_3
} scancode_set_t;

/* 按键状态 */
typedef enum {
    KEY_RELEASED = 0,
    KEY_PRESSED = 1
} key_state_t;

/* 修饰键状态 */
typedef struct {
    uint8_t left_shift : 1;
    uint8_t right_shift : 1;
    uint8_t left_ctrl : 1;
    uint8_t right_ctrl : 1;
    uint8_t left_alt : 1;
    uint8_t right_alt : 1;
    uint8_t caps_lock : 1;
    uint8_t num_lock : 1;
    uint8_t scroll_lock : 1;
    uint8_t reserved : 6;
} __attribute__((packed)) modifier_keys_t;

/* 键盘事件 */
typedef struct {
    uint8_t scancode;           /* 扫描码 */
    char ascii;                 /* ASCII字符 */
    key_state_t state;          /* 按键状态 */
    modifier_keys_t modifiers;  /* 修饰键状态 */
} keyboard_event_t;

/* 键盘缓冲区 */
#define KEYBOARD_BUFFER_SIZE  256

typedef struct {
    keyboard_event_t buffer[KEYBOARD_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} keyboard_buffer_t;

/* 键盘处理器函数指针 */
typedef void (*keyboard_handler_t)(const keyboard_event_t *event);

/* 函数声明 */

/**
 * @brief 初始化键盘驱动
 * @return 0成功，-1失败
 */
int keyboard_init(void);

/**
 * @brief 键盘中断处理函数
 */
void keyboard_interrupt_handler(void);

/**
 * @brief 读取键盘事件
 * @param event 事件存储位置
 * @return 0成功，-1缓冲区空，-2错误
 */
int keyboard_read_event(keyboard_event_t *event);

/**
 * @brief 检查是否有键盘事件
 * @return 1有事件，0无事件
 */
int keyboard_has_event(void);

/**
 * @brief 清空键盘缓冲区
 */
void keyboard_flush(void);

/**
 * @brief 设置LED状态
 * @param leds LED状态
 */
void keyboard_set_leds(uint8_t leds);

/**
 * @brief 获取LED状态
 * @return LED状态
 */
uint8_t keyboard_get_leds(void);

/**
 * @brief 设置重复率
 * @param delay 延迟(0-3)
 * @param rate 重复率(0-31)
 * @return 0成功，-1失败
 */
int keyboard_set_repeat_rate(uint8_t delay, uint8_t rate);

/**
 * @brief 启用键盘
 */
void keyboard_enable(void);

/**
 * @brief 禁用键盘
 */
void keyboard_disable(void);

/**
 * @brief 重置键盘
 * @return 0成功，-1失败
 */
int keyboard_reset(void);

/**
 * @brief 获取修饰键状态
 * @return 修饰键状态
 */
modifier_keys_t keyboard_get_modifiers(void);

/**
 * @brief 设置扫描码集合
 * @param set 扫描码集合
 * @return 0成功，-1失败
 */
int keyboard_set_scancode_set(scancode_set_t set);

/**
 * @brief 获取扫描码集合
 * @return 扫描码集合
 */
scancode_set_t keyboard_get_scancode_set(void);

/**
 * @brief 设置键盘处理器
 * @param handler 处理器函数
 * @return 0成功，-1失败
 */
int keyboard_set_handler(keyboard_handler_t handler);

/**
 * @brief 移除键盘处理器
 */
void keyboard_remove_handler(void);

/**
 * @brief 扫描码转ASCII
 * @param scancode 扫描码
 * @param modifiers 修饰键状态
 * @return ASCII字符，0表示不可打印字符
 */
char scancode_to_ascii(uint8_t scancode, modifier_keys_t modifiers);

/**
 * @brief 检查是否为扩展键
 * @param scancode 扫描码
 * @return 1是扩展键，0不是
 */
int is_extended_key(uint8_t scancode);

/**
 * @brief 等待按键
 * @param timeout 超时时间(毫秒)，0表示无限等待
 * @return 0成功，-1超时，-2错误
 */
int keyboard_wait_for_key(uint32_t timeout);

/* 内联函数 */
static inline int keyboard_is_output_buffer_full(void) {
    return inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUT_BUF_FULL;
}

static inline int keyboard_is_input_buffer_full(void) {
    return inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_IN_BUF_FULL;
}

static inline uint8_t keyboard_read_data(void) {
    return inb(KEYBOARD_DATA_PORT);
}

static inline void keyboard_write_data(uint8_t data) {
    outb(KEYBOARD_DATA_PORT, data);
}

static inline void keyboard_write_command(uint8_t command) {
    outb(KEYBOARD_COMMAND_PORT, command);
}

#endif /* _KEYBOARD_H */