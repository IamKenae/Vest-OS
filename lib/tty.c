/**
 * @file tty.c
 * @brief TTY用户空间接口实现
 * @author Vest-OS Team
 * @date 2024
 */

#include <sys/tty.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief 打开TTY设备
 */
int tty_open(const char *tty_name, int flags) {
    if (!tty_name) {
        errno = EINVAL;
        return -1;
    }

    return open(tty_name, flags);
}

/**
 * @brief 关闭TTY设备
 */
int tty_close(int fd) {
    return close(fd);
}

/**
 * @brief 读取TTY数据
 */
ssize_t tty_read(int fd, void *buffer, size_t count) {
    if (!buffer || count == 0) {
        errno = EINVAL;
        return -1;
    }

    return read(fd, buffer, count);
}

/**
 * @brief 写入TTY数据
 */
ssize_t tty_write(int fd, const void *buffer, size_t count) {
    if (!buffer || count == 0) {
        errno = EINVAL;
        return -1;
    }

    return write(fd, buffer, count);
}

/**
 * @brief 获取TTY信息
 */
int tty_get_info(int fd, struct tty_info *info) {
    if (!info) {
        errno = EINVAL;
        return -1;
    }

    if (ioctl(fd, TTY_IOC_GET, info) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief 设置TTY信息
 */
int tty_set_info(int fd, const struct tty_info *info) {
    if (!info) {
        errno = EINVAL;
        return -1;
    }

    if (ioctl(fd, TTY_IOC_SET, info) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief 发送BREAK信号
 */
int tty_send_break(int fd, int duration) {
    return tcsendbreak(fd, duration);
}

/**
 * @brief 重启TTY输出
 */
int tty_start_output(int fd) {
    return tcflow(fd, TCOON);
}

/**
 * @brief 停止TTY输出
 */
int tty_stop_output(int fd) {
    return tcflow(fd, TCOOFF);
}

/**
 * @brief 刷新TTY队列
 */
int tty_flush(int fd, int queue) {
    return tcflush(fd, queue);
}

/**
 * @brief 获取当前TTY
 */
char *tty_get_current(void) {
    static char tty_name[32];
    const char *tty_term = getenv("TTY");

    if (tty_term) {
        strncpy(tty_name, tty_term, sizeof(tty_name) - 1);
    } else {
        /* 尝试从标准输入获取 */
        if (isatty(STDIN_FILENO)) {
            if (ttyname_r(STDIN_FILENO, tty_name, sizeof(tty_name)) != 0) {
                strcpy(tty_name, TTY_CONSOLE);
            }
        } else {
            strcpy(tty_name, TTY_CONSOLE);
        }
    }

    return tty_name;
}

/**
 * @brief 切换TTY
 */
int tty_switch(const char *tty_name) {
    if (!tty_name) {
        errno = EINVAL;
        return -1;
    }

    /* 这里需要内核支持切换TTY */
    /* 临时实现：设置环境变量 */
    if (setenv("TTY", tty_name, 1) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief 创建伪终端对
 */
int tty_create_pty(int *master, int *slave, char *name,
                  const struct termios *termp, const struct winsize *winp) {
    if (!master || !slave) {
        errno = EINVAL;
        return -1;
    }

    return openpty(master, slave, name, termp, winp);
}

/**
 * @brief 检查是否为TTY设备
 */
int tty_isatty(int fd) {
    return isatty(fd);
}

/**
 * @brief 获取TTY名称
 */
int tty_get_name(int fd, char *name, size_t size) {
    if (!name || size == 0) {
        errno = EINVAL;
        return -1;
    }

    if (ttyname_r(fd, name, size) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief 设置TTY窗口大小
 */
int tty_set_winsize(int fd, int rows, int cols) {
    struct winsize ws;

    ws.ws_row = rows;
    ws.ws_col = cols;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    return ioctl(fd, TIOCSWINSZ, &ws);
}

/**
 * @brief 获取TTY窗口大小
 */
int tty_get_winsize(int fd, int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(fd, TIOCGWINSZ, &ws) == -1) {
        return -1;
    }

    if (rows) {
        *rows = ws.ws_row;
    }
    if (cols) {
        *cols = ws.ws_col;
    }

    return 0;
}

/**
 * @brief 获取终端类型
 */
int tty_get_type(int fd, char *type, size_t size) {
    struct tty_info info;

    if (ioctl(fd, TTY_IOC_GET, &info) == -1) {
        return -1;
    }

    if (type && size > 0) {
        strncpy(type, info.type, size - 1);
        type[size - 1] = '\0';
    }

    /* 根据名称推断类型 */
    if (strstr(info.name, "tty0")) {
        return TTY_FLAG_CONSOLE;
    } else if (strstr(info.name, "tty")) {
        return TTY_FLAG_VIRTUAL;
    } else if (strstr(info.name, "pts")) {
        return TTY_FLAG_PTY;
    } else if (strstr(info.name, "ttyS")) {
        return TTY_FLAG_SERIAL;
    }

    return 0;
}

/**
 * @brief 设置终端超时
 */
int tty_set_timeout(int fd, int timeout_sec, int timeout_usec) {
    struct termios termios;

    if (tcgetattr(fd, &termios) == -1) {
        return -1;
    }

    termios.c_cc[VTIME] = timeout_sec * 10 + timeout_usec / 100000;
    termios.c_cc[VMIN] = 0;  /* 非阻塞读取 */

    if (tcsetattr(fd, TCSANOW, &termios) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief 终端原始模式设置
 */
int tty_set_raw_mode(int fd, struct termios *original) {
    struct termios termios;

    if (tcgetattr(fd, &termios) == -1) {
        return -1;
    }

    if (original) {
        *original = termios;
    }

    /* 设置原始模式 */
    termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    termios.c_oflag &= ~OPOST;
    termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    termios.c_cflag &= ~(CSIZE | PARENB);
    termios.c_cflag |= CS8;

    termios.c_cc[VMIN] = 1;   /* 最少读取1个字符 */
    termios.c_cc[VTIME] = 0;  /* 无超时 */

    if (tcsetattr(fd, TCSANOW, &termios) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief 终端规范模式设置
 */
int tty_set_canonical_mode(int fd, const struct termios *original) {
    if (!original) {
        errno = EINVAL;
        return -1;
    }

    return tcsetattr(fd, TCSANOW, original);
}

/**
 * @brief 设置终端回显
 */
int tty_set_echo(int fd, int echo) {
    struct termios termios;

    if (tcgetattr(fd, &termios) == -1) {
        return -1;
    }

    if (echo) {
        termios.c_lflag |= ECHO;
    } else {
        termios.c_lflag &= ~ECHO;
    }

    return tcsetattr(fd, TCSANOW, &termios);
}

/**
 * @brief 设置终端信号处理
 */
int tty_set_signals(int fd, int signals) {
    struct termios termios;

    if (tcgetattr(fd, &termios) == -1) {
        return -1;
    }

    if (signals) {
        termios.c_lflag |= ISIG;
    } else {
        termios.c_lflag &= ~ISIG;
    }

    return tcsetattr(fd, TCSANOW, &termios);
}

/**
 * @brief 终端输出缓冲区刷新
 */
int tty_flush_output(int fd) {
    return tcflush(fd, TCOFLUSH);
}

/**
 * @brief 终端输入缓冲区刷新
 */
int tty_flush_input(int fd) {
    return tcflush(fd, TCIFLUSH);
}

/**
 * @brief 等待终端输出完成
 */
int tty_drain_output(int fd) {
    return tcdrain(fd);
}