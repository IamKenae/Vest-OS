; Vest-OS 64位内核启动代码
; Multiboot2兼容启动代码

[BITS 32]

; Multiboot2头定义
MULTIBOOT2_MAGIC      equ 0x36d76289
MULTIBOOT2_ARCH       equ 0     ; i386
MULTIBOOT2_HEADER_LEN equ header_end - header_start
MULTIBOOT2_CHECKSUM   equ -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_HEADER_LEN)

section .multiboot
align 8
header_start:
    dd MULTIBOOT2_MAGIC
    dd MULTIBOOT2_ARCH
    dd MULTIBOOT2_HEADER_LEN
    dd MULTIBOOT2_CHECKSUM

    ; 结束标签
    dd 0
    dd 8
header_end:

; 内核栈
section .bss
align 16
stack_bottom:
    resb 16384 ; 16KB栈空间
stack_top:

; 内核入口点
section .text
global _start
extern kernel_main

_start:
    ; 栈设置
    mov esp, stack_top

    ; 保存Multiboot2信息
    push eax      ; Multiboot2魔数
    push ebx      ; Multiboot2信息结构

    ; 检查CPU是否支持64位模式
    call check_cpuid
    call check_long_mode

    ; 设置基本页表
    call setup_page_tables

    ; 启用分页
    mov eax, page_table_l4
    mov cr3, eax

    ; 启用PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 设置长模式
    mov ecx, 0xC0000080  ; EFER MSR
    rdmsr
    or eax, 1 << 8       ; LME
    wrmsr

    ; 启用分页和长模式
    mov eax, cr0
    or eax, 1 << 31 | 1 << 0  ; PG和PE
    mov cr0, eax

    ; 远跳转到64位代码
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

; 检查CPUID支持
check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    mov esi, error_no_cpuid
    call error
    ret

; 检查长模式支持
check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov esi, error_no_long_mode
    call error
    ret

; 设置页表
setup_page_tables:
    ; 映射前2GB内存，使用2MB页面
    mov eax, page_table_l3
    or eax, 0b11  ; present + writable
    mov [page_table_l4], eax

    mov eax, page_table_l2
    or eax, 0b11
    mov [page_table_l3], eax

    ; 创建512个2MB页表项，映射前1GB
    mov ecx, 0
.map_p2_entry:
    mov eax, 0x200000  ; 2MB
    mul ecx
    or eax, 0b10000011  ; present + writable + huge page
    mov [page_table_l2 + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2_entry
    ret

; 错误处理
error:
    ; 简单的错误输出到VGA缓冲区
    mov edi, 0xB8000
    mov ah, 0x4F  ; 白底红字
.loop:
    lodsb
    test al, al
    jz .hang
    stosw
    jmp .loop
.hang:
    cli
    hlt
    jmp .hang

; 64位代码段
[BITS 64]
section .text
long_mode_start:
    ; 更新栈指针为高地址
    mov rsp, stack_top + 0xFFFFFFFF80000000

    ; 调用C语言内核主函数
    call kernel_main

    ; 如果kernel_main返回，进入死循环
.hang:
    cli
    hlt
    jmp .hang

; 64位GDT
section .rodata
gdt64:
    ; 空描述符
    dq 0

    ; 内核代码段 (64位)
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)

    ; 内核数据段 (64位)
    dq (1<<44) | (1<<47) | (1<<41)

    ; 用户代码段 (64位)
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53) | (3<<45)

    ; 用户数据段 (64位)
    dq (1<<44) | (1<<47) | (1<<41) | (3<<45)

.pointer:
    dw $ - gdt64 - 1
    dq gdt64

; 错误消息
section .rodata
error_no_cpuid: db "错误: CPU不支持CPUID指令", 0
error_no_long_mode: db "错误: CPU不支持长模式", 0

; 页表
section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 4096