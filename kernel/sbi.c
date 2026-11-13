
#include "types.h"
#include "riscv.h"
#include "sbi.h"

/*
 * SBI BOOT REQUIREMENTS - MEMBER A
 *
 * This file assumes that:
 * 1. OpenSBI is loaded as the M-mode firmware.
 * 2. xv6 kernel is loaded at 0x80200000.
 * 3. OpenSBI transfers control to the kernel in S-mode.
 * 4. OpenSBI passes hart ID in a0 and DTB address in a1.
 *
 * TODO (Member A):
 * - Change QEMU boot configuration from -bios none to -bios default.
 * - Update kernel/kernel.ld load address to 0x80200000.
 * - Update entry.S to use the hart ID passed in a0.
 * - Update start.c for S-mode boot; remove M-mode initialization
 *   and mret transition.
 *
 * Do not test SBI calls with the old direct M-mode boot configuration.
 */

// Generic SBI call
// a7 = EID, a6 = FID, a0-a5 = arguments
// Return: a0 = error, a1 = value
struct sbiret
sbi_ecall(long eid, long fid, long a0, long a1, long a2, long a3, long a4,
          long a5)
{
  register long r0 asm("a0") = a0;
  register long r1 asm("a1") = a1;
  register long r2 asm("a2") = a2;
  register long r3 asm("a3") = a3;
  register long r4 asm("a4") = a4;
  register long r5 asm("a5") = a5;
  register long r6 asm("a6") = fid;
  register long r7 asm("a7") = eid;

  asm volatile("ecall"
               : "+r"(r0), "+r"(r1)
               : "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7)
               : "memory");

  struct sbiret ret = {
    .error = r0,
    .value = r1,
  };

  return ret;
}

// Legacy Console Putchar
// Output one character through OpenSBI.
void
sbi_putchar(int c)
{
  sbi_ecall(SBI_EXT_LEGACY_CONSOLE_PUTCHAR, 0, (long)c, 0, 0, 0, 0, 0);
}

// Legacy Console Getchar
// Return received character, or -1 if no character is available.
int
sbi_getchar(void)
{
  struct sbiret ret;

  ret = sbi_ecall(SBI_EXT_LEGACY_CONSOLE_GETCHAR, 0, 0, 0, 0, 0, 0, 0);

  return (int)ret.error;
}

// SBI TIME Extension
// Set the next supervisor timer interrupt using an absolute time value.
// TODO (Member B): Integrate this function into the kernel timer handler.
void
sbi_set_timer(unsigned long stime_value)
{
  sbi_ecall(
      SBI_EXT_TIME,           // a7 = EID
      SBI_EXT_TIME_SET_TIMER, // a6 = FID
      (long)stime_value,      // a0 = stime_value
       0, 0, 0, 0, 0);  
}

// SBI System Reset (shutdown)
// TODO (Member B): Integrate shutdown into the kernel halt path (R6).
void
sbi_shutdown(void)
{
  struct sbiret ret;

  ret = sbi_ecall(SBI_EXT_SRST, SBI_EXT_SRST_SYS_RESET, SBI_SRST_TYPE_SHUTDOWN,
                  SBI_SRST_REASON_NONE, 0, 0, 0, 0);

  if (ret.error == SBI_ERR_NOT_SUPPORTED) {
    sbi_ecall(SBI_EXT_LEGACY_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0);
  }

  for (;;) {
    asm volatile("wfi");
  }
}

// HSM Hart Start
// TODO (Member C): Integrate this function into multi-hart startup.
struct sbiret
sbi_hart_start(unsigned long hartid, unsigned long start_addr,
               unsigned long opaque)
{
  return sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_START, (long)hartid,
                   (long)start_addr, (long)opaque, 0, 0, 0);
}

// HSM Hart Get Status
// TODO (Member C): Use this function to check hart status.
struct sbiret
sbi_hart_get_status(unsigned long hartid)
{
  return sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_GET_STATUS, (long)hartid, 0, 0,
                   0, 0, 0);
}

// SBI Base Extension Probe
// Check whether an SBI extension is supported by OpenSBI.
int
sbi_probe_extension(long ext_id)
{
  struct sbiret ret;

  ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_PROBE_EXT, ext_id, 0, 0, 0, 0, 0);

  if (ret.error != SBI_SUCCESS)
    return 0;

  return ret.value != 0;
}
