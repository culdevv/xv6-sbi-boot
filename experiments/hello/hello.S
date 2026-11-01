    .include "common.inc"
    .section .text.init
    .globl _start
_start:
    mv   s0, a0              # hartid
    mv   s1, a1              # dia chi DTB
    la   sp, stack_top
    STR  msg
    HEX  s0
    NL
    li   a7, 0x53525354      # SRST: system_reset
    li   a6, 0
    li   a0, 0               # type 0 = shutdown
    li   a1, 0               # reason
    ecall
1:  wfi
    j    1b
    .include "lib.inc"
    .section .rodata
msg: .asciz "hello S-mode, hartid="
    .section .bss
    .align 4
stack:     .space 4096
stack_top:
