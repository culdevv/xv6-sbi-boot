củaW4-A1: link kernel at 0x80200000, split RAMBASE/KERNBASE, use OpenSBI

Đã làm:
- kernel/kernel.ld: đổi địa chỉ đặt đầu 0x80000000 -> 0x80200000
  (2 MB đầu RAM do OpenSBI chiếm).
- kernel/memlayout.h: tách RAMBASE=0x80000000, KERNBASE=0x80200000,
  PHYSTOP=RAMBASE+128MB (tính từ đầu RAM, không tính từ KERNBASE,
  tránh vượt cuối RAM 2 MB).
- Makefile: QEMUOPTS đổi -bios none -> -bios default.
- Xóa dòng sbi_stub.o còn sót trong OBJS (của tuần 3).

Lệnh kiểm tra:
  make clean && make kernel/kernel
  nm -n kernel/kernel | head -5
  riscv64-unknown-elf-readelf -l kernel/kernel
  make fmt && git diff --exit-code

Kết quả:
- Build không warning/lỗi.
- nm: _entry tại 0000000080200000 [dán dòng nm thật].
- readelf: LOAD PhysAddr bắt đầu 0x80200000 [xác nhận].
- make qemu CPUS=1: treo sau banner OpenSBI. Đây là dự kiến, vì
  entry.S còn đọc mhartid (xử lý ở A2).
