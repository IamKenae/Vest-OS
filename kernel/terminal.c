/**
 * @file terminal.c
 * @brief 终端管理器实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <kernel/terminal.h>
#include <drivers/tty.h>
#include <drivers/vga.h>
#include <kernel/string.h>
#include <kernel/memory.h>
#include <kernel/spinlock.h>

/* 终端管理器全局实例 */
static terminal_manager_t terminal_manager;
static spinlock_t terminal_lock;

/* ANSI转义序列状态 */
#define ANSI_STATE_NONE        0
#define ANSI_STATE_ESCAPE      1
#define ANSI_STATE_CSI         2
#define ANSI_STATE_PARAM       3

/* ANSI控制序列 */
#define ANSI_CSI_CURSOR_UP     'A'
#define ANSI_CSI_CURSOR_DOWN   'B'
#define ANSI_CSI_CURSOR_RIGHT  'C'
#define ANSI_CSI_CURSOR_LEFT   'D'
#define ANSI_CSI_CURSOR_HOME   'H'
#define ANSI_CSI_CLEAR_SCREEN  'J'
#define ANSI_CSI_CLEAR_LINE    'K'
#define ANSI_CSI_COLOR         'm'

/* 内部函数声明 */
static int terminal_process_csi_sequence(terminal_t *terminal);
static void terminal_reset_escape_state(terminal_t *terminal);
static int terminal_execute_control(terminal_t *terminal, terminal_control_t ctrl,
                                   const char *params);

/**
 * @brief 初始化终端管理器
 */
int terminal_manager_init(void) {
    if (terminal_manager.initialized) {
        return 0;  /* 已经初始化 */
    }

    /* 初始化锁 */
    spinlock_init(&terminal_lock);

    /* 清零管理器 */
    memset(&terminal_manager, 0, sizeof(terminal_manager));

    /* 初始化TTY系统 */
    if (tty_init() != 0) {
        return -1;
    }

    /* 创建默认控制台终端 */
    terminal_t *console = terminal_create("console", TERMINAL_TYPE_CONSOLE, 0);
    if (!console) {
        return -1;
    }

    /* 设置为活动终端 */
    terminal_manager.active_terminal = 0;
    terminal_manager.focused_terminal = 0;
    console->state = TERMINAL_STATE_FOCUSED;

    terminal_manager.next_session_id = 1;
    terminal_manager.initialized = 1;

    return 0;
}

/**
 * @brief 创建终端
 */
terminal_t *terminal_create(const char *name, terminal_type_t type, int tty_minor) {
    if (!name || !tty_is_valid_minor(tty_minor)) {
        return NULL;
    }

    spinlock_lock(&terminal_lock);

    /* 查找空闲槽位 */
    int slot = -1;
    for (int i = 0; i < MAX_TTYS; i++) {
        if (terminal_manager.terminals[i].state == TERMINAL_STATE_INACTIVE) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        spinlock_unlock(&terminal_lock);
        return NULL;  /* 没有空闲槽位 */
    }

    terminal_t *terminal = &terminal_manager.terminals[slot];

    /* 初始化终端 */
    memset(terminal, 0, sizeof(terminal_t));
    strncpy(terminal->name, name, sizeof(terminal->name) - 1);
    terminal->type = type;
    terminal->state = TERMINAL_STATE_ACTIVE;
    terminal->tty_minor = tty_minor;
    terminal->session_id = 0;
    terminal->process_id = 0;

    /* 设置默认属性 */
    terminal->width = VGA_WIDTH;
    terminal->height = VGA_HEIGHT;
    terminal->default_fg = VGA_COLOR_LIGHT_GREY;
    terminal->default_bg = VGA_COLOR_BLACK;

    /* 设置功能标志 */
    terminal->auto_wrap = 1;
    terminal->insert_mode = 0;
    terminal->cursor_visible = 1;
    terminal->bell_enabled = 1;
    terminal->history_enabled = 1;

    /* 重置转义序列状态 */
    terminal_reset_escape_state(terminal);

    /* 打开关联的TTY */
    if (tty_open(tty_minor) != 0) {
        spinlock_unlock(&terminal_lock);
        return NULL;
    }

    terminal_manager.terminal_count++;
    spinlock_unlock(&terminal_lock);

    return terminal;
}

/**
 * @brief 销毁终端
 */
int terminal_destroy(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    spinlock_lock(&terminal_lock);

    /* 关闭关联的TTY */
    tty_close(terminal->tty_minor);

    /* 清空终端 */
    memset(terminal, 0, sizeof(terminal_t));
    terminal->state = TERMINAL_STATE_INACTIVE;

    terminal_manager.terminal_count--;
    spinlock_unlock(&terminal_lock);

    return 0;
}

/**
 * @brief 查找终端
 */
terminal_t *terminal_find_by_name(const char *name) {
    if (!name) {
        return NULL;
    }

    spinlock_lock(&terminal_lock);

    for (int i = 0; i < MAX_TTYS; i++) {
        terminal_t *terminal = &terminal_manager.terminals[i];
        if (terminal->state != TERMINAL_STATE_INACTIVE &&
            strcmp(terminal->name, name) == 0) {
            spinlock_unlock(&terminal_lock);
            return terminal;
        }
    }

    spinlock_unlock(&terminal_lock);
    return NULL;
}

/**
 * @brief 根据TTY查找终端
 */
terminal_t *terminal_find_by_tty(int tty_minor) {
    if (!tty_is_valid_minor(tty_minor)) {
        return NULL;
    }

    spinlock_lock(&terminal_lock);

    for (int i = 0; i < MAX_TTYS; i++) {
        terminal_t *terminal = &terminal_manager.terminals[i];
        if (terminal->state != TERMINAL_STATE_INACTIVE &&
            terminal->tty_minor == tty_minor) {
            spinlock_unlock(&terminal_lock);
            return terminal;
        }
    }

    spinlock_unlock(&terminal_lock);
    return NULL;
}

/**
 * @brief 切换活动终端
 */
int terminal_switch_to(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    spinlock_lock(&terminal_lock);

    /* 查找终端索引 */
    int index = -1;
    for (int i = 0; i < MAX_TTYS; i++) {
        if (&terminal_manager.terminals[i] == terminal) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        spinlock_unlock(&terminal_lock);
        return -1;
    }

    /* 保存当前终端状态 */
    if (terminal_manager.active_terminal >= 0) {
        terminal_t *current = &terminal_manager.terminals[terminal_manager.active_terminal];
        if (current->state == TERMINAL_STATE_FOCUSED) {
            current->state = TERMINAL_STATE_ACTIVE;
        }
    }

    /* 切换TTY */
    tty_switch(terminal->tty_minor);

    /* 更新状态 */
    terminal->state = TERMINAL_STATE_FOCUSED;
    terminal_manager.active_terminal = index;
    terminal_manager.focused_terminal = index;

    spinlock_unlock(&terminal_lock);

    return 0;
}

/**
 * @brief 设置焦点终端
 */
int terminal_set_focus(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    /* 设置焦点终端 */
    terminal_manager.focused_terminal = terminal - terminal_manager.terminals;

    return 0;
}

/**
 * @brief 获取当前活动终端
 */
terminal_t *terminal_get_active(void) {
    if (terminal_manager.active_terminal < 0) {
        return NULL;
    }

    return &terminal_manager.terminals[terminal_manager.active_terminal];
}

/**
 * @brief 获取当前焦点终端
 */
terminal_t *terminal_get_focused(void) {
    if (terminal_manager.focused_terminal < 0) {
        return NULL;
    }

    return &terminal_manager.terminals[terminal_manager.focused_terminal];
}

/**
 * @brief 写入终端数据
 */
int terminal_write(terminal_t *terminal, const char *data, size_t size) {
    if (!terminal || !data || size == 0) {
        return -1;
    }

    size_t written = 0;

    for (size_t i = 0; i < size; i++) {
        /* 处理ANSI转义序列 */
        if (terminal_parse_ansi_escape(terminal, data[i]) == 0) {
            written++;
        }
    }

    /* 直接写入TTY */
    return tty_write(terminal->tty_minor, data, size);
}

/**
 * @brief 从终端读取数据
 */
int terminal_read(terminal_t *terminal, char *buffer, size_t size) {
    if (!terminal || !buffer || size == 0) {
        return -1;
    }

    return tty_read(terminal->tty_minor, buffer, size);
}

/**
 * @brief 解析ANSI转义序列
 */
int terminal_parse_ansi_escape(terminal_t *terminal, char ch) {
    switch (terminal->escape_state) {
        case ANSI_STATE_NONE:
            if (ch == '\033') {
                terminal->escape_state = ANSI_STATE_ESCAPE;
                terminal->escape_pos = 0;
                terminal->escape_buffer[terminal->escape_pos++] = ch;
                return 1;  /* 消费字符 */
            }
            break;

        case ANSI_STATE_ESCAPE:
            terminal->escape_buffer[terminal->escape_pos++] = ch;
            if (ch == '[') {
                terminal->escape_state = ANSI_STATE_CSI;
                return 1;  /* 消费字符 */
            } else {
                /* 简单转义序列 */
                terminal_reset_escape_state(terminal);
                return 1;
            }
            break;

        case ANSI_STATE_CSI:
            terminal->escape_buffer[terminal->escape_pos++] = ch;

            if (ch >= 'A' && ch <= 'Z') {
                /* CSI序列结束 */
                terminal->escape_buffer[terminal->escape_pos] = '\0';
                terminal_process_csi_sequence(terminal);
                terminal_reset_escape_state(terminal);
                return 1;  /* 消费字符 */
            } else if (ch >= 'a' && ch <= 'z') {
                /* CSI序列结束 */
                terminal->escape_buffer[terminal->escape_pos] = '\0';
                terminal_process_csi_sequence(terminal);
                terminal_reset_escape_state(terminal);
                return 1;  /* 消费字符 */
            }
            break;
    }

    return 0;  /* 不消费字符 */
}

/**
 * @brief 处理CSI序列
 */
static int terminal_process_csi_sequence(terminal_t *terminal) {
    char *params = terminal->escape_buffer + 2;  /* 跳过ESC[ */
    char cmd = terminal->escape_buffer[terminal->escape_pos - 1];

    switch (cmd) {
        case ANSI_CSI_CURSOR_UP:
            return terminal_execute_control(terminal, TERM_CTRL_CURSOR_UP, params);
        case ANSI_CSI_CURSOR_DOWN:
            return terminal_execute_control(terminal, TERM_CTRL_CURSOR_DOWN, params);
        case ANSI_CSI_CURSOR_RIGHT:
            return terminal_execute_control(terminal, TERM_CTRL_CURSOR_RIGHT, params);
        case ANSI_CSI_CURSOR_LEFT:
            return terminal_execute_control(terminal, TERM_CTRL_CURSOR_LEFT, params);
        case ANSI_CSI_CURSOR_HOME:
            return terminal_execute_control(terminal, TERM_CTRL_CURSOR_HOME, params);
        case ANSI_CSI_CLEAR_SCREEN:
            return terminal_execute_control(terminal, TERM_CTRL_CLEAR_SCREEN, params);
        case ANSI_CSI_CLEAR_LINE:
            return terminal_execute_control(terminal, TERM_CTRL_CLEAR_LINE, params);
        case ANSI_CSI_COLOR:
            return terminal_execute_control(terminal, TERM_CTRL_COLOR_SET, params);
        default:
            break;
    }

    return 0;
}

/**
 * @brief 执行控制命令
 */
static int terminal_execute_control(terminal_t *terminal, terminal_control_t ctrl,
                                   const char *params) {
    int param1 = 1, param2 = 1;

    /* 解析参数 */
    if (params) {
        sscanf(params, "%d;%d", &param1, &param2);
    }

    switch (ctrl) {
        case TERM_CTRL_CURSOR_UP:
            for (int i = 0; i < param1 && terminal->cursor.y > 0; i++) {
                terminal->cursor.y--;
            }
            tty_set_cursor(terminal->tty_minor, terminal->cursor.x, terminal->cursor.y);
            break;

        case TERM_CTRL_CURSOR_DOWN:
            for (int i = 0; i < param1 && terminal->cursor.y < terminal->height - 1; i++) {
                terminal->cursor.y++;
            }
            tty_set_cursor(terminal->tty_minor, terminal->cursor.x, terminal->cursor.y);
            break;

        case TERM_CTRL_CURSOR_RIGHT:
            for (int i = 0; i < param1 && terminal->cursor.x < terminal->width - 1; i++) {
                terminal->cursor.x++;
            }
            tty_set_cursor(terminal->tty_minor, terminal->cursor.x, terminal->cursor.y);
            break;

        case TERM_CTRL_CURSOR_LEFT:
            for (int i = 0; i < param1 && terminal->cursor.x > 0; i++) {
                terminal->cursor.x--;
            }
            tty_set_cursor(terminal->tty_minor, terminal->cursor.x, terminal->cursor.y);
            break;

        case TERM_CTRL_CURSOR_HOME:
            terminal->cursor.x = param2 - 1;
            terminal->cursor.y = param1 - 1;
            tty_set_cursor(terminal->tty_minor, terminal->cursor.x, terminal->cursor.y);
            break;

        case TERM_CTRL_CLEAR_SCREEN:
            if (param1 == 2) {
                terminal_clear_screen(terminal);
            }
            break;

        case TERM_CTRL_CLEAR_LINE:
            /* 这里可以实现清除行的逻辑 */
            break;

        case TERM_CTRL_COLOR_SET:
            /* 处理颜色设置 */
            switch (param1) {
                case 0:  /* 重置颜色 */
                    terminal_set_colors(terminal, terminal->default_fg, terminal->default_bg);
                    break;
                case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
                    /* 前景色 */
                    terminal_set_colors(terminal, param1 - 30, terminal->default_bg);
                    break;
                case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
                    /* 背景色 */
                    terminal_set_colors(terminal, terminal->default_fg, param1 - 40);
                    break;
            }
            break;

        default:
            break;
    }

    return 0;
}

/**
 * @brief 重置转义序列状态
 */
static void terminal_reset_escape_state(terminal_t *terminal) {
    terminal->escape_state = ANSI_STATE_NONE;
    terminal->escape_pos = 0;
    memset(terminal->escape_buffer, 0, sizeof(terminal->escape_buffer));
}

/**
 * @brief 设置终端大小
 */
int terminal_set_size(terminal_t *terminal, uint8_t width, uint8_t height) {
    if (!terminal || width == 0 || height == 0) {
        return -1;
    }

    terminal->width = width;
    terminal->height = height;

    return 0;
}

/**
 * @brief 设置终端颜色
 */
int terminal_set_colors(terminal_t *terminal, vga_color_t foreground, vga_color_t background) {
    if (!terminal) {
        return -1;
    }

    terminal->default_fg = foreground;
    terminal->default_bg = background;

    return tty_set_color(terminal->tty_minor, foreground, background);
}

/**
 * @brief 清除终端屏幕
 */
int terminal_clear_screen(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    return tty_clear(terminal->tty_minor);
}

/**
 * @brief 移动光标
 */
int terminal_move_cursor(terminal_t *terminal, uint8_t x, uint8_t y) {
    if (!terminal) {
        return -1;
    }

    terminal->cursor.x = x;
    terminal->cursor.y = y;

    return tty_set_cursor(terminal->tty_minor, x, y);
}

/**
 * @brief 显示光标
 */
int terminal_show_cursor(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    terminal->cursor_visible = 1;
    return 0;
}

/**
 * @brief 隐藏光标
 */
int terminal_hide_cursor(terminal_t *terminal) {
    if (!terminal) {
        return -1;
    }

    terminal->cursor_visible = 0;
    return 0;
}

/**
 * @brief 响铃
 */
int terminal_bell(terminal_t *terminal) {
    if (!terminal || !terminal->bell_enabled) {
        return -1;
    }

    /* 这里可以实现系统响铃 */
    return 0;
}

/**
 * @brief 创建终端会话
 */
terminal_session_t *terminal_session_create(terminal_t *terminal, uint32_t user_id,
                                           const char *username, const char *shell) {
    if (!terminal || !username || !shell) {
        return NULL;
    }

    /* 分配会话结构 */
    terminal_session_t *session = kmalloc(sizeof(terminal_session_t));
    if (!session) {
        return NULL;
    }

    /* 初始化会话 */
    memset(session, 0, sizeof(terminal_session_t));
    session->session_id = terminal_manager.next_session_id++;
    session->user_id = user_id;
    session->group_id = user_id;  /* 简化处理 */
    strncpy(session->username, username, sizeof(session->username) - 1);
    strncpy(session->shell, shell, sizeof(session->shell) - 1);
    strncpy(session->working_dir, "/", sizeof(session->working_dir) - 1);
    session->terminal = terminal;

    /* 关联终端和会话 */
    terminal->session_id = session->session_id;

    /* 添加到会话链表 */
    session->next = terminal_manager.sessions;
    terminal_manager.sessions = session;

    return session;
}

/**
 * @brief 终端状态变化通知
 */
void terminal_notify_state_change(terminal_t *terminal, terminal_state_t old_state,
                                 terminal_state_t new_state) {
    /* 这里可以实现状态变化的通知逻辑 */
}

/**
 * @brief 创建终端会话
 */
terminal_session_t *terminal_session_create(terminal_t *terminal, uint32_t user_id,
                                           const char *username, const char *shell) {
    if (!terminal || !username || !shell) {
        return NULL;
    }

    terminal_session_t *session = kmalloc(sizeof(terminal_session_t));
    if (!session) {
        return NULL;
    }

    memset(session, 0, sizeof(terminal_session_t));
    session->session_id = terminal_manager.next_session_id++;
    session->user_id = user_id;
    session->group_id = user_id;
    strncpy(session->username, username, sizeof(session->username) - 1);
    strncpy(session->shell, shell, sizeof(session->shell) - 1);
    strncpy(session->working_dir, "/", sizeof(session->working_dir) - 1);
    session->terminal = terminal;

    terminal->session_id = session->session_id;
    session->next = terminal_manager.sessions;
    terminal_manager.sessions = session;

    return session;
}