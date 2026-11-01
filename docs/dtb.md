# KHẢO SÁT DEVICE TREE TRÊN QEMU RISC-V

**Họ và tên:** Truong Dinh Thanh  
**Mã số sinh viên:** 20233642    
**Ngày thực hiện:** 01/11/2026

---

## 1. Mục tiêu

Mục tiêu của bài thực hành là khảo sát Device Tree trên máy ảo QEMU RISC-V, tìm hiểu cách thông tin phần cứng được mô tả trong Device Tree và phân tích sự thay đổi của các thông số khi thay đổi cấu hình QEMU.

Các nội dung chính:

- Tạo file DTB từ cấu hình máy ảo QEMU.
- Chuyển đổi DTB sang DTS để đọc và phân tích.
- Khảo sát dung lượng RAM và địa chỉ bắt đầu của RAM.
- Khảo sát số lượng CPU/Hart.
- Xác định địa chỉ UART và thông tin ngắt.
- Xác định tần số timebase của hệ thống.

## 2. Môi trường thực hiện

| Thành phần | Thông tin |
|---|---|
| Hệ điều hành | Ubuntu 26.04.1 LTS |
| Kiến trúc | RISC-V 64-bit |
| Trình giả lập | QEMU |
| Máy ảo | QEMU virt |
| Công cụ chuyển đổi Device Tree | Device Tree Compiler (dtc) |
| RAM thử nghiệm | 64 MiB, 128 MiB, 256 MiB |
| Số lượng Hart thử nghiệm | 1, 4, 8 |

## 3. Phương pháp thực hiện

### 3.1. Tạo file DTB

DTB (Device Tree Blob) là dạng nhị phân của Device Tree, chứa thông tin mô tả phần cứng mà hệ điều hành có thể sử dụng.

Sử dụng tùy chọn `dumpdtb` của QEMU để xuất thông tin Device Tree theo từng cấu hình.

Ví dụ, với máy ảo có 4 CPU và 128 MiB RAM:

```bash
qemu-system-riscv64 \
  -machine virt,dumpdtb=experiments/dtb/virt-128m-4cpu.dtb \
  -m 128M \
  -smp 4 \
  -nographic
dtc -I dtb -O dts \
  -o experiments/dtb/virt-128m-4cpu.dts \
  experiments/dtb/virt-128m-4cpu.dtb
