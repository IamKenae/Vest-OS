/* Vest-OS Simple Kernel for Testing */

// Basic type definitions
typedef unsigned int size_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

// Multiboot header
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Video memory
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colors
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_WHITE 15

// Multiboot header structure
struct multiboot_header {
    unsigned int magic;
    unsigned int flags;
    unsigned int checksum;
    unsigned int header_addr;
    unsigned int load_addr;
    unsigned int load_end_addr;
    unsigned int bss_end_addr;
    unsigned int entry_addr;
};

// Multiboot information structure
struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length;
    unsigned int mmap_addr;
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned int vbe_mode;
    unsigned int vbe_interface_seg;
    unsigned int vbe_interface_off;
    unsigned int vbe_interface_len;
};

// Screen position
static int screen_x = 0;
static int screen_y = 0;

// Clear screen
void clear_screen(void) {
    unsigned char *video_memory = (unsigned char *)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i++) {
        video_memory[i] = 0;
    }
    screen_x = 0;
    screen_y = 0;
}

// Put character to screen
void put_char(char c) {
    unsigned char *video_memory = (unsigned char *)VGA_MEMORY;
    int offset = (screen_y * VGA_WIDTH + screen_x) * 2;

    if (c == '\n') {
        screen_x = 0;
        screen_y++;
    } else {
        video_memory[offset] = c;
        video_memory[offset + 1] = VGA_COLOR_WHITE;
        screen_x++;
    }

    // Handle screen overflow
    if (screen_x >= VGA_WIDTH) {
        screen_x = 0;
        screen_y++;
    }

    if (screen_y >= VGA_HEIGHT) {
        screen_y = 0;
        clear_screen();
    }
}

// Print string
void print_string(const char *str) {
    while (*str) {
        put_char(*str++);
    }
}

// Print number
void print_number(unsigned int num) {
    if (num == 0) {
        put_char('0');
        return;
    }

    char buffer[11];
    int i = 0;
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        put_char(buffer[j]);
    }
}

// Simple memory test
void test_memory(void) {
    print_string("Testing memory...\n");

    // Test some memory locations
    unsigned int *test_addr = (unsigned int *)0x100000;
    *test_addr = 0xDEADBEEF;

    if (*test_addr == 0xDEADBEEF) {
        print_string("Memory test: PASSED\n");
    } else {
        print_string("Memory test: FAILED\n");
    }
}

// Print multiboot information
void print_multiboot_info(struct multiboot_info *mbi) {
    print_string("Multiboot Information:\n");
    print_string("  Flags: 0x");
    print_number(mbi->flags);
    print_string("\n");

    if (mbi->flags & 0x01) {
        print_string("  Memory: ");
        print_number(mbi->mem_lower);
        print_string("KB low, ");
        print_number(mbi->mem_upper);
        print_string("KB high\n");
    }

    if (mbi->flags & 0x02) {
        print_string("  Boot device: 0x");
        print_number(mbi->boot_device);
        print_string("\n");
    }

    if (mbi->flags & 0x04 && mbi->cmdline) {
        print_string("  Command line: ");
        print_string((char *)mbi->cmdline);
        print_string("\n");
    }

    if (mbi->flags & 0x200 && mbi->boot_loader_name) {
        print_string("  Boot loader: ");
        print_string((char *)mbi->boot_loader_name);
        print_string("\n");
    }
}

// Main kernel function
void kernel_main(struct multiboot_info *mbi, unsigned int magic) {
    clear_screen();

    // Welcome message
    print_string("========================================\n");
    print_string("       Welcome to Vest-OS v0.1.0       \n");
    print_string("========================================\n");
    print_string("\n");

    // Check multiboot magic
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        print_string("ERROR: Invalid multiboot magic: 0x");
        print_number(magic);
        print_string("\n");
        return;
    }

    print_string("Boot successful!\n");
    print_string("\n");

    // Print system information
    print_multiboot_info(mbi);
    print_string("\n");

    // Run basic tests
    print_string("Running basic system tests...\n");
    test_memory();
    print_string("\n");

    // Show features
    print_string("Vest-OS Features:\n");
    print_string("  - 32-bit x86 support\n");
    print_string("  - VGA text mode console\n");
    print_string("  - Basic memory management\n");
    print_string("  - Multiboot specification\n");
    print_string("  - Modular kernel design\n");
    print_string("\n");

    print_string("System initialized successfully!\n");
    print_string("Type 'help' for commands (not implemented yet)\n");
    print_string("Press Ctrl+Alt+Del to reboot\n");

    // Simple kernel loop
    while (1) {
        // In a real kernel, we would have an idle loop here
        // For now, just halt the CPU
        __asm__ __volatile__("hlt");
    }
}

// Multiboot header in assembly
__asm__(".text\n"
        ".global multiboot_entry\n"
        "multiboot_entry:\n"
        "    movl $stack_top, %esp\n"
        "    push %eax\n"
        "    push %ebx\n"
        "    call kernel_main\n"
        "1:  hlt\n"
        "    jmp 1b\n"
        "\n"
        ".align 4\n"
        "multiboot_header:\n"
        "    .long 0x1BADB002\n"
        "    .long 0x00000003\n"
        "    .long -(0x1BADB002 + 0x00000003)\n"
        "    .long multiboot_header\n"
        "    .long 0x00100000\n"
        "    .long 0x00100000\n"
        "    .long 0x00100000 + 16 * 1024\n"
        "    .long multiboot_entry\n"
        "\n"
        ".bss\n"
        ".align 32\n"
        "stack_bottom:\n"
        ".skip 16 * 1024\n"
        "stack_top:\n");