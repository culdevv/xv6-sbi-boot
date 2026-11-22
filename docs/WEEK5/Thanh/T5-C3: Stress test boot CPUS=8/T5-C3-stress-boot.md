# T5-C3: Kiểm thử stress quá trình khởi động xv6 với 8 hart

## 1. Mục tiêu

T5-C3 thực hiện kiểm thử lặp quá trình khởi động xv6 trên QEMU với cấu hình `CPUS=8`, nhằm đánh giá tính ổn định của quá trình khởi tạo hệ thống đa hart.

Các tiêu chí cần kiểm tra trong mỗi lần boot:

- Kernel xuất hiện thông báo khởi động.
- Đủ 8 hart khác nhau (hart 0–7) được đưa vào hoạt động.
- xv6 hoàn thành khởi động và hiển thị shell prompt `$`.

## 2. Khảo sát và triển khai

Trước khi triển khai, kiểm tra hệ thống test hiện có:

- `tests/lib.py`: cung cấp lớp `Qemu`, các hàm kiểm tra output và lưu log.
- `tests/run_tests.py`: tự động phát hiện và chạy các file test có tên `t_*.py`.
- `tests/t_boot.py`: kiểm tra quá trình boot cơ bản với `CPUS=1`.

Từ đó, tạo mới file **`tests/t_stress.py`** để bổ sung bài kiểm thử stress mà không cần sửa các thành phần hiện có.

Bài test sử dụng biến môi trường `BOOT_REPEAT` để cấu hình số lần chạy:

```bash
BOOT_REPEAT=20 python3 tests/run_tests.py
```

Nếu không thiết lập biến này, chương trình mặc định chạy 5 lần.

Mỗi lần kiểm thử khởi chạy một tiến trình QEMU độc lập với `CPUS=8` và kiểm tra ba điều kiện: kernel boot banner, đủ 8 hart riêng biệt và shell prompt. Kết quả từng lần được ghi nhận bằng `check()`, đồng thời QEMU được đóng sau mỗi lần chạy để tránh tồn đọng tiến trình.

Sau khi triển khai, chạy kiểm tra cú pháp:

```bash
python3 -m py_compile tests/t_stress.py
```

Kết quả: không có lỗi cú pháp.

## 3. Kết quả kiểm thử

### 3.1. Kiểm thử ban đầu

Chạy toàn bộ test suite với cấu hình mặc định:

```bash
python3 tests/run_tests.py
```

Bài stress test mặc định thực hiện 5 lần boot.

| Lần kiểm thử | Cấu hình | Kết quả |
|---|---|---|
| 1–5 | CPUS=8 | 5/5 PASS |

Cả 5 lần đều khởi động thành công, nhận đủ 8 hart và đi đến shell.

### 3.2. Kiểm thử chính thức

Tăng số lần lặp lên 20 và lưu output:

```bash
set -o pipefail
BOOT_REPEAT=20 python3 tests/run_tests.py 2>&1 | tee /tmp/t5-c3-stress-20.log
```

Kết quả bài T15:

| Tiêu chí | Kết quả |
|---|---:|
| Tổng số lần boot | 20 |
| Số lần PASS | 20 |
| Số lần FAIL | 0 |
| Tỷ lệ thành công | 100% |

Cả 20 lần đều thỏa mãn ba điều kiện kiểm tra.

## 4. Xác minh và lưu trữ bằng chứng

Sau khi chạy kiểm thử, thu thập 20 file log tương ứng với lần chạy chính thức.

Trong quá trình xác minh, phát hiện log QEMU sử dụng ký tự xuống dòng CRLF (`\r\n`), khiến lệnh `grep` ban đầu không nhận diện đúng các dòng `hart starting`. Sau khi chuẩn hóa bằng `tr -d '\r'`, kết quả xác minh cho thấy:

| Nội dung xác minh | Kết quả |
|---|---:|
| Số file log kiểm tra | 20 |
| Log có đủ 8 dòng hart starting | 20/20 |
| Log có đủ 8 hart riêng biệt | 20/20 |
| Log thiếu hart | 0 |

Lưu trữ bằng chứng tại:

```text
docs/WEEK5/Thanh/T5-C3: Stress test boot CPUS=8/
```

Thư mục bao gồm:
- `T5-C3-stress-boot.md`: báo cáo thực hiện.
- `stress-test-output.log`: output của lần chạy 20 vòng.
- 20 file `stress-cpus8-*.log`: log riêng của từng lần boot.

## 5. Các thay đổi trong repository

Các file được bổ sung:

```text
tests/
└── t_stress.py

docs/WEEK5/Thanh/
└── T5-C3: Stress test boot CPUS=8/
    ├── T5-C3-stress-boot.md
    ├── stress-test-output.log
    └── stress-cpus8-*.log
```

Trong quá trình thực hiện:
- Chỉ bổ sung bài kiểm thử mới `tests/t_stress.py`.
- Không sửa `tests/lib.py`, `tests/run_tests.py` và `tests/t_boot.py`.
- Không thay đổi mã nguồn khởi tạo hart trong kernel hoặc Makefile.

**Lưu ý:** Test suite có kết quả `24/25 passed`. Bài T04 (`timer ticks advance`) có kết quả FAIL; đây là bài kiểm tra timer có sẵn, độc lập với T5-C3. Không sửa T04 trong phạm vi nhiệm vụ này.

## 6. Kết luận

T5-C3 đã hoàn thành các mục tiêu kiểm thử stress boot với cấu hình 8 hart.

Kết quả thực nghiệm:
- Kiểm thử ban đầu: 5/5 lần PASS.
- Kiểm thử chính thức: 20/20 lần PASS.
- Tất cả 20 lần đều nhận đủ 8 hart và đi đến shell prompt.
- Xác minh độc lập 20 file log: không có lần nào thiếu hart.

Kết quả cho thấy quá trình khởi động xv6 với `CPUS=8` ổn định trong phạm vi thử nghiệm đã thực hiện. Tuy nhiên, 20 lần chạy thành công chưa đủ để khẳng định tuyệt đối hệ thống không tồn tại race condition.

**Kết luận: T5-C3 – PASS, 20/20 lần boot CPUS=8 thành công.**
