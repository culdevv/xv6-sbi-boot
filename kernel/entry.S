        # qemu -kernel loads the kernel at 0x 8020 0000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80200000.
    .section .text
    .global _entry
	# set up a stack for C.
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)

_entry:
# OpenSBI: a0 = hartid, a1 = dtb. Chỉ dùng t0..t2, giữ nguyên a0/a1.
        li   t0, 8                  # NCPU
        bgeu a0, t0, spin
        la   sp, stack0
        li   t0, 4096
        addi t1, a0, 1
        mul  t0, t0, t1
        add  sp, sp, t0
        call start
spin:
        wfi
        j    spin

        .global _secondary_entry
_secondary_entry:
        # HSM start: a0 = hartid, a1 = opaque
        li   t0, 8
        bgeu a0, t0, spin
        la   sp, stack0
        li   t0, 4096
        addi t1, a0, 1
        mul  t0, t0, t1
        add  sp, sp, t0
        call secondary_start
        j    spin







