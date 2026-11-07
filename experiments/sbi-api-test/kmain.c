/* ===== Lop goi SBI (phac thao) ===== */
#define SBI_EXT_BASE 0x10L
#define SBI_EXT_TIME 0x54494D45L
#define SBI_EXT_HSM  0x48534DL
#define SBI_EXT_SRST 0x53525354L
#define SBI_EXT_DBCN 0x4442434EL

struct sbiret { long error; long value; };

static struct sbiret sbi_ecall(long eid, long fid, long a0, long a1, long a2)
{
  register long r0 asm("a0") = a0;
  register long r1 asm("a1") = a1;
  register long r2 asm("a2") = a2;
  register long r6 asm("a6") = fid;
  register long r7 asm("a7") = eid;
  asm volatile("ecall" : "+r"(r0), "+r"(r1) : "r"(r2), "r"(r6), "r"(r7) : "memory");
  struct sbiret ret = { r0, r1 };
  return ret;
}

static long sbi_probe_extension(long eid) { return sbi_ecall(SBI_EXT_BASE, 3, eid, 0, 0).value; }
static int  sbi_set_timer(unsigned long t) { return (int)sbi_ecall(SBI_EXT_TIME, 0, t, 0, 0).error; }
static int  sbi_putchar(int c) { return (int)sbi_ecall(1, 0, c, 0, 0).error; }   /* legacy */
static int  sbi_getchar(void)  { return (int)sbi_ecall(2, 0, 0, 0, 0).error; }   /* legacy: ket qua o a0 */
static int  sbi_hart_start(long id, long addr, long opaque) { return (int)sbi_ecall(SBI_EXT_HSM, 0, id, addr, opaque).error; }
static int  sbi_hart_get_status(long id)
{
  struct sbiret r = sbi_ecall(SBI_EXT_HSM, 2, id, 0, 0);
  return r.error ? (int)r.error : (int)r.value;
}

/* ===== In an ===== */
static void puts(const char *s) { while (*s) sbi_putchar(*s++); }
static void putdec(long x)
{
  char b[24]; int n = 0;
  if (x < 0) { sbi_putchar('-'); x = -x; }
  if (x == 0) sbi_putchar('0');
  while (x > 0) { b[n++] = '0' + x % 10; x /= 10; }
  while (n > 0) sbi_putchar(b[--n]);
}
static void puthex(unsigned long x)
{
  const char *d = "0123456789abcdef"; puts("0x");
  for (int i = 60; i >= 0; i -= 4) sbi_putchar(d[(x >> i) & 0xf]);
}
static void line(const char *name, long v) { puts(name); puts(" -> "); putdec(v); puts("\n"); }

extern void secondary_entry(void);
extern volatile unsigned long sec_flag, sec_hartid, sec_opaque;

void kmain(unsigned long hartid, unsigned long dtb)
{
  puts("== THI NGHIEM HAM SBI ==\n");
  puts("boot hartid="); putdec(hartid); puts(" dtb="); puthex(dtb); puts("\n");

  puts("-- Base --\n");
  struct sbiret r;
  r = sbi_ecall(SBI_EXT_BASE, 0, 0, 0, 0); puts("spec_version value="); puthex(r.value); puts(" error="); putdec(r.error); puts("\n");
  r = sbi_ecall(SBI_EXT_BASE, 1, 0, 0, 0); puts("impl_id value="); putdec(r.value); puts("\n");
  r = sbi_ecall(SBI_EXT_BASE, 2, 0, 0, 0); puts("impl_version value="); puthex(r.value); puts("\n");
  line("probe TIME", sbi_probe_extension(SBI_EXT_TIME));
  line("probe HSM ", sbi_probe_extension(SBI_EXT_HSM));
  line("probe SRST", sbi_probe_extension(SBI_EXT_SRST));
  line("probe DBCN", sbi_probe_extension(SBI_EXT_DBCN));
  line("probe 0x12345678 (khong ton tai)", sbi_probe_extension(0x12345678L));

  puts("-- Loi chung --\n");
  r = sbi_ecall(0x12345678L, 0, 0, 0, 0); line("ecall EID la, error", r.error);
  r = sbi_ecall(SBI_EXT_TIME, 9, 0, 0, 0); line("TIME FID=9 khong co, error", r.error);

  puts("-- TIME --\n");
  line("set_timer(~0: hen vo cung xa)", sbi_set_timer(~0UL));

  puts("-- Console --\n");
  line("legacy getchar khi khong co phim", sbi_getchar());

  puts("-- HSM --\n");
  line("get_status(hart 0 = chinh minh)", sbi_hart_get_status(0));
  line("get_status(hart 1)", sbi_hart_get_status(1));
  line("get_status(hart 99, khong ton tai)", sbi_hart_get_status(99));
  line("hart_start(hart 99, ...)", sbi_hart_start(99, (long)secondary_entry, 0));
  line("hart_start(hart 0 = chinh minh, ...)", sbi_hart_start(0, (long)secondary_entry, 0));

  int e = sbi_hart_start(1, (long)secondary_entry, 0xABCD);
  line("hart_start(hart 1, secondary_entry, opaque=0xABCD)", e);
  if (e == 0) {
    long tries = 0;
    while (!sec_flag && tries < 50000000) tries++;
    line("hart 1 da chay? sec_flag", sec_flag);
    puts("hart phu nhan a0(hartid)="); putdec(sec_hartid);
    puts(" a1(opaque)="); puthex(sec_opaque); puts("\n");
    line("get_status(hart 1) sau khi start", sbi_hart_get_status(1));
    line("hart_start(hart 1 lan nua)", sbi_hart_start(1, (long)secondary_entry, 0));
  }

  puts("-- Them --\n");
  r = sbi_ecall(SBI_EXT_DBCN, 2, 'X', 0, 0); puts("\n"); line("DBCN write_byte('X'), error", r.error);
  r = sbi_ecall(SBI_EXT_SRST, 0, 99, 0, 0); line("SRST type=99 (sai), error", r.error);
  r = sbi_ecall(0, 0, ~0UL, 0, 0); line("legacy set_timer EID0, a0", r.error);

  puts("-- SRST: tat may --\n");
  sbi_ecall(SBI_EXT_SRST, 0, 0, 0, 0);
  puts("KHONG DUOC toi day neu SRST chay\n");
  for (;;) ;
}
