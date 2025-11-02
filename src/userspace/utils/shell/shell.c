/*
 * Vest-OS Shell程序
 * 提供命令行界面和基本的shell功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

// 最大命令长度
#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// 内置命令结构
struct builtin_command {
    const char *name;
    int (*func)(int argc, char *argv[]);
    const char *help;
};

// 前向声明
static int builtin_help(int argc, char *argv[]);
static int builtin_exit(int argc, char *argv[]);
static int builtin_cd(int argc, char *argv[]);
static int builtin_pwd(int argc, char *argv[]);
static int builtin_ls(int argc, char *argv[]);
static int builtin_cat(int argc, char *argv[]);
static int builtin_echo(int argc, char *argv[]);
static int builtin_mkdir(int argc, char *argv[]);
static int builtin_rm(int argc, char *argv[]);
static int builtin_clear(int argc, char *argv[]);
static int builtin_ps(int argc, char *argv[]);
static int builtin_kill(int argc, char *argv[]);
static int builtin_reboot(int argc, char *argv[]);
static int builtin_shutdown(int argc, char *argv[]);

// 内置命令表
static struct builtin_command builtin_commands[] = {
    {"help", builtin_help, "显示帮助信息"},
    {"exit", builtin_exit, "退出shell"},
    {"cd", builtin_cd, "切换目录"},
    {"pwd", builtin_pwd, "显示当前目录"},
    {"ls", builtin_ls, "列出目录内容"},
    {"cat", builtin_cat, "显示文件内容"},
    {"echo", builtin_echo, "输出文本"},
    {"mkdir", builtin_mkdir, "创建目录"},
    {"rm", builtin_rm, "删除文件或目录"},
    {"clear", builtin_clear, "清屏"},
    {"ps", builtin_ps, "显示进程列表"},
    {"kill", builtin_kill, "发送信号到进程"},
    {"reboot", builtin_reboot, "重启系统"},
    {"shutdown", builtin_shutdown, "关闭系统"},
    {NULL, NULL, NULL}
};

// Shell状态
static int shell_running = 1;
static char current_dir[PATH_MAX];

/**
 * 获取提示符
 */
static void get_prompt(char *prompt, size_t size)
{
    char hostname[256];
    char *user = getenv("USER");

    if (!user) user = "user";

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "vestos");
    }

    snprintf(prompt, size, "\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ",
             user, hostname, current_dir);
}

/**
 * 解析命令行
 */
static int parse_command(const char *cmdline, char *argv[])
{
    static char cmd[MAX_CMD_LEN];
    int argc = 0;
    char *token;
    char *saveptr;

    // 复制命令行以便修改
    strncpy(cmd, cmdline, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';

    // 分割命令行
    token = strtok_r(cmd, " \t\n", &saveptr);
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    argv[argc] = NULL;

    return argc;
}

/**
 * 查找内置命令
 */
static struct builtin_command* find_builtin(const char *name)
{
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(builtin_commands[i].name, name) == 0) {
            return &builtin_commands[i];
        }
    }
    return NULL;
}

/**
 * 执行外部命令
 */
static int execute_external_command(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0) {
        // 子进程
        execvp(argv[0], argv);

        // 如果exec失败
        fprintf(stderr, "sh: %s: %s\n", argv[0], strerror(errno));
        exit(127);
    } else if (pid > 0) {
        // 父进程
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "sh: 程序被信号 %d 终止\n", WTERMSIG(status));
            return 128 + WTERMSIG(status);
        }
        return 1;
    } else {
        fprintf(stderr, "sh: fork失败: %s\n", strerror(errno));
        return 1;
    }
}

/**
 * 执行命令
 */
static int execute_command(int argc, char *argv[])
{
    if (argc == 0) {
        return 0;
    }

    // 查找内置命令
    struct builtin_command *builtin = find_builtin(argv[0]);
    if (builtin) {
        return builtin->func(argc, argv);
    }

    // 执行外部命令
    return execute_external_command(argc, argv);
}

/**
 * 内置命令实现
 */

static int builtin_help(int argc, char *argv[])
{
    printf("Vest-OS Shell 内置命令:\n");
    printf("\n");

    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        printf("  %-10s - %s\n", builtin_commands[i].name, builtin_commands[i].help);
    }

    printf("\n");
    printf("外部命令请使用完整路径或确保在PATH中\n");
    return 0;
}

