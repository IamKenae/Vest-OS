; Vest-OS 32位内核启动代码
; Multiboot兼容启动代码

[BITS 32]

; Multiboot头定义
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

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

    ; 保存Multiboot信息
    push eax      ; Multiboot魔数
    push ebx      ; Multiboot信息结构

    ; 清除BSS段
    call clear_bss

    ; 调用C语言内核主函数
    call kernel_main

    ; 如果kernel_main返回，进入死循环
.hang:
    hlt
    jmp .hang

; 清除BSS段
clear_bss:
    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    ret

; GDT定义
section .rodata
gdt:
    ; 空描述符
    dq 0

    ; 内核代码段 (32位)
    dw 0xFFFF       ; 段界限15:0
    dw 0x0000       ; 段基址15:0
    db 0x00         ; 段基址23:16
    db 10011010b    ; 类型，P=1, DPL=0, S=1, 类型=代码段
    db 11001111b    ; 段界限19:16, G=1, D=1, L=0, AVL=0
    db 0x00         ; 段基址31:24

    ; 内核数据段 (32位)
    dw 0xFFFF       ; 段界限15:0
    dw 0x0000       ; 段基址15:0
    db 0x00         ; 段基址23:16
    db 10010010b    ; 类型，P=1, DPL=0, S=1, 类型=数据段
    db 11001111b    ; 段界限19:16, G=1, D=1, L=0, AVL=0
    db 0x00         ; 段基址31:24

    ; 用户代码段 (32位)
    dw 0xFFFF       ; 段界限15:0
    dw 0x0000       ; 段基址15:0
    db 0x00         ; 段基址23:16
    db 11111010b    ; 类型，P=1, DPL=3, S=1, 类型=代码段
    db 11001111b    ; 段界限19:16, G=1, D=1, L=0, AVL=0
    db 0x00         ; 段基址31:24

    ; 用户数据段 (32位)
    dw 0xFFFF       ; 段界限15:0
    dw 0x0000       ; 段基址15:0
    db 0x00         ; 段基址23:16
    db 11110010b    ; 类型，P=1, DPL=3, S=1, 类型=数据段
    db 11001111b    ; 段界限19:16, G=1, D=1, L=0, AVL=0
    db 0x00         ; 段基址31:24

gdt_end:

gdt_ptr:
    dw gdt_end - gdt - 1    ; GDT界限
    dd gdt                  ; GDT基址

; 段选择子定义
%define KERNEL_CS 0x08  ; 内核代码段选择子
%define KERNEL_DS 0x10  ; 内核数据段选择子
%define USER_CS   0x18  ; 用户代码段选择子
%define USER_DS   0x20  ; 用户数据段选择子

; 加载GDT
global load_gdt
load_gdt:
    lgdt [gdt_ptr]

    ; 重新加载段寄存器
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 远跳转更新CS
    jmp KERNEL_CS:.reload_cs
.reload_cs:
    ret

; 中断处理程序入口
global interrupt_entry
interrupt_entry:
    ; 保存所有寄存器
    pushad
    push ds
    push es
    push fs
    push gs

    ; 加载内核数据段
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 调用C语言中断处理函数
    call c_interrupt_handler

    ; 恢复寄存器
    pop gs
    pop fs
    pop es
    pop ds
    popad

    ; 返回中断
    iret

; TSS和任务状态段
section .bss
align 16
tss:
    resb 104  ; TSS大小104字节

section .rodata
; TSS描述符
tss_descriptor:
    dw 0x0067       ; 段界限15:0 (103)
    dw 0x0000       ; 段基址15:0
    db 0x00         ; 段基址23:16
    db 10001001b    ; 类型=可用TSS32, P=1, DPL=0
    db 00000000b    ; 段界限19:16, G=0, D=0, L=0, AVL=0
    db 0x00         ; 段基址31:24