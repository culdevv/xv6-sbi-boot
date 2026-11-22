/* gốc
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if (cpuid() == 0) {
    consoleinit();
    printkinit();
    printk("\n");
    printk("xv6 kernel is booting\n");
    printk("\n");
    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging
    procinit();         // process table
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    plicinit();         // set up interrupt controller
    plicinithart();     // ask PLIC for device interrupts
    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    virtio_disk_init(); // emulated hard disk
    userinit();         // first user process

    __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
  } 

  else {
    while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
      ;

    printk("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  printk("[BOOT] Before scheduler, hart=%d\n", cpuid()); //test
  scheduler();
  printk("[BOOT] Scheduler returned, hart=%d\n", cpuid()); //test
}

*/

//test 14-11 lan 2 T4-B2
#include "types.h" // Khai báo kiểu uint64
#include "param.h" // Khai báo hằng số NCPU
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h" // Khai báo sbi_hart_start (phải nằm dưới types.h)

extern void _secondary_entry(void);

static volatile int started = 0;
static volatile int boot_hart_done = 0;
// Khoi dong cac hart phu bang SBI HSM.
static void
start_secondary_harts(void)
{
  int boot_id = cpuid();

  // Kiem tra OpenSBI co ho tro HSM khong.
  if (!sbi_probe_extension(SBI_EXT_HSM)) {
    printk("HSM extension is not supported\n");
    return;
  }

  for (int i = 0; i < NCPU; i++) {
    if (i == boot_id)
      continue;

    struct sbiret status = sbi_hart_get_status(i);

    if (status.error != SBI_SUCCESS) {
      printk("hart %d: get_status error %d\n", i, (int)status.error);
      continue;
    }

    if (status.value == SBI_HSM_STATE_STARTED) {
      printk("hart %d is already started\n", i);
      continue;
    }

    if (status.value != SBI_HSM_STATE_STOPPED) {
      printk("hart %d is not stopped, state=%d\n", i, (int)status.value);
      continue;
    }

    struct sbiret ret = sbi_hart_start(i, (uint64)_secondary_entry, 0);

    if (ret.error == SBI_SUCCESS) {
      printk("hart %d start request succeeded\n", i);
    } else if (ret.error == SBI_ERR_ALREADY_AVAILABLE ||
               ret.error == SBI_ERR_ALREADY_STARTED) {
      printk("hart %d already available or started\n", i);
    } else {
      printk("hart %d start failed, error=%d\n", i, (int)ret.error);
    }
  }
}

// Boot hart: khoi tao tai nguyen dung chung va danh thuc cac hart phu.

void
main()
{
  // Chi mot hart duoc phep khoi tao tai nguyen dung chung.
  if (__sync_lock_test_and_set(&boot_hart_done, 1) == 0) {
    consoleinit();
    printkinit();

    printk("\n");
    printk("xv6 kernel is booting\n");
    printk("\n");

    // Khoi tao tai nguyen dung chung.
    kinit();
    kvminit();
    kvminithart();
    procinit();
    trapinit();
    trapinithart();
    plicinit();
    plicinithart();
    binit();
    iinit();
    fileinit();
    virtio_disk_init();
    userinit();

    // Bao hieu khoi tao dung chung da hoan tat.
    __atomic_store_n(&started, 1, __ATOMIC_RELEASE);

    printk("boot hart %d initialized\n", cpuid());
    printk("hart %d starting\n", cpuid());

    // Khoi dong cac hart phu.
    start_secondary_harts();
  } else {
    // Hart khac cho boot hart khoi tao tai nguyen dung chung.
    while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
      ;

    printk("hart %d starting\n", cpuid());

    // Khoi tao rieng cho hart nay.
    kvminithart();
    trapinithart();
    plicinithart();
  }

  scheduler();
}

void
secondary_start(uint64 hartid)
{
  w_tp(hartid);

  // Cho boot hart khoi tao xong tai nguyen dung chung.
  while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
    ;

  kvminithart();
  trapinithart();
  plicinithart();

  printk("hart %d starting\n", cpuid());

  scheduler();
}
