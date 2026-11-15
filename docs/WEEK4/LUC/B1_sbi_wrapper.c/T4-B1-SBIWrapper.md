# Báo cáo Task T4-B1: Xây dựng Lớp Giao tiếp Lõi SBI (`kernel/sbi.h` & `kernel/sbi.c`)

- **Mã công việc:** T4-B1 (PROJ-B1)
- **Nhánh thực hiện:** `feat/PROJ-B1-sbi-base`
- **Người thực hiện:** Vũ Đình Lực (20233513) — Nhóm 3 (Project 26101: *Booting xv6 in S-mode on OpenSBI*)
- **Trạng thái:** Hoàn thành & Đã tích hợp vào `main`

---

## 1. Mục tiêu công việc
Xây dựng lớp giao tiếp nền tảng (SBI Wrapper Layer) giữa nhân hệ điều hành xv6 (chạy ở chế độ đặc quyền **S-mode**) và firmware **OpenSBI** (chạy ở chế độ đặc quyền **M-mode**) thông qua lệnh bẫy đặc quyền `ecall`[cite: 3, 4, 5]:
1. Định nghĩa đầy đủ giao diện lập trình (API Header) trong `kernel/sbi.h` theo đặc tả **RISC-V SBI Specification v2.0** (cấu trúc `struct sbiret`, bảng 10 mã lỗi chuẩn, các hằng số Extension ID và Function ID)[cite: 3].
2. Cài đặt hàm gọi lõi `sbi_ecall()` bằng Inline Assembly cùng các hàm bao bọc (helper wrappers) cho Console, Timer, System Reset, HSM và Base Extension trong `kernel/sbi.c`[cite: 4].
3. Tích hợp `$K/sbi.o` vào `Makefile` đảm bảo biên dịch sạch không cảnh báo và giữ nguyên điểm nạp khởi động của kernel tại `0x80200000`[cite: 4, 6].

---

## 2. Cơ sở kỹ thuật & Thiết kế triển khai

### 2.1. Quy ước truyền nhận thanh ghi RISC-V SBI (`Calling Convention`)
Khi thực thi lệnh `ecall` từ S-mode sang M-mode, OpenSBI nhận tham số và trả kết quả theo quy ước thanh ghi cố định[cite: 3, 4]:
- **Thanh ghi đầu vào:**
  - `a7`: **Extension ID (EID)** — Định danh nhóm dịch vụ (`BASE`, `TIME`, `HSM`, `SRST`, `DBCN`, `Legacy`)[cite: 3, 4].
  - `a6`: **Function ID (FID)** — Định danh chức năng cụ thể trong nhóm dịch vụ[cite: 3, 4].
  - `a0 .. a5`: Tối đa 6 tham số truyền vào hàm[cite: 3, 4].
- **Thanh ghi đầu ra (đóng gói vào `struct sbiret`):**
  - `a0` (`error`): Mã trạng thái thực thi (`0` = `SBI_SUCCESS`, số âm biểu thị mã lỗi cụ thể)[cite: 3, 4].
  - `a1` (`value`): Giá trị dữ liệu trả về từ OpenSBI (nếu có)[cite: 3, 4].

### 2.2. Chi tiết mã nguồn đã triển khai

#### A. Tệp giao diện `kernel/sbi.h`
- **Cấu trúc trả về:** Khai báo `struct sbiret { long error; long value; };` ánh xạ trực tiếp cặp thanh ghi `(a0, a1)`[cite: 3].
- **Bảng 10 mã lỗi chuẩn (`SBI Error Codes`):** Định nghĩa đầy đủ từ `SBI_SUCCESS (0)` đến `SBI_ERR_NO_SHMEM (-9)` (`FAILED`, `NOT_SUPPORTED`, `INVALID_PARAM`, `DENIED`, `INVALID_ADDRESS`, `ALREADY_AVAILABLE`, `ALREADY_STARTED`, `ALREADY_STOPPED`, `NO_SHMEM`) giúp kernel bắt lỗi an toàn[cite: 3].
- **Các nhóm Extension ID & Function ID:**
  - **Base Extension (`0x10`):** Các FID từ `0` đến `6` (đặc biệt là `SBI_EXT_BASE_PROBE_EXT = 3` để dò tính năng của OpenSBI)[cite: 3].
  - **Legacy Extensions (`0x00` – `0x08`):** Giữ lại đầy đủ các EID truyền thống (`SET_TIMER = 0x00`, `CONSOLE_PUTCHAR = 0x01`, `CONSOLE_GETCHAR = 0x02`, `SHUTDOWN = 0x08`, ...)[cite: 3].
  - **TIME Extension (`0x54494D45`):** `SBI_EXT_TIME_SET_TIMER (0)`[cite: 3].
  - **HSM Extension (`0x48534D`):** Các FID `HART_START (0)`, `HART_STOP (1)`, `HART_GET_STATUS (2)`, `HART_SUSPEND (3)` cùng 7 hằng số trạng thái hart (`SBI_HSM_STATE_STARTED` đến `RESUME_PENDING`)[cite: 3].
  - **SRST Extension (`0x53525354`):** `SBI_EXT_SRST_SYS_RESET (0)` cùng các tham số loại reset (`SHUTDOWN = 0`, `COLD_REBOOT = 1`, `WARM_REBOOT = 2`) và lý do (`NONE = 0`, `SYSTEM_FAILURE = 1`)[cite: 3].
  - **DBCN Extension (`0x4442434E`):** `CONSOLE_WRITE (0)`, `CONSOLE_READ (1)`, `CONSOLE_WRITE_BYTE (2)`[cite: 3].

#### B. Tệp cài đặt `kernel/sbi.c`
| Hàm cài đặt | Cơ chế hoạt động & Xử lý an toàn |
| :--- | :--- |
| **`sbi_ecall(...)`** | Nạp 8 thanh ghi `a0`–`a7` và thực thi lệnh `asm volatile("ecall" : "+r"(r0), "+r"(r1) : ... : "memory")`, đóng gói `r0` và `r1` vào `struct sbiret`[cite: 4]. |
| **`sbi_putchar(int c)`** | Xuất 1 ký tự qua Legacy Console (`SBI_EXT_LEGACY_CONSOLE_PUTCHAR = 0x01`), đảm bảo tính đơn giản và ổn định tuyệt đối ở giai đoạn Core (Tuần 3–4)[cite: 3, 4]. |
| **`sbi_console_write(buf, len)`** | Ghi chuỗi qua `DBCN` (`SBI_EXT_DBCN_CONSOLE_WRITE`, `FID = 0`) với địa chỉ vật lý `pa = (unsigned long)buf`; tự động lùi về vòng lặp `sbi_putchar()` nếu firmware trả về `SBI_ERR_NOT_SUPPORTED (-2)`[cite: 3, 4]. |
| **`sbi_getchar(void)`** | Đọc 1 ký tự từ console qua `SBI_EXT_LEGACY_CONSOLE_GETCHAR (0x02)`, trả về ký tự nhận được hoặc `-1` nếu không có dữ liệu[cite: 4]. |
| **`sbi_set_timer(stime_value)`** | Lập lịch ngắt timer S-mode tiếp theo bằng giá trị thời gian tuyệt đối qua `SBI_EXT_TIME` (`FID = 0`)[cite: 4]. |
| **`sbi_shutdown(void)`** | Tắt hệ thống qua `SBI_EXT_SRST` (`SHUTDOWN`, `REASON_NONE`); nếu nhận lỗi `SBI_ERR_NOT_SUPPORTED` thì fallback sang `SBI_EXT_LEGACY_SHUTDOWN (0x08)`, sau đó khóa CPU bằng vòng lặp `wfi`[cite: 4]. |
| **`sbi_hart_start(...)` & `sbi_hart_get_status(...)`** | Khởi động và truy vấn trạng thái của các nhân CPU phụ qua `SBI_EXT_HSM` (`FID = 0` và `FID = 2`)[cite: 4]. |
| **`sbi_probe_extension(ext_id)`** | Kiểm tra sự tồn tại của một SBI Extension thông qua `SBI_EXT_BASE_PROBE_EXT (FID = 3)`, trả về `1` nếu được hỗ trợ (`error == SBI_SUCCESS` và `value != 0`), ngược lại trả về `0`[cite: 3, 4]. |

#### C. Tệp cấu hình biên dịch `Makefile`
- Bổ sung `$K/sbi.o` vào danh sách đối tượng biên dịch `OBJS` (đặt sau `$K/entry.o` để giữ nguyên điểm vào chương trình tại `0x80200000`)[cite: 6].

---

## 3. Kết quả kiểm thử & Xác minh

1. **Biên dịch sạch cảnh báo (`Zero Warnings`):**
   - Thực thi `make clean && make kernel/kernel` với cờ `-Wall -Werror`: quá trình biên dịch `kernel/sbi.c` và liên kết hoàn tất không phát sinh bất kỳ lỗi hay cảnh báo nào.
2. **Kiểm tra địa chỉ nạp và bảng ký hiệu (`ELF Symbols`):**
   - Kiểm tra bằng `readelf -h kernel/kernel`: địa chỉ `Entry point address` giữ nguyên chuẩn `0x80200000`[cite: 6], toàn bộ các hàm `sbi_*` được liên kết chính xác vào phân đoạn `.text`.
3. **Định dạng chuẩn & Tích hợp CI:**
   - Đã chạy `make fmt` chuẩn hóa mã nguồn theo `.clang-format` của dự án, đẩy nhánh `feat/PROJ-B1-sbi-base` lên GitHub (`git commit -m "PROJ-B1: add SBI ecall wrapper and helpers"`), vượt qua toàn bộ kiểm tra tự động trên GitHub Actions và được merge vào `main`[cite: 6].

---

## 4. Vai trò nền tảng đối với các Task tiếp theo
Việc hoàn tất **T4-B1** cung cấp giao diện API thống nhất và cơ chế fallback an toàn cho toàn bộ các module chức năng phía sau của nhóm:
- **T4-B2 (Timer):** Sử dụng trực tiếp `sbi_set_timer()` trong `trap.c` để lập lịch ngắt đồng hồ S-mode (`STIP`)[cite: 9].
- **T4-B3 (Console):** Sử dụng `sbi_putchar()` (và tùy chọn nâng cấp `sbi_console_write()` qua `sbi_probe_extension(SBI_EXT_DBCN)`) trong `uart.c`[cite: 3].
- **T4-B4 (Shutdown):** Sử dụng `sbi_shutdown()` cho lời gọi hệ thống `sys_halt()` để tắt QEMU an toàn[cite: 4].
- **Tuần 5 (Multi-hart Boot):** Sử dụng `sbi_hart_start()` và `sbi_hart_get_status()` để kích hoạt và kiểm tra các nhân CPU phụ[cite: 4, 9].
