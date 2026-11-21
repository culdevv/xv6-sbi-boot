.section .text.init
.globl _start
.globl _secondary

# Entry point của boot hart.
# Boot hart ID nằm trong a0, DTB address nằm trong a1.
_start:
    la sp, stacks
    slli t0, a0, 12
    add sp, sp, t0
    li t0, 4096
    add sp, sp, t0
    andi sp, sp, -16

    # Luôn chạy main(), không phụ thuộc hart ID.
    call main
    j park

# Entry point được truyền vào sbi_hart_start().
# a0 = hartid, a1 = opaque.
_secondary:
    la sp, stacks
    slli t0, a0, 12
    add sp, sp, t0
    li t0, 4096
    add sp, sp, t0
    andi sp, sp, -16

    call secondary_main
    j park

# Giữ hart ở trạng thái hoạt động.
park:
    wfi
    j park

.section .bss.stack
.align 12
stacks:
    .space 32768