static int builtin_exit(int argc, char *argv[])
{
    shell_running = 0;
    return 0;
}

static int builtin_cd(int argc, char *argv[])
{
    const char *path;

    if (argc == 1) {
        path = getenv("HOME");
        if (!path) path = "/";
    } else {
        path = argv[1];
    }

    if (chdir(path) < 0) {
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    // 更新当前目录
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        strcpy(current_dir, "/");
    }

    return 0;
}

static int builtin_pwd(int argc, char *argv[])
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        fprintf(stderr, "pwd: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}

static int builtin_ls(int argc, char *argv[])
{
    const char *path = ".";
    DIR *dir;
    struct dirent *entry;

    if (argc > 1) {
        path = argv[1];
    }

    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}

static int builtin_cat(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "cat: 缺少文件参数\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
            continue;
        }

        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            fwrite(buffer, 1, bytes, stdout);
        }

        fclose(file);
    }

    return 0;
}

static int builtin_echo(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\n");
    return 0;
}

static int builtin_mkdir(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "mkdir: 缺少目录参数\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i], 0755) < 0) {
            fprintf(stderr, "mkdir: %s: %s\n", argv[i], strerror(errno));
            return 1;
        }
    }

    return 0;
}

static int builtin_rm(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "rm: 缺少文件参数\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (unlink(argv[i]) < 0) {
            fprintf(stderr, "rm: %s: %s\n", argv[i], strerror(errno));
            return 1;
        }
    }

    return 0;
}

static int builtin_clear(int argc, char *argv[])
{
    printf("\033[2J\033[H");
    return 0;
}

static int builtin_ps(int argc, char *argv[])
{
    printf("PID   PPID  STAT COMMAND\n");

    DIR *dir = opendir("/proc");
    if (!dir) {
        fprintf(stderr, "ps: 无法访问/proc目录\n");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            int pid = atoi(entry->d_name);

            // 读取进程信息
            char stat_path[256];
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file) {
                int ppid;
                char state;
                char comm[256];

                fscanf(stat_file, "%d %s %c %d", &pid, comm, &state, &ppid);
                printf("%-5d %-5d %-4c %s\n", pid, ppid, state, comm);
                fclose(stat_file);
            }
        }
    }

    closedir(dir);
    return 0;
}

static int builtin_kill(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "kill: 缺少进程ID参数\n");
        return 1;
    }

    int signum = SIGTERM;  // 默认信号
    int pid;

    if (argc >= 3 && argv[1][0] == '-') {
        signum = atoi(argv[1] + 1);
        pid = atoi(argv[2]);
    } else {
        pid = atoi(argv[1]);
    }

    if (kill(pid, signum) < 0) {
        fprintf(stderr, "kill: %d: %s\n", pid, strerror(errno));
        return 1;
    }

    return 0;
}

static int builtin_reboot(int argc, char *argv[])
{
    printf("正在重启系统...\n");
    sync();
    reboot(RB_AUTOBOOT);
    return 0;
}

static int builtin_shutdown(int argc, char *argv[])
{
    printf("正在关闭系统...\n");
    sync();
    reboot(RB_POWER_OFF);
    return 0;
}

/**
 * 初始化shell
 */
static void init_shell(void)
{
    // 初始化readline
    rl_bind_key('\t', rl_complete);

    // 设置当前目录
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        strcpy(current_dir, "/");
    }

    // 设置信号处理
    signal(SIGINT, SIG_IGN);  // 忽略Ctrl+C
    signal(SIGQUIT, SIG_IGN); // 忽略Ctrl+\
}

/**
 * Shell主循环
 */
static void shell_loop(void)
{
    char *line;
    char *argv[MAX_ARGS];
    int argc;

    while (shell_running) {
        char prompt[256];
        get_prompt(prompt, sizeof(prompt));

        // 读取命令行
        line = readline(prompt);
        if (!line) {
            printf("\n");
            break;  // EOF
        }

        // 添加到历史
        if (*line) {
            add_history(line);
        }

        // 解析并执行命令
        argc = parse_command(line, argv);
        if (argc > 0) {
            execute_command(argc, argv);
        }

        free(line);
    }
}

/**
 * 主函数
 */
int main(int argc, char *argv[])
{
    // 设置环境变量
    setenv("SHELL", "/bin/sh", 1);

    // 初始化shell
    init_shell();

    // 运行shell主循环
    shell_loop();

    printf("再见！\n");
    return 0;
}