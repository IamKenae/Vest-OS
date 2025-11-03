; Vest-OS Minimal Kernel
; A simple kernel that prints a message and halts

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002      ; Magic number
    dd 0x00000003      ; Flags
    dd -(0x1BADB002 + 0x00000003) ; Checksum

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top

    ; Clear screen (simple approach)
    mov edi, 0xB8000
    mov eax, 0x07200720  ; Space with white color
    mov ecx, 2000         ; 80*25 characters
    cld
    rep stosd

    ; Print welcome message
    mov esi, welcome_msg
    mov edi, 0xB8000
    call print_string

    ; Print system info
    mov esi, sys_info
    mov edi, 0xB8000 + 160  ; Next line
    call print_string

    ; Print status
    mov esi, status_msg
    mov edi, 0xB8000 + 320  ; Third line
    call print_string

    ; Halt the system
.halt_loop:
    hlt
    jmp .halt_loop

print_string:
    ; Print string at ESI to screen at EDI
    mov ah, 0x0F  ; White color
.print_char:
    lodsb
    cmp al, 0
    je .done
    stosw
    jmp .print_char
.done:
    ret

welcome_msg db "========================================", 0
sys_info    db "      Welcome to Vest-OS v0.1.0      ", 0
status_msg  db "    Kernel loaded successfully!     ", 0

section .bss
align 32
stack_bottom:
    resb 16 * 1024     ; 16KB stack
stack_top: