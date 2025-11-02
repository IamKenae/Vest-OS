/*
 * Vest-OS TTY驱动程序
 * 提供虚拟终端支持
 */

#include <kernel.h>
#include <drivers/tty.h>
#include <hal/cpu.h>

// 最大TTY设备数
#define MAX_TTY_DEVICES 16

// TTY设备数组
static struct tty_device tty_devices[MAX_TTY_DEVICES];
static spinlock_t tty_lock;

// 当前前台TTY
static int current_tty = 0;

// VGA显示缓冲区
static uint16_t *vga_buffer = (uint16_t*)VGA_BASE;

// 光标位置
static int cursor_x = 0;
static int cursor_y = 0;

// 颜色定义
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

// 默认颜色
#define VGA_DEFAULT_COLOR       (VGA_COLOR_BLACK << 4) | VGA_COLOR_WHITE

/**
 * 创建VGA颜色值
 */
static inline uint8_t vga_color(uint8_t fg, uint8_t bg)
{
    return fg | (bg << 4);
}

/**
 * 创建VGA字符
 */
static inline uint16_t vga_entry(char c, uint8_t color)
{
    return (uint16_t)c | (uint16_t)color << 8;
}

/**
 * 更新光标位置
 */
static void update_cursor(int x, int y)
{
    uint16_t pos = y * VGA_WIDTH + x;

    outb(0x3D4, 0x0F);  // 光标位置低字节
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);  // 光标位置高字节
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/**
 * 清屏
 */
static void clear_screen(void)
{
    uint16_t color = vga_entry(' ', VGA_DEFAULT_COLOR);

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = color;
    }

    cursor_x = 0;
    cursor_y = 0;
    update_cursor(cursor_x, cursor_y);
}

/**
 * 滚屏
 */
static void scroll_screen(void)
{
    uint16_t color = vga_entry(' ', VGA_DEFAULT_COLOR);

    // 向上移动一行
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }

    // 清空最后一行
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        vga_buffer[i] = color;
    }

    cursor_y = VGA_HEIGHT - 1;
}

/**
 * 在屏幕上写入字符
 */
static void screen_putchar(char c)
{
    switch (c) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;

        case '\r':
            cursor_x = 0;
            break;

        case '\t':
            cursor_x = (cursor_x + 8) & ~7;
            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
            break;

        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
                vga_buffer[pos] = vga_entry(' ', VGA_DEFAULT_COLOR);
            }
            break;

        default:
            if (c >= ' ') {
                uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
                vga_buffer[pos] = vga_entry(c, VGA_DEFAULT_COLOR);
                cursor_x++;
            }
            break;
    }

    // 检查是否需要换行
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // 检查是否需要滚屏
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }

    update_cursor(cursor_x, cursor_y);
}

/**
 * TTY写操作
 */
static int tty_write_console(struct tty_device *tty, const char *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        screen_putchar(data[i]);
    }
    return len;
}

/**
 * TTY读操作（从键盘）
 */
static int tty_read_console(struct tty_device *tty, char *data, size_t len)
{
    // 这里应该从键盘缓冲区读取数据
    // 暂时返回0，表示没有数据可读
    return 0;
}

/**
 * TTY刷新操作
 */
static void tty_flush_console(struct tty_device *tty)
{
    // 对于控制台，刷新操作不需要做什么
}

/**
 * 初始化TTY设备
 */
static int init_tty_device(int tty_id)
{
    struct tty_device *tty = &tty_devices[tty_id];

    // 初始化TTY结构
    memset(tty, 0, sizeof(struct tty_device));
    tty->tty_id = tty_id;

    // 分配缓冲区
    tty->buffer_size = 4096;
    tty->buffer = kmalloc(tty->buffer_size);
    if (!tty->buffer) {
        return -ERROR_NOMEM;
    }

    // 初始化读写位置
    tty->read_pos = 0;
    tty->write_pos = 0;

    // 设置默认termios
    tty->termios.c_iflag = ICRNL | IXON;
    tty->termios.c_oflag = OPOST | ONLCR;
    tty->termios.c_cflag = B38400 | CS8 | CREAD | HUPCL;
    tty->termios.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
    tty->termios.c_ispeed = B38400;
    tty->termios.c_ospeed = B38400;

    // 设置操作函数
    if (tty_id == 0) {
        // 控制台TTY
        tty->write = tty_write_console;
        tty->read = tty_read_console;
        tty->flush = tty_flush_console;
        tty->hardware_data = NULL;
    } else {
        // 串口TTY（暂时不实现）
        tty->write = NULL;
        tty->read = NULL;
        tty->flush = NULL;
        tty->hardware_data = NULL;
    }

    return 0;
}

/**
 * 获取TTY设备
 */
struct tty_device* get_tty_device(int tty_id)
{
    if (tty_id < 0 || tty_id >= MAX_TTY_DEVICES) {
        return NULL;
    }

    if (!tty_devices[tty_id].buffer) {
        return NULL;
    }

    return &tty_devices[tty_id];
}

/**
 * 切换TTY
 */
int switch_tty(int tty_id)
{
    if (tty_id < 0 || tty_id >= MAX_TTY_DEVICES) {
        return -ERROR_INVALID;
    }

    if (!tty_devices[tty_id].buffer) {
        return -ERROR_INVALID;
    }

    current_tty = tty_id;

    // 保存当前TTY的状态（如果需要的话）
    // 恢复目标TTY的状态（如果需要的话）

    // 重新清屏并切换到目标TTY
    clear_screen();

    return 0;
}

/**
 * 获取当前TTY
 */
int get_current_tty(void)
{
    return current_tty;
}

/**
 * 写入数据到TTY
 */
int tty_write(int tty_id, const char *data, size_t len)
{
    struct tty_device *tty = get_tty_device(tty_id);
    if (!tty || !tty->write) {
        return -ERROR_INVALID;
    }

    return tty->write(tty, data, len);
}

/**
 * 从TTY读取数据
 */
int tty_read(int tty_id, char *data, size_t len)
{
    struct tty_device *tty = get_tty_device(tty_id);
    if (!tty || !tty->read) {
        return -ERROR_INVALID;
    }

    return tty->read(tty, data, len);
}

/**
 * TTY控制操作
 */
int tty_ioctl(int tty_id, uint32_t cmd, void *arg)
{
    struct tty_device *tty = get_tty_device(tty_id);
    if (!tty) {
        return -ERROR_INVALID;
    }

    switch (cmd) {
        case TCGETS:
            if (arg) {
                memcpy(arg, &tty->termios, sizeof(struct termios));
            }
            break;

        case TCSETS:
            if (arg) {
                memcpy(&tty->termios, arg, sizeof(struct termios));
            }
            break;

        case TIOCSWINSZ:
            // 设置窗口大小（控制台不支持）
            break;

        case TIOCGWINSZ:
            // 获取窗口大小
            if (arg) {
                struct winsize *ws = (struct winsize*)arg;
                ws->ws_row = VGA_HEIGHT;
                ws->ws_col = VGA_WIDTH;
                ws->ws_xpixel = 0;
                ws->ws_ypixel = 0;
            }
            break;

        default:
            return -ERROR_INVALID;
    }

    return 0;
}

/**
 * 内核输出函数
 */
void kernel_printk(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 输出到当前TTY
    tty_write(current_tty, buffer, strlen(buffer));
}

/**
 * 初始化TTY子系统
 */
void tty_init(void)
{
    kernel_printk("初始化TTY子系统...\n");

    // 初始化锁
    spinlock_init(&tty_lock);

    // 初始化TTY设备
    for (int i = 0; i < MAX_TTY_DEVICES; i++) {
        if (init_tty_device(i) == 0) {
            kernel_printk("  TTY%d 初始化成功\n", i);
        }
    }

    // 清屏
    clear_screen();

    // 设置当前TTY为0
    current_tty = 0;

    kernel_printk("TTY子系统初始化完成\n");
}