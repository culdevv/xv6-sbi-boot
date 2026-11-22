# Báo cáo Kỹ thuật Task T5-B4: Bộ Kiểm thử Tự động (Halt, Timer, Console)

- **Mã Task (Jira):** T5-B4
- **Người thực hiện:** Vũ Đình Lực (MSSV: 20233513)
- **Nhánh thực hiện:** `feat/T5-B4-tests`
- **Yêu cầu liên quan:** D3 (Automated Test Suite), R3 (SBI Timer), R4 (SBI Console), R6 (SBI Shutdown / Halt)

---

## 1. Mục tiêu công việc
Xây dựng bộ kịch bản kiểm thử tự động bằng Python tích hợp vào khung `tests/run_tests.py` (`make test`), bao phủ 6 test cases từ **T09 đến T14** nhằm tự động hóa việc nghiệm thu các tính năng cốt lõi trong Tuần 5 (Mốc Giữa kỳ — Midterm):
1. Kiểm chứng lệnh `halt` tắt QEMU sạch sẽ với mã thoát `0` cả khi hệ thống rảnh và khi có tiến trình chiếm dụng 100% CPU (`spin &`).
2. Kiểm chứng bộ định thời (Timer) qua `ticks` và đo tần số ngắt thực tế (`tickrate`) nằm trong sai số $\pm 5\%$ so với tần số chuẩn $10\text{ Hz}$.
3. Kiểm chứng luồng xuất/nhập Console qua SBI (`printtest` xuất 200 dòng không mất ký tự và `echo` nhận phản hồi bàn phím).

---

## 2. Chi tiết các tệp đã tạo và thay đổi

| Tệp mã nguồn | Thao tác | Mô tả chi tiết nội dung |
| :--- | :--- | :--- |
| `tests/t_halt.py` | Tạo mới | Cài đặt **T09** (`halt` ở `CPUS=1` thoát mã `0` trong 20s) và **T10** (`spin &` ở `CPUS=1` sau đó gọi `halt`, kiểm tra timer preemption hoạt động đúng và QEMU tắt mã `0` trong 30s). |
| `tests/t_timer.py` | Tạo mới | Cài đặt **T11** (`ticks` ở `CPUS=2` kiểm tra bước nhảy tick) và **T12** (`tickrate` đo tần số ngắt trong ngưỡng `9500..10500` mili-Hz, tức $10\text{ Hz} \pm 5\%$). |
| `tests/t_console.py` | Tạo mới | Cài đặt **T13** (`printtest` kiểm tra nhận đủ 200 dòng `PT 0..199` qua SBI không rơi ký tự) và **T14** (`echo midterm-ok` kiểm tra luồng nhận ký tự UART RX và phản hồi). |
| `docs/T5-B4.md` | Tạo mới | Tài liệu báo cáo kết quả triển khai, gỡ lỗi và kiểm thử cho Task T5-B4. |

---

## 3. Kết quả kiểm thử cục bộ (`make test`)
Chạy kiểm thử tự động trên Ubuntu 24.04 LTS (`qemu-system-riscv64` với OpenSBI mặc định):

- **T09 (`halt` powers off QEMU with exit code 0):** `PASS`
- **T10 (`halt` works while a process spins, `CPUS=1`):** `PASS`
- **T11 (`ticks` advance by `pause(20)`, `CPUS=2`):** `PASS` *(ghi nhận lưu ý độ trễ lập lịch đa nhân trên QEMU)*
- **T12 (timer rate within 5% of 10 Hz):** `PASS` (đo thực tế: `~9.945 Hz`, sai số `~0.55%` < ngưỡng `5%`)
- **T13 (200 `printf` lines via SBI, none lost):** `PASS`
- **T14 (console input echo works):** `PASS`

---

## 4. Sự cố gặp phải 
1.  **Sai số nhịp `ticks` khi chạy song song trên QEMU đa nhân (`CPUS=2`):**
   - *Hiện tượng:* Khi chạy tay ở `CPUS=1`, `ticks` cho hiệu số chính xác `20` ticks (`TICKS 103 123`), nhưng khi chạy tự động ở `CPUS=2` trong lúc hệ thống đang tải I/O, độ trễ đánh thức tiến trình giữa các hart có thể làm hiệu số tăng thêm 1–3 ticks.
  
---
