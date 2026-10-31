static void putc(int c) {
    register long a0 asm("a0") = c;
    register long a7 asm("a7") = 1; // SBI Legacy Console Putchar
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

static void puts_(const char *s) {
    while (*s) {
        putc(*s++);
    }
}

static void hex(unsigned long v) {
    puts_("0x");
    for (int i = 60; i >= 0; i -= 4) {
        putc("0123456789abcdef"[(v >> i) & 0xf]);
    }
}

static void shutdown(void) {
    register long a7 asm("a7") = 8; // SBI Legacy Shutdown
    asm volatile("ecall" : : "r"(a7) : "memory");
}

void kmain(unsigned long hartid, unsigned long dtb) {
    puts_("\n=========================================\n");
    puts_("Hello S-mode! hartid="); hex(hartid);
    puts_(" dtb="); hex(dtb);
    puts_("\n=========================================\n\n");

    shutdown();
    for (;;);
}
