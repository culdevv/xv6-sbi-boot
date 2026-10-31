putc:                       # legacy putchar: EID 1, a0 = ky tu
    li   a7, 1
    ecall
    ret
puts:                       # in chuoi ket thuc bang 0, dia chi trong a0
    addi sp, sp, -16
    sd   ra, 0(sp)
    mv   t4, a0
1:  lbu  a0, 0(t4)
    beqz a0, 2f
    call putc
    addi t4, t4, 1
    j    1b
2:  ld   ra, 0(sp)
    addi sp, sp, 16
    ret
printhex:                   # in a0 thanh 16 chu so hex
    addi sp, sp, -16
    sd   ra, 0(sp)
    mv   t0, a0
    li   t1, 60
1:  srl  t2, t0, t1
    andi t2, t2, 15
    li   t3, 10
    blt  t2, t3, 2f
    addi a0, t2, 'a'-10
    j    3f
2:  addi a0, t2, '0'
3:  call putc
    addi t1, t1, -4
    bgez t1, 1b
    ld   ra, 0(sp)
    addi sp, sp, 16
    ret
