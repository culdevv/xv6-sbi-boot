# Báo cáo Hoàn thành Task T4-B3: Chuyển đổi Console Output sang OpenSBI (Requirement R4)

- **Mã Task Jira:** T4-B3 (Thuộc Epic R4: SBI Console)
- **Người thực hiện:** Thành viên B (Vũ Đình Lực)
- **Nhánh Git:** `feat/T4-B3-R4-SBIconsole`
- **Trạng thái:** Hoàn thành & Đã kiểm thử đạt chuẩn

---

## 1. Mục tiêu & Quyết định Thiết kế
Thực hiện yêu cầu **R4 (Console qua SBI)** theo đúng phương án đã chốt ở Tuần 3 (W3-A1):
1. **Luồng xuất ký tự (Output):** Chuyển toàn bộ thao tác in ký tự từ việc ghi trực tiếp thanh ghi MMIO `THR` của chip UART16550 (`0x10000000`) sang gọi dịch vụ `sbi_putchar(c)` của OpenSBI.
2. **Luồng nhập phím (Input):** Giữ nguyên cơ chế ngắt ngoài phần cứng UART RX thông qua bộ điều khiển ngắt PLIC (`uartintr()` $\rightarrow$ `uartgetc()` $\rightarrow$ `consoleintr()`) để đảm bảo độ phản hồi mượt mà, không mất phím khi gõ nhanh.
3. **Ngắt truyền UART TX:** Tắt hoàn toàn cờ cho phép ngắt truyền (`IER_TX_ENABLE`) để tránh tình trạng thanh ghi truyền trống liên tục kích hoạt bão ngắt ngoài (`irq=10`).

---

## 2. Chi tiết Các Thay đổi Mã nguồn

### 2.1. Tệp `kernel/console.c`
* Bổ sung `#include "sbi.h"` để sử dụng giao diện dịch vụ SBI.
* Cập nhật hàm `consputc(int c)`:
  * Thay thế lời gọi `uartputc_sync(c)` bằng `sbi_putchar(c)`.
  * Giữ nguyên logic xử lý phím xóa lùi (`BACKSPACE`): gửi tuần tự 3 ký tự `'\b'`, `' '`, `'\b'` qua `sbi_putchar()` để xóa ký tự trên màn hình terminal.

### 2.2. Tệp `kernel/uart.c`
* Bổ sung `#include "sbi.h"`.
* **Hàm `uartinit(void)`:**
  * Sửa cấu hình thanh ghi cho phép ngắt từ `WriteReg(IER, IER_RX_ENABLE | IER_TX_ENABLE)` thành **`WriteReg(IER, IER_RX_ENABLE)`**, chỉ giữ lại ngắt nhận phím (RX) và tắt ngắt truyền (TX).
* **Hàm `uartwrite(char buf[], int n)`:**
  * Loại bỏ vòng lặp chờ thanh ghi trạng thái đường truyền `LSR_TX_IDLE`, ghi `THR` và gọi `sleep(&tx_chan)`.
  * Thay thế bằng vòng lặp gửi từng byte dữ liệu trực tiếp qua `sbi_putchar(buf[i])`, đồng thời giữ lại khóa ngủ `tx_lock` (`acquiresleep` / `releasesleep`) để tránh các tiến trình user in đè ký tự lên nhau.
* **Hàm `uartputc_sync(int c)`:**
  * Bỏ vòng lặp bận chờ `LSR_TX_IDLE` và lệnh ghi MMIO `WriteReg(THR, c)`, chuyển sang gọi trực tiếp `sbi_putchar(c)` (vẫn giữ `push_off()` / `pop_off()` và kiểm tra `panicked`).
* **Hàm `uartintr(void)`:**
  * Gỡ bỏ đoạn mã kiểm tra `LSR_TX_IDLE` và đánh thức hàng đợi `wakeup(&tx_chan)` (cùng biến tĩnh `tx_chan`), chỉ giữ lại phần đọc ký tự từ bộ đệm nhận `uartgetc()` và đẩy sang `consoleintr(c)`.

---

## 4. Kết quả Kiểm thử & Xác nhận (Definition of Done)
* **Biên dịch & Định dạng:**
  * Chạy `make fmt && git diff --exit-code` sạch lỗi định dạng.
  * Biên dịch `make clean && make kernel/kernel` không phát sinh warning hay error.
* **Kiểm thử tương tác trên QEMU (`CPUS=1`):**
  * Boot thành công vào shell `$` với đầy đủ thông báo khởi động từ OpenSBI và xv6.
  * Các lệnh xuất chuẩn (`echo hello sbi console`, `ls`, `cat README`) hiển thị đầy đủ, không mất ký tự.
  * Thao tác nhập liệu bàn phím (gõ chuỗi dài, nhấn `Backspace` xóa ký tự, nhấn `Enter`) hoạt động mượt mà, không bị lặp ký tự hay treo ngắt.
* **Kiểm thử hồi quy tự động:**
  * Chạy `usertests -q` trong shell xv6 đạt **`ALL TESTS PASSED`**.
  
