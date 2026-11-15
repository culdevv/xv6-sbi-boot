# T4-A4: Khung `make test` và 4 test đầu

| Mục | Nội dung |
|---|---|
| Người thực hiện | Nguyễn Hải Nam (A) |
| Task Jira | T4-A4 (ước lượng 4 giờ) |
| Nhánh | `feat/W4-A4-test-harness` |
| Tệp chính | `tests/run_tests.py`, mục `test:` trong `Makefile` |
| Yêu cầu đề | Mỗi test tự chạy, tự kiểm kết quả, in PASS/FAIL; cả bộ chạy bằng một lệnh `make test`, thoát khác 0 nếu có FAIL |

---

## 1. Mục tiêu

Xây khung kiểm thử tự động để nhóm không phải gõ lệnh vào shell xv6 bằng tay. Các test về sau (timer, console, hart, ...) chỉ cần thêm hàm vào khung này.

## 2. Thiết kế

`tests/run_tests.py` điều khiển QEMU như một tiến trình con:

1. Mở QEMU bằng `make -s qemu CPUS=N`, nối stdin/stdout qua pipe.
2. Một luồng phụ đọc đầu ra QEMU đưa vào hàng đợi.
3. `expect(mẫu, timeout)` chờ đến khi thấy chuỗi khớp regex (trả về match) hoặc hết giờ (trả về `None`). Phần đã khớp được cắt khỏi bộ đệm để các `expect` sau không đọc lại.
4. `send(chuỗi)` ghi vào stdin QEMU, tương đương gõ vào shell xv6.
5. `close()` giết **cả nhóm tiến trình** QEMU (`start_new_session=True` + `os.killpg`). Bắt buộc, nếu không QEMU chạy mồ côi sau khi script kết thúc.
6. `check(tên, điều_kiện)` in `PASS: ...` hoặc `FAIL: ...` và ghi lại kết quả.
7. Cuối cùng in `x/y passed` và `sys.exit(0)` nếu tất cả pass, ngược lại `sys.exit(1)`.

Mục trong `Makefile` (dòng lệnh bắt đầu bằng ký tự Tab):

```make
test: $K/kernel fs.img
	python3 tests/run_tests.py
```

## 3. Bốn test đầu (phạm vi Tuần 4)

| ID | Cấu hình | Kiểm tra | Timeout |
|---|---|---|---|
| T01 | CPUS=1 | Thấy `xv6 kernel is booting` | 30 s |
| T02 | CPUS=1 | Thấy dấu nhắc `$ ` | 30 s |
| T03 | CPUS=1 | Gõ `ls`, thấy `README` | 10 s |
| T04 | CPUS=1 | Gõ `ticks`, thấy `TICKS a b` với `20 <= b - a <= 22` | 20 s |

T05 (CPUS=2 thấy `hart 1 starting`) chỉ thêm sau khi PR của Thanh (hart phụ) được merge.

T04 phụ thuộc hai thứ ngoài phạm vi Nam: chương trình `user/ticks.c` (T4-B4) và timer qua SBI (T4-B2). Chương trình `ticks` gọi `sleep(20)` rồi in `TICKS a b`; nếu timer không chạy thì `sleep` không bao giờ trở về và test báo FAIL do hết giờ.

## 4. Kết quả chạy

Chụp lúc 21:30 ngày 15/11:

```
$ make test
python3 tests/run_tests.py
PASS: T01 boot banner (CPUS=1)
PASS: T02 shell prompt (CPUS=1)
PASS: T03 ls shows README (CPUS=1)
(đang chờ T04)
```

- T01 đến T03 PASS.
- T04 đang chờ `TICKS a b` tối đa 20 giây rồi sẽ FAIL. Nguyên nhân quan sát được: trên nhánh này gõ `ticks` cho `exec ticks failed` và danh sách `ls` không có `ticks`, tức `user/ticks.c` chưa được merge vào nhánh (việc của T4-B4).

Kết quả kỳ vọng sau khi B2 và B4 được merge và nhánh được cập nhật (`git pull origin main`, `make clean`):

```
PASS: T01 boot banner (CPUS=1)
PASS: T02 shell prompt (CPUS=1)
PASS: T03 ls shows README (CPUS=1)
PASS: T04 timer ticks advance (CPUS=1)
4/4 passed
```
với exit code 0. 

## 5. Phát hiện và cải tiến cần làm

1. **`fs.img` không được tạo lại trước mỗi lần chạy.** `ls` trong ảnh chụp còn các file rác từ lần chạy `usertests` trước (`stopforking`, `x`, `bigarg-ok`, `junk`). Kết quả test có thể phụ thuộc trạng thái cũ. Đề xuất:
   ```make
   test: $K/kernel
   	rm -f fs.img
   	$(MAKE) fs.img
   	python3 tests/run_tests.py
   ```
2. **Dòng debug** `[USERINIT] pid=1 state=3` còn xuất hiện trong output boot, cần xóa trước khi mở PR.
3. **Tên thư mục:** script nằm ở `tests/` (số nhiều), tài liệu kế hoạch ghi `test/`. Giữ `tests/` và dùng nhất quán trong Makefile, tài liệu, CI.
4. **Kiểm tra cố ý phá test:** đổi một chuỗi mong đợi, xác nhận script in `FAIL` và `echo $?` khác 0.
5. **Độ ổn định:** chạy `for i in $(seq 10); do make test || echo "LAN $i FAIL"; done`, cả 10 lần phải qua, test chập chờn là dấu hiệu race và cần ghi bug.
6. **Dọn tiến trình** sau khi ngắt script bằng Ctrl-C: `pkill qemu-system-riscv64`.

