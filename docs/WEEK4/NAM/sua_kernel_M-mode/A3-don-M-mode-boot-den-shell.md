W4-A3: clean up leftover M-mode code, boot to shell

Đã làm:
- Xóa timervec trong kernelvec.S và timer_scratch trong start.c
  (kiểu CLINT: chỉ xóa sau khi B2 xong).
- Xóa hằng CLINT không còn dùng trong memlayout.h.
- Giữ các hàm r_m*/w_m* trong riscv.h, xác nhận không còn nơi gọi
  (xóa hẳn ở tuần 5).

Lệnh kiểm tra:
  grep -rn "timervec\|timer_scratch\|CLINT" kernel/
  make clean && make qemu CPUS=1
  (trong xv6) ls ; cat README ; usertests -q
  make fmt && git diff --exit-code

Kết quả:
- grep: không còn tham chiếu (docs/WEEK4/NAM/logs)
- Build sạch, boot đến "$", ls/cat chạy đúng.
- usertests -q: bị treo, không thể tiếp tục 
  thời gian do chưa có timer, không phải lỗi file/pipe.
