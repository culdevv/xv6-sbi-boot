.section .text.entry
.globl _start

_start:

    # Initialize global pointer
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # Initialize stack pointer
    la sp, stack_top

    # Call C kernel
    call kmain

1:
    wfi
    j 1b

.section .bss
.align 4

.globl stack0
stack0:
    .space 4096

.globl stack_top
stack_top:
