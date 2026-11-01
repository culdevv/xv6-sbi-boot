    .include "common.inc"

    .section .text.init
    .globl _start
_start:
    mv   s0, a0
    mv   s1, a1
    la   sp, stack_top

    STR  m_hart
    HEX  s0
    NL
    STR  m_dtb
    HEX  s1
    NL
    STR  m_magic
    lwu  a0, 0(s1)           # 4 byte dau cua DTB
    call printhex
    NL

    # phien ban SBI (Base FID 0)
    STR  m_spec
    li   a7, 0x10
    li   a6, 0
    ecall
    mv   s5, a1
    HEX  s5
    NL

    # probe 4 extension (Base FID 3)
    la   s3, exts
    li   s4, 4
4:  ld   s6, 0(s3)
    HEX  s6
    STR  m_sp
    li   a7, 0x10
    li   a6, 3
    mv   a0, s6
    ecall
    mv   s5, a1
    HEX  s5
    NL
    addi s3, s3, 8
    addi s4, s4, -1
    bnez s4, 4b

    # EID khong ton tai
    STR  m_bad
    li   a7, 0x12345678
    li   a6, 0
    li   a0, 0
    ecall
    mv   s5, a0
    HEX  s5
    NL

    # rdtime delta qua mot vong lap
    STR  m_time
    rdtime s7
    li   t0, 10000000
5:  addi t0, t0, -1
    bnez t0, 5b
    rdtime s8
    sub  s5, s8, s7
    HEX  s5
    NL

    # HSM get_status cho cac hart trong bang hlist
    la   s3, hlist
    li   s4, 5
6:  ld   s9, 0(s3)
    STR  m_st
    HEX  s9
    STR  m_sp
    li   a7, 0x48534D
    li   a6, 2
    mv   a0, s9
    ecall
    mv   s6, a0
    mv   s5, a1
    HEX  s6
    STR  m_sp
    HEX  s5
    NL
    addi s3, s3, 8
    addi s4, s4, -1
    bnez s4, 6b

    # hart_start tren hart dang chay (0) va hart khong ton tai (99)
    STR  m_hs0
    li   a7, 0x48534D
    li   a6, 0
    mv   a0, 0
    la   a1, _start
    li   a2, 0
    ecall
    mv   s4, a0
    HEX  s4
    NL
    STR  m_hs99
    li   a7, 0x48534D
    li   a6, 0
    li   a0, 99
    la   a1, _start
    li   a2, 0
    ecall
    mv   s4, a0
    HEX  s4
    NL

    li   a7, 0x53525354
    li   a6, 0
    mv   a0, 0
    li   a1, 0
    ecall
7:  wfi
    j    7b

    .include "lib.inc"

    .section .rodata
m_hart: .asciz "hartid="
m_dtb:  .asciz "dtb(a1)="
m_magic:.asciz "dtb_first4="
m_spec: .asciz "spec_version="
m_sp:   .asciz " "
m_bad:  .asciz "bad_eid_err="
m_time: .asciz "rdtime_delta="
m_st:   .asciz "hsm_status hart="
m_hs0:  .asciz "hart_start(0,self)_err="
m_hs99: .asciz "hart_start(99)_err="
    .align 3
hlist:
    .dword 0
    .dword 1
    .dword 2
    .dword 3
    .dword 99
exts:
    .dword 0x54494D45        # TIME
    .dword 0x48534D          # HSM
    .dword 0x53525354        # SRST
    .dword 0x4442434E        # DBCN

    .section .bss
    .align 4
stack:     .space 4096
stack_top:
