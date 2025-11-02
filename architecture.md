# Vest-OS Operating System Architecture Design

## Overview

Vest-OS is a modern operating system based on microkernel architecture, supporting both 32-bit and 64-bit processor architectures. This document describes in detail the system's architecture design, module decomposition, and key design decisions.

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                        User Space                                   │
├─────────────────────────────────────────────────────────────────────┤
│  Application Layer                                                │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │
│  │ GUI Apps │ │ CLI Apps │ │ System   │ │ Services │ │ Drivers  │   │
│  │          │ │          │ │ Utilities│ │          │ │ (User)   │   │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘   │
├─────────────────────────────────────────────────────────────────────┤
│  System Services Layer                                            │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │
│  │ File     │ │ Network  │ │ Process  │ │ Memory   │ │ Device   │   │
│  │ System   │ │ Stack    │ │ Manager  │ │ Manager  │ │ Manager  │   │
│  │ Service  │ │ Service  │ │ Service  │ │ Service  │ │ Service  │   │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘   │
├─────────────────────────────────────────────────────────────────────┤
│  System Call Interface Layer                                      │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │              System Call Gateway                                │ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │ │
│  │  │ POSIX API   │ │ VestOS API  │ │ Network API │ │ Device API  │ │ │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ │ │
│  └─────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                        Kernel Space                                │
├─────────────────────────────────────────────────────────────────────┤
│  Microkernel                                                       │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                     Kernel Core                                  │ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │ │
│  │  │ Scheduler   │ │ IPC Manager │ │ Memory Mgmt │ │ Interrupt   │ │ │
│  │  │             │ │             │ │             │ │ Handler     │ │ │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ │ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │ │
│  │  │ TTY Driver  │ │ Timer       │ │ Exception   │ │ System Call │ │ │
│  │  │             │ │ Manager     │ │ Handler     │ │ Handler     │ │ │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ │ │
│  └─────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│  Hardware Abstraction Layer (HAL)                                 │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │ │
│  │  │ CPU HAL     │ │ Memory HAL  │ │ I/O HAL     │ │ Interrupt   │ │ │
│  │  │ (x86/x64)   │ │             │ │             │ │ HAL         │ │ │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ │ │
│  └─────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                          Hardware                                   │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐   │
│  │   CPU       │ │    RAM      │ │    I/O      │ │  Storage    │   │
│  │ (32/64-bit) │ │             │ │ Devices     │ │ Devices     │   │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

## Core Design Principles

### 1. Microkernel Architecture
- **Minimal Kernel**: Kernel contains only the most basic functions
- **External Services**: Most system services run in user space
- **Modular Design**: Each service is an independent module
- **Fault Isolation**: Single service crash does not affect the entire system

### 2. Layered Architecture
- **Clear Hierarchy**: Each layer only interacts with adjacent layers
- **Abstract Interfaces**: Standard interfaces for inter-layer communication
- **Replaceability**: Each layer can be independently replaced and upgraded

### 3. Modular Design
- **Loose Coupling**: Minimize dependencies between modules
- **High Cohesion**: Each module has clear functionality
- **Scalability**: New features can be added as new modules

## Module Decomposition

### Kernel Modules

#### 1. Kernel Core
```
kernel/
├── core/
│   ├── scheduler.c/h      # Process scheduler
│   ├── ipc.c/h           # Inter-process communication
│   ├── memory.c/h        # Memory management
│   ├── interrupt.c/h     # Interrupt handling
│   ├── syscall.c/h       # System call handling
│   ├── exception.c/h     # Exception handling
│   └── timer.c/h         # Timer management
```

#### 2. Device Drivers
```
kernel/
├── drivers/
│   ├── tty/
│   │   ├── tty.c/h       # TTY terminal driver
│   │   ├── console.c/h   # Console driver
│   │   └── keyboard.c/h  # Keyboard driver
│   ├── storage/
│   │   ├── ata.c/h       # ATA/SATA driver
│   │   ├── ahci.c/h      # AHCI driver
│   │   └── nvme.c/h      # NVMe driver
│   └── network/
│       ├── rtl8139.c/h   # Network card driver example
│       └── e1000.c/h     # Intel network card driver
```

#### 3. Hardware Abstraction Layer
```
kernel/
├── hal/
│   ├── x86/
│   │   ├── 32bit/        # 32-bit architecture support
│   │   │   ├── cpu.c/h
│   │   │   ├── memory.c/h
│   │   │   └── interrupt.c/h
│   │   └── 64bit/        # 64-bit architecture support
│   │       ├── cpu.c/h
│   │       ├── memory.c/h
│   │       └── interrupt.c/h
│   ├── arm/              # ARM architecture support (reserved)
│   └── common/           # Common HAL components
```

### User Space Modules

#### 1. System Services
```
userspace/
├── services/
│   ├── filesystem/
│   │   ├── vfs.c/h       # Virtual file system
│   │   ├── ext2.c/h      # EXT2 file system
│   │   └── fat32.c/h     # FAT32 file system
│   ├── network/
│   │   ├── tcpip.c/h     # TCP/IP protocol stack
│   │   ├── udp.c/h       # UDP protocol
│   │   └── dhcp.c/h      # DHCP client
│   ├── process/
│   │   ├── process_manager.c/h
│   │   └── thread_manager.c/h
│   └── memory/
│       ├── memory_manager.c/h
│       └── allocator.c/h
```

#### 2. System Libraries
```
userspace/
├── lib/
│   ├── libc/             # C standard library
│   ├── libposix/         # POSIX compatibility library
│   ├── libvestos/        # VestOS specific library
│   └── libgui/           # GUI library (optional)
```

#### 3. System Utilities
```
userspace/
├── utils/
│   ├── shell/            # Shell program
│   ├── init/             # Initialization program
│   ├── coreutils/        # Core utilities
│   └── system/           # System management tools
```

## Kernel and User Space Separation Strategy

### 1. Address Space Separation
- **Kernel Space**: High address space (0xC0000000 - 0xFFFFFFFF for 32-bit)
- **User Space**: Low address space (0x00000000 - 0xBFFFFFFF for 32-bit)
- **Protection Mechanism**: Hardware-level memory protection, preventing user programs from accessing kernel space

### 2. System Call Mechanism
```c
// System call interface design
typedef struct {
    uint32_t syscall_number;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint32_t arg5;
} syscall_args_t;

// System call handling flow
// User program -> INT 0x80 -> Kernel syscall_handler -> Specific service implementation
```

### 3. Inter-Process Communication (IPC)
- **Message Passing**: Asynchronous communication based on message queues
- **Shared Memory**: High-performance data sharing mechanism
- **Signal Mechanism**: Lightweight event notification

### 4. Service Management
- **Service Registration**: Dynamic service registration and discovery
- **Service Monitoring**: Service health status monitoring
- **Service Restart**: Automatic failure recovery mechanism

## 32/64-bit Compatibility Design

### 1. Architecture Abstraction Layer
```c
// Architecture-specific abstract interface
typedef struct {
    uint32_t bits;         // 32 or 64
    void (*cpu_init)(void);
    void (*memory_init)(void);
    void (*interrupt_init)(void);
    size_t (*get_page_size)(void);
    uintptr_t (*virt_to_phys)(uintptr_t virt);
    uintptr_t (*phys_to_virt)(uintptr_t phys);
} arch_interface_t;
```

### 2. Data Type Compatibility
```c
// Unified data type definitions
#include <stdint.h>

// Pointer size handling
#if defined(__x86_64__)
    typedef uint64_t ptr_t;
    typedef uint64_t size_t;
    #define PTR_FMT "0x%016lx"
#elif defined(__i386__)
    typedef uint32_t ptr_t;
    typedef uint32_t size_t;
    #define PTR_FMT "0x%08x"
#endif
```

### 3. Memory Management Compatibility
- **Page Size**: Unified 4KB page support
- **Address Space**: 32-bit supports 4GB, 64-bit supports larger address space
- **Memory Layout**: Compatible memory layout design

### 4. Compile-time Configuration
```makefile
# Architecture configuration in Makefile
ifeq ($(ARCH), x86_64)
    CFLAGS += -m64 -D__x86_64__
    ASFLAGS += --64
    LINKER_SCRIPT = linker_64.ld
else ifeq ($(ARCH), i386)
    CFLAGS += -m32 -D__i386__
    ASFLAGS += --32
    LINKER_SCRIPT = linker_32.ld
endif
```

## TTY Terminal Support

### 1. TTY Architecture Design
```
TTY Subsystem Architecture:
┌─────────────────────────────────────────────────┐
│              User Space TTY Applications         │
├─────────────────────────────────────────────────┤
│               TTY System Call Interface          │
├─────────────────────────────────────────────────┤
│                TTY Driver Layer                 │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │
│  │ TTY Core    │ │ Console     │ │ Serial      │ │
│  │ Driver      │ │ Driver      │ │ Driver      │ │
│  └─────────────┘ └─────────────┘ └─────────────┘ │
├─────────────────────────────────────────────────┤
│                Hardware Layer                  │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ │
│  │ VGA Display │ │ Keyboard    │ │ Serial Port │ │
│  └─────────────┘ └─────────────┘ └─────────────┘ │
└─────────────────────────────────────────────────┘
```

### 2. TTY Data Structures
```c
// TTY device structure
typedef struct tty_device {
    int tty_id;                    // TTY number
    char *buffer;                  // Input buffer
    size_t buffer_size;           // Buffer size
    size_t read_pos;              // Read position
    size_t write_pos;             // Write position

    // TTY configuration
    struct termios termios;       // Terminal configuration

    // Callback functions
    int (*write)(struct tty_device *tty, const char *data, size_t len);
    int (*read)(struct tty_device *tty, char *data, size_t len);
    void (*flush)(struct tty_device *tty);

    // Hardware related
    void *hardware_data;          // Hardware-specific data
} tty_device_t;
```

### 3. Multi-threading Support

#### Thread Management Architecture
```c
// Thread control block
typedef struct thread {
    uint32_t tid;                 // Thread ID
    uint32_t pid;                 // Process ID

    // Execution context
    cpu_context_t context;        // CPU register context

    // Stack information
    void *stack_base;             // Stack base address
    size_t stack_size;            // Stack size

    // Scheduling information
    thread_state_t state;         // Thread state
    uint32_t priority;            // Priority
    uint32_t time_slice;          // Time slice

    // Synchronization primitives
    struct thread *next;          // Linked list pointers
    struct thread *prev;

    // Wait information
    void *wait_object;            // Wait object
    uint32_t wait_reason;         // Wait reason
} thread_t;
```

#### Thread Scheduling Policies
- **Preemptive Scheduling**: Time slice-based preemptive multitasking
- **Priority Scheduling**: Support for static and dynamic priorities
- **Fair Scheduling**: Ensure fair thread execution

## Key Design Decisions

### ADR-001: Microkernel Architecture Choice
**Decision**: Adopt microkernel architecture instead of monolithic kernel architecture

**Rationale**:
- **Fault Isolation**: Service failures do not cause system crashes
- **Modularity**: Easier development and maintenance
- **Scalability**: New services can be developed and deployed independently
- **Security**: Reduced attack surface, improved system security

**Consequences**:
- **Performance Overhead**: Performance loss due to IPC
- **Complexity**: Need to design efficient IPC mechanisms
- **Development Difficulty**: Increased complexity of distributed system design

### ADR-002: 32/64-bit Compatibility Design
**Decision**: Support 32-bit and 64-bit architectures at source code level

**Rationale**:
- **Market Coverage**: Support broader hardware platforms
- **Migration Path**: Facilitate migration from 32-bit to 64-bit
- **Cost Control**: Lower maintenance cost for single codebase

**Consequences**:
- **Code Complexity**: Need to handle architectural differences
- **Testing Burden**: Need to test on multiple architectures
- **Performance Optimization**: Difficult to deeply optimize for specific architectures

### ADR-003: TTY Subsystem Design
**Decision**: Adopt layered TTY subsystem design

**Rationale**:
- **Scalability**: Easy to add new TTY devices
- **Unified Interface**: Provide unified TTY interface for upper-layer applications
- **Modularity**: Separate hardware drivers from TTY core logic

**Consequences**:
- **Abstraction Overhead**: Performance loss due to multiple abstraction layers
- **Complexity**: Need to design well-defined interfaces

## Performance Considerations

### 1. Kernel Performance Optimization
- **Fast System Calls**: Use SYSENTER/SYSEXIT instructions
- **Zero-copy IPC**: Shared memory for efficient IPC
- **Cache Optimization**: Optimize data structure layout for better cache hit rates

### 2. Memory Management Optimization
- **Lazy Allocation**: Allocate physical memory on demand
- **Memory Pools**: Pre-allocate commonly used memory block sizes
- **Large Page Support**: Support large pages to reduce TLB misses

### 3. Scheduling Optimization
- **O(1) Scheduling**: Use multi-level queues for O(1) scheduling
- **CPU Affinity**: Support CPU affinity settings
- **Real-time Support**: Support real-time scheduling policies

## Security Design

### 1. Memory Protection
- **User/Kernel Separation**: Hardware-level memory protection
- **ASLR**: Address Space Layout Randomization
- **Stack Protection**: Stack canaries and non-executable stack

### 2. Access Control
- **Capability System**: Capability-based access control
- **Permission Isolation**: Principle of least privilege
- **Audit Logging**: System call auditing

### 3. Defense Mechanisms
- **SMEP/SMAP**: Prevent kernel from executing user code
- **NX Bit**: Non-executable data protection
- **KPTI**: Kernel Page Table Isolation

## Summary

Vest-OS adopts modern microkernel architecture design with the following characteristics:

1. **Modular Design**: Clear module boundaries for easier development and maintenance
2. **Architecture Compatibility**: Support for both 32-bit and 64-bit architectures
3. **Scalability**: Good extension mechanisms for adding new features
4. **Security**: Multi-layer security protection mechanisms
5. **Performance**: Optimization for performance-critical paths

This architecture design provides a solid foundation for Vest-OS, supporting future feature expansion and performance optimization.