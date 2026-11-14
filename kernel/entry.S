        # qemu -kernel loads the kernel at 0x80200000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80200000.
        .section .text
        .global _entry
_entry:
>>>>>>> 336dff6 (W4-A2: take hartid/dtb from OpenSBI, remove M-mode setup in start)
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
<<<<<<< HEAD
        j spin

        .global _secondary_entry
_secondary_entry:
        # OpenSBI HSM: a0 = hartid, a1 = opaque
        li   t0, 8
        bgeu a0, t0, spin

        # Mỗi hart sử dụng một stack 4096 byte riêng.
        la   sp, stack0
        li   t0, 4096
        addi t1, a0, 1
        mul  t0, t0, t1
        add  sp, sp, t0

        # Chuyển sang quy trình khởi tạo hart phụ.
        call secondary_start

        j    spin
