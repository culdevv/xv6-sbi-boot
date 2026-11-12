#include "sbi.h"      // đầu file

void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE);   // [A] bật ngắt ngoài + timer
  sbi_set_timer(r_time() + 1000000);       // [B] hẹn timer đầu tiên
}

// Trong clockintr(): thay dòng ghi stimecmp bằng
  sbi_set_timer(r_time() + 1000000);

// Kiểu CLINT (xv6 cũ): trong devintr đổi điều kiện nhận ngắt sang
//   scause == 0x8000000000000005L, xóa lệnh w_sip(r_sip() & ~2),
//   và thêm dòng sbi_set_timer(...) ở trên vào clockintr().
