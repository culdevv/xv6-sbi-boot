W4-A4: add make test harness with PASS/FAIL (T01-T04)

Đã làm:
- test/run_tests.py: mở QEMU bằng "make -s qemu CPUS=N", luồng phụ đọc
  output vào hàng đợi, expect(mẫu, timeout) chờ kết quả, gõ lệnh qua
  stdin, kết thúc thì giết cả nhóm tiến trình QEMU.
- Makefile: thêm mục "test" (phụ thuộc kernel/kernel, fs.img).
- 4 test (CPUS=1): T01 banner boot; T02 prompt "$";
  T03 "ls" thấy README; T04 "ticks" cho 20 <= b-a <= 22.
- Chạy "make test" thoát khác 0 nếu có FAIL.

Lệnh kiểm tra:
  make test ; echo "exit code = $?"
  (phá thử) đổi chuỗi mong đợi của một test -> make test
  for i in 1 2 3 4 5 6 7 8 9 10; do make test || echo "LAN $i FAIL"; done
  pgrep qemu-system-riscv64     # không được còn tiến trình mồ côi

Kết quả:
- make test: PASS T01..T04, [4/4 passed], exit code 0.
- Phá test cố ý: in FAIL và exit code khác 0 [xác nhận].
- Lặp 10 lần: [10/10 qua] không chập chờn.
- Không còn QEMU mồ côi.
- Phụ thuộc: T04 cần user/ticks.c (B4). T05 (CPUS=2) thêm sau khi
  PR của C merge.
