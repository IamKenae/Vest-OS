/**
 * @file tty.h
 * @brief TTY用户空间接口
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _SYS_TTY_H
#define _SYS_TTY_H

#include <sys/types.h>
#include <sys/termios.h>

/* TTY设备文件名 */
#define TTY_CONSOLE     "/dev/tty0"     /* 控制台终端 */
#define TTY1            "/dev/tty1"     /* 虚拟终端1 */
#define TTY2            "/dev/tty2"     /* 虚拟终端2 */
#define TTY3            "/dev/tty3"     /* 虚拟终端3 */
#define TTY4            "/dev/tty4"     /* 虚拟终端4 */
#define TTY5            "/dev/tty5"     /* 虚拟终端5 */
#define TTY6            "/dev/tty6"     /* 虚拟终端6 */
#define TTY7            "/dev/tty7"     /* 虚拟终端7 */
#define PTY_MASTER      "/dev/ptmx"     /* 伪终端主设备 */
#define PTY_SLAVE       "/dev/pts/%d"   /* 伪终端从设备 */

/* TTY设备号 */
#define TTY_MAJOR       4
#define TTY_CONSOLE_MINOR  0

/* TTY控制命令 */
#define TTY_IOC_GET    0x5401   /* 获取TTY信息 */
#define TTY_IOC_SET    0x5402   /* 设置TTY信息 */
#define TTY_IOC_BREAK  0x5403   /* 发送BREAK */
#define TTY_IOC_START  0x5404   /* 重启输出 */
#define TTY_IOC_STOP   0x5405   /* 停止输出 */
#define TTY_IOC_FLUSH  0x5406   /* 刷新队列 */

/* TTY信息结构 */
struct tty_info {
    char name[32];          /* TTY名称 */
    char type[16];          /* TTY类型 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    pid_t session;          /* 会话ID */
    pid_t pgrp;             /* 进程组ID */
    struct winsize winsize; /* 窗口大小 */
    int flags;              /* 标志 */
};

/* TTY标志 */
#define TTY_FLAG_CONSOLE   0x01   /* 控制台终端 */
#define TTY_FLAG_VIRTUAL   0x02   /* 虚拟终端 */
#define TTY_FLAG_SERIAL    0x04   /* 串口终端 */
#define TTY_FLAG_PTY       0x08   /* 伪终端 */
#define TTY_FLAG_NETWORK   0x10   /* 网络终端 */

/* 函数声明 */

/**
 * @brief 打开TTY设备
 * @param tty_name TTY设备名称
 * @param flags 打开标志
 * @return 文件描述符，-1失败
 */
int tty_open(const char *tty_name, int flags);

/**
 * @brief 关闭TTY设备
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_close(int fd);

/**
 * @brief 读取TTY数据
 * @param fd 文件描述符
 * @param buffer 缓冲区
 * @param count 要读取的字节数
 * @return 实际读取字节数，-1失败
 */
ssize_t tty_read(int fd, void *buffer, size_t count);

/**
 * @brief 写入TTY数据
 * @param fd 文件描述符
 * @param buffer 数据缓冲区
 * @param count 要写入的字节数
 * @return 实际写入字节数，-1失败
 */
ssize_t tty_write(int fd, const void *buffer, size_t count);

/**
 * @brief 获取TTY信息
 * @param fd 文件描述符
 * @param info TTY信息结构
 * @return 0成功，-1失败
 */
int tty_get_info(int fd, struct tty_info *info);

/**
 * @brief 设置TTY信息
 * @param fd 文件描述符
 * @param info TTY信息结构
 * @return 0成功，-1失败
 */
int tty_set_info(int fd, const struct tty_info *info);

/**
 * @brief 发送BREAK信号
 * @param fd 文件描述符
 * @param duration 持续时间(毫秒)
 * @return 0成功，-1失败
 */
int tty_send_break(int fd, int duration);

/**
 * @brief 重启TTY输出
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_start_output(int fd);

/**
 * @brief 停止TTY输出
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_stop_output(int fd);

/**
 * @brief 刷新TTY队列
 * @param fd 文件描述符
 * @param queue 队列选择(TCIFLUSH/TCOFLUSH/TCIOFLUSH)
 * @return 0成功，-1失败
 */
int tty_flush(int fd, int queue);

/**
 * @brief 获取当前TTY
 * @return TTY名称，NULL失败
 */
char *tty_get_current(void);

/**
 * @brief 切换TTY
 * @param tty_name 目标TTY名称
 * @return 0成功，-1失败
 */
int tty_switch(const char *tty_name);

/**
 * @brief 创建伪终端对
 * @param master 主端文件描述符指针
 * @param slave 从端文件描述符指针
 * @param name 从端设备名存储位置
 * @param termp 终端属性
 * @param winp 窗口大小
 * @return 0成功，-1失败
 */
int tty_create_pty(int *master, int *slave, char *name,
                  const struct termios *termp, const struct winsize *winp);

/**
 * @brief 检查是否为TTY设备
 * @param fd 文件描述符
 * @return 1是TTY，0不是TTY
 */
int tty_isatty(int fd);

/**
 * @brief 获取TTY名称
 * @param fd 文件描述符
 * @param name 名称存储位置
 * @param size 缓冲区大小
 * @return 0成功，-1失败
 */
int tty_get_name(int fd, char *name, size_t size);

/**
 * @brief 设置TTY窗口大小
 * @param fd 文件描述符
 * @param rows 行数
 * @param cols 列数
 * @return 0成功，-1失败
 */
int tty_set_winsize(int fd, int rows, int cols);

/**
 * @brief 获取TTY窗口大小
 * @param fd 文件描述符
 * @param rows 行数存储位置
 * @param cols 列数存储位置
 * @return 0成功，-1失败
 */
int tty_get_winsize(int fd, int *rows, int *cols);

/**
 * @brief 获取终端类型
 * @param fd 文件描述符
 * @param type 类型存储位置
 * @param size 缓冲区大小
 * @return 0成功，-1失败
 */
int tty_get_type(int fd, char *type, size_t size);

/**
 * @brief 设置终端超时
 * @param fd 文件描述符
 * @param timeout_sec 超时秒数
 * @param timeout_usec 超时微秒数
 * @return 0成功，-1失败
 */
int tty_set_timeout(int fd, int timeout_sec, int timeout_usec);

/**
 * @brief 终端原始模式设置
 * @param fd 文件描述符
 * @param original 原始termios存储位置
 * @return 0成功，-1失败
 */
int tty_set_raw_mode(int fd, struct termios *original);

/**
 * @brief 终端规范模式设置
 * @param fd 文件描述符
 * @param original 原始termios
 * @return 0成功，-1失败
 */
int tty_set_canonical_mode(int fd, const struct termios *original);

/**
 * @brief 设置终端回显
 * @param fd 文件描述符
 * @param echo 是否回显
 * @return 0成功，-1失败
 */
int tty_set_echo(int fd, int echo);

/**
 * @brief 设置终端信号处理
 * @param fd 文件描述符
 * @param signals 是否启用信号
 * @return 0成功，-1失败
 */
int tty_set_signals(int fd, int signals);

/**
 * @brief 终端输出缓冲区刷新
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_flush_output(int fd);

/**
 * @brief 终端输入缓冲区刷新
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_flush_input(int fd);

/**
 * @brief 等待终端输出完成
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tty_drain_output(int fd);

/* 便利宏定义 */
#define tty_is_console(fd) (tty_get_type(fd, NULL, 0) == 0 && \
                           strstr(tty_get_name(fd, NULL, 0), "tty0") != NULL)

#define tty_is_virtual(fd) (tty_get_type(fd, NULL, 0) == 0 && \
                           strstr(tty_get_name(fd, NULL, 0), "tty") != NULL && \
                           !tty_is_console(fd))

#define tty_is_pty(fd) (tty_get_type(fd, NULL, 0) == 0 && \
                       strstr(tty_get_name(fd, NULL, 0), "pts") != NULL)

#endif /* _SYS_TTY_H */