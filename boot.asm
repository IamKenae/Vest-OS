; Vest-OS Boot Assembly Code
; Multiboot-compliant kernel entry point

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002      ; Magic number
    dd 0x00000003      ; Flags
    dd -(0x1BADB002 + 0x00000003) ; Checksum
    dd multiboot_header ; Header address
    dd 0x00100000      ; Load address
    dd 0x00100000      ; Load end address
    dd 0x00104000      ; BSS end address (16KB after load)
    dd multiboot_entry  ; Entry point

section .text
global multiboot_entry
extern kernel_main

multiboot_entry:
    ; Set up stack
    mov esp, stack_top

    ; Push multiboot magic and info pointer
    push eax            ; Magic number (passed by bootloader)
    push ebx            ; Multiboot info pointer

    ; Call C kernel main function
    call kernel_main

    ; If kernel_main returns, halt the system
.halt_loop:
    hlt
    jmp .halt_loop

section .bss
align 32
stack_bottom:
    resb 16 * 1024     ; 16KB stack
stack_top: