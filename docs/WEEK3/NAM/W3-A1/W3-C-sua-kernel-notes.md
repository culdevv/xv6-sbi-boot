# Ghi chú R1: Boot xv6 trên OpenSBI (Tuần 3)

- Người thực hiện: Nguyễn Hải Nam - 20233546
- Task Jira:  mã [W3-C]
- Nhánh: feat/W3-C1-load-addr
- Ngày: 6/11/2026

## 1. Mục tiêu
Đưa kernel xv6 nạp tại 0x80200000 (OpenSBI chiếm 0x80000000-0x801FFFFF),
chạy ngay ở S-mode, nhận hartid (a0) và địa chỉ DTB (a1) từ OpenSBI, và in
được dòng "booted" qua SBI. Chưa dọn mã M-mode trong start.c (việc của R2).

## 2. Môi trường
- Hệ điều hành: Ubuntu 24.04 LTS
- QEMU: QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18)
- Toolchain: riscv64-unknown-elf-gcc (13.2.0-11ubuntu1+12) 13.2.0
- Tag/commit xv6 gốc: tag base-xv6

## 3. Những gì đã sửa

| File | Thay đổi | Lý do |
|---|---|---|
| kernel/kernel.ld | Địa chỉ nạp 0x80000000 -> 0x80200000 | OpenSBI nhảy vào kernel tại 0x80200000 và chiếm vùng bên dưới |
| kernel/memlayout.h | KERNBASE = 0x80200000L; PHYSTOP = 0x80000000L + 128*1024*1024 (cố định) | PHYSTOP không được tính theo KERNBASE, nếu không sẽ vượt hết RAM (0x88200000 > 0x88000000) |
| Makefile | `-bios none` -> `-bios default` | Để QEMU nạp OpenSBI |
| kernel/entry.S | Cất a0/a1, kiểm tra hartid < NCPU, đặt tp và sp theo hartid, gọi boot_stub | mhartid là thanh ghi M-mode, ở S-mode phải dùng hartid do OpenSBI truyền |
| kernel/sbi_stub.c (mới) | Hàm boot_stub in "booted, hartid=N" bằng SBI legacy putchar rồi tắt máy bằng SRST | Bằng chứng nhìn thấy được rằng đường boot hoạt động |
| Makefile | thêm `sbi_stub.o` vào cuối danh sách OBJS | liên kết với hàm boot_stub mới chạy được SBI |

Không đụng tới: kernel/start.c (vẫn còn mã M-mode, bị bỏ qua bởi entry.S).

## 4. Lệnh đã chạy và kết quả

### 4.1 Build
Lệnh: `make clean && make`
Kết quả: Thành công
Log đầy đủ: docs/logs/w3-build.log

### 4.2 Kiểm tra địa chỉ nạp
Lệnh: `readelf -l kernel/kernel`
Kết quả: VirtAddr của segment LOAD đầu tiên = 0x0000000080200000
Lệnh: `riscv64-unknown-elf-nm kernel/kernel | grep _entry`
Kết quả: 0000000080200000 T _entry
Đạt yêu cầu khi cả hai bằng 0x80200000: Đạt 

### 4.3 Chạy QEMU 1 hart
Lệnh: `timeout 30 make qemu CPUS=1`
Kết quả: thấy banner OpenSBI, thấy dòng "booted, hartid=0", sau đó QEMU tự thoát
Log: docs/logs/w3-qemu-cpus1.log

### 4.4 Chạy QEMU 4 hart
Lệnh: `timeout 30 make qemu CPUS=4`
Kết quả chạy 4 lần: 4 dòng "booted" in ra, hartid là 0-2-1-2
Nhận xét: chạy 4 lần, hartid thay đổi từ 0-2-1-2
Log: docs/logs/w3-qemu-cpus4.log

## 5. Kết luận
Kết quả đạt 

## 6. Lỗi gặp và cách xử lý

| Lỗi | Nguyên nhân | Cách xử lý |
|---|---|---|
| `kernel/memlayout.h:44: error: expected ')' before 'MB'` khi biên dịch kalloc.c | Viết chữ `MB` vào biểu thức trong #define PHYSTOP | Đổi thành `(0x80000000L + 128*1024*1024)` |
| `undefined reference to boot_stub` | quên thêm sbi_stub.o vào OBJS trong Makefile | thêm vào cuối danh sách OBJS trong Makefile |

## 7. Hạn chế hiện tại
- Kernel chưa vào shell. Sau commit này nhánh main không chạy xv6 đầy đủ cho tới khi
  làm xong R2 (dọn start.c) ở Tuần 4. Bản xv6 gốc vẫn ở tag base-xv6.
- boot_stub là mã tạm, sẽ thay bằng sbi.c ở Tuần 4.
- Chưa làm: bỏ M-mode (R2), timer (R3), console qua SBI (R4), hart start (R5).

## 8. Việc tiếp theo
- R2: dọn start.c (bỏ mstatus, mepc, medeleg, mideleg, PMP, timerinit, mret).
- Gọi main() thật thay vì boot_stub.
- Nhờ Lực review sbi.h để viết sbi.c.

## 9. Cách tái lập kết quả
1. git clone https://github.com/culdevv && git checkout feat/W3-C1-load-addr
2. make clean && make
3. make qemu CPUS=1
