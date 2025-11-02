; Vest-OS 引导加载程序
; 兼容Multiboot规范的引导程序

[BITS 32]

[ORG 0x100000]

; Multiboot头
multiboot_header:
    dd 0x1BADB002          ; Multiboot魔数
    dd 0x00000003          ; 标志 (页对齐 + 内存信息)
    dd -(0x1BADB002 + 0x00000003)  ; 校验和
    dd multiboot_header    ; 头地址
    dd 0x00000000          ; 加载地址
    dd 0x00000000          ; 结束地址
    dd 0x00000000          ; BSS结束地址
    dd multiboot_entry     ; 入口地址

; 引导程序入口点
multiboot_entry:
    ; 栈设置
    mov esp, stack_bottom

    ; 保存Multiboot信息
    mov dword [multiboot_magic], eax
    mov dword [multiboot_info], ebx

    ; 检查Multiboot魔数
    cmp eax, 0x2BADB002
    jne .error

    ; 清屏
    call clear_screen

    ; 显示启动信息
    mov esi, boot_message
    call print_string

    ; 检查CPU
    call check_cpu

    ; 检查内存
    call check_memory

    ; 加载内核
    call load_kernel

    ; 跳转到内核
    jmp kernel_entry

.error:
    mov esi, error_message
    call print_string
.hang:
    hlt
    jmp .hang

; 清屏函数
clear_screen:
    pusha
    mov edi, 0xB8000
    mov eax, 0x07200720  ; 空格字符和属性
    mov ecx, 2000        ; 80x25/2 (字数)
    cld
    rep stosd
    popa
    ret

; 打印字符串函数
print_string:
    pusha
    mov edi, 0xB8000
    mov ah, 0x07         ; 灰底黑字
.loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .loop
.done:
    popa
    ret

; 检查CPU
check_cpu:
    pusha
    mov esi, cpu_check_msg
    call print_string

    ; 检查CPUID支持
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    jz .no_cpuid

    ; 获取CPU信息
    mov eax, 0
    cpuid
    mov dword [cpu_vendor], ebx
    mov dword [cpu_vendor + 4], edx
    mov dword [cpu_vendor + 8], ecx

    mov esi, cpu_vendor
    call print_string

    mov esi, newline
    call print_string
    popa
    ret

.no_cpuid:
    mov esi, cpu_error
    call print_string
    jmp .hang

; 检查内存
check_memory:
    pusha
    mov esi, memory_check_msg
    call print_string

    ; 从Multiboot信息获取内存大小
    mov ebx, [multiboot_info]
    mov eax, [ebx + 8]     ; mem_lower
    mov ecx, [ebx + 12]    ; mem_upper
    add eax, ecx

    ; 转换为字符串
    call int_to_string
    mov esi, eax
    call print_string

    mov esi, kb_msg
    call print_string

    popa
    ret

; 加载内核
load_kernel:
    pusha
    mov esi, load_msg
    call print_string

    ; 内核已经在0x100000处，直接设置跳转地址
    mov dword [kernel_entry], kernel_start

    mov esi, load_done_msg
    call print_string
    popa
    ret

; 整数转字符串（简单版本）
int_to_string:
    pusha
    mov ebx, 10
    xor ecx, ecx
    mov edi, number_buffer + 19
    mov byte [edi], 0
    dec edi

.divide:
    xor edx, edx
    div ebx
    add dl, '0'
    mov [edi], dl
    dec edi
    inc ecx
    test eax, eax
    jnz .divide

    inc edi
    mov eax, edi
    popa
    ret

; 数据段
section .data
boot_message    db "Vest-OS 引导加载程序 v0.1", 13, 10, 0
error_message   db "错误: Multiboot规范不兼容", 13, 10, 0
cpu_check_msg   db "检查CPU: ", 0
cpu_error       db "错误: CPU不支持CPUID指令", 13, 10, 0
memory_check_msg db "检查内存: ", 0
load_msg        db "加载内核...", 13, 10, 0
load_done_msg   db "内核加载完成", 13, 10, 0
newline         db 13, 10, 0
kb_msg          db " KB", 0

section .bss
multiboot_magic: resd 1
multiboot_info:  resd 1
cpu_vendor:     resb 13
kernel_entry:   resd 1
number_buffer:  resb 20
stack_bottom:   resb 4096  ; 4KB栈空间

; 内核符号（将在链接时解析）
extern kernel_start