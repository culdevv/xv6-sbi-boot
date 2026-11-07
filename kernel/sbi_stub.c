// Tự chứa ecall để không phụ thuộc sbi.h
static inline long
sbi_legacy_putchar(int c)
{
  register long a0 asm("a0") = c;
  register long a7 asm("a7") = 1;
  asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
  return a0;
}

static void
puts_sbi(const char *s)
{
  for (; *s; s++)
    sbi_legacy_putchar(*s);
}

static void
put_digit(long v)
{
  sbi_legacy_putchar('0' + (v % 10));
}

void
boot_stub(long hartid, long dtb)
{
  puts_sbi("booted, hartid=");
  put_digit(hartid);
  sbi_legacy_putchar('\n');

  // SRST: shutdown
  register long a0 asm("a0") = 0;
  register long a1 asm("a1") = 0;
  register long a6 asm("a6") = 0;
  register long a7 asm("a7") = 0x53525354;
  asm volatile("ecall" : "+r"(a0), "+r"(a1) : "r"(a6), "r"(a7) : "memory");
  for (;;)
    ;
}
