# Vest-OS 故障排除指南

## 目录
- [启动问题](#启动问题)
- [系统性能问题](#系统性能问题)
- [网络问题](#网络问题)
- [存储问题](#存储问题)
- [内存问题](#内存问题)
- [进程问题](#进程问题)
- [设备驱动问题](#设备驱动问题)
- [应用程序问题](#应用程序问题)
- [内核崩溃](#内核崩溃)
- [调试工具](#调试工具)

## 启动问题

### 1. 系统无法启动

#### 症状：黑屏，无任何输出
**可能原因：**
- 引导加载器损坏
- 内核文件损坏
- 硬件故障

**解决步骤：**
```bash
# 1. 检查引导加载器
# 使用启动介质进入救援模式
sudo mount /dev/sda1 /mnt
sudo grub-install --boot-directory=/mnt/boot /dev/sda

# 2. 检查内核文件
ls -la /mnt/boot/vmlinuz-*
ls -la /mnt/boot/initramfs-*

# 3. 重新安装内核
sudo apt install --reinstall linux-image-$(uname -r)

# 4. 检查硬件
# 运行内存测试
memtest86

# 检查磁盘
sudo smartctl -a /dev/sda
```

#### 症状：引导循环
**可能原因：**
- 文件系统损坏
- 配置错误
- 硬件问题

**解决步骤：**
```bash
# 1. 进入恢复模式
# 在GRUB菜单选择Advanced options -> recovery mode

# 2. 检查文件系统
fsck -y /dev/sda1
fsck -y /dev/sda2

# 3. 检查关键配置文件
cat /etc/fstab
cat /etc/default/grub

# 4. 重新生成initramfs
update-initramfs -u -k all

# 5. 更新GRUB
update-grub
```

#### 症状：内核恐慌（Kernel Panic）
**常见错误信息：**
```
Kernel panic - not syncing: Attempted to kill init!
Kernel panic - not syncing: VFS: Unable to mount root fs
```

**解决步骤：**
```bash
# 1. 记录完整的panic信息
# 2. 检查内核参数
cat /proc/cmdline

# 3. 检查init进程
ls -la /sbin/init

# 4. 检查root文件系统
mount /dev/sda1 /mnt
ls -la /mnt/sbin/init
ls -la /mnt/lib/systemd/systemd

# 5. 临时修复
# 在GRUB编辑模式添加内核参数：
# init=/bin/bash
# 然后手动修复系统
```

### 2. 启动速度慢

#### 诊断命令
```bash
# 分析启动时间
systemd-analyze
systemd-analyze blame
systemd-analyze critical-chain

# 查看启动日志
journalctl -b -p err
journalctl -b -p warning

# 查看服务启动时间
systemd-analyze plot > boot.svg
```

#### 优化措施
```bash
# 1. 禁用不必要的服务
systemctl disable bluetooth
systemctl disable cups
systemctl disable avahi-daemon

# 2. 并行化启动
# 编辑 /etc/systemd/system.conf
# DefaultDependencies=yes
# DefaultTimeoutStartSec=90s

# 3. 优化文件系统检查
# 编辑 /etc/fstab，添加noatime选项
/dev/sda1 / ext4 defaults,noatime 0 1

# 4. 启用预读
echo 'READAHEAD="4096"' >> /etc/initramfs-tools/conf.d/resume
update-initramfs -u
```

## 系统性能问题

### 1. CPU使用率过高

#### 诊断步骤
```bash
# 1. 查看CPU使用情况
top -p $(pidof process_name)
htop

# 2. 查看CPU详情
mpstat -P ALL 1
iostat -c 1

# 3. 查看进程CPU使用
ps aux --sort=-%cpu | head

# 4. 查看系统调用
strace -p <pid>

# 5. 查看热点代码
perf top -p <pid>
```

#### 常见原因及解决

**死循环：**
```bash
# 找到问题进程
ps aux | grep -i <process>

# 使用GDB调试
gdb -p <pid>
(gdb) thread apply all bt
(gdb) info registers
(gdb) p $eip
(gdb) x/10i $eip
```

**大量中断：**
```bash
# 查看中断统计
cat /proc/interrupts
watch -n 1 'cat /proc/interrupts'

# 查看软中断
cat /proc/softirqs

# 优化中断亲和性
echo 2 > /proc/irq/<irq>/smp_affinity
```

### 2. 内存使用过高

#### 诊断步骤
```bash
# 1. 查看内存使用
free -h
cat /proc/meminfo

# 2. 查看进程内存使用
ps aux --sort=-%mem | head
pmap -x <pid>

# 3. 查看内存详细信息
vmstat 1
sar -r 1

# 4. 查找内存泄漏
valgrind --leak-check=full ./program
```

#### 内存泄漏检测
```c
// 编译时添加调试标志
gcc -g -fsanitize=address program.c

// 运行程序
./program

// 或使用mtrace
export MALLOC_TRACE=mtrace.log
./program
mtrace program mtrace.log
```

#### 内存优化
```bash
# 1. 清理系统缓存
echo 3 > /proc/sys/vm/drop_caches

# 2. 调整swap使用
echo 10 > /proc/sys/vm/swappiness

# 3. 限制进程内存
ulimit -v unlimited
prlimit --as=104857600 ./program

# 4. 使用cgroup限制
systemctl set-property user-1000.slice MemoryLimit=512M
```

### 3. 磁盘I/O性能问题

#### 诊断步骤
```bash
# 1. 查看磁盘使用
iostat -xz 1
iotop

# 2. 查看磁盘活动
vmstat 1
sar -d 1

# 3. 查找I/O密集进程
pidstat -d 1

# 4. 分析I/O延迟
bpftrace -e 'tracepoint:block:block_rq_complete { @[args->dev, args->cmd] = hist(args->duration); }'
```

#### 优化措施
```bash
# 1. 调整I/O调度器
echo mq-deadline > /sys/block/sda/queue/scheduler

# 2. 调整队列深度
echo 128 > /sys/block/sda/queue/nr_requests

# 3. 启用写入缓存
echo write back > /sys/block/sda/queue/write_cache

# 4. 使用SSD优化
echo 0 > /sys/block/sda/queue/rotational
```

## 网络问题

### 1. 网络连接失败

#### 诊断步骤
```bash
# 1. 检查网络接口
ip addr show
ethtool eth0

# 2. 检查路由表
ip route show
route -n

# 3. 检查DNS
nslookup google.com
dig google.com

# 4. 检查连通性
ping -c 4 8.8.8.8
traceroute google.com

# 5. 检查端口
telnet google.com 80
nmap -p 80 google.com
```

#### 常见问题解决

**IP地址冲突：**
```bash
# 查找冲突的IP
arp-scan -l

# 查看ARP表
arp -a

# 清除ARP缓存
ip -s -s neigh flush all
```

**DNS解析失败：**
```bash
# 测试DNS服务器
nslookup google.com 8.8.8.8
nslookup google.com 1.1.1.1

# 重启网络服务
systemctl restart systemd-resolved

# 清除DNS缓存
systemd-resolve --flush-caches
```

### 2. 网络性能差

#### 性能测试
```bash
# 带宽测试
iperf3 -c server_ip -t 60
speedtest-cli

# 延迟测试
ping -c 100 google.com | tail -1
mtr google.com

# 吞吐量测试
netperf -H server_ip -t TCP_STREAM
```

#### 优化配置
```bash
# 1. 调整TCP窗口
echo 'net.core.rmem_max = 16777216' >> /etc/sysctl.conf
echo 'net.core.wmem_max = 16777216' >> /etc/sysctl.conf

# 2. 启用TCP窗口缩放
echo 'net.ipv4.tcp_window_scaling = 1' >> /etc/sysctl.conf

# 3. 调整队列长度
echo 'net.core.netdev_max_backlog = 5000' >> /etc/sysctl.conf

# 4. 应用配置
sysctl -p
```

### 3. 防火墙问题

#### 诊断防火墙
```bash
# 查看防火墙状态
ufw status verbose
firewall-cmd --list-all
iptables -L -n

# 查看日志
journalctl -u ufw
tail -f /var/log/iptables.log
```

#### 解决方案
```bash
# 1. 临时关闭防火墙测试
ufw disable
# 或
iptables -F

# 2. 添加规则
ufw allow 22/tcp
ufw allow 80/tcp

# 3. 高级规则
iptables -A INPUT -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -p tcp --dport 443 -j ACCEPT
```

## 存储问题

### 1. 磁盘空间不足

#### 查找大文件
```bash
# 查看磁盘使用
df -h
du -sh /* | sort -rh | head

# 查找大文件
find / -type f -size +100M 2>/dev/null
du -a / | sort -rh | head -n 20

# 查看inode使用
df -i
```

#### 清理空间
```bash
# 1. 清理包缓存
apt-get clean
apt-get autoremove

# 2. 清理日志
journalctl --vacuum-time=7d
rm -rf /var/log/*.gz

# 3. 清理临时文件
rm -rf /tmp/*
rm -rf /var/tmp/*

# 4. 查找并删除重复文件
fdupes -r /home/user
```

### 2. 文件系统损坏

#### 检测和修复
```bash
# 1. 检查文件系统
fsck -n /dev/sda1  # 只检查不修复

# 2. 修复文件系统（需要卸载）
umount /dev/sda1
fsck -y /dev/sda1

# 3. 强制检查
touch /forcefsck
reboot

# 4. 查看超级块
dumpe2fs /dev/sda1 | head
```

#### 数据恢复
```bash
# 1. 使用testdisk恢复分区
testdisk /dev/sda

# 2. 使用photorec恢复文件
photorec /dev/sda1

# 3. 使用dd创建镜像
dd if=/dev/sda of=disk.img bs=4M conv=noerror,sync

# 4. 使用foremost恢复文件
foremost -i disk.img -o recovered/
```

### 3. RAID问题

#### RAID状态检查
```bash
# 查看RAID状态
cat /proc/mdstat
mdadm --detail /dev/md0

# 查看磁盘状态
mdadm --examine /dev/sda1

# 监控RAID
watch -n 1 'cat /proc/mdstat'
```

#### RAID恢复
```bash
# 1. 添加新磁盘
mdadm --add /dev/md0 /dev/sdb1

# 2. 重建阵列
mdadm --assemble /dev/md0 /dev/sda1 /dev/sdb1

# 3. 停止阵列
mdadm --stop /dev/md0

# 4. 创建备份
dd if=/dev/md0 of=raid-backup.img
```

## 内存问题

### 1. OOM Killer触发

#### 诊断OOM
```bash
# 查看OOM日志
dmesg | grep -i "killed process"
journalctl -k | grep -i oom

# 查看内存使用
cat /proc/meminfo | grep -E "(MemTotal|MemFree|MemAvailable)"

# 查看进程OOM分数
cat /proc/<pid>/oom_score
```

#### OOM调优
```bash
# 1. 调整OOM killer行为
echo -1000 > /proc/<pid>/oom_score_adj

# 2. 配置系统OOM
echo 'vm.panic_on_oom = 1' >> /etc/sysctl.conf

# 3. 限制进程内存
systemctl set-property myservice.service MemoryLimit=2G

# 4. 使用cgroup
cgcreate -g memory:limited
echo 1G > /sys/fs/cgroup/memory/limited/memory.limit_in_bytes
```

### 2. 内存碎片

#### 检测碎片
```bash
# 查看内存碎片
cat /proc/buddyinfo
cat /proc/pagetypeinfo

# 查看slab信息
cat /proc/slabinfo
slabtop

# 监控内存分配
perf record -e kmem:kmalloc -a
perf report
```

#### 减少碎片
```bash
# 1. 配置内存回收
echo 'vm.min_free_kbytes = 65536' >> /etc/sysctl.conf

# 2. 调整zone reclaim
echo 'vm.zone_reclaim_mode = 0' >> /etc/sysctl.conf

# 3. 使用hugepages
echo 1024 > /proc/sys/vm/nr_hugepages
```

## 进程问题

### 1. 僵尸进程

#### 查找僵尸进程
```bash
# 查看僵尸进程
ps aux | grep Z
ps -ef | grep defunct

# 查看进程树
pstree -p
```

#### 清理僵尸进程
```bash
# 1. 找到父进程
ps -eo ppid,stat | grep Z

# 2. 终止父进程
kill -9 <ppid>

# 3. 使用init进程（PID 1）回收
kill -CHRP 1
```

### 2. 进程无响应

#### 调试无响应进程
```bash
# 1. 查看进程状态
ps aux | grep <process>
cat /proc/<pid>/status

# 2. 查看堆栈
cat /proc/<pid>/stack
pstack <pid>

# 3. 跟踪系统调用
strace -p <pid>

# 4. 查看文件描述符
ls -la /proc/<pid>/fd
lsof -p <pid>
```

#### 解决方法
```bash
# 1. 发送信号
kill -TERM <pid>  # 正常终止
kill -KILL <pid>  # 强制终止

# 2. 使用GDB调试
gdb -p <pid>
(gdb) continue
(gdb) interrupt
(gdb) bt full

# 3. 查看锁信息
cat /proc/<pid>/locks
```

## 设备驱动问题

### 1. 设备未识别

#### 检查设备
```bash
# 查看所有设备
lspci
lsusb
lshw -short

# 查看内核日志
dmesg | grep -i usb
dmesg | grep -i pci

# 查看驱动状态
lspci -k
lsmod | grep <driver>
```

#### 安装驱动
```bash
# 1. 编译模块
make -C /lib/modules/$(uname -r)/build M=$PWD modules

# 2. 安装模块
insmod driver.ko
modprobe driver

# 3. 永久安装
echo driver >> /etc/modules
```

### 2. 驱动崩溃

#### 调试驱动崩溃
```bash
# 1. 查看oops信息
dmesg | tail
cat /var/log/kern.log

# 2. 使用crash工具
crash /usr/lib/debug/lib/modules/$(uname -r)/vmlinux /proc/vmcore

# 3. 启用调试选项
echo '1' > /proc/sys/kernel/sysrq
echo 'c' > /proc/sysrq-trigger
```

## 应用程序问题

### 1. 程序崩溃

#### 分析core dump
```bash
# 1. 启用core dump
ulimit -c unlimited
echo '/tmp/core-%e-%p-%t' > /proc/sys/kernel/core_pattern

# 2. 分析core文件
gdb ./program core.1234
(gdb) bt full
(gdb) info registers
(gdb) list

# 3. 使用addr2line
addr2line -e ./program 0x400123
```

### 2. 程序性能差

#### 性能分析
```bash
# 1. CPU性能分析
perf record -g ./program
perf report

# 2. 内存性能分析
valgrind --tool=massif ./program
ms_print massif.out.*

# 3. I/O性能分析
strace -c -o trace.log ./program
```

## 内核崩溃

### 1. Oops分析

#### 收集Oops信息
```bash
# 1. 保存oops信息
dmesg > oops.txt
journalctl -k > kern.log

# 2. 解码符号
addr2line -e /usr/lib/debug/lib/modules/$(uname -r)/vmlinux <addr>

# 3. 使用ksymoops
ksymoops vmlinux < oops.txt
```

### 2. 使用kdump

#### 配置kdump
```bash
# 1. 安装kexec-tools
apt install kexec-tools

# 2. 配置crashkernel
# 编辑 /etc/default/grub
# GRUB_CMDLINE_LINUX_DEFAULT="crashkernel=128M"

# 3. 启用kdump
systemctl enable kdump
systemctl start kdump

# 4. 测试kdump
echo c > /proc/sysrq-trigger
```

#### 分析vmcore
```bash
# 1. 安装crash
apt install crash

# 2. 调试vmcore
crash /usr/lib/debug/lib/modules/$(uname -r)/vmlinux /var/crash/vmcore

# 3. 常用crash命令
crash> bt
crash> sys
crash> ps
crash> files
crash> net
```

## 调试工具参考

### 内核调试工具
- **ftrace**: 函数跟踪
- **perf**: 性能分析
- **eBPF/bpftrace**: 动态跟踪
- **kprobes/uprobes**: 动态探测点
- **systemtap**: 系统探测

### 用户空间调试工具
- **gdb**: 源码级调试
- **strace/ltrace**: 系统调用/库函数跟踪
- **valgrind**: 内存调试
- **massif**: 堆分析
- **helgrind**: 线程错误检测

### 性能分析工具
- **perf**: Linux性能分析
- **sysstat**: 系统统计
- **iotop**: I/O监控
- **nethogs**: 网络监控
- **htop/atop**: 进程监控

---

*文档版本：1.0*
*最后更新：2024年1月*