# Báo cáo Tổng kết Task T4-B2: Chuyển đổi Ngắt Timer sang OpenSBI & S-mode Boot

- **Thành viên thực hiện:** Vũ Đình Lực (MSSV: 20233513)
- **Nhánh Git:** `feat/T4-B2-R3-timerSBI`
- **Mục tiêu (Requirement R3):** Loại bỏ cơ chế hẹn giờ trực tiếp qua thanh ghi M-mode (`stimecmp` / CLINT), chuyển sang đặt lịch ngắt đồng hồ S-mode thông qua giao diện `sbi_set_timer()` của OpenSBI, đồng thời đảm bảo hệ điều hành khởi động ổn định trên nền tảng S-mode và vượt qua bộ kiểm thử hồi quy.

---

## 1. Các thay đổi mã nguồn đã thực hiện

1. **Bổ sung lớp giao tiếp OpenSBI (`kernel/sbi.c`, `kernel/sbi.h`, `Makefile`):**
   - Triển khai hàm gọi chuẩn RISC-V `sbi_ecall()` tuân thủ quy ước thanh ghi (`a7` = EID, `a6` = FID, `a0..a5` = tham số, trả về `a0` = mã lỗi và `a1` = giá trị).
   - Xây dựng các hàm bọc (wrapper) gồm `sbi_set_timer()` (EID `0x54494D45`, FID `0`), `sbi_putchar()`, `sbi_getchar()`, `sbi_shutdown()` và `sbi_hart_start()`.
   - Thêm `$K/sbi.o` vào danh sách biên dịch `OBJS` trong `Makefile` và đổi cấu hình QEMU từ `-bios none` sang `-bios default`.

2. **Chuyển đổi luồng khởi động sang S-mode (`kernel/kernel.ld`, `kernel/memlayout.h`, `kernel/entry.S`, `kernel/start.c`, `kernel/main.c`):**
   - Cập nhật địa chỉ nạp kernel lên `0x80200000` để nhường vùng `0x80000000–0x801FFFFF` cho firmware OpenSBI.
   - Sửa `entry.S` để nhận trực tiếp `hartid` từ thanh ghi `a0` và con trỏ DTB từ `a1` do OpenSBI truyền sang, thay vì đọc `mhartid`.
   - Rút gọn `start.c` (loại bỏ toàn bộ thao tác ghi thanh ghi đặc quyền M-mode và lệnh `mret`), lưu `hartid` vào thanh ghi `tp` và chuyển thẳng vào `main()`.
   - Cập nhật `main.c` dùng khóa nguyên tử (`__sync_lock_test_and_set`) để chọn đúng 1 Boot Hart bất kỳ do OpenSBI chỉ định.

3. **Cấu hình ngắt Timer và PLIC ở S-mode (`kernel/trap.c`, `kernel/uart.c`):**
   - Thay thế lệnh ghi trực tiếp `w_stimecmp(...)` trong `clockintr()` bằng `sbi_set_timer(r_time() + TIMER_INTERVAL)` với `TIMER_INTERVAL = 1000000` (tương ứng tần số chuẩn $10\text{ Hz}$ trên bộ đếm $10\text{ MHz}$ của QEMU `virt`).
   - Cấu hình thanh ghi `sie` bật đầy đủ cả 3 nguồn ngắt S-mode: `SIE_SEIE` (ngắt ngoài PLIC cho đĩa VirtIO/UART), `SIE_STIE` (ngắt đồng hồ) và `SIE_SSIE` (ngắt mềm).
   - Tắt ngắt truyền phần cứng (`IER_TX_ENABLE`) trong `uartinit()`, chỉ giữ lại `IER_RX_ENABLE`.

---

## 2. Tổng hợp các lỗi gặp phải & Cách khắc phục

| STT | Hiện tượng lỗi gặp phải | Nguyên nhân kỹ thuật | Cách khắc phục đã áp dụng |
| :-- | :--- | :--- | :--- |
| **1** | **CI thất bại ở `usertests` (`FAIL: match failed '^ALL TESTS PASSED'`)** | Hàm `sbi_timerinit()` trong `trap.c` chỉ bật mỗi `SIE_STIE` mà thiếu `SIE_SEIE` (ngắt ngoài PLIC), khiến driver đĩa `virtio_disk` không nhận được ngắt hoàn tất đọc/ghi và treo cứng lúc nạp tiến trình đầu tiên. Ngoài ra `main.c` giữ cứng `cpuid() == 0` trong khi OpenSBI có thể chọn Hart ID khác làm Boot Hart. | Bật đầy đủ `SIE_SEIE | SIE_STIE | SIE_SSIE` trong `trap.c`; thay điều kiện `cpuid() == 0` bằng biến cờ nguyên tử `boot_hart_done` trong `main.c`. |
| **2** | **Màn hình lặp vô hạn `[PLIC] External interrupt: irq=10`** | Trong `uartinit()`, bit ngắt truyền `IER_TX_ENABLE` vẫn bật nên mỗi lần in log ra console lại kích hoạt ngắt ngoài UART (`irq=10`), tạo vòng lặp tự kích ngắt làm nghẽn CPU. | Sửa `WriteReg(IER, IER_RX_ENABLE)` trong `kernel/uart.c` để tắt ngắt truyền phần cứng. |
| **3** | **Màn hình trôi liên tục dòng `[SCHED] hart=0 no RUNNABLE process, entering wfi`** | Ngắt Timer đánh thức CPU khỏi `wfi` mỗi chu kỳ, và câu lệnh `printf` debug đặt trong vòng lặp `scheduler()` liên tục in log làm trôi dấu nhắc shell `$`. | Xóa các câu lệnh `printf`/`printk` debug trong vòng lặp nóng (`scheduler()`, `virtio_disk.c`, `trap.c`) và chuẩn hóa `TIMER_INTERVAL = 1000000`. |
| **4** | **GitHub Actions báo lỗi `make fmt && git diff --exit-code` (Exit code 1)** | Một số dòng comment mới thêm trong `kernel/main.c` và `kernel/proc.c` chưa tuân thủ quy chuẩn khoảng trắng và thụt lề của tệp `.clang-format`. | Chạy `make fmt` tại máy local để tự động chuẩn hóa định dạng, sau đó commit và push lại. |
| **5** | **GitHub Actions báo `FAIL` ở bài test `./test-xv6.py crash` (lần chạy đầu)** | Do log debug in nhiều làm chậm nhịp I/O trong 20 lần thử ngắt ngẫu nhiên của `test-xv6.py`. Sau khi dọn sạch log debug và chuẩn hóa `TIMER_INTERVAL`, bài test phục hồi nhật ký và inode mồ côi đã vượt qua cả 3/3 lượt kiểm tra (`OK`). | Dọn sạch log debug, chạy lại kiểm chứng local (`EXIT CODE = 0`) và re-run job trên GitHub Actions. |

---

