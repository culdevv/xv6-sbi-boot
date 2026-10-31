    .macro HEX r
    mv   a0, \r
    call printhex
    .endm
    .macro STR l
    la   a0, \l
    call puts
    .endm
    .macro NL
    li   a0, 10
    call putc
    .endm
