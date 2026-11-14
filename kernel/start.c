#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();


__attribute__((aligned(16))) char stack0[4096 * NCPU];
uint64 dtb_pa; // địa chỉ DTB do OpenSBI đưa, dùng ở tuần 6-7


void
start(uint64 hartid, uint64 dtb)
{
  w_tp(hartid); // cpuid() đọc từ tp
  dtb_pa = dtb;
  main();
}
