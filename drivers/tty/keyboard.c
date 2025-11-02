/**
 * @file keyboard.c
 * @brief 键盘输入驱动实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <drivers/keyboard.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <string.h>

/* 当前扫描码集合 */
static scancode_set_t current_scancode_set = SCANCODE_SET_1;

/* 键盘状态 */
static modifier_keys_t keyboard_modifiers = {0};
static uint8_t keyboard_leds = 0;
static keyboard_handler_t user_handler = NULL;

/* 键盘缓冲区 */
static keyboard_buffer_t keyboard_buffer;

/* 扫描码映射表 - 美国键盘布局 */
static const char scancode_to_ascii_table_set1[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, '5', 0, '+', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_to_ascii_shift_table_set1[128] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, '5', 0, '+', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 扩展键扫描码 */
static const uint8_t extended_scancodes[] = {
    0x1C,  /* Enter (小键盘) */
    0x1D,  /* Right Ctrl */
    0x35,  /* / (小键盘) */
    0x37,  /* Print Screen */
    0x38,  /* Right Alt */
    0x46,  /* Break */
    0x47,  /* Home */
    0x48,  /* Up */
    0x49,  /* Page Up */
    0x4B,  /* Left */
    0x4D,  /* Right */
    0x4F,  /* End */
    0x50,  /* Down */
    0x51,  /* Page Down */
    0x52,  /* Insert */
    0x53   /* Delete */
};

/* 内部函数声明 */
static void process_scancode(uint8_t scancode);
static void update_modifiers(uint8_t scancode, key_state_t state);
static int is_modifier_key(uint8_t scancode);
static void buffer_push_event(const keyboard_event_t *event);
static int buffer_pop_event(keyboard_event_t *event);
static int wait_for_keyboard_ready(void);

/**
 * @brief 初始化键盘驱动
 */
int keyboard_init(void) {
    /* 初始化缓冲区 */
    memset(&keyboard_buffer, 0, sizeof(keyboard_buffer));

    /* 初始化修饰键状态 */
    memset(&keyboard_modifiers, 0, sizeof(keyboard_modifiers));
    keyboard_leds = 0;

    /* 重置键盘 */
    if (keyboard_reset() != 0) {
        return -1;
    }

    /* 启用键盘 */
    keyboard_enable();

    /* 设置LED状态 */
    keyboard_set_leds(keyboard_leds);

    return 0;
}

/**
 * @brief 键盘中断处理函数
 */
void keyboard_interrupt_handler(void) {
    /* 检查输出缓冲区是否有数据 */
    if (!keyboard_is_output_buffer_full()) {
        return;
    }

    /* 读取扫描码 */
    uint8_t scancode = keyboard_read_data();

    /* 处理扫描码 */
    process_scancode(scancode);
}

/**
 * @brief 处理扫描码
 */
static void process_scancode(uint8_t scancode) {
    keyboard_event_t event;
    memset(&event, 0, sizeof(event));

    /* 确定按键状态 */
    if (scancode & 0x80) {
        event.state = KEY_RELEASED;
        scancode &= 0x7F;
    } else {
        event.state = KEY_PRESSED;
    }

    event.scancode = scancode;
    event.modifiers = keyboard_modifiers;

    /* 更新修饰键状态 */
    update_modifiers(scancode, event.state);

    /* 转换为ASCII */
    event.ascii = scancode_to_ascii(scancode, keyboard_modifiers);

    /* 调用用户处理器 */
    if (user_handler) {
        user_handler(&event);
    }

    /* 将事件加入缓冲区 */
    buffer_push_event(&event);
}

/**
 * @brief 更新修饰键状态
 */
static void update_modifiers(uint8_t scancode, key_state_t state) {
    switch (scancode) {
        case 0x2A:  /* Left Shift */
            keyboard_modifiers.left_shift = (state == KEY_PRESSED);
            break;
        case 0x36:  /* Right Shift */
            keyboard_modifiers.right_shift = (state == KEY_PRESSED);
            break;
        case 0x1D:  /* Ctrl */
            keyboard_modifiers.left_ctrl = (state == KEY_PRESSED);
            break;
        case 0x38:  /* Alt */
            keyboard_modifiers.left_alt = (state == KEY_PRESSED);
            break;
        case 0x3A:  /* Caps Lock */
            if (state == KEY_PRESSED) {
                keyboard_modifiers.caps_lock = !keyboard_modifiers.caps_lock;
                keyboard_leds ^= KEYBOARD_LED_CAPS_LOCK;
                keyboard_set_leds(keyboard_leds);
            }
            break;
        case 0x45:  /* Num Lock */
            if (state == KEY_PRESSED) {
                keyboard_modifiers.num_lock = !keyboard_modifiers.num_lock;
                keyboard_leds ^= KEYBOARD_LED_NUM_LOCK;
                keyboard_set_leds(keyboard_leds);
            }
            break;
        case 0x46:  /* Scroll Lock */
            if (state == KEY_PRESSED) {
                keyboard_modifiers.scroll_lock = !keyboard_modifiers.scroll_lock;
                keyboard_leds ^= KEYBOARD_LED_SCROLL_LOCK;
                keyboard_set_leds(keyboard_leds);
            }
            break;
    }
}

/**
 * @brief 扫描码转ASCII
 */
char scancode_to_ascii(uint8_t scancode, modifier_keys_t modifiers) {
    if (scancode >= 128) {
        return 0;
    }

    char ascii = 0;

    if (modifiers.left_shift || modifiers.right_shift) {
        ascii = scancode_to_ascii_shift_table_set1[scancode];
    } else {
        ascii = scancode_to_ascii_table_set1[scancode];
    }

    /* 处理Caps Lock */
    if (modifiers.caps_lock && ascii >= 'a' && ascii <= 'z') {
        ascii = ascii - 'a' + 'A';
    } else if (modifiers.caps_lock && ascii >= 'A' && ascii <= 'Z') {
        ascii = ascii - 'A' + 'a';
    }

    /* 处理Num Lock */
    if (modifiers.num_lock) {
        switch (scancode) {
            case 0x47: ascii = '7'; break;  /* Home */
            case 0x48: ascii = '8'; break;  /* Up */
            case 0x49: ascii = '9'; break;  /* Page Up */
            case 0x4B: ascii = '4'; break;  /* Left */
            case 0x4C: ascii = '5'; break;  /* Center */
            case 0x4D: ascii = '6'; break;  /* Right */
            case 0x4F: ascii = '1'; break;  /* End */
            case 0x50: ascii = '2'; break;  /* Down */
            case 0x51: ascii = '3'; break;  /* Page Down */
            case 0x52: ascii = '0'; break;  /* Insert */
            case 0x53: ascii = '.'; break;  /* Delete */
        }
    }

    return ascii;
}

/**
 * @brief 检查是否为扩展键
 */
int is_extended_key(uint8_t scancode) {
    for (size_t i = 0; i < sizeof(extended_scancodes); i++) {
        if (extended_scancodes[i] == scancode) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 读取键盘事件
 */
int keyboard_read_event(keyboard_event_t *event) {
    if (!event) {
        return -2;
    }

    return buffer_pop_event(event);
}

/**
 * @brief 检查是否有键盘事件
 */
int keyboard_has_event(void) {
    return keyboard_buffer.count > 0;
}

/**
 * @brief 清空键盘缓冲区
 */
void keyboard_flush(void) {
    keyboard_buffer.head = 0;
    keyboard_buffer.tail = 0;
    keyboard_buffer.count = 0;
}

/**
 * @brief 设置LED状态
 */
void keyboard_set_leds(uint8_t leds) {
    keyboard_leds = leds;

    /* 等待输入缓冲区为空 */
    if (wait_for_keyboard_ready() != 0) {
        return;
    }

    /* 发送设置LED命令 */
    keyboard_write_command(KEYBOARD_CMD_SET_LEDS);

    /* 等待键盘响应 */
    if (wait_for_keyboard_ready() != 0) {
        return;
    }

    /* 发送LED状态 */
    keyboard_write_data(leds);
}

/**
 * @brief 获取LED状态
 */
uint8_t keyboard_get_leds(void) {
    return keyboard_leds;
}

/**
 * @brief 设置重复率
 */
int keyboard_set_repeat_rate(uint8_t delay, uint8_t rate) {
    if (delay > 3 || rate > 31) {
        return -1;
    }

    /* 等待输入缓冲区为空 */
    if (wait_for_keyboard_ready() != 0) {
        return -1;
    }

    /* 发送设置重复率命令 */
    keyboard_write_command(KEYBOARD_CMD_SET_RATE);

    /* 等待键盘响应 */
    if (wait_for_keyboard_ready() != 0) {
        return -1;
    }

    /* 发送重复率参数 */
    uint8_t param = (delay << 5) | rate;
    keyboard_write_data(param);

    return 0;
}

/**
 * @brief 启用键盘
 */
void keyboard_enable(void) {
    if (wait_for_keyboard_ready() != 0) {
        return;
    }
    keyboard_write_command(KEYBOARD_CMD_ENABLE);
}

/**
 * @brief 禁用键盘
 */
void keyboard_disable(void) {
    if (wait_for_keyboard_ready() != 0) {
        return;
    }
    keyboard_write_command(KEYBOARD_CMD_DISABLE);
}

/**
 * @brief 重置键盘
 */
int keyboard_reset(void) {
    if (wait_for_keyboard_ready() != 0) {
        return -1;
    }

    keyboard_write_command(KEYBOARD_CMD_RESET);

    /* 等待键盘响应 */
    for (int i = 0; i < 1000; i++) {
        if (keyboard_is_output_buffer_full()) {
            uint8_t response = keyboard_read_data();
            if (response == 0xAA) {  /* 自检成功 */
                return 0;
            } else if (response == 0xFC) {  /* 自检失败 */
                return -1;
            }
        }
    }

    return -1;  /* 超时 */
}

/**
 * @brief 获取修饰键状态
 */
modifier_keys_t keyboard_get_modifiers(void) {
    return keyboard_modifiers;
}

/**
 * @brief 设置键盘处理器
 */
int keyboard_set_handler(keyboard_handler_t handler) {
    user_handler = handler;
    return 0;
}

/**
 * @brief 移除键盘处理器
 */
void keyboard_remove_handler(void) {
    user_handler = NULL;
}

/**
 * @brief 缓冲区推送事件
 */
static void buffer_push_event(const keyboard_event_t *event) {
    if (keyboard_buffer.count >= KEYBOARD_BUFFER_SIZE) {
        /* 缓冲区满，丢弃最旧的事件 */
        keyboard_buffer.head = (keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
        keyboard_buffer.count--;
    }

    keyboard_buffer.buffer[keyboard_buffer.tail] = *event;
    keyboard_buffer.tail = (keyboard_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer.count++;
}

/**
 * @brief 缓冲区弹出事件
 */
static int buffer_pop_event(keyboard_event_t *event) {
    if (keyboard_buffer.count == 0) {
        return -1;  /* 缓冲区空 */
    }

    *event = keyboard_buffer.buffer[keyboard_buffer.head];
    keyboard_buffer.head = (keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer.count--;

    return 0;
}

/**
 * @brief 等待键盘就绪
 */
static int wait_for_keyboard_ready(void) {
    for (int i = 0; i < 100000; i++) {
        if (!keyboard_is_input_buffer_full()) {
            return 0;
        }
    }
    return -1;  /* 超时 */
}

/**
 * @brief 等待按键
 */
int keyboard_wait_for_key(uint32_t timeout) {
    uint32_t start_time = 0;  /* 这里需要系统时间支持 */

    while (1) {
        if (keyboard_has_event()) {
            return 0;
        }

        /* 检查超时 */
        if (timeout > 0) {
            uint32_t current_time = 0;  /* 这里需要系统时间支持 */
            if ((current_time - start_time) >= timeout) {
                return -1;  /* 超时 */
            }
        }
    }

    return 0;
}