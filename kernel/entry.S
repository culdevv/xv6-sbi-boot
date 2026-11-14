        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
        # OpenSBI: a0 = hartid, a1 = dtb. Chỉ dùng t0..t1, giữ nguyên a0/a1.
        li   t0, 8                  # NCPU
        bgeu a0, t0, spin           # hartid >= 8: không có stack, cho ngủ
        la   sp, stack0
        li   t0, 4096
        addi t1, a0, 1
        mul  t0, t0, t1             # t0 = 4096 * (hartid + 1)
        add  sp, sp, t0             # sp = stack0 + 4096*(hartid+1)
        call start                  # start(hartid, dtb)
spin:
        wfi
        j spin
