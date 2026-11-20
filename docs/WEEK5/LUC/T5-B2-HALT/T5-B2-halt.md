# Báo cáo Kỹ thuật Task T5-B2: Triển khai System Call `halt` & Chương trình `user/halt.c` (Yêu cầu R6)

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Vũ Đình Lực — Thành viên B (MSSV: 20233513)
* **Nhánh thực hiện:** `feat/PROJ-B2-halt`
* **Trạng thái:** Hoàn thành (QEMU thoát sạch với `exit code = 0` trên cả cấu hình đơn nhân và đa nhân)

---

## 1. Mục tiêu công việc
Hiện thực hóa yêu cầu kỹ thuật **R6 (Shutdown qua OpenSBI)**: bổ sung lời gọi hệ thống `sys_halt()` và ứng dụng không gian người dùng `user/halt.c`, cho phép người dùng gõ lệnh `halt` trong shell xv6 để tắt toàn bộ hệ thống và thoát tiến trình QEMU một cách sạch sẽ với mã thoát `0` thông qua dịch vụ **SBI System Reset Extension (SRST)** (kèm cơ chế dự phòng Legacy Shutdown).

---

## 2. Kiến trúc luồng xử lý (5 lớp thực thi)
Lệnh `halt` đi qua 5 tầng từ không gian người dùng xuống phần cứng ảo hóa:
1. **User Program (`user/halt.c`):** Ứng dụng người dùng gọi hàm thư viện `halt()`.
2. **Syscall Stub (`user/usys.pl` & `user/user.h`):** Nạp số hiệu `SYS_halt` vào thanh ghi `a7` và phát lệnh bẫy `ecall` từ U-mode vào S-mode.
3. **Kernel Trap & Dispatch (`kernel/syscall.h`, `kernel/syscall.c`):** Bộ điều phối syscall nhận số hiệu `SYS_halt` và gọi hàm xử lý `sys_halt()`.
4. **Kernel Handler (`kernel/sysproc.c`):** In thông báo tắt hệ thống (`halting system`) và gọi lớp bọc `sbi_shutdown()`.
5. **SBI Call & Firmware (`kernel/sbi.c` $\rightarrow$ OpenSBI):** Phát lệnh `ecall` gọi SRST Extension (`EID = 0x53525354`, `FID = 0`, `type = SHUTDOWN`, `reason = NONE`), hoặc tự động lùi về Legacy Shutdown (`EID = 8`) nếu bị từ chối; OpenSBI thực hiện tắt toàn bộ các hart và thoát QEMU với mã trạng thái `0`.

---

## 3. Các thay đổi đã thực hiện (Files Changed)

| Tệp tin | Hành động | Mô tả chi tiết thay đổi |
| :--- | :--- | :--- |
| `kernel/syscall.h` | **Cập nhật** | Định nghĩa hằng số hiệu lời gọi hệ thống mới: `#define SYS_halt 22` (số hiệu kế tiếp còn trống sau các syscall hiện tại). |
| `kernel/syscall.c` | **Cập nhật** | Khai báo nguyên mẫu `extern uint64 sys_halt(void);` và ánh xạ `[SYS_halt] sys_halt,` vào bảng điều phối `syscalls[]`. |
| `kernel/sysproc.c` | **Cập nhật** | Bổ sung `#include "sbi.h"` và cài đặt hàm `sys_halt(void)` thực hiện in thông báo `"halting system\n"`, gọi `sbi_shutdown()`, và trả về `-1` trong trường hợp OpenSBI từ chối yêu cầu tắt máy. |
| `user/usys.pl` | **Cập nhật** | Bổ sung `entry("halt");` để trình sinh mã tạo stub hợp ngữ `ecall` cho lời gọi hệ thống `halt`. |
| `user/user.h` | **Cập nhật** | Khai báo nguyên mẫu hàm `int halt(void);` cho các chương trình ở không gian người dùng. |
| `user/halt.c` | **Tạo mới** | Xây dựng chương trình người dùng gọi `halt()`; nếu hàm trả về (tắt máy thất bại), in thông báo lỗi `halt: failed\n` ra stderr và thoát với `exit(1)`. |
| `Makefile` | **Cập nhật** | Bổ sung `$U/_halt\` vào danh sách `UPROGS` để biên dịch và đóng gói tệp thực thi `_halt` vào ảnh hệ thống tệp `fs.img`. |

---

## 4. Kết quả kiểm thử
Sau khi biên dịch sạch (`make clean && make qemu`), tiến hành kiểm thử thủ công trên cả cấu hình đơn nhân (`CPUS=1`) và đa nhân (`CPUS=3`):

1. **Kiểm thử trên đơn nhân (`make qemu CPUS=1`):**
   ```text
   $ halt
   halting system
   ```
   * Kết quả: Tiến trình QEMU tự động đóng ngay lập tức và trả về dấu nhắc Terminal Ubuntu. Kiểm tra bằng lệnh `echo $?` cho kết quả chính xác bằng **`0`**.

2. **Kiểm thử trên đa nhân (`make qemu CPUS=3`):**
   ```text
   $ halt
   halting system
   ```
   * Kết quả: Tất cả các hart đều dừng sạch sẽ qua OpenSBI SRST, không xảy ra hiện tượng treo hart phụ hay lỗi kernel panic; mã thoát hệ điều hành `echo $?` bằng **`0`**.

3. **Kiểm tra chuẩn định dạng & hồi quy:**
   * Chạy `make fmt` đảm bảo mã nguồn tuân thủ quy tắc `.clang-format` của dự án và chạy `make test` xác nhận các bài kiểm thử hiện có tiếp tục `PASS`.

