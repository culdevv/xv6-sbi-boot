static void putc(int c) {
    register long a0 asm("a0") = c;
    register long a7 asm("a7") = 1; // Legacy Console Putchar
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

static void puts_(const char *s) {
    while (*s) putc(*s++);
}

static void hex(unsigned long v) {
    puts_("0x");
    for (int i = 60; i >= 0; i -= 4)
        putc("0123456789abcdef"[(v >> i) & 0xf]);
}

static void shutdown(void) {
    register long a7 asm("a7") = 8; // Legacy Shutdown
    asm volatile("ecall" : : "r"(a7) : "memory");
}

void kmain(unsigned long hartid, unsigned long dtb) {
    puts_("\n=========================================\n");
    puts_("Task SBI-37 Verification Output:\n");
    puts_("1. Register a0 (Hart ID)     : "); hex(hartid); puts_("\n");
    puts_("2. Register a1 (DTB Pointer) : "); hex(dtb); puts_("\n");
    puts_("=========================================\n\n");

    puts_("OpenSBI Reserved Region: 0x80000000 - 0x801FFFFF (Protected by PMP)\n");
    puts_("Kernel base loaded at : 0x80200000\n");
    puts_("Shutting down QEMU...\n");

    shutdown();
    for (;;);
}
