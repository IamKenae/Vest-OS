# System Call API Documentation

## Overview
Vest-OS system calls provide the interface for user-space programs to interact with the kernel. All system calls are accessed through wrapper functions provided by the standard library.

## Table of Contents
- [Process Management](#process-management)
- [File System](#file-system)
- [Memory Management](#memory-management)
- [Inter-Process Communication](#inter-process-communication)
- [Network](#network)
- [Device Control](#device-control)
- [Time Management](#time-management)
- [Signal Handling](#signal-handling)

## Process Management

### fork()
Create child process

```c
#include <unistd.h>
pid_t fork(void);
```

**Return values:**
- Success: Parent process returns child PID, child process returns 0
- Failure: Returns -1, sets errno

**Example:**
```c
pid_t pid = fork();
if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
} else if (pid == 0) {
    // Child process code
    printf("Child process\n");
    exit(EXIT_SUCCESS);
} else {
    // Parent process code
    printf("Parent process, child PID: %d\n", pid);
}
```

### execve()
Execute new program

```c
#include <unistd.h>
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**Parameters:**
- `pathname`: Path to the program to execute
- `argv`: Argument array, NULL-terminated
- `envp`: Environment variable array, NULL-terminated

**Example:**
```c
char *args[] = {"/bin/ls", "-l", "/home", NULL};
char *env[] = {"PATH=/bin:/usr/bin", NULL};

execve("/bin/ls", args, env);
perror("execve");  // Only executed if execve fails
```

### waitpid()
Wait for child process state change

```c
#include <sys/types.h>
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options);
```

**Parameters:**
- `pid`: Child process PID to wait for
  - `-1`: Wait for any child process
  - `0`: Wait for any child process in same process group as caller
  - `>0`: Wait for child process with specified PID
- `status`: Store child process status information
- `options`: Option flags
  - `WNOHANG`: Non-blocking mode
  - `WUNTRACED`: Also return stopped child processes
  - `WCONTINUED`: Also return continued child processes

**Example:**
```c
int status;
pid_t child_pid = fork();

if (child_pid == 0) {
    // Child process
    exit(42);
} else {
    // Parent process
    waitpid(child_pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("Child exited with status: %d\n", WEXITSTATUS(status));
    }
}
```

### getpid()/getppid()
Get process ID

```c
#include <unistd.h>
pid_t getpid(void);    // Get current process ID
pid_t getppid(void);   // Get parent process ID
```

### kill()
Send signal to process

```c
#include <signal.h>
int kill(pid_t pid, int sig);
```

**Parameters:**
- `pid`: Target process ID
- `sig`: Signal number

## File System

### open()
Open file

```c
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
int open(const char *pathname, int flags, mode_t mode);
```

**Parameters:**
- `pathname`: File path
- `flags`: Open flags
  - `O_RDONLY`: Read-only
  - `O_WRONLY`: Write-only
  - `O_RDWR`: Read-write
  - `O_CREAT`: Create if doesn't exist
  - `O_APPEND`: Append mode
  - `O_TRUNC`: Truncate file
  - `O_NONBLOCK`: Non-blocking mode
- `mode`: File permissions (used only when creating)

**Example:**
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
Read from file

```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);
```

**Parameters:**
- `fd`: File descriptor
- `buf`: Buffer
- `count`: Number of bytes to read

**Return values:**
- Success: Returns actual number of bytes read
- 0: End of file reached
- Failure: Returns -1, sets errno

### write()
Write to file

```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);
```

### close()
Close file

```c
#include <unistd.h>
int close(int fd);
```

### stat()/lstat()/fstat()
Get file status

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int stat(const char *pathname, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
```

**stat structure:**
```c
struct stat {
    dev_t st_dev;         // Device ID
    ino_t st_ino;         // inode number
    mode_t st_mode;       // File type and permissions
    nlink_t st_nlink;     // Number of hard links
    uid_t st_uid;         // Owner user ID
    gid_t st_gid;         // Owner group ID
    dev_t st_rdev;        // Device ID (if special file)
    off_t st_size;        // File size (bytes)
    blksize_t st_blksize; // Filesystem block size
    blkcnt_t st_blocks;   // Number of allocated blocks
    time_t st_atime;      // Last access time
    time_t st_mtime;      // Last modification time
    time_t st_ctime;      // Last status change time
};
```

### mkdir()
Create directory

```c
#include <sys/stat.h>
#include <sys/types.h>
int mkdir(const char *pathname, mode_t mode);
```

### rmdir()
Remove directory

```c
#include <unistd.h>
int rmdir(const char *pathname);
```

### link()/unlink()
Create/remove hard link

```c
#include <unistd.h>
int link(const char *oldpath, const char *newpath);
int unlink(const char *pathname);
```

### symlink()/readlink()
Create/read symbolic link

```c
#include <unistd.h>
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
```

### chmod()
Change file permissions

```c
#include <sys/stat.h>
int chmod(const char *pathname, mode_t mode);
```

### chown()
Change file ownership

```c
#include <unistd.h>
int chown(const char *pathname, uid_t owner, gid_t group);
```

## Memory Management

### brk()/sbrk()
Change program break

```c
#include <unistd.h>
int brk(void *addr);
void *sbrk(intptr_t increment);
```

**Note:** Modern programs should use malloc/free instead of these system calls.

### mmap()
Memory mapping

```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
```

**Parameters:**
- `addr`: Suggested mapping address (NULL means let kernel choose)
- `length`: Mapping length
- `prot`: Protection flags
  - `PROT_READ`: Readable
  - `PROT_WRITE`: Writable
  - `PROT_EXEC`: Executable
  - `PROT_NONE`: Inaccessible
- `flags`: Mapping flags
  - `MAP_SHARED`: Shared mapping
  - `MAP_PRIVATE`: Private mapping
  - `MAP_ANONYMOUS`: Anonymous mapping
  - `MAP_FIXED`: Fixed address mapping
- `fd`: File descriptor (use -1 for anonymous mapping)
- `offset`: File offset

**Example:**
```c
// Anonymous memory mapping
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
Unmap memory

```c
#include <sys/mman.h>
int munmap(void *addr, size_t length);
```

### mprotect()
Change memory protection

```c
#include <sys/mman.h>
int mprotect(void *addr, size_t len, int prot);
```

## Inter-Process Communication

### pipe()
Create pipe

```c
#include <unistd.h>
int pipe(int pipefd[2]);
```

**Parameters:**
- `pipefd[0]`: Read end file descriptor
- `pipefd[1]`: Write end file descriptor

**Example:**
```c
int pipefd[2];
if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
}

pid_t pid = fork();
if (pid == 0) {
    // Child process - write
    close(pipefd[0]);
    write(pipefd[1], "Hello Parent", 12);
    close(pipefd[1]);
} else {
    // Parent process - read
    close(pipefd[1]);
    char buf[128];
    read(pipefd[0], buf, sizeof(buf));
    printf("Received: %s\n", buf);
    close(pipefd[0]);
}
```

### mkfifo()
Create named pipe (FIFO)

```c
#include <sys/stat.h>
#include <fcntl.h>
int mkfifo(const char *pathname, mode_t mode);
```

### shmget()/shmat()/shmdt()
Shared memory

```c
#include <sys/ipc.h>
#include <sys/shm.h>
int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
```

**Example:**
```c
// Create shared memory
int shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
if (shmid == -1) {
    perror("shmget");
    return -1;
}

// Attach to process address space
char *shared_mem = (char *)shmat(shmid, NULL, 0);
if (shared_mem == (void *)-1) {
    perror("shmat");
    return -1;
}

// Use shared memory
strcpy(shared_mem, "Shared Data");

// Detach
shmdt(shared_mem);
```

### msgget()/msgsnd()/msgrcv()
Message queues

```c
#include <sys/msg.h>
int msgget(key_t key, int msgflg);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz,
               long msgtyp, int msgflg);
```

## Network

### socket()
Create socket

```c
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```

**Parameters:**
- `domain`: Communication domain
  - `AF_INET`: IPv4
  - `AF_INET6`: IPv6
  - `AF_UNIX`: Local communication
- `type`: Socket type
  - `SOCK_STREAM`: TCP
  - `SOCK_DGRAM`: UDP
  - `SOCK_RAW`: Raw socket
- `protocol`: Protocol (usually 0)

### bind()
Bind socket to address

```c
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### listen()
Listen for connections

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```

### accept()
Accept connection

```c
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### connect()
Initiate connection

```c
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen);
```

### send()/recv()
Send/receive data

```c
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

## Device Control

### ioctl()
Device control

```c
#include <sys/ioctl.h>
int ioctl(int fd, unsigned long request, ...);
```

**Example:**
```c
// Get terminal window size
struct winsize ws;
ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
printf("Rows: %d, Cols: %d\n", ws.ws_row, ws.ws_col);
```

## Time Management

### gettimeofday()
Get time

```c
#include <sys/time.h>
int gettimeofday(struct timeval *tv, struct timezone *tz);
```

### nanosleep()
High precision sleep

```c
#include <time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);
```

## Signal Handling

### signal()
Set signal handler

```c
#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
```

### sigaction()
Advanced signal handling

```c
#include <signal.h>
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);
```

**sigaction structure:**
```c
struct sigaction {
    void (*sa_handler)(int);            // Signal handler function
    void (*sa_sigaction)(int, siginfo_t *, void *); // SA_SIGINFO handler
    sigset_t sa_mask;                   // Blocked signals
    int sa_flags;                       // Flags
    void (*sa_restorer)(void);          // Deprecated
};
```

**Example:**
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

## Error Codes

All system calls set `errno` on failure:

| Error Code | Meaning |
|------------|---------|
| EACCES | Permission denied |
| EAGAIN | Resource temporarily unavailable |
| EBADF | Invalid file descriptor |
| EEXIST | File exists |
| EINVAL | Invalid argument |
| EIO | I/O error |
| ENOENT | File or directory not found |
| ENOMEM | Not enough memory |
| ENOSPC | No space left on device |
| EPERM | Operation not permitted |
| EPIPE | Broken pipe |

---

*Documentation Version: 1.0*
*Last updated: January 2024*