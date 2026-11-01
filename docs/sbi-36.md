# SBI-36: Quy ước ecall và mã lỗi SBI

## 1. Mục tiêu

Tìm hiểu cơ chế gọi dịch vụ SBI (Supervisor Binary Interface) từ S-mode thông qua lệnh `ecall`, cách truyền tham số bằng thanh ghi RISC-V và cách nhận kết quả trả về, bao gồm mã lỗi.

Thực nghiệm được thực hiện trên QEMU với firmware OpenSBI.

## 2. Tổng quan về SBI

SBI là giao diện tiêu chuẩn cho phép phần mềm chạy ở Supervisor mode (S-mode), chẳng hạn như hệ điều hành, yêu cầu dịch vụ từ firmware chạy ở Machine mode (M-mode).

OpenSBI là một triển khai của SBI. Trong thực nghiệm này, chương trình kiểm thử chạy ở S-mode và sử dụng `ecall` để gọi các dịch vụ do OpenSBI cung cấp.

## 3. Quy ước gọi SBI ecall

Khi thực hiện một SBI call, các thanh ghi được sử dụng như sau:

| Thanh ghi | Chức năng |
|---|---|
| `a0` – `a5` | Truyền tối đa 6 tham số |
| `a6` | Function ID (FID) |
| `a7` | Extension ID (EID) |

Sau khi thực hiện lệnh `ecall`, kết quả được trả về:

| Thanh ghi | Chức năng |
|---|---|
| `a0` | Mã trạng thái hoặc mã lỗi |
| `a1` | Giá trị trả về |

Cấu trúc kết quả tương ứng trong chương trình C:

```c
struct sbiret {
    long error;
    long value;
};
```

Trong đó, `error = 0` biểu thị lời gọi thành công. Nếu lời gọi thất bại, `error` chứa mã lỗi SBI tương ứng.

## 4. Các mã lỗi SBI

| Mã | Tên | Ý nghĩa |
|---:|---|---|
| 0 | `SBI_SUCCESS` | Thành công |
| -1 | `SBI_ERR_FAILED` | Thất bại |
| -2 | `SBI_ERR_NOT_SUPPORTED` | Không được hỗ trợ |
| -3 | `SBI_ERR_INVALID_PARAM` | Tham số không hợp lệ |
| -4 | `SBI_ERR_DENIED` | Bị từ chối |
| -5 | `SBI_ERR_INVALID_ADDRESS` | Địa chỉ không hợp lệ |
| -6 | `SBI_ERR_ALREADY_AVAILABLE` | Đã khả dụng |
| -7 | `SBI_ERR_ALREADY_STARTED` | Đã khởi động |
| -8 | `SBI_ERR_ALREADY_STOPPED` | Đã dừng |
| -9 | `SBI_ERR_NO_SHMEM` | Không có vùng nhớ chia sẻ |
| -10 | `SBI_ERR_INVALID_STATE` | Trạng thái không hợp lệ |
| -11 | `SBI_ERR_BAD_RANGE` | Phạm vi không hợp lệ |
| -12 | `SBI_ERR_TIMEOUT` | Hết thời gian chờ |
| -13 | `SBI_ERR_IO` | Lỗi vào/ra |
| -14 | `SBI_ERR_DENIED_LOCKED` | Bị từ chối do trạng thái khóa |

Theo đặc tả SBI, khi EID hoặc FID không được hỗ trợ, lời gọi phải trả về `SBI_ERR_NOT_SUPPORTED` (-2).

## 5. Thiết kế chương trình thực nghiệm

Chương trình được viết bằng C và Assembly, gồm các thành phần:

- `start.S`: thiết lập stack và chuyển sang hàm `main`.
- `main.c`: thực hiện các SBI ecall và phân tích kết quả.
- `uart.c`, `uart.h`: xuất kết quả ra UART.
- `linker.ld`: thiết lập địa chỉ nạp chương trình.
- `Makefile`: biên dịch chương trình bằng RISC-V cross compiler.

Hàm `sbi_call()` sử dụng các thanh ghi `a0`, `a1`, `a2`, `a6`, `a7` để truyền tham số và nhận kết quả sau `ecall`.

## 6. Môi trường thực nghiệm

- Máy ảo: QEMU RISC-V `virt`.
- Firmware: OpenSBI v1.8.
- SBI Specification Version: 3.0.
- Kiến trúc: RISC-V 64-bit.
- Chế độ thực thi của chương trình: S-mode.
- Số hart: 1.

Lệnh thực thi:

```bash
qemu-system-riscv64 \
-machine virt \
-bios /usr/share/qemu/opensbi-riscv64-generic-fw_dynamic.bin \
-kernel sbi36.elf \
-m 128M \
-smp 1 \
-nographic
```

## 7. Kết quả thực nghiệm

Chương trình thực hiện 5 phép thử:

| Phép thử | EID | FID | Kết quả |
|---|---:|---:|---|
| Lấy phiên bản SBI | `0x10` | 0 | `error = 0`, `value = 0x03000000` |
| Lấy Implementation ID | `0x10` | 1 | `error = 0`, `value = 1` |
| Lấy Implementation Version | `0x10` | 2 | `error = 0`, `value = 0x00010008` |
| Kiểm tra TIME Extension | `0x10` | 3 | `error = 0`, `value = 1` |
| Gọi extension không tồn tại | `0xDEADBEEF` | 0 | `error = -2` |

### Phân tích

**Phép thử 1:** OpenSBI trả về `0x03000000`, tương ứng phiên bản SBI 3.0.

**Phép thử 2:** Giá trị Implementation ID bằng 1, tương ứng với OpenSBI.

**Phép thử 3:** Firmware trả về Implementation Version `0x00010008`.

**Phép thử 4:** Giá trị trả về bằng 1 cho biết TIME Extension được hỗ trợ.

**Phép thử 5:** Extension không tồn tại trả về `-2`, tương ứng `SBI_ERR_NOT_SUPPORTED`. Đây là phép thử xác nhận cơ chế xử lý lỗi của SBI.

## 8. Kết luận

Qua thực nghiệm, chương trình đã thực hiện thành công các SBI ecall từ S-mode thông qua OpenSBI. Kết quả cho thấy quy ước truyền tham số bằng các thanh ghi RISC-V, cách nhận giá trị trả về qua `a0` và `a1`, cũng như cơ chế thông báo lỗi của SBI hoạt động đúng như mong đợi.

Đặc biệt, chương trình đã kiểm chứng được trường hợp lời gọi thành công và trường hợp gọi một extension không được hỗ trợ.

## 9. Minh chứng

- Log thực nghiệm: `docs/evidence/sbi-36.log`
- Ảnh chụp kết quả: `docs/evidence/sbi-36.png`
