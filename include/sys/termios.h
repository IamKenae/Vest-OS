/**
 * @file termios.h
 * @brief 终端I/O系统调用接口
 * @author Vest-OS Team
 * @date 2024
 */

#ifndef _SYS_TERMIOS_H
#define _SYS_TERMIOS_H

#include <stdint.h>
#include <sys/types.h>

/* 终端控制标志 */
typedef uint32_t tcflag_t;

/* 输入模式标志 */
#define IGNBRK   0000001  /* 忽略BREAK条件 */
#define BRKINT   0000002  /* BREAK产生SIGINT */
#define IGNPAR   0000004  /* 忽略奇偶校验错误 */
#define PARMRK   0000010  /* 标记奇偶校验错误 */
#define INPCK    0000020  /* 启用输入奇偶校验 */
#define ISTRIP   0000040  /* 剥离第8位 */
#define INLCR    0000100  /* NL映射到CR */
#define IGNCR    0000200  /* 忽略CR */
#define ICRNL    0000400  /* CR映射到NL */
#define IUCLC    0001000  /* 大写映射到小写 */
#define IXON     0002000  /* 启用XON/XOFF流控 */
#define IXANY    0004000  /* 任何字符重启输出 */
#define IXOFF    0010000  /* 启用XON/XOFF输入流控 */
#define IMAXBEL  0020000  /* 响铃输入队列满 */
#define IUTF8    0040000  /* UTF-8输入处理 */

/* 输出模式标志 */
#define OPOST    0000001  /* 启用输出处理 */
#define OLCUC    0000002  /* 小写映射到大写 */
#define ONLCR    0000004  /* NL映射到CR-NL */
#define OCRNL    0000010  /* CR映射到NL */
#define ONOCR    0000020  /* 不在0列输出CR */
#define ONLRET   0000040  /* NL执行CR功能 */
#define OFILL    0000100  /* 填充字符用于延迟 */
#define OFDEL    0000200  /* 填充字符为DEL */
#define NLDLY    0000400  /* 换行延迟 */
#define NL0      0000000
#define NL1      0000400
#define CRDLY    0003000  /* 回车延迟 */
#define CR0      0000000
#define CR1      0001000
#define CR2      0002000
#define CR3      0003000
#define TABDLY   0014000  /* 水平制表符延迟 */
#define TAB0     0000000
#define TAB1     0004000
#define TAB2     0010000
#define TAB3     0014000
#define XTABS    0014000  /* 制表符扩展为空格 */
#define BSDLY    0020000  /* 退格延迟 */
#define BS0      0000000
#define BS1      0020000
#define VTDLY    0040000  /* 垂直制表符延迟 */
#define VT0      0000000
#define VT1      0040000
#define FFDLY    0100000  /* 换页延迟 */
#define FF0      0000000
#define FF1      0100000

/* 控制模式标志 */
#define CBAUD    0010017  /* 波特率掩码 */
#define B0       0000000  /* 挂断 */
#define B50      0000001
#define B75      0000002
#define B110     0000003
#define B134     0000004
#define B150     0000005
#define B200     0000006
#define B300     0000007
#define B600     0000010
#define B1200    0000011
#define B1800    0000012
#define B2400    0000013
#define B4800    0000014
#define B9600    0000015
#define B19200   0000016
#define B38400   0000017
#define EXTA     B19200
#define EXTB     B38400
#define CSIZE    0000060  /* 字符长度掩码 */
#define CS5      0000000
#define CS6      0000020
#define CS7      0000040
#define CS8      0000060
#define CSTOPB   0000100  /* 停止位 */
#define CREAD    0000200  /* 启用接收器 */
#define PARENB   0000400  /* 奇偶校验使能 */
#define PARODD   0001000  /* 奇校验 */
#define HUPCL    0002000  /* 挂起时关闭 */
#define CLOCAL   0004000  /* 忽略调制解调器状态线 */
#define CBAUDEX  0010000  /* 扩展波特率 */
#define B57600   0010001
#define B115200  0010002
#define B230400  0010003
#define B460800  0010004
#define B500000  0010005
#define B576000  0010006
#define B921600  0010007
#define B1000000 0010010
#define B1152000 0010011
#define B1500000 0010012
#define B2000000 0010013
#define B2500000 0010014
#define B3000000 0010015
#define B3500000 0010016
#define B4000000 0010017
#define CIBAUD   002003600000  /* 输入波特率 */
#define CRTSCTS  020000000000  /* RTS/CTS流控 */

/* 本地模式标志 */
#define ISIG     0000001  /* 启用信号 */
#define ICANON   0000002  /* 规范模式 */
#define XCASE    0000004  /* 大小写映射 */
#define ECHO     0000010  /* 回显 */
#define ECHOE    0000020  /* 回显擦除字符 */
#define ECHOK    0000040  /* 回显KILL */
#define ECHONL   0000100  /* 回显NL */
#define NOFLSH   0000200  /* 禁用刷新 */
#define TOSTOP   0000400  /* 后台进程输出 */
#define ECHOCTL  0001000  /* 回显控制字符 */
#define ECHOPRT  0002000  /* 回显擦除字符 */
#define ECHOKE   0004000  /* 回显KILL */
#define FLUSHO   0010000  /* 输出正在刷新 */
#define PENDIN   0040000  /* 重新输入待处理 */
#define IEXTEN   0100000  /* 扩展功能 */

/* 控制字符 */
#define NCCS     32
#define VINTR    0   /* 中断字符 */
#define VQUIT    1   /* 退出字符 */
#define VERASE   2   /* 擦除字符 */
#define VKILL    3   /* 杀死字符 */
#define VEOF     4   /* 文件结束字符 */
#define VTIME    5   /* 超时值 */
#define VMIN     6   /* 最小字符数 */
#define VSWTC    7   /* 交换字符 */
#define VSTART   8   /* 开始字符 */
#define VSTOP    9   /* 停止字符 */
#define VSUSP    10  /* 挂起字符 */
#define VEOL     11  /* 行结束字符 */
#define VREPRINT 12  /* 重新打印字符 */
#define VDISCARD 13  /* 丢弃字符 */
#define VWERASE  14  /* 单词擦除字符 */
#define VLNEXT   15  /* 字面下一个字符 */
#define VEOL2    16  /* 第二个行结束字符 */

/* 终端IO结构 */
struct termios {
    tcflag_t c_iflag;      /* 输入模式标志 */
    tcflag_t c_oflag;      /* 输出模式标志 */
    tcflag_t c_cflag;      /* 控制模式标志 */
    tcflag_t c_lflag;      /* 本地模式标志 */
    cc_t c_cc[NCCS];       /* 控制字符 */
    speed_t c_ispeed;      /* 输入速度 */
    speed_t c_ospeed;      /* 输出速度 */
};

/* 终端大小结构 */
struct winsize {
    unsigned short ws_row;     /* 行数 */
    unsigned short ws_col;     /* 列数 */
    unsigned short ws_xpixel;  /* X像素 */
    unsigned short ws_ypixel;  /* Y像素 */
};

/* 系统调用函数声明 */

/**
 * @brief 获取终端属性
 * @param fd 文件描述符
 * @param termios_p 终端属性结构
 * @return 0成功，-1失败
 */
int tcgetattr(int fd, struct termios *termios_p);

/**
 * @brief 设置终端属性
 * @param fd 文件描述符
 * @param optional_actions 操作方式
 * @param termios_p 终端属性结构
 * @return 0成功，-1失败
 */
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

/**
 * @brief 发送break
 * @param fd 文件描述符
 * @param duration 持续时间
 * @return 0成功，-1失败
 */
int tcsendbreak(int fd, int duration);

/**
 * @brief 等待传输完成
 * @param fd 文件描述符
 * @return 0成功，-1失败
 */
int tcdrain(int fd);

/**
 * @brief 刷新输入/输出队列
 * @param fd 文件描述符
 * @param queue_selector 队列选择器
 * @return 0成功，-1失败
 */
int tcflush(int fd, int queue_selector);

/**
 * @brief 控制数据流
 * @param fd 文件描述符
 * @param action 动作
 * @return 0成功，-1失败
 */
int tcflow(int fd, int action);

/**
 * @brief 获取前台进程组ID
 * @param fd 文件描述符
 * @param pgrp 进程组ID存储位置
 * @return 0成功，-1失败
 */
int tcgetpgrp(int fd, pid_t *pgrp);

/**
 * @brief 设置前台进程组ID
 * @param fd 文件描述符
 * @param pgrp 进程组ID
 * @return 0成功，-1失败
 */
int tcsetpgrp(int fd, pid_t pgrp);

/**
 * @brief 获取终端会话ID
 * @param fd 文件描述符
 * @return 会话ID，-1失败
 */
pid_t tcgetsid(int fd);

/**
 * @brief 获取终端窗口大小
 * @param fd 文件描述符
 * @param ws 窗口大小结构
 * @return 0成功，-1失败
 */
int ioctl(int fd, int request, ...);

/**
 * @brief 创建伪终端
 * @param master 主端文件描述符
 * @param slave 从端文件描述符
 * @return 0成功，-1失败
 */
int openpty(int *master, int *slave, char *name, const struct termios *termp,
           const struct winsize *winp);

/**
 * @brief 分支伪终端
 * @param master 主端文件描述符
 * @return 子进程ID，-1失败
 */
pid_t forkpty(int *master, char *name, const struct termios *termp,
             const struct winsize *winp);

/* 常量定义 */
#define TCSANOW   0   /* 立即更改 */
#define TCSADRAIN 1   /* 传输完成后更改 */
#define TCSAFLUSH 2   /* 传输完成并刷新输入队列后更改 */

#define TCIFLUSH  0   /* 刷新输入队列 */
#define TCOFLUSH  1   /* 刷新输出队列 */
#define TCIOFLUSH 2   /* 刷新输入输出队列 */

#define TCOOFF    0   /* 挂起输出 */
#define TCOON     1   /* 重启输出 */
#define TCIOFF    2   /* 挂起输入 */
#define TCION     3   /* 重启输入 */

/* ioctl请求 */
#define TIOCGWINSZ 0x5413  /* 获取窗口大小 */
#define TIOCSWINSZ 0x5414  /* 设置窗口大小 */
#define TIOCGPGRP  0x540F  /* 获取进程组 */
#define TIOCSPGRP  0x5410  /* 设置进程组 */

#endif /* _SYS_TERMIOS_H */