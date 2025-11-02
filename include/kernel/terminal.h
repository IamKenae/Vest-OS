/**
 * @file terminal.h
 * @brief 终端管理器头文件
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdint.h>
#include <drivers/tty.h>

/* 终端类型 */
typedef enum {
    TERMINAL_TYPE_CONSOLE = 0,    /* 控制台终端 */
    TERMINAL_TYPE_SERIAL,          /* 串口终端 */
    TERMINAL_TYPE_NETWORK,         /* 网络终端 */
    TERMINAL_TYPE_VIRTUAL          /* 虚拟终端 */
} terminal_type_t;

/* 终端状态 */
typedef enum {
    TERMINAL_STATE_INACTIVE = 0,   /* 非活动 */
    TERMINAL_STATE_ACTIVE,         /* 活动 */
    TERMINAL_STATE_FOCUSED,        /* 焦点 */
    TERMINAL_STATE_SUSPENDED       /* 挂起 */
} terminal_state_t;

/* 终端控制序列类型 */
typedef enum {
    TERM_CTRL_NONE = 0,
    TERM_CTRL_CURSOR_UP,           /* 光标上移 */
    TERM_CTRL_CURSOR_DOWN,         /* 光标下移 */
    TERM_CTRL_CURSOR_LEFT,         /* 光标左移 */
    TERM_CTRL_CURSOR_RIGHT,        /* 光标右移 */
    TERM_CTRL_CURSOR_HOME,         /* 光标归位 */
    TERM_CTRL_CURSOR_END,          /* 光标到行尾 */
    TERM_CTRL_CLEAR_LINE,          /* 清除行 */
    TERM_CTRL_CLEAR_SCREEN,        /* 清除屏幕 */
    TERM_CTRL_SAVE_CURSOR,         /* 保存光标 */
    TERM_CTRL_RESTORE_CURSOR,      /* 恢复光标 */
    TERM_CTRL_SCROLL_UP,           /* 向上滚动 */
    TERM_CTRL_SCROLL_DOWN,         /* 向下滚动 */
    TERM_CTRL_COLOR_SET,           /* 设置颜色 */
    TERM_CTRL_COLOR_RESET          /* 重置颜色 */
} terminal_control_t;

/* 终端属性 */
typedef struct {
    terminal_type_t type;          /* 终端类型 */
    terminal_state_t state;        /* 终端状态 */
    char name[32];                 /* 终端名称 */
    char description[128];         /* 终端描述 */
    int tty_minor;                 /* 关联的TTY次设备号 */
    uint32_t session_id;           /* 会话ID */
    uint32_t process_id;           /* 进程ID */

    /* 显示属性 */
    uint8_t width;                 /* 终端宽度 */
    uint8_t height;                /* 终端高度 */
    vga_color_t default_fg;        /* 默认前景色 */
    vga_color_t default_bg;        /* 默认背景色 */

    /* 功能标志 */
    uint8_t auto_wrap : 1;         /* 自动换行 */
    uint8_t insert_mode : 1;       /* 插入模式 */
    uint8_t cursor_visible : 1;    /* 光标可见 */
    uint8_t bell_enabled : 1;      /* 响铃使能 */
    uint8_t history_enabled : 1;   /* 历史记录使能 */
    uint8_t reserved : 2;

    /* 控制序列解析状态 */
    uint8_t escape_state;          /* 转义状态 */
    char escape_buffer[32];        /* 转义序列缓冲区 */
    uint8_t escape_pos;            /* 转义序列位置 */
} terminal_t;

/* 终端会话 */
typedef struct {
    uint32_t session_id;           /* 会话ID */
    uint32_t user_id;              /* 用户ID */
    uint32_t group_id;             /* 组ID */
    char username[32];             /* 用户名 */
    char working_dir[256];         /* 工作目录 */
    char shell[64];                /* Shell程序 */
    terminal_t *terminal;          /* 关联终端 */
    struct terminal_session *next; /* 下一个会话 */
} terminal_session_t;

/* 终端历史记录 */
typedef struct {
    char buffer[1024];             /* 历史记录缓冲区 */
    size_t length;                 /* 记录长度 */
    struct terminal_history *next; /* 下一个记录 */
} terminal_history_t;

/* 终端管理器 */
typedef struct {
    terminal_t terminals[MAX_TTYS]; /* 终端数组 */
    int terminal_count;             /* 终端数量 */
    int active_terminal;            /* 活动终端 */
    int focused_terminal;           /* 焦点终端 */

    terminal_session_t *sessions;   /* 会话链表 */
    uint32_t next_session_id;       /* 下一个会话ID */

    uint8_t initialized;            /* 初始化标志 */
} terminal_manager_t;

/* 函数声明 */

/**
 * @brief 初始化终端管理器
 * @return 0成功，-1失败
 */
int terminal_manager_init(void);

/**
 * @brief 创建终端
 * @param name 终端名称
 * @param type 终端类型
 * @param tty_minor 关联的TTY次设备号
 * @return 终端指针，NULL失败
 */
terminal_t *terminal_create(const char *name, terminal_type_t type, int tty_minor);

/**
 * @brief 销毁终端
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_destroy(terminal_t *terminal);

/**
 * @brief 查找终端
 * @param name 终端名称
 * @return 终端指针，NULL未找到
 */
terminal_t *terminal_find_by_name(const char *name);

/**
 * @brief 根据TTY查找终端
 * @param tty_minor TTY次设备号
 * @return 终端指针，NULL未找到
 */
terminal_t *terminal_find_by_tty(int tty_minor);

/**
 * @brief 切换活动终端
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_switch_to(terminal_t *terminal);

/**
 * @brief 设置焦点终端
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_set_focus(terminal_t *terminal);

/**
 * @brief 获取当前活动终端
 * @return 终端指针
 */
terminal_t *terminal_get_active(void);

/**
 * @brief 获取当前焦点终端
 * @return 终端指针
 */
terminal_t *terminal_get_focused(void);

/**
 * @brief 写入终端数据
 * @param terminal 终端指针
 * @param data 数据
 * @param size 数据大小
 * @return 实际写入大小，-1失败
 */
int terminal_write(terminal_t *terminal, const char *data, size_t size);

/**
 * @brief 从终端读取数据
 * @param terminal 终端指针
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 实际读取大小，-1失败
 */
int terminal_read(terminal_t *terminal, char *buffer, size_t size);

/**
 * @brief 处理终端控制序列
 * @param terminal 终端指针
 * @param sequence 控制序列
 * @return 0成功，-1失败
 */
int terminal_process_control_sequence(terminal_t *terminal, const char *sequence);

/**
 * @brief 解析ANSI转义序列
 * @param terminal 终端指针
 * @param ch 字符
 * @return 0成功，-1失败
 */
int terminal_parse_ansi_escape(terminal_t *terminal, char ch);

/**
 * @brief 设置终端大小
 * @param terminal 终端指针
 * @param width 宽度
 * @param height 高度
 * @return 0成功，-1失败
 */
int terminal_set_size(terminal_t *terminal, uint8_t width, uint8_t height);

/**
 * @brief 设置终端颜色
 * @param terminal 终端指针
 * @param foreground 前景色
 * @param background 背景色
 * @return 0成功，-1失败
 */
int terminal_set_colors(terminal_t *terminal, vga_color_t foreground, vga_color_t background);

/**
 * @brief 清除终端屏幕
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_clear_screen(terminal_t *terminal);

/**
 * @brief 移动光标
 * @param terminal 终端指针
 * @param x 列位置
 * @param y 行位置
 * @return 0成功，-1失败
 */
int terminal_move_cursor(terminal_t *terminal, uint8_t x, uint8_t y);

/**
 * @brief 显示光标
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_show_cursor(terminal_t *terminal);

/**
 * @brief 隐藏光标
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_hide_cursor(terminal_t *terminal);

/**
 * @brief 响铃
 * @param terminal 终端指针
 * @return 0成功，-1失败
 */
int terminal_bell(terminal_t *terminal);

/**
 * @brief 创建终端会话
 * @param terminal 终端指针
 * @param user_id 用户ID
 * @param username 用户名
 * @param shell Shell程序
 * @return 会话指针，NULL失败
 */
terminal_session_t *terminal_session_create(terminal_t *terminal, uint32_t user_id,
                                           const char *username, const char *shell);

/**
 * @brief 销毁终端会话
 * @param session 会话指针
 * @return 0成功，-1失败
 */
int terminal_session_destroy(terminal_session_t *session);

/**
 * @brief 查找会话
 * @param session_id 会话ID
 * @return 会话指针，NULL未找到
 */
terminal_session_t *terminal_session_find(uint32_t session_id);

/**
 * @brief 设置会话工作目录
 * @param session 会话指针
 * @param path 工作目录路径
 * @return 0成功，-1失败
 */
int terminal_session_set_workdir(terminal_session_t *session, const char *path);

/**
 * @brief 添加历史记录
 * @param terminal 终端指针
 * @param command 命令
 * @return 0成功，-1失败
 */
int terminal_add_history(terminal_t *terminal, const char *command);

/**
 * @brief 获取历史记录
 * @param terminal 终端指针
 * @param index 索引
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 0成功，-1失败
 */
int terminal_get_history(terminal_t *terminal, int index, char *buffer, size_t size);

/**
 * @brief 终端状态变化通知
 * @param terminal 终端指针
 * @param old_state 旧状态
 * @param new_state 新状态
 */
void terminal_notify_state_change(terminal_t *terminal, terminal_state_t old_state,
                                 terminal_state_t new_state);

/* 内联函数 */
static inline int terminal_is_active(terminal_t *terminal) {
    return (terminal && terminal->state == TERMINAL_STATE_ACTIVE);
}

static inline int terminal_is_focused(terminal_t *terminal) {
    return (terminal && terminal->state == TERMINAL_STATE_FOCUSED);
}

static inline int terminal_has_session(terminal_t *terminal) {
    return (terminal && terminal->session_id != 0);
}

#endif /* _TERMINAL_H */