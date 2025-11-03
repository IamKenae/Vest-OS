/* Vest-OS Linker Script */
ENTRY(multiboot_entry)

SECTIONS
{
    /* Begin at 1MB */
    . = 1M;

    /* Multiboot header and code */
    .text :
    {
        *(.text)
        *(.rodata*)
    }

    /* Data section */
    .data :
    {
        *(.data)
    }

    /* BSS section */
    .bss :
    {
        *(.bss)
        *(COMMON)
    }

    /* End of kernel */
    . = ALIGN(4096);
    end = .;

    /* Discard sections we don't need */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
}