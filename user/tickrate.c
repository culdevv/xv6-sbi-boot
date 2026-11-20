#include "kernel/types.h"
#include "user/user.h"

#define TIMEBASE 10000000UL // QEMU virt: 10 MHz

int
main(int argc, char *argv[])
{
  int n = 30;
  if (argc > 1)
    n = atoi(argv[1]);

  int t = uptime();
  while (uptime() == t) // Đợi đến cạnh của tick
    ;
  uint64 t0 = gettime();
  int a = uptime();
  pause(n);
  int b = uptime();
  uint64 t1 = gettime();

  uint64 dt = t1 - t0;
  int milli = (int)((uint64)(b - a) * 1000UL * TIMEBASE / dt);
  printf("TICKRATE_MILLIHZ %d ticks=%d\n", milli, b - a);
  exit(0);
}