# Vest-OS 文档索引

## 文档体系概览

Vest-OS提供了完整的文档体系，帮助用户、开发者和系统管理员了解和使用系统。

## 📚 文档结构

### 核心文档

| 文档 | 路径 | 描述 | 读者 |
|------|------|------|------|
| **系统总览** | [CLAUDE.md](CLAUDE.md) | 系统整体介绍、架构、快速开始 | 所有用户 |
| **构建部署** | [docs/build-deploy-guide.md](docs/build-deploy-guide.md) | 构建系统、编译、打包、部署 | 开发者、运维 |
| **开发指南** | [docs/developer-guide.md](docs/developer-guide.md) | 开发环境、编码规范、贡献流程 | 开发者 |
| **故障排除** | [docs/troubleshooting.md](docs/troubleshooting.md) | 常见问题、调试技巧、解决方案 | 所有用户 |

### API参考

| 文档 | 路径 | 描述 |
|------|------|------|
| **API总览** | [api/README.md](api/README.md) | API使用指南 |
| **系统调用** | [api/syscalls/README.md](api/syscalls/README.md) | 系统调用API参考 |
| **库函数** | [api/libraries/README.md](api/libraries/README.md) | 标准库和扩展库API |
| **驱动API** | [api/drivers/README.md](api/drivers/README.md) | 设备驱动开发API |
| **网络API** | [api/network/README.md](api/network/README.md) | 网络编程API |
| **安全API** | [api/security/README.md](api/security/README.md) | 安全相关API |

### 模块文档

| 模块 | 路径 | 描述 |
|------|------|------|
| **内核** | [modules/kernel/CLAUDE.md](modules/kernel/CLAUDE.md) | 内核架构、调度、内存管理 |
| **文件系统** | [modules/filesystem/CLAUDE.md](modules/filesystem/CLAUDE.md) | VFS、EXT4、BTRFS、分布式FS |
| **网络** | [modules/network/CLAUDE.md](modules/network/CLAUDE.md) | TCP/IP协议栈、网络设备 |
| **进程** | [modules/process/CLAUDE.md](modules/process/CLAUDE.md) | 进程管理、调度 |
| **设备** | [modules/device/CLAUDE.md](modules/device/CLAUDE.md) | 设备驱动框架 |
| **安全** | [modules/security/CLAUDE.md](modules/security/CLAUDE.md) | 安全机制、权限管理 |
| **UI** | [modules/ui/CLAUDE.md](modules/ui/CLAUDE.md) | 图形界面系统 |

## 🔍 快速导航

### 新用户
1. 阅读 [CLAUDE.md](CLAUDE.md) 了解系统概况
2. 查看 [快速开始](CLAUDE.md#快速开始) 安装系统
3. 参考 [使用指南](CLAUDE.md#使用指南) 学习基本操作

### 开发者
1. 阅读 [开发指南](docs/developer-guide.md) 搭建开发环境
2. 查看 [构建部署指南](docs/build-deploy-guide.md) 了解构建流程
3. 参考 [API文档](api/) 进行开发
4. 遵循 [贡献指南](docs/developer-guide.md#贡献指南) 提交代码

### 系统管理员
1. 阅读 [系统配置](CLAUDE.md#配置管理) 了解系统配置
2. 参考 [故障排除](docs/troubleshooting.md) 解决问题
3. 查看 [API参考](api/) 了解系统接口

## 📖 文档使用说明

### 文档格式
- 所有文档使用Markdown格式
- 代码块包含语法高亮
- 支持搜索和交叉引用

### 符号说明
- ✅ 已完成
- 🚧 开发中
- 📋 计划中

### 反馈与贡献
- 发现问题请提交Issue
- 改进建议欢迎提交PR
- 文档贡献遵循[贡献指南](docs/developer-guide.md#贡献指南)

## 🆘 获取帮助

### 在线资源
- **官方网站**: https://vest-os.org
- **文档网站**: https://docs.vest-os.org
- **API参考**: https://api.vest-os.org

### 社区支持
- **论坛**: https://forum.vest-os.org
- **邮件列表**: dev@vest-os.org
- **IRC**: #vest-os on Libera.Chat
- **GitHub**: https://github.com/vest-os/vest-os

### 商业支持
- **技术支持**: support@vest-os.org
- **培训**: training@vest-os.org
- **咨询**: consulting@vest-os.org

## 📅 文档更新日志

### v1.0.0 (2024-01-01)
- ✅ 创建主文档 CLAUDE.md
- ✅ 完成模块文档（内核、文件系统、网络）
- ✅ 完成API参考文档
- ✅ 完成构建和部署指南
- ✅ 完成开发者指南
- ✅ 完成故障排除指南

### 计划中 (v1.1.0)
- 📋 添加更多模块文档
- 📋 完善API文档
- 📋 添加更多示例代码
- 📋 创建视频教程

---

*最后更新：2024年1月*