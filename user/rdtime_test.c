#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define TIMEBASE_FREQ 10000000ULL
#define TEST_TICKS    TIMEBASE_FREQ

static inline uint64
rdtime(void)
{
  uint64 value;
  asm volatile("rdtime %0" : "=r"(value));
  return value;
}

int
main(void)
{
  uint64 start, end, elapsed;
  uint64 seconds, remainder;
  uint64 milliseconds;

  printf("=== RISC-V rdtime test ===\n");
  printf("Expected timebase frequency: %llu Hz\n", TIMEBASE_FREQ);
  start = rdtime();

  // Wait until the counter increases by 10 million ticks.
  while (rdtime() - start < TEST_TICKS)
    ;

  end = rdtime();
  elapsed = end - start;

  seconds = elapsed / TIMEBASE_FREQ;
  remainder = elapsed % TIMEBASE_FREQ;
  milliseconds = (remainder * 1000) / TIMEBASE_FREQ;

  printf("Start rdtime: %ld\n", start);
  printf("End rdtime:   %ld\n", end);
  printf("Elapsed ticks: %ld\n", elapsed);
  printf("Measured time: %ld.%ld seconds\n", seconds, milliseconds);

  printf("Expected ticks for 1 second: %llu\n", TEST_TICKS);
  if (elapsed >= TEST_TICKS)
    printf("RESULT: PASS - time counter increased as expected.\n");
  else
    printf("RESULT: FAIL\n");

  exit(0);
}
