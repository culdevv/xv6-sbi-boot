.section .text
.globl _start

_start:
    # OpenSBI passes hart ID in a0
    # Set up stack
    la sp, stack_top

    # Jump to C program
    call main

stop:
    j stop

.section .bss
.align 12
stack:
    .space 4096
stack_top:
