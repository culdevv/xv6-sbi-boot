T4-A2: Sửa điểm vào, bỏ M-mode, nhận hartid/DTB từ OpenSBI (R2)

| Mục |	Nội dung |
| Người thực hiện | Nguyễn Hải Nam |
| Task Jira | T4-A2 |
| Nhánh	feat/W4-A2-remove-M-mode-setup-in-start |
| Pull request	#31 |
| Commit liên quan | 336dff6 |
| Yêu cầu đề | R2: kernel không còn chạy ở M-mode, chạy hoàn toàn ở S-mode dưới OpenSBI |

1. Mục tiêu

xv6 gốc khởi động ở M-mode: entry.S đọc mhartid, start() cấu hình mstatus, mepc, ủy quyền ngắt (medeleg, mideleg), PMP, thiết lập timer, rồi mret xuống S-mode. Khi chạy dưới OpenSBI, firmware đã làm toàn bộ phần đó, kernel được vào thẳng ở S-mode nên mọi lệnh truy cập CSR của M-mode sẽ gây lỗi lệnh trái phép (scause = 2).

Task này loại bỏ phần M-mode và để kernel nhận thông tin từ OpenSBI.

2. Đã làm:
- Lập bảng rà soát lệnh M-mode (dòng | làm gì | kernel mới làm gì),
  lưu tại docs/WEEK4/NAM/logs.
- kernel/entry.S: dùng t0-t2, giữ nguyên a0 (hartid) và a1 (dtb);
  kiểm tra hartid < NCPU, sai thì wfi; sp = stack0 + (hartid+1)*4096;
  gọi start(hartid, dtb).
- kernel/start.c: bỏ mstatus/mepc/medeleg/mideleg/pmp/timerinit/mret;
  start() chỉ w_tp(hartid), lưu dtb_pa, gọi main().
- kernel/trap.c: trapinithart() bật SIE_SEIE | SIE_STIE
  (nếu thiếu, virtio không báo xong và treo khi đọc file).

3. Lệnh kiểm tra:
  grep -n "mhartid\|mstatus\|mepc\|medeleg\|mideleg\|pmp\|timervec\|mret" kernel/*.c kernel/*.S
  make clean && make qemu CPUS=1
  (trong xv6) ls ; cat README ; echo hi
  Debug: make qemu-gdb CPUS=1 -> b _entry -> info registers a0 a1

4. Kết quả:
- grep: không còn lệnh M-mode được gọi lưu tại docs/WEEK4/NAM/logs.
- Boot: banner OpenSBI -> "xv6 kernel is booting" -> "init: starting sh" -> "$".
- ls, cat README, echo hi chạy đúng.
- Chưa có timer nên chưa chia thời gian giữa tiến trình (việc của B2).
(Update: sau khi có timer, thì usertests -q đi qua copyin, copyout, copyinstr1-3, rwsbrk, truncate1-3 đều OK)
