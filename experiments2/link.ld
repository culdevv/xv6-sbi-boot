OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80200000;

    .text : ALIGN(4)
    {
        KEEP(*(.text.entry))
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(8)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(8)
    {
        *(.data)
        *(.data.*)
    }

    .sdata : ALIGN(8)
    {
        __global_pointer$ = . + 0x800;
        *(.sdata)
        *(.sdata.*)
    }

    .bss : ALIGN(16)
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    .sbss : ALIGN(8)
    {
        *(.sbss)
        *(.sbss.*)
    }

    _end = .;

    /DISCARD/ :
    {
        *(.note*)
        *(.eh_frame*)
        *(.comment)
    }
}
