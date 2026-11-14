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
#include "types.h"      // Khai báo kiểu uint64
#include "param.h"      // Khai báo hằng số NCPU
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"        // Khai báo sbi_hart_start (phải nằm dưới types.h)

extern void _entry(void); // Khai báo điểm nhập _entry từ file entry.S[cite: 18]

volatile static int started = 0;
volatile static int boot_hart_done = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  // [SỬA 4] Thay "if (cpuid() == 0)" bằng atomic test-and-set
  // Bất kể OpenSBI chọn hart 0, 1, 2 hay 3 làm boot hart đều khởi tạo được
  if (__sync_lock_test_and_set(&boot_hart_done, 1) == 0) {
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

    // [SỬA 5] Chủ động đánh thức các hart phụ đang ở trạng thái STOPPED
    int boot_id = cpuid();
    for (int i = 0; i < NCPU; i++) {
      if (i != boot_id) {
        sbi_hart_start(i, (uint64)_entry, 0);
      }
    }
  } else {
    while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
      ;

    printk("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  scheduler();
}