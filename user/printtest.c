#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  for (int i = 0; i < 200; i++)
    printf("PT %d ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n", i);
  printf("PT_DONE\n");
  exit(0);
}