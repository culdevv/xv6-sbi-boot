OUTPUT_ARCH(riscv)
ENTRY(_entry)
SECTIONS
{
  . = 0x80200000;
  .text   : { *(.text.entry) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data   : { *(.data .data.*) }
  .bss    : { *(.bss .bss.*) *(COMMON) }
}
