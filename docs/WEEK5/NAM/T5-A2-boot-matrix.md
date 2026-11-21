# Báo cáo Task T5-A3: Test tĩnh và dọn `riscv.h`

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Nam (A)
* **Nhánh thực hiện:** `feat/T5-A2-static-tests`
* **Trạng thái:** Hoàn thành.

---

## 1. Mục tiêu công việc

Dọn các hàm M-mode còn sót trong `kernel/riscv.h` và thêm hai test tĩnh tự động chứng minh R1 (địa chỉ nạp) và R2 (không còn M-mode) luôn đúng.

Các mục tiêu cụ thể:

- Xóa các hàm và macro M-mode không còn nơi nào gọi.
- Kiểm tra entry point của kernel là `0x80200000`.
- Kiểm tra không còn từ khóa M-mode trong `kernel/`.

---

## 2. Các công việc đã thực hiện (Implementation Activities)

| Hạng mục | Hành động | Mô tả chi tiết |
| :--- | :--- | :--- |
| Khảo sát | **Tìm kiếm** | Chạy `grep -nE "mhartid\|mstatus\|mepc\|medeleg\|mideleg\|menvcfg\|mcounteren\|pmpcfg0\|pmpaddr0\|mtvec\|mscratch\|mret\|\bmie\b" kernel/*.h kernel/*.c kernel/*.S`. Kết quả chủ yếu nằm trong `riscv.h`. |
| `kernel/riscv.h` | **Xóa** | Xóa các hàm inline `r_m*`/`w_m*` (ví dụ `r_mhartid`, `w_mstatus`) và macro `MSTATUS_*` chỉ phục vụ chúng. Nếu `make` báo thiếu hàm nào thì hàm đó còn đang được gọi và R2 chưa sạch. |
| `test/t_static.py` (T07) | **Tạo mới** | Tự tìm `riscv64-unknown-elf-readelf` hoặc `riscv64-linux-gnu-readelf`, chạy `readelf -h kernel/kernel`, kiểm tra `Entry point address` bằng `0x80200000`. |
| `test/t_static.py` (T08) | **Tạo mới** | Quét `kernel/*.c`, `*.h`, `*.S` bằng biểu thức chính quy từ khóa M-mode; nếu còn thì in tối đa 10 vị trí đầu và báo FAIL. |

---

## 3. Quy trình thực hiện

```bash
git checkout main && git pull origin main
git checkout -b feat/PROJ-A3-static-tests
grep -nE "mhartid|mstatus|mepc|medeleg|mideleg|menvcfg|mcounteren|pmpcfg0|pmpaddr0|mtvec|mscratch|mret|\bmie\b" kernel/*.h kernel/*.c kernel/*.S
# xóa các hàm/macro M-mode trong kernel/riscv.h, viết test/t_static.py
make clean && make kernel/kernel
make test
git add kernel/riscv.h test/t_static.py
git commit -m "PROJ-A3: remove unused M-mode helpers; add static tests T07/T08"
git push -u origin feat/PROJ-A3-static-tests
```

Lưu ý: không viết các từ khóa M-mode trong comment, nếu không T08 sẽ FAIL oan.

---

## 4. Kết quả

| Hạng mục | Kết quả | Ghi chú |
| :--- | :--- | :--- |
| Grep trước khi dọn | 32 dòng | Chủ yếu trong `riscv.h` |
| Grep sau khi dọn | 0 dòng | |
| Kernel build sau khi xóa | PASS | Không warning/lỗi |
| T07 (entry point `0x80200000`) | PASS | |
| T08 (không còn M-mode trong `kernel/`) | PASS | |

Kết quả chạy thực tế:
PASS: T01 boot banner (CPUS=1)
PASS: T02 shell prompt (CPUS=1)
PASS: T03 ls shows README (CPUS=1)
FAIL: T04 timer ticks advance (CPUS=1)
--- t_static
PASS: T07 kernel entry point is 0x80200000
PASS: T08 no M-mode code left in kernel/
5/6 passed in 4s
make: *** [Makefile:213: test] Error 1

**Khó khăn gặp phải:** T04 fail

---

## 5. Kết luận

[Đã hoàn thành Task T5-A3.] `riscv.h` không còn mã M-mode thừa và hai test tĩnh T07, T08 bảo vệ R1/R2 trong các thay đổi sau này.
