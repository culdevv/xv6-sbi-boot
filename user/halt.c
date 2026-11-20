#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  halt();
  fprintf(2, "halt: failed\n");
  exit(1);
}