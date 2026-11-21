        .section .text.entry
        .globl _entry
_entry:
        mv      tp, a0
        li      t0, 8
        bgeu    a0, t0, park
        la      sp, stack0
        addi    t1, a0, 1
        slli    t1, t1, 12
        add     sp, sp, t1
        call    kmain
park:
        wfi
        j       park

        .globl secondary_entry
        .align 2
secondary_entry:
        la      t0, sec_hartid
        sd      a0, 0(t0)
        la      t0, sec_opaque
        sd      a1, 0(t0)
        fence   rw, rw
        la      t0, sec_flag
        li      t1, 1
        sd      t1, 0(t0)
        fence   rw, rw
1:      wfi
        j       1b

        .section .data
        .align 3
        .globl sec_flag
        .globl sec_hartid
        .globl sec_opaque
sec_flag:   .dword 0
sec_hartid: .dword 0
sec_opaque: .dword 0

        .section .bss
        .align 12
        .globl stack0
stack0:
        .space 8*4096
