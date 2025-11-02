# 系统调用API文档

## 概述
Vest-OS系统调用提供了用户空间程序与内核交互的接口。所有系统调用都通过标准库提供的包装函数进行访问。

## 目录
- [进程管理](#进程管理)
- [文件系统](#文件系统)
- [内存管理](#内存管理)
- [进程间通信](#进程间通信)
- [网络](#网络)
- [设备控制](#设备控制)
- [时间管理](#时间管理)
- [信号处理](#信号处理)

## 进程管理

### fork()
创建子进程

```c
#include <unistd.h>
pid_t fork(void);
```

**返回值：**
- 成功：父进程返回子进程PID，子进程返回0
- 失败：返回-1，设置errno

**示例：**
```c
pid_t pid = fork();
if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
} else if (pid == 0) {
    // 子进程代码
    printf("Child process\n");
    exit(EXIT_SUCCESS);
} else {
    // 父进程代码
    printf("Parent process, child PID: %d\n", pid);
}
```

### execve()
执行新程序

```c
#include <unistd.h>
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**参数：**
- `pathname`: 要执行的程序路径
- `argv`: 参数数组，以NULL结尾
- `envp`: 环境变量数组，以NULL结尾

**示例：**
```c
char *args[] = {"/bin/ls", "-l", "/home", NULL};
char *env[] = {"PATH=/bin:/usr/bin", NULL};

execve("/bin/ls", args, env);
perror("execve");  // 只有在execve失败时才执行
```

### waitpid()
等待子进程状态改变

```c
#include <sys/types.h>
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options);
```

**参数：**
- `pid`: 要等待的子进程PID
  - `-1`: 等待任意子进程
  - `0`: 等待与调用者同进程组的任意子进程
  - `>0`: 等待指定PID的子进程
- `status`: 存储子进程状态信息
- `options`: 选项标志
  - `WNOHANG`: 非阻塞模式
  - `WUNTRACED`: 也返回已停止的子进程
  - `WCONTINUED`: 也返回继续执行的子进程

**示例：**
```c
int status;
pid_t child_pid = fork();

if (child_pid == 0) {
    // 子进程
    exit(42);
} else {
    // 父进程
    waitpid(child_pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("Child exited with status: %d\n", WEXITSTATUS(status));
    }
}
```

### getpid()/getppid()
获取进程ID

```c
#include <unistd.h>
pid_t getpid(void);    // 获取当前进程ID
pid_t getppid(void);   // 获取父进程ID
```

### kill()
发送信号给进程

```c
#include <signal.h>
int kill(pid_t pid, int sig);
```

**参数：**
- `pid`: 目标进程ID
- `sig`: 信号编号

## 文件系统

### open()
打开文件

```c
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
int open(const char *pathname, int flags, mode_t mode);
```

**参数：**
- `pathname`: 文件路径
- `flags`: 打开标志
  - `O_RDONLY`: 只读
  - `O_WRONLY`: 只写
  - `O_RDWR`: 读写
  - `O_CREAT`: 如果不存在则创建
  - `O_APPEND`: 追加模式
  - `O_TRUNC`: 截断文件
  - `O_NONBLOCK`: 非阻塞模式
- `mode`: 文件权限（仅在创建时使用）

**示例：**
```c
int fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
if (fd == -1) {
    perror("open");
    return -1;
}
write(fd, "Hello World", 11);
close(fd);
```

### read()
读取文件

```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);
```

**参数：**
- `fd`: 文件描述符
- `buf`: 缓冲区
- `count`: 要读取的字节数

**返回值：**
- 成功：返回实际读取的字节数
- 0：到达文件末尾
- 失败：返回-1，设置errno

### write()
写入文件

```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);
```

### close()
关闭文件

```c
#include <unistd.h>
int close(int fd);
```

### stat()/lstat()/fstat()
获取文件状态

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int stat(const char *pathname, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
```

**stat结构：**
```c
struct stat {
    dev_t st_dev;         // 设备ID
    ino_t st_ino;         // inode号
    mode_t st_mode;       // 文件类型和权限
    nlink_t st_nlink;     // 硬链接数
    uid_t st_uid;         // 所有者用户ID
    gid_t st_gid;         // 所有者组ID
    dev_t st_rdev;        // 设备ID（如果是特殊文件）
    off_t st_size;        // 文件大小（字节）
    blksize_t st_blksize; // 文件系统块大小
    blkcnt_t st_blocks;   // 分配的块数
    time_t st_atime;      // 最后访问时间
    time_t st_mtime;      // 最后修改时间
    time_t st_ctime;      // 最后状态改变时间
};
```

### mkdir()
创建目录

```c
#include <sys/stat.h>
#include <sys/types.h>
int mkdir(const char *pathname, mode_t mode);
```

### rmdir()
删除目录

```c
#include <unistd.h>
int rmdir(const char *pathname);
```

### link()/unlink()
创建/删除硬链接

```c
#include <unistd.h>
int link(const char *oldpath, const char *newpath);
int unlink(const char *pathname);
```

### symlink()/readlink()
创建/读取符号链接

```c
#include <unistd.h>
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
```

### chmod()
改变文件权限

```c
#include <sys/stat.h>
int chmod(const char *pathname, mode_t mode);
```

### chown()
改变文件所有者

```c
#include <unistd.h>
int chown(const char *pathname, uid_t owner, gid_t group);
```

## 内存管理

### brk()/sbrk()
改变程序断点

```c
#include <unistd.h>
int brk(void *addr);
void *sbrk(intptr_t increment);
```

**注意：** 现代程序应使用malloc/free代替这些系统调用。

### mmap()
内存映射

```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
```

**参数：**
- `addr`: 建议的映射地址（NULL表示让内核选择）
- `length`: 映射长度
- `prot`: 保护标志
  - `PROT_READ`: 可读
  - `PROT_WRITE`: 可写
  - `PROT_EXEC`: 可执行
  - `PROT_NONE`: 不可访问
- `flags`: 映射标志
  - `MAP_SHARED`: 共享映射
  - `MAP_PRIVATE`: 私有映射
  - `MAP_ANONYMOUS`: 匿名映射
  - `MAP_FIXED`: 固定地址映射
- `fd`: 文件描述符（匿名映射时使用-1）
- `offset`: 文件偏移量

**示例：**
```c
// 匿名内存映射
void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (ptr == MAP_FAILED) {
    perror("mmap");
    return -1;
}
strcpy(ptr, "Hello");
munmap(ptr, 4096);
```

### munmap()
取消内存映射

```c
#include <sys/mman.h>
int munmap(void *addr, size_t length);
```

### mprotect()
改变内存保护属性

```c
#include <sys/mman.h>
int mprotect(void *addr, size_t len, int prot);
```

## 进程间通信

### pipe()
创建管道

```c
#include <unistd.h>
int pipe(int pipefd[2]);
```

**参数：**
- `pipefd[0]`: 读端文件描述符
- `pipefd[1]`: 写端文件描述符

**示例：**
```c
int pipefd[2];
if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
}

pid_t pid = fork();
if (pid == 0) {
    // 子进程 - 写入
    close(pipefd[0]);
    write(pipefd[1], "Hello Parent", 12);
    close(pipefd[1]);
} else {
    // 父进程 - 读取
    close(pipefd[1]);
    char buf[128];
    read(pipefd[0], buf, sizeof(buf));
    printf("Received: %s\n", buf);
    close(pipefd[0]);
}
```

### mkfifo()
创建命名管道（FIFO）

```c
#include <sys/stat.h>
#include <fcntl.h>
int mkfifo(const char *pathname, mode_t mode);
```

### shmget()/shmat()/shmdt()
共享内存

```c
#include <sys/ipc.h>
#include <sys/shm.h>
int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
```

**示例：**
```c
// 创建共享内存
int shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
if (shmid == -1) {
    perror("shmget");
    return -1;
}

// 附加到进程地址空间
char *shared_mem = (char *)shmat(shmid, NULL, 0);
if (shared_mem == (void *)-1) {
    perror("shmat");
    return -1;
}

// 使用共享内存
strcpy(shared_mem, "Shared Data");

// 分离
shmdt(shared_mem);
```

### msgget()/msgsnd()/msgrcv()
消息队列

```c
#include <sys/msg.h>
int msgget(key_t key, int msgflg);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz,
               long msgtyp, int msgflg);
```

## 网络

### socket()
创建套接字

```c
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```

**参数：**
- `domain`: 通信域
  - `AF_INET`: IPv4
  - `AF_INET6`: IPv6
  - `AF_UNIX`: 本地通信
- `type`: 套接字类型
  - `SOCK_STREAM`: TCP
  - `SOCK_DGRAM`: UDP
  - `SOCK_RAW`: 原始套接字
- `protocol`: 协议（通常为0）

### bind()
绑定套接字到地址

```c
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### listen()
监听连接

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```

### accept()
接受连接

```c
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### connect()
发起连接

```c
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen);
```

### send()/recv()
发送/接收数据

```c
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

## 设备控制

### ioctl()
设备控制

```c
#include <sys/ioctl.h>
int ioctl(int fd, unsigned long request, ...);
```

**示例：**
```c
// 获取终端窗口大小
struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
printf("Rows: %d, Cols: %d\n", ws.ws_row, ws.ws_col);
```

## 时间管理

### gettimeofday()
获取时间

```c
#include <sys/time.h>
int gettimeofday(struct timeval *tv, struct timezone *tz);
```

### nanosleep()
高精度睡眠

```c
#include <time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);
```

## 信号处理

### signal()
设置信号处理函数

```c
#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
```

### sigaction()
高级信号处理

```c
#include <signal.h>
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);
```

**sigaction结构：**
```c
struct sigaction {
    void (*sa_handler)(int);            // 信号处理函数
    void (*sa_sigaction)(int, siginfo_t *, void *); // SA_SIGINFO的处理函数
    sigset_t sa_mask;                   // 阻塞的信号
    int sa_flags;                       // 标志
    void (*sa_restorer)(void);          // 已废弃
};
```

**示例：**
```c
#include <signal.h>
#include <stdio.h>

void sigint_handler(int sig) {
    printf("Received SIGINT\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);

    while (1) {
        pause();
    }

    return 0;
}
```

## 错误代码

所有系统调用在失败时都会设置`errno`：

| 错误代码 | 含义 |
|---------|------|
| EACCES | 权限被拒绝 |
| EAGAIN | 资源暂时不可用 |
| EBADF | 无效的文件描述符 |
| EEXIST | 文件已存在 |
| EINVAL | 无效参数 |
| EIO | I/O错误 |
| ENOENT | 文件或目录不存在 |
| ENOMEM | 内存不足 |
| ENOSPC | 设备上没有空间 |
| EPERM | 操作不允许 |
| EPIPE | 管道破裂 |

---

*文档版本：1.0*
*最后更新：2024年1月*