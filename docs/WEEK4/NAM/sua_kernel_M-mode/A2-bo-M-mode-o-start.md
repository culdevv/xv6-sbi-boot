W4-A2: take hartid/dtb from OpenSBI, remove M-mode setup in start

Đã làm:
- Lập bảng rà soát lệnh M-mode (dòng | làm gì | kernel mới làm gì),
  lưu tại [đường dẫn file bảng].
- kernel/entry.S: dùng t0-t2, giữ nguyên a0 (hartid) và a1 (dtb);
  kiểm tra hartid < NCPU, sai thì wfi; sp = stack0 + (hartid+1)*4096;
  gọi start(hartid, dtb).
- kernel/start.c: bỏ mstatus/mepc/medeleg/mideleg/pmp/timerinit/mret;
  start() chỉ w_tp(hartid), lưu dtb_pa, gọi main().
- kernel/trap.c: trapinithart() bật SIE_SEIE | SIE_STIE
  (nếu thiếu, virtio không báo xong và treo khi đọc file).

Lệnh kiểm tra:
  grep -n "mhartid\|mstatus\|mepc\|medeleg\|mideleg\|pmp\|timervec\|mret" kernel/*.c kernel/*.S
  make clean && make qemu CPUS=1
  (trong xv6) ls ; cat README ; echo hi
  Debug: make qemu-gdb CPUS=1 -> b _entry -> info registers a0 a1

Kết quả:
- grep: không còn lệnh M-mode được gọi [dán kết quả].
- Boot: banner OpenSBI -> "xv6 kernel is booting" -> "init: starting sh" -> "$".
- ls, cat README, echo hi chạy đúng.
- GDB: tại _entry a0=[0], a1=[0x...] (địa chỉ DTB).
- Chưa có timer nên chưa chia thời gian giữa tiến trình (việc của B2).
