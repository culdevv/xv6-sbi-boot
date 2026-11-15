
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int a = uptime();

  pause(20);

  int b = uptime();

  printf("TICKS %d %d\n", a, b);

  exit(0);
}
