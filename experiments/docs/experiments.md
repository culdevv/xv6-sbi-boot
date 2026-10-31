# Nhật ký Thực nghiệm Hello S-mode & OpenSBI Calls - Vũ Đình Lực (20233513)

- **Ngày thực hiện:** 31/10/2026
- **Môi trường:** Ubuntu 24.04 LTS (QEMU 8.2.2, OpenSBI v1.3)
- **File thực nghiệm:** `experiments/hello.elf`

## Kết quả Thực nghiệm Thực tế

1. **Thông số nạp từ OpenSBI:**
   - Entry point (`Domain0 Next Address`): `0x80200000`
   - DTB Pointer (`a1`): `0x87e00000`
   - Mode Execution: `S-mode`

2. **Kết quả Output từ `hello.elf`:**

Hello S-mode! hartid=0x0000000000000000 dtb=0x0000000087e00000

3. **Cơ chế Tắt máy (Shutdown):**
- Gọi thành công dịch vụ Legacy Shutdown (`a7 = 8`) qua `ecall`, giúp QEMU tự động thoát sạch sẽ về dòng lệnh Terminal không bị treo.

# Nhật ký 4 Thực nghiệm Hello S-mode & OpenSBI Calls - Vũ Đình Lực

1. **Thông số nạp mặc định (-smp 1):**
   - Entry Address: `0x80200000`
   - `hartid` = `0x0`
   - Con trỏ DTB (`a1`) = `0x87e00000`

2. **Kiểm thử Đa nhân (-smp 4):**
   - Chỉ có 1 dòng "Hello S-mode" duy nhất in ra từ Hart 0.
   - **Kết luận:** Các Hart phụ mặc định ở trạng thái `STOPPED`, kernel bắt buộc phải dùng lệnh SBI HSM (`sbi_hart_start`) để khởi động.

3. **Kiểm thử EID không hợp lệ (0xDEADBEEF):**
   - Giá trị trả về trong thanh ghi `a0` là `-2` (`SBI_ERR_NOT_SUPPORTED`).

4. **So sánh Cơ chế Shutdown:**
   - Cả SBI Legacy Shutdown (`a7 = 8`) và SRST System Reset Extension (`a7 = 0x53525354`) đều tắt QEMU an toàn và tự động trả về dòng lệnh Terminal.
