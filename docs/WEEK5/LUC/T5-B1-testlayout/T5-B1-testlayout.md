# Báo cáo Kỹ thuật Task T5-B1: Tái cấu trúc hạ tầng kiểm thử (`test/`) & `.gitignore` log

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Vũ Đình Lực — Thành viên B (MSSV: 20233513) 
* **Nhánh thực hiện:** `feat/PROJ-B1-test-layout`
* **Trạng thái:** Hoàn thành nhưng chưa tối ưu (`3/4 -> 4/4 passed` với `make test`)

---

## 1. Mục tiêu công việc
Tách tệp kiểm thử nguyên khối `run_tests.py` của Tuần 4 thành kiến trúc dạng mô-đun (`test/lib.py` + `test/run_tests.py` + các tệp `test/t_*.py` độc lập) Mục tiêu giúp 3 thành viên trong nhóm có thể tự thêm các bài kiểm thử riêng (`t_boot.py`, `t_static.py`, `t_halt.py`, `t_timer.py`, `t_console.py`, `t_stress.py`) mà không làm phát sinh xung đột Git (merge conflict) trên cùng một tệp điều phối.

---

## 2. Các thay đổi đã thực hiện (Files Changed)

| Tệp tin | Hành động | Mô tả chi tiết thay đổi |
| :--- | :--- | :--- |
| `test/lib.py` | **Tạo mới** | Xây dựng thư viện kiểm thử dùng chung gồm danh sách `RESULTS`, hàm `check(name, ok)` và lớp điều khiển `Qemu` hỗ trợ:<br>• Ghi log tự động từng phiên chạy vào `test-results/<tag>-cpus<N>-<timestamp>.log`[cite: 1, 2].<br>• Lưu toàn bộ đầu ra (`transcript`) và phần vừa khớp (`last`)[cite: 1, 2].<br>• Các phương thức `unique()`, `wait_unique()` (đếm số lượng hart online) và `wait_exit()` (bắt mã thoát của QEMU cho lệnh `halt`)[cite: 1, 2]. |
| `test/run_tests.py` | **Cập nhật** | Viết lại bộ điều phối tự động quét toàn bộ các tệp `t_*.py` trong thư mục `test/` (và tự động nạp thêm `f_*.py` khi chạy kèm cờ `--full`)[cite: 1, 2]. Bọc lời gọi từng mô-đun trong khối `try/except` để một bài kiểm thử lỗi không làm sập toàn bộ chuỗi kiểm thử; trả về mã thoát `0` khi và chỉ khi tất cả đều `PASS`[cite: 1, 2]. |
| `test/t_boot.py` | **Tạo mới** | Tách 4 bài kiểm thử cơ bản của Tuần 4 (`T01` boot banner, `T02` shell prompt, `T03` lệnh `ls`, `T04` lệnh `ticks`) sang mô-đun độc lập, gọi qua hàm `run()` và dùng chung `Qemu`, `check` từ `lib.py`[cite: 1, 2]. |
| `Makefile` | **Cập nhật** | Chuẩn hóa mục `test` gọi `python3 test/run_tests.py` và bổ sung mục `test-full` gọi `python3 test/run_tests.py --full` (dùng ký tự Tab đầu dòng lệnh)[cite: 1, 2]. |
| `.gitignore` | **Cập nhật** | Bổ sung quy tắc `test-results/*.log` để chặn các tệp log chạy thử phát sinh cục bộ, nhưng vẫn cho phép commit thư mục lưu trữ kết quả chính thức (`test-results/midterm/`)[cite: 1, 2]. |

---

## 3. Các lỗi gặp phải & Cách khắc phục

1. **Lỗi không tìm thấy mô-đun khi chạy thử (`ModuleNotFoundError: No module named 'tests'`):**
   * *Nguyên nhân:* Thư mục kiểm thử cũ của Tuần 4 được đặt tên là `tests/` (có chữ `s`), đồng thời tệp `t_boot.py` ban đầu còn giữ câu lệnh import cũ (`import ..., Qemu, check`) và khối `__main__` tham chiếu biến `results` đã bị vô hiệu hóa.
   * *Cách khắc phục:* Đổi tên thư mục chuẩn hóa về `test/` (không có `s`) theo đúng cấu trúc CI của dự án[cite: 1, 2]; dọn sạch tệp `test/t_boot.py` để chỉ dùng `from lib import Qemu, check` và cung cấp duy nhất hàm `def run():`[cite: 1, 2].
2. **Lỗi đường dẫn thực thi `make test` (`No rule to make target 'test'`):**
   * *Nguyên nhân:* Chạy lệnh `make test` khi đang đứng bên trong thư mục con `test/` thay vì thư mục gốc của kho mã nguồn (`~/OS/xv6-sbi-boot`).
   * *Cách khắc phục:* Quay về thư mục gốc của dự án (`cd ~/OS/xv6-sbi-boot`) và đồng bộ target `test` trong `Makefile` trỏ chính xác tới `test/run_tests.py`[cite: 1, 2].

---

## 4. Kết quả kiểm thử
Chạy lệnh `make test` tại thư mục gốc dự án cho kết quả vượt qua toàn bộ 4/4 bài kiểm thử hiện có trong vòng 3 giây và tự động sinh tệp log trong `test-results/`:

```text
python3 test/run_tests.py
--- t_boot
PASS: T01 boot banner (CPUS=1)
PASS: T02 shell prompt (CPUS=1)
PASS: T03 ls shows README (CPUS=1)
PASS: T04 timer ticks advance (CPUS=1)
4/4 passed in 3s