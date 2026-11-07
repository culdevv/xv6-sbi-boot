        # qemu -kernel loads the kernel at 0x 8020 0000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80200000.
    .section .text
    .globl _entry
	# set up a stack for C.
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)

_entry:
    mv   s0, a0              # hartid (cất trước khi dùng a0)
    mv   s1, a1              # địa chỉ DTB
    li   t0, 8               # NCPU
    bgeu s0, t0, park        # hartid không hợp lệ -> ngủ
    mv   tp, s0
    la   sp, stack0
    li   t1, 4096
    addi t2, s0, 1
    mul  t1, t1, t2
    add  sp, sp, t1          # sp = stack0 + 4096*(hartid+1)
    mv   a0, s0
    mv   a1, s1
    call boot_stub
park:
    wfi
    j    park







