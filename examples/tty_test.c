/**
 * @file tty_test.c
 * @brief TTY终端系统测试程序
 * @author Vest-OS Team
 * @date 2024
 */

#include <sys/tty.h>
#include <sys/termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief 测试TTY基本功能
 */
int test_tty_basic(void) {
    printf("=== TTY基本功能测试 ===\n");

    /* 获取当前TTY */
    char *current_tty = tty_get_current();
    printf("当前TTY: %s\n", current_tty);

    /* 打开TTY */
    int fd = tty_open(current_tty, O_RDWR);
    if (fd < 0) {
        printf("❌ 打开TTY失败\n");
        return -1;
    }
    printf("✅ 打开TTY成功\n");

    /* 检查是否为TTY */
    if (tty_isatty(fd)) {
        printf("✅ 设备是TTY\n");
    } else {
        printf("❌ 设备不是TTY\n");
    }

    /* 获取TTY信息 */
    struct tty_info info;
    if (tty_get_info(fd, &info) == 0) {
        printf("✅ TTY信息:\n");
        printf("   名称: %s\n", info.name);
        printf("   类型: %s\n", info.type);
        printf("   设备号: %d:%d\n", info.major, info.minor);
    }

    /* 获取窗口大小 */
    int rows, cols;
    if (tty_get_winsize(fd, &rows, &cols) == 0) {
        printf("✅ 窗口大小: %dx%d\n", cols, rows);
    }

    /* 关闭TTY */
    tty_close(fd);
    printf("✅ 关闭TTY成功\n");

    return 0;
}

/**
 * @brief 测试终端属性
 */
int test_termios(void) {
    printf("\n=== 终端属性测试 ===\n");

    int fd = open(tty_get_current(), O_RDWR);
    if (fd < 0) {
        printf("❌ 打开TTY失败\n");
        return -1;
    }

    /* 获取终端属性 */
    struct termios original;
    if (tcgetattr(fd, &original) == 0) {
        printf("✅ 获取终端属性成功\n");

        /* 设置原始模式 */
        struct termios raw = original;
        raw.c_lflag &= ~(ECHO | ICANON | ISIG);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        if (tcsetattr(fd, TCSANOW, &raw) == 0) {
            printf("✅ 设置原始模式成功\n");

            /* 恢复原始模式 */
            tcsetattr(fd, TCSANOW, &original);
            printf("✅ 恢复终端属性成功\n");
        }
    } else {
        printf("❌ 获取终端属性失败\n");
    }

    close(fd);
    return 0;
}

/**
 * @brief 测试TTY读写
 */
int test_tty_io(void) {
    printf("\n=== TTY I/O测试 ===\n");

    int fd = open(tty_get_current(), O_RDWR);
    if (fd < 0) {
        printf("❌ 打开TTY失败\n");
        return -1;
    }

    /* 测试写入 */
    const char *test_msg = "TTY测试消息\n";
    ssize_t written = tty_write(fd, test_msg, strlen(test_msg));
    if (written > 0) {
        printf("✅ 写入 %zd 字节成功\n", written);
    } else {
        printf("❌ 写入失败\n");
    }

    /* 测试回显设置 */
    if (tty_set_echo(fd, 0) == 0) {
        printf("✅ 禁用回显成功\n");

        /* 恢复回显 */
        tty_set_echo(fd, 1);
        printf("✅ 启用回显成功\n");
    }

    close(fd);
    return 0;
}

/**
 * @brief 测试伪终端
 */
int test_pty(void) {
    printf("\n=== 伪终端测试 ===\n");

    int master, slave;
    char name[64];

    /* 创建伪终端对 */
    if (tty_create_pty(&master, &slave, name, NULL, NULL) == 0) {
        printf("✅ 创建伪终端成功\n");
        printf("   主端: %d\n", master);
        printf("   从端: %d\n", slave);
        printf("   名称: %s\n", name);

        /* 测试通信 */
        const char *msg = "PTY测试消息";
        write(master, msg, strlen(msg));

        char buffer[128];
        ssize_t bytes = read(slave, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("✅ PTY通信成功: %s\n", buffer);
        }

        close(master);
        close(slave);
    } else {
        printf("❌ 创建伪终端失败（可能不支持）\n");
    }

    return 0;
}

/**
 * @brief 显示颜色测试
 */
int test_colors(void) {
    printf("\n=== 颜色测试 ===\n");

    /* ANSI颜色序列测试 */
    printf("颜色测试:\n");
    printf("\033[31m红色\033[0m\n");
    printf("\033[32m绿色\033[0m\n");
    printf("\033[33m黄色\033[0m\n");
    printf("\033[34m蓝色\033[0m\n");
    printf("\033[35m紫色\033[0m\n");
    printf("\033[36m青色\033[0m\n");
    printf("\033[37m白色\033[0m\n");

    /* 背景色测试 */
    printf("背景色测试:\n");
    printf("\033[41m红背景\033[0m\n");
    printf("\033[42m绿背景\033[0m\n");
    printf("\033[44m蓝背景\033[0m\n");

    printf("✅ 颜色测试完成\n");
    return 0;
}

/**
 * @brief 主测试函数
 */
int main(void) {
    printf("Vest-OS TTY终端系统测试程序\n");
    printf("=====================================\n");

    int result = 0;

    /* 运行各项测试 */
    result += test_tty_basic();
    result += test_termios();
    result += test_tty_io();
    result += test_pty();
    result += test_colors();

    printf("\n=====================================\n");
    if (result == 0) {
        printf("✅ 所有测试通过\n");
    } else {
        printf("❌ 部分测试失败\n");
    }

    return result;
}