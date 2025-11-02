# Vest-OS TTY Terminal System

## Overview

The Vest-OS TTY terminal system is a complete terminal input/output subsystem that provides text mode display, keyboard input processing, and multi-virtual terminal support for the operating system. This system adopts a modular design, providing complete interfaces from low-level hardware to user space.

## System Architecture

### Core Components

1. **VGA Text Mode Driver** (`drivers/tty/vga.c`)
   - 80x25 text mode display
   - 16-color foreground/background support
   - Hardware cursor control
   - Screen scrolling and area operations

2. **Keyboard Input Driver** (`drivers/tty/keyboard.c`)
   - US keyboard layout support
   - Modifier key handling (Shift, Ctrl, Alt)
   - LED indicator control
   - Keyboard buffer management

3. **TTY Driver Core** (`drivers/tty/tty.c`)
   - TTY device management
   - Line discipline processing
   - Input/output buffers
   - Multi-terminal support

4. **Terminal Manager** (`kernel/terminal.c`)
   - Virtual terminal switching
   - ANSI escape sequence parsing
   - Terminal session management
   - History support

5. **User Space Interface** (`lib/tty.c`)
   - POSIX compatible termios interface
   - System call wrappers
   - Pseudo-terminal support
   - Terminal control utilities

## Directory Structure

```
Vest-OS/
├── drivers/tty/          # TTY drivers
│   ├── vga.c            # VGA display driver
│   ├── keyboard.c       # Keyboard input driver
│   └── tty.c            # TTY core driver
├── kernel/              # Kernel support
│   ├── terminal.c       # Terminal manager
│   ├── string.c         # String functions
│   ├── memory.c         # Memory management
│   └── spinlock.c       # Spinlocks
├── arch/x86/            # Architecture support
│   ├── io.c             # I/O port operations
│   └── interrupt.c      # Interrupt handling
├── lib/                 # User space library
│   └── tty.c            # TTY user interface
├── include/             # Header files
│   ├── drivers/         # Driver headers
│   ├── kernel/          # Kernel headers
│   ├── sys/             # System call headers
│   └── arch/            # Architecture headers
├── examples/            # Example programs
│   └── tty_test.c       # TTY test program
└── Makefile             # Build system
```

## Features

### Display Features
- ✅ 80x25 standard text mode
- ✅ 16-color foreground/background support
- ✅ Hardware cursor control
- ✅ Screen scrolling and clearing
- ✅ Area copy and fill
- ✅ ANSI color sequence support

### Input Features
- ✅ Standard QWERTY keyboard support
- ✅ Modifier key handling (Shift, Ctrl, Alt)
- ✅ LED indicator control (NumLock, CapsLock, ScrollLock)
- ✅ Keyboard repeat rate setting
- ✅ Input buffer management

### Terminal Features
- ✅ Multi-virtual terminal support (up to 8)
- ✅ Terminal switching
- ✅ Line discipline processing
- ✅ Canonical and raw mode
- ✅ Echo control
- ✅ Signal handling

### User Interface
- ✅ POSIX termios compatible
- ✅ Standard I/O redirection
- ✅ Pseudo-terminal support
- ✅ Terminal attribute control
- ✅ Window size control

## Build and Installation

### Build System

```bash
# Create directory structure
make dirs

# Build all targets
make all

# View help
make help

# Clean build files
make clean

# Deep clean
make distclean
```

### Installation

```bash
# Install header files
make install-headers

# Install library files
make install

# Full installation
make install-all
```

### Build Targets

- `libtty.a` - TTY user space library
- `tty_driver.o` - TTY driver object file
- `kernel_tty.o` - Kernel TTY support

## Usage Examples

### Basic TTY Operations

```c
#include <sys/tty.h>
#include <stdio.h>

int main() {
    // Open TTY device
    int fd = tty_open("/dev/tty0", O_RDWR);

    // Write data
    tty_write(fd, "Hello TTY!\n", 12);

    // Read data
    char buffer[256];
    ssize_t bytes = tty_read(fd, buffer, sizeof(buffer));

    // Close device
    tty_close(fd);

    return 0;
}
```

### Terminal Attribute Control

```c
#include <sys/termios.h>
#include <sys/tty.h>

// Set raw mode
struct termios original, raw;
int fd = tty_open("/dev/tty0", O_RDWR);

tcgetattr(fd, &original);
raw = original;
raw.c_lflag &= ~(ECHO | ICANON);
tcsetattr(fd, TCSANOW, &raw);

// Restore original mode
tcsetattr(fd, TCSANOW, &original);
```

### Color Output

```c
// ANSI color sequences
printf("\033[31mRed Text\033[0m\n");
printf("\033[32;44mGreen on Blue\033[0m\n");
```

## API Reference

### TTY Device Operations

| Function | Description |
|----------|-------------|
| `tty_open()` | Open TTY device |
| `tty_close()` | Close TTY device |
| `tty_read()` | Read TTY data |
| `tty_write()` | Write TTY data |
| `tty_isatty()` | Check if device is a TTY |

### Terminal Attribute Control

| Function | Description |
|----------|-------------|
| `tcgetattr()` | Get terminal attributes |
| `tcsetattr()` | Set terminal attributes |
| `tcflush()` | Flush input/output queues |
| `tcflow()` | Control data flow |
| `tcdrain()` | Wait for output completion |

### Terminal Information

| Function | Description |
|----------|-------------|
| `tty_get_info()` | Get TTY information |
| `tty_get_winsize()` | Get window size |
| `tty_set_winsize()` | Set window size |
| `tty_get_name()` | Get TTY name |

## Configuration Options

### Compile-time Configuration

- `MAX_TTYS` - Maximum TTY count (default 8)
- `TTY_BUFFER_SIZE` - TTY buffer size (default 4096)
- `VGA_WIDTH` - VGA width (default 80)
- `VGA_HEIGHT` - VGA height (default 25)

### Runtime Configuration

```c
// Set TTY mode
tty_set_mode(fd, TTY_MODE_RAW);     // Raw mode
tty_set_mode(fd, TTY_MODE_COOKED);  // Cooked mode

// Set color
tty_set_color(fd, VGA_COLOR_GREEN, VGA_COLOR_BLACK);

// Set echo
tty_set_echo(fd, 1);  // Enable echo
tty_set_echo(fd, 0);  // Disable echo
```

## Test Programs

The project includes a complete test program `examples/tty_test.c` that tests the following features:

- Basic TTY operations
- Terminal attribute control
- Input/output operations
- Pseudo-terminal support
- Color display

Run the test:

```bash
# Compile test program
gcc -o tty_test examples/tty_test.c -L. -ltty

# Run test
./tty_test
```

## Technical Specifications

### Hardware Requirements

- x86 architecture processor
- VGA compatible graphics card
- Standard AT/PS2 keyboard
- Minimum 1MB memory

### Performance Metrics

- Display response time: <1ms
- Keyboard response time: <1ms
- Terminal switching time: <10ms
- Concurrent terminals supported: Up to 8

### Memory Usage

- VGA buffer: 4KB
- Keyboard buffer: 2KB
- TTY buffer: 4KB per terminal
- Total memory usage: <64KB

## Compatibility

### Standard Compliance

- IEEE Std 1003.1 (POSIX.1)
- ANSI X3.64-1979 (Escape sequences)
- ISO 6429 (Control sequences)

### Platform Support

- ✅ x86/i386
- ⏳ x86_64 (planned)
- ⏳ ARM (planned)

## Troubleshooting

### Common Issues

**Q: TTY device cannot be opened**
A: Check if device node exists, confirm permissions are correct

**Q: No keyboard input response**
A: Check keyboard driver initialization, confirm interrupt handling is correct

**Q: Display abnormalities**
A: Check VGA initialization, confirm video memory mapping is correct

**Q: Terminal switching failure**
A: Check terminal manager status, confirm target terminal is available

### Debug Options

```c
// Enable debug output
#define DEBUG_TTY 1

// Check TTY status
struct tty_info info;
tty_get_info(fd, &info);
printf("TTY status: %d\n", info.state);
```

## Contributing

### Development Environment

- GCC 7.0+
- NASM 2.13+
- Make 4.0+

### Code Style

- Follow Linux kernel coding style
- Use Doxygen comment format
- Function names use underscore separation
- Variable names use descriptive names

### Submission Process

1. Fork project repository
2. Create feature branch
3. Submit code changes
4. Run tests for verification
5. Submit Pull Request

## License

This project is licensed under GPL v2. See LICENSE file for details.

## Contact

- Project Homepage: https://github.com/vestos/tty
- Issue Reports: https://github.com/vestos/tty/issues
- Email: vestos@example.com

---

**Vest-OS TTY Terminal System** - Providing reliable terminal support for modern operating systems