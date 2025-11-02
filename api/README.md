# Vest-OS API Reference Documentation

This directory contains the complete API reference documentation for the Vest-OS operating system.

## Documentation Structure

### [System Call API](syscalls/README.md)
- Process Management API
- File System API
- Memory Management API
- Inter-Process Communication API
- Network API
- Device Control API

### [Library Function API](libraries/README.md)
- Standard C Library (libc)
- Thread Library (pthread)
- Network Library (libnet)
- Graphics Library (libgui)
- Cryptography Library (libcrypto)

### [Device Driver API](drivers/README.md)
- Character Device Drivers
- Block Device Drivers
- Network Device Drivers
- USB Device Drivers
- PCI Device Drivers

### [Network API](network/README.md)
- Socket API
- Network Configuration API
- Routing API
- Firewall API
- Network Monitoring API

### [Security API](security/README.md)
- Authentication API
- Authorization API
- Encryption API
- Key Management API
- Security Policy API

## API Usage Guidelines

### Version Control
- API version follows Semantic Versioning (SemVer)
- Major version: Incompatible API changes
- Minor version: Backward-compatible functional additions
- Patch version: Backward-compatible bug fixes

### Error Handling
All API calls should check return values:
```c
#include <errno.h>
#include <stdio.h>

int ret = some_api_call();
if (ret < 0) {
    fprintf(stderr, "API failed: %s\n", strerror(-ret));
    // Handle error
}
```

### Thread Safety
- APIs marked as thread-safe can be used in multi-threaded environments
- Non-thread-safe APIs require external synchronization
- Some APIs provide thread-safe versions (suffixed with _ts)

### Memory Management
- Memory allocated by APIs must be freed by the caller
- Use paired allocation/deallocation functions
- Follow RAII principles to avoid resource leaks

## Example Code

For complete usage examples, please refer to the example sections in each API documentation.

---

*API Documentation Version: 1.0*
*Last updated: January 2024*