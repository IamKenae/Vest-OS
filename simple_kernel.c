/* Vest-OS Simple Kernel for Testing */

// Basic type definitions
typedef unsigned int size_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// VGA text mode constants
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE 15

// Simple screen functions
static int screen_x = 0;
static int screen_y = 0;

void clear_screen(void) {
    uint8_t *video_memory = (uint8_t *)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i++) {
        video_memory[i] = 0;
    }
    screen_x = 0;
    screen_y = 0;
}

void put_char(char c) {
    uint8_t *video_memory = (uint8_t *)VGA_MEMORY;
    int offset = (screen_y * VGA_WIDTH + screen_x) * 2;

    if (c == '\n') {
        screen_x = 0;
        screen_y++;
    } else if (c >= ' ' && c <= '~') {  // Printable characters
        video_memory[offset] = c;
        video_memory[offset + 1] = VGA_COLOR_WHITE;
        screen_x++;
    }

    if (screen_x >= VGA_WIDTH) {
        screen_x = 0;
        screen_y++;
    }

    if (screen_y >= VGA_HEIGHT) {
        screen_y = 0;
        clear_screen();
    }
}

void print_string(const char *str) {
    while (*str) {
        put_char(*str++);
    }
}

void print_hex(uint32_t num) {
    const char *hex_chars = "0123456789ABCDEF";
    put_char('0');
    put_char('x');
    for (int i = 28; i >= 0; i -= 4) {
        put_char(hex_chars[(num >> i) & 0xF]);
    }
}

// Simple multiboot header (simplified)
extern void multiboot_entry(void);
extern uint32_t multiboot_magic;
extern uint32_t multiboot_info_ptr;

// Simple kernel main function
void kernel_main(uint32_t magic, uint32_t mbi_ptr) {
    clear_screen();

    // Welcome message
    print_string("========================================\n");
    print_string("       Welcome to Vest-OS v0.1.0       \n");
    print_string("========================================\n");
    print_string("\n");

    // Print multiboot information
    print_string("Multiboot magic: ");
    print_hex(magic);
    print_string("\n");

    print_string("Multiboot info: ");
    print_hex(mbi_ptr);
    print_string("\n\n");

    // Check multiboot magic
    if (magic == 0x2BADB002) {
        print_string("Boot: SUCCESS - Multiboot detected\n");
    } else {
        print_string("Boot: WARNING - Magic mismatch\n");
        print_string("Expected: 0x2BADB002, Got: ");
        print_hex(magic);
        print_string("\n");
    }

    print_string("\nSystem Features:\n");
    print_string("- 32-bit x86 kernel\n");
    print_string("- VGA text mode console\n");
    print_string("- Basic I/O functions\n");
    print_string("- Multiboot support\n");
    print_string("\n");

    print_string("Kernel initialized successfully!\n");
    print_string("System is ready.\n");

    // Simple kernel loop
    while (1) {
        // Halt CPU until next interrupt
        __asm__ volatile("hlt");
    }
}