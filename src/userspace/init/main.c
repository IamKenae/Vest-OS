/*
 * Vest-OS init进程
 * 系统初始化程序，负责启动系统服务和用户程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

// 默认系统服务
static const char* default_services[] = {
    "/sbin/filesystem",
    "/sbin/network",
    "/sbin/process_manager",
    "/sbin/memory_manager",
    "/sbin/device_manager",
    NULL
};

// 默认启动脚本
static const char* startup_scripts[] = {
    "/etc/rc.d/rc.sysinit",
    "/etc/rc.d/rc.local",
    NULL
};

/**
 * 打印系统启动信息
 */
static void print_startup_message(void)
{
    printf("\n");
    printf("================================================\n");
    printf("           Vest-OS 操作系统启动中...\n");
    printf("           版本: %d.%d.%d\n",
           VESTOS_VERSION_MAJOR, VESTOS_VERSION_MINOR, VESTOS_VERSION_PATCH);
    printf("           架构: %d位\n", sizeof(void*) * 8);
    printf("================================================\n");
    printf("\n");
}

/**
 * 挂载文件系统
 */
static int mount_filesystems(void)
{
    printf("挂载文件系统...\n");

    // 创建必要的目录
    mkdir("/dev", 0755);
    mkdir("/proc", 0755);
    mkdir("/sys", 0755);
    mkdir("/tmp", 0777);
    mkdir("/var", 0755);
    mkdir("/var/log", 0755);
    mkdir("/var/run", 0755);
    mkdir("/home", 0755);
    mkdir("/root", 0700);

    // 挂载虚拟文件系统
    if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
        printf("警告: 挂载proc文件系统失败: %s\n", strerror(errno));
    }

    if (mount("sysfs", "/sys", "sysfs", 0, NULL) < 0) {
        printf("警告: 挂载sysfs文件系统失败: %s\n", strerror(errno));
    }

    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) < 0) {
        printf("警告: 挂载devtmpfs失败: %s\n", strerror(errno));
    }

    // 挂载根文件系统（如果是只读的）
    if (mount("/", "/", "ext2", MS_REMOUNT, NULL) < 0) {
        printf("警告: 重新挂载根文件系统失败: %s\n", strerror(errno));
    }

    printf("文件系统挂载完成\n");
    return 0;
}

/**
 * 启动系统服务
 */
static int start_system_services(void)
{
    printf("启动系统服务...\n");

    for (int i = 0; default_services[i] != NULL; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // 子进程
            printf("启动服务: %s\n", default_services[i]);
            execl(default_services[i], default_services[i], NULL);

            // 如果exec失败
            printf("错误: 无法启动服务 %s: %s\n",
                   default_services[i], strerror(errno));
            exit(1);
        } else if (pid > 0) {
            // 父进程
            printf("服务 %s 已启动 (PID: %d)\n",
                   default_services[i], pid);
        } else {
            printf("错误: fork失败: %s\n", strerror(errno));
            return -1;
        }
    }

    printf("系统服务启动完成\n");
    return 0;
}

/**
 * 执行启动脚本
 */
static int run_startup_scripts(void)
{
    printf("执行启动脚本...\n");

    for (int i = 0; startup_scripts[i] != NULL; i++) {
        if (access(startup_scripts[i], X_OK) == 0) {
            printf("执行脚本: %s\n", startup_scripts[i]);

            pid_t pid = fork();
            if (pid == 0) {
                execl(startup_scripts[i], startup_scripts[i], NULL);
                printf("错误: 无法执行脚本 %s: %s\n",
                       startup_scripts[i], strerror(errno));
                exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);

                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    printf("警告: 脚本 %s 执行失败，退出码: %d\n",
                           startup_scripts[i], WEXITSTATUS(status));
                }
            } else {
                printf("错误: fork失败: %s\n", strerror(errno));
                return -1;
            }
        } else {
            printf("跳过不存在的脚本: %s\n", startup_scripts[i]);
        }
    }

    printf("启动脚本执行完成\n");
    return 0;
}

/**
 * 设置信号处理
 */
static void setup_signal_handlers(void)
{
    struct sigaction sa;

    // 忽略SIGCHLD，避免僵尸进程
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT | SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    // 设置SIGTERM处理程序（优雅关机）
    sa.sa_handler = SIG_DFL;
    sigaction(SIGTERM, &sa, NULL);

    // 设置SIGINT处理程序（Ctrl+C）
    sa.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sa, NULL);
}

/**
 * 清理函数
 */
static void cleanup(void)
{
    printf("正在关闭系统服务...\n");

    // 向所有进程发送SIGTERM信号
    kill(-1, SIGTERM);

    // 等待一段时间让进程正常退出
    sleep(2);

    // 强制杀死剩余进程
    kill(-1, SIGKILL);

    printf("系统关闭完成\n");
}

/**
 * 启动shell
 */
static int start_shell(void)
{
    const char* shell = "/bin/sh";

    // 检查shell是否存在
    if (access(shell, X_OK) != 0) {
        printf("错误: 找不到shell程序 %s\n", shell);
        return -1;
    }

    printf("启动shell: %s\n", shell);

    pid_t pid = fork();
    if (pid == 0) {
        // 设置环境变量
        setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin", 1);
        setenv("HOME", "/root", 1);
        setenv("TERM", "linux", 1);
        setenv("USER", "root", 1);
        setenv("LOGNAME", "root", 1);

        // 切换到root目录
        chdir("/root");

        // 启动shell
        execl(shell, shell, NULL);

        // 如果exec失败
        printf("错误: 无法启动shell: %s\n", strerror(errno));
        exit(1);
    } else if (pid > 0) {
        // 等待shell退出
        int status;
        waitpid(pid, &status, 0);

        printf("Shell已退出，状态: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("错误: fork失败: %s\n", strerror(errno));
        return -1;
    }
}

/**
 * 系统主循环
 */
static void system_loop(void)
{
    while (1) {
        // 启动shell
        int shell_status = start_shell();

        if (shell_status != 0) {
            printf("Shell异常退出，等待5秒后重启...\n");
            sleep(5);
        } else {
            printf("Shell正常退出，系统可以关机\n");
            break;
        }
    }
}

/**
 * 主函数
 */
int main(int argc, char *argv[])
{
    printf("Vest-OS init进程启动 (PID: %d)\n", getpid());

    // 打印启动信息
    print_startup_message();

    // 设置信号处理
    setup_signal_handlers();

    // 挂载文件系统
    if (mount_filesystems() < 0) {
        printf("严重错误: 文件系统挂载失败\n");
        exit(1);
    }

    // 启动系统服务
    if (start_system_services() < 0) {
        printf("严重错误: 系统服务启动失败\n");
        exit(1);
    }

    // 执行启动脚本
    if (run_startup_scripts() < 0) {
        printf("警告: 启动脚本执行过程中出现错误\n");
    }

    // 创建pid文件
    FILE *pidfile = fopen("/var/run/init.pid", "w");
    if (pidfile) {
        fprintf(pidfile, "%d\n", getpid());
        fclose(pidfile);
    }

    printf("\n系统启动完成，Vest-OS已准备就绪！\n");
    printf("输入help查看可用命令\n\n");

    // 进入系统主循环
    system_loop();

    // 清理并退出
    cleanup();

    // 删除pid文件
    unlink("/var/run/init.pid");

    printf("系统关闭完成\n");
    return 0;
}