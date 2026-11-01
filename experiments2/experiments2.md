# Nhật ký Thực nghiệm Task SBI-37: OpenSBI fw_jump, a0/a1, Vùng bị cấm

- **Thành viên thực hiện:** Vũ Đình Lực (MSSV: 20233513)
- **Môi trường:** Ubuntu 24.04 LTS (QEMU 8.2.2, OpenSBI v1.3)
- **Thư mục thực nghiệm:** `experiments2/`

---

## 1. Kết quả Thu hoạch Kỹ thuật
1. **Cơ chế Jump (`fw_jump`):** OpenSBI khởi chạy ở M-mode tại `0x80000000`, sau khi chuẩn bị xong phần cứng sẽ chuyển giao quyền điều khiển sang S-mode cho Kernel nạp tại `0x80200000`.
2. **Giá trị thanh ghi `a0` và `a1`:**
   - `a0`: Chứa Boot Hart ID (`0x0`).
   - `a1`: Chứa con trỏ địa chỉ vật lý dẫn tới Flattened Device Tree (`0x87e00000`).
3. **Vùng bộ nhớ bị cấm (Reserved Region):**
   - Vùng `0x80000000 - 0x801FFFFF` (2MB) thuộc sở hữu riêng của OpenSBI.
   - Kernel S-mode không được phép cấp phát hay ghi đè vào vùng này.
