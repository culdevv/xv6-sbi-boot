W4-A1: link kernel at 0x80200000, split RAMBASE/KERNBASE, use OpenSBI
| Mục |	Nội dung |
| Người thực hiện | Nguyễn Hải Nam |
| Task Jira | T4-A1 |
| Nhánh	feat/W4-A1-load-address |
| Pull request #30 |
| Commit liên quan | 39a197a ("Tuan 4: doi dia chi kernel thanh 0x80200000, tach RAMBASE/KERNBASE...") |
| Yêu cầu đề | R1: kernel được nạp tại địa chỉ do OpenSBI quy định |

1. Mục tiêu

OpenSBI (chạy ở M-mode) chiếm vùng đầu RAM từ 0x80000000. Nó nạp kernel S-mode tại 0x80200000 rồi nhảy vào. xv6 gốc link kernel tại 0x80000000 và chạy với -bios none, nên không dùng được trực tiếp. Task này đổi địa chỉ link và cách khai báo vùng nhớ để kernel khớp với OpenSBI.

2. Đã làm:
- kernel/kernel.ld: đổi địa chỉ đặt đầu 0x80000000 -> 0x80200000
  (2 MB đầu RAM do OpenSBI chiếm).
- kernel/memlayout.h: tách RAMBASE=0x80000000, KERNBASE=0x80200000,
  PHYSTOP=RAMBASE+128MB (tính từ đầu RAM, không tính từ KERNBASE,
  tránh vượt cuối RAM 2 MB).
- Makefile: QEMUOPTS đổi -bios none -> -bios default.
- Xóa dòng sbi_stub.o còn sót trong OBJS (của tuần 3).

3. Lệnh kiểm tra:
  make clean && make kernel/kernel
  nm -n kernel/kernel | head -5
  riscv64-unknown-elf-readelf -l kernel/kernel
  make fmt && git diff --exit-code
4. Kết quả:
- Build không warning/lỗi.
- nm: _entry tại 0000000080200000.
- readelf: LOAD PhysAddr bắt đầu 0x80200000.
- make qemu CPUS=1: treo sau banner OpenSBI. Đây là dự kiến, vì
  entry.S còn đọc mhartid (xử lý ở A2).
