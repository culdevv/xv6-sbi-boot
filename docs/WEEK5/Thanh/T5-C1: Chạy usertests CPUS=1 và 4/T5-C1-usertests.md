# Báo cáo Kỹ thuật Task T5-C1: Kiểm thử `usertests` với nhiều cấu hình CPU

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Trương Đình Thanh
* **Nhánh thực hiện:** `feat/T5-C1-usertests`
* **Trạng thái:** Hoàn thành — PASS trên cả hai cấu hình `CPUS=1` và `CPUS=4`.

---

## 1. Mục tiêu công việc

Thực hiện kiểm thử toàn bộ chương trình `usertests` của xv6-sbi-boot trên hai cấu hình CPU khác nhau nhằm đánh giá tính ổn định của kernel khi chạy các bài kiểm thử ở chế độ một CPU và đa CPU.

Các mục tiêu cụ thể:

- Kiểm tra hoạt động của các system call và chức năng quản lý tiến trình.
- Kiểm tra các chức năng bộ nhớ, file system, pipe và xử lý tiến trình đồng thời.
- Đánh giá khả năng hoạt động của hệ thống khi thay đổi số lượng CPU.
- Lưu lại kết quả kiểm thử để làm minh chứng cho Task T5-C1.

---

## 2. Các công việc đã thực hiện (Testing Activities)

| Hạng mục | Hành động | Mô tả chi tiết |
| :--- | :--- | :--- |
| `CPUS=1` | **Kiểm thử** | Chạy toàn bộ chương trình `usertests` trên QEMU với một CPU. Ghi lại kết quả vào `docs/WEEK5/Thanh/T5-C1: Chạy usertests CPUS=1 và 4/usertests-cpus1.log`. |
| `CPUS=4` | **Kiểm thử** | Chạy toàn bộ chương trình `usertests` trên QEMU với bốn CPU. Ghi lại kết quả vào `docs/WEEK5/Thanh/T5-C1: Chạy usertests CPUS=1 và 4/usertests-cpus4.log`. |
| Log kiểm thử | **Lưu trữ** | Lưu đầu ra của từng phiên chạy để phục vụ kiểm tra, đối chiếu và báo cáo. |
| Phân loại kết quả | **Đánh giá** | Kiểm tra trạng thái hoàn thành của chương trình, các dòng báo lỗi và dấu hiệu kernel panic. |

---

## 3. Quy trình kiểm thử

Sử dụng QEMU để khởi động xv6-sbi-boot và chạy chương trình `usertests` với hai cấu hình:

**Cấu hình 1 CPU:**
```bash
make qemu CPUS=1
```
Sau khi vào shell xv6, chạy:
```bash
usertests
```

**Cấu hình 4 CPU:**
```bash
make qemu CPUS=4
```
Sau khi vào shell xv6, chạy:
```bash
usertests
```

Quá trình kiểm thử được tự động hóa bằng Python để khởi chạy QEMU, gửi lệnh kiểm thử, thu thập đầu ra và lưu log riêng cho từng cấu hình.

---

## 4. Kết quả kiểm thử

Kết quả thực nghiệm:

| Cấu hình | Kết quả | Ghi chú |
| :--- | :--- | :--- |
| `CPUS=1` | **PASS** | Xuất hiện `ALL TESTS PASSED`. |
| `CPUS=4` | **PASS** | Xuất hiện `ALL TESTS PASSED`. |

Cả hai cấu hình đều hoàn thành toàn bộ chương trình `usertests`. Không ghi nhận test thất bại hoặc kernel panic trong các phiên kiểm thử.

**Kết quả tổng hợp:**
```text
CPUS=1: PASS
ALL TESTS PASSED

CPUS=4: PASS
ALL TESTS PASSED
```

---

## 5. Kết luận

Đã hoàn thành Task T5-C1. Chương trình `usertests` vượt qua toàn bộ bài kiểm thử trên cả hai cấu hình `CPUS=1` và `CPUS=4`.

Kết quả cho thấy các chức năng cơ bản của kernel xv6-sbi-boot tiếp tục hoạt động ổn định trong hai cấu hình CPU đã thử nghiệm. Hai file log được lưu lại làm minh chứng cho quá trình kiểm thử.
