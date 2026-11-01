# RISC-V TIME Counter và lệnh `rdtime` trong xv6-riscv
** Truong Dinh Thanh 20233642

## 1. Mục tiêu

Tìm hiểu cơ chế đọc bộ đếm thời gian phần cứng TIME trên kiến trúc RISC-V, cách xv6 sử dụng bộ đếm này để tạo ngắt định thời và cách cho phép chương trình ở User Mode đọc TIME bằng lệnh `rdtime`.

## 2. Tần số bộ đếm thời gian trong Device Tree

Trong Device Tree của QEMU, thông số:

```dts
timebase-frequency = <0x989680>;
```

Giá trị `0x989680` ở hệ thập lục phân tương đương `10.000.000` ở hệ thập phân.

Do đó:

- Tần số TIME: 10 MHz.
- Một tick tương ứng 100 ns.
- 10.000.000 tick tương ứng khoảng 1 giây.

Công thức tính thời gian:

\[
t = \frac{rdtime_{end}-rdtime_{start}}{10.000.000}
\]

## 3. Cách xv6 đọc bộ đếm TIME

Trong `kernel/riscv.h`, xv6 định nghĩa hàm `r_time()`:

```c
static inline uint64
r_time()
{
  uint64 x;
  asm volatile("csrr %0, time" : "=r"(x));
  return x;
}
```

Lệnh `csrr` đọc giá trị CSR `time` của RISC-V. Đây là cách đọc tương đương với pseudo-instruction `rdtime`.

Cần phân biệt `time` với `cycle`: `time` phản ánh bộ đếm thời gian, còn `cycle` phản ánh số chu kỳ bộ xử lý.

## 4. Khởi tạo bộ định thời trong xv6

Trong `kernel/start.c`, hàm `timerinit()` thực hiện:

- Bật quyền sử dụng bộ so sánh thời gian `stimecmp` thông qua SSTC.
- Cho phép Supervisor Mode truy cập bộ đếm `time` bằng bit tương ứng trong `mcounteren`.
- Đặt thời điểm ngắt đầu tiên bằng giá trị TIME hiện tại cộng 1.000.000 tick.

Với timebase 10 MHz, khoảng 1.000.000 tick tương ứng 0,1 giây.

## 5. Xử lý ngắt định thời

Trong `kernel/trap.c`, hàm `clockintr()`:

- Tăng biến `ticks` khi ngắt xảy ra trên CPU 0.
- Gọi `wakeup(&ticks)` để đánh thức các tiến trình đang chờ.
- Cập nhật `stimecmp` bằng thời điểm hiện tại cộng 1.000.000 tick để thiết lập lần ngắt tiếp theo.

Biến `ticks` của xv6 là bộ đếm phần mềm, khác với bộ đếm TIME phần cứng.

## 6. Cho phép User Mode đọc TIME

Để chương trình người dùng thực hiện `rdtime`, cần cho phép truy cập bộ đếm tương ứng thông qua thanh ghi `scounteren`.

Trong `kernel/riscv.h`, bổ sung hàm đọc và ghi `scounteren`:

```c
static inline void
w_scounteren(uint64 x)
{
  asm volatile("csrw scounteren, %0" : : "r"(x));
}

static inline uint64
r_scounteren()
{
  uint64 x;
  asm volatile("csrr %0, scounteren" : "=r"(x));
  return x;
}
```

Trong hàm `prepare_return()` của `kernel/trap.c`, cho phép User Mode đọc TIME:

```c
w_scounteren(r_scounteren() | 2);
```

Bit 1 được bật để cho phép truy cập bộ đếm `time` từ User Mode.

## 7. Chương trình kiểm thử

Tạo chương trình `user/rdtime_test.c` để:

1. Đọc giá trị TIME ban đầu.
2. Chờ bộ đếm tăng thêm 10.000.000 tick.
3. Đọc giá trị TIME kết thúc.
4. Tính số tick đã tăng và thời gian tương ứng.
5. Kiểm tra kết quả.

Chương trình được thêm vào danh sách `UPROGS` trong Makefile để xv6 biên dịch và đưa vào filesystem image.

## 8. Kết quả kiểm thử

Chạy xv6 trên QEMU và thực thi chương trình `rdtime_test`.

Kết quả thu được:

```text
=== RISC-V rdtime test ===
Expected timebase frequency: 10000000 Hz
Start rdtime: 83410960
End rdtime:   93411195
Elapsed ticks: 10000235
Measured time: 1.0 seconds
Expected ticks for 1 second: 10000000
RESULT: PASS - time counter increased as expected.
```

Số tick thực tế tăng thêm là 10.000.235, chênh lệch 235 tick so với giá trị kỳ vọng 10.000.000 tick, tương đương khoảng 0,00235%.

Kết quả cho thấy bộ đếm TIME tăng đúng theo tần số timebase đã cấu hình và chương trình ở User Mode có thể đọc được giá trị này.
### 8.1. Ảnh minh chứng

Kết quả thực thi chương trình `rdtime_test` trên QEMU:

![Kết quả kiểm thử rdtime](evidence/rdtime-test.png)

## 9. Kết luận

Qua quá trình tìm hiểu và kiểm thử, đã xác định được mối liên hệ giữa Device Tree, bộ đếm TIME của RISC-V và cơ chế ngắt định thời trong xv6.

Việc bổ sung quyền truy cập `scounteren` cho phép chương trình User Mode sử dụng lệnh `rdtime` để đo khoảng thời gian dựa trên bộ đếm phần cứng.

Kết quả kiểm thử trên QEMU đạt PASS, xác nhận cơ chế đọc TIME hoạt động theo cấu hình timebase 10 MHz.
