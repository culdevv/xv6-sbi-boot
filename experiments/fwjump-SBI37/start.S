.section .text.entry
.globl _start

_start:
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    la sp, stack_top
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
