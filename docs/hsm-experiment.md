# SBI-84 - Thực nghiệm HSM trong "hello" (bảng mã lỗi)

**Môi trường thực nghiệm:** QEMU `virt`, OpenSBI v1.8, SBI 3.0, RAM 128 MiB.

## 1. Bảng kết quả thực nghiệm

| Nội dung kiểm tra | SMP 1 | SMP 2 | SMP 8 |
|---|---|---|---|
| Boot hart ID | 0 | 0 | 7 |
| Target hart ID | 1 | 1 | 0 |
| GET_STATUS trước khi khởi động | -3 | 0 (STOPPED) | 0 (STOPPED) |
| START target hart | -3 | 0 | 0 |
| Hart phụ thực thi `_secondary` | Không | Có | Có |
| Giá trị opaque nhận được | - | `0xCAFE` | `0xCAFE` |
| GET_STATUS sau khi khởi động | -3 | 0 (STARTED) | 0 (STARTED) |
| START target hart lần thứ hai | -3 | -6 | -6 |
| START boot hart | -6 | -6 | -6 |
| START hart ID 99 | -3 | -3 | -3 |

## 2. Bảng mã lỗi SBI quan sát được

| Mã lỗi | Tên mã lỗi | Ý nghĩa |
|---:|---|---|
| 0 | `SBI_SUCCESS` | Lời gọi SBI thành công |
| -3 | `SBI_ERR_INVALID_PARAM` | Tham số không hợp lệ hoặc hart ID không tồn tại |
| -6 | `SBI_ERR_ALREADY_AVAILABLE` | Hart đã ở trạng thái hoạt động hoặc khả dụng |

## 3. Nhận xét kết quả

- **Với SMP 1:** QEMU chỉ có hart 0, do đó hart 1 không tồn tại. Các thao tác truy vấn trạng thái và khởi động hart 1 đều trả về mã lỗi `-3` (`SBI_ERR_INVALID_PARAM`).

- **Với SMP 2:** Boot hart là hart 0, target hart là hart 1. Ban đầu hart 1 ở trạng thái `STOPPED`. Sau khi gọi `sbi_hart_start()`, hart 1 được khởi động thành công, thực thi hàm `_secondary` và nhận đúng giá trị opaque `0xCAFE`. Trạng thái sau đó chuyển thành `STARTED`.

- **Với SMP 8:** OpenSBI chọn hart 7 làm boot hart, trong khi target hart là hart 0. Hart 0 được khởi động thành công và thực thi hàm `_secondary`. Kết quả này chứng minh boot hart không nhất thiết phải là hart 0.

- Khi khởi động lại một hart đã hoạt động hoặc khởi động chính boot hart, OpenSBI v1.8 trả về `SBI_ERR_ALREADY_AVAILABLE` (`-6`).

- Khi thử khởi động hart ID 99 không hợp lệ, lời gọi trả về `SBI_ERR_INVALID_PARAM` (`-3`).

## 4. Kết luận

Thực nghiệm xác nhận SBI HSM có thể được sử dụng để khởi động hart phụ thông qua hàm `sbi_hart_start()`. Hart phụ bắt đầu thực thi tại entry point `_secondary` được truyền vào lời gọi SBI và nhận được tham số opaque tương ứng.

Kết quả cũng cho thấy không được giả định boot hart luôn có ID bằng 0. Khi tích hợp vào xv6, cần xác định đúng boot hart, thiết lập stack riêng cho từng hart và xử lý phù hợp các mã lỗi do OpenSBI trả về, đặc biệt là `SBI_ERR_ALREADY_AVAILABLE`.
