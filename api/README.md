# Vest-OS API 参考文档

本目录包含Vest-OS操作系统的完整API参考文档。

## 文档结构

### [系统调用API](syscalls/README.md)
- 进程管理API
- 文件系统API
- 内存管理API
- 进程间通信API
- 网络API
- 设备控制API

### [库函数API](libraries/README.md)
- 标准C库 (libc)
- 线程库 (pthread)
- 网络库 (libnet)
- 图形库 (libgui)
- 加密库 (libcrypto)

### [设备驱动API](drivers/README.md)
- 字符设备驱动
- 块设备驱动
- 网络设备驱动
- USB设备驱动
- PCI设备驱动

### [网络API](network/README.md)
- Socket API
- 网络配置API
- 路由API
- 防火墙API
- 网络监控API

### [安全API](security/README.md)
- 认证API
- 授权API
- 加密API
- 密钥管理API
- 安全策略API

## API使用指南

### 版本控制
- API版本遵循语义化版本控制 (SemVer)
- 主版本号：不兼容的API修改
- 次版本号：向下兼容的功能性新增
- 修订号：向下兼容的问题修正

### 错误处理
所有API调用都应检查返回值：
```c
#include <errno.h>
#include <stdio.h>

int ret = some_api_call();
if (ret < 0) {
    fprintf(stderr, "API failed: %s\n", strerror(-ret));
    // 处理错误
}
```

### 线程安全
- 标记为线程安全的API可在多线程环境中使用
- 非线程安全的API需要外部同步
- 某些API提供线程安全版本（以_ts结尾）

### 内存管理
- API分配的内存必须由调用者释放
- 使用配对的分配/释放函数
- 遵循RAII原则，避免资源泄露

## 示例代码

完整的使用示例请参考各API文档中的示例章节。

---

*API文档版本：1.0*
*最后更新：2024年1月*