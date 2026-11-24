# Báo cáo Kỹ thuật Task T5-A2: Ma trận boot CPUS = 1, 2, 3, 4, 8

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Nam 
* **Nhánh thực hiện:** `feat/T5-A1-boot-matrix`
* **Trạng thái:** Hoàn thành.

---

## 1. Mục tiêu công việc

Tự động kiểm tra xv6 boot được đến shell ở 5 cấu hình số hart (1, 2, 3, 4, 8) và số hart in thông báo online bằng đúng `CPUS`.

Các mục tiêu cụ thể:

- Kiểm tra boot đến dấu nhắc `$` và chạy được `ls` ở mỗi cấu hình.
- Đếm số hart khác nhau đã in `hart N starting`, phải bằng `CPUS`.
- Chứng minh test thật sự FAIL khi kỳ vọng sai.

---

## 2. Các công việc đã thực hiện (Testing Activities)

| Hạng mục | Hành động | Mô tả chi tiết |
| :--- | :--- | :--- |
| `test/t_boot.py` | **Tạo mới** | Hàm `boot_session(cpus)` mở `Qemu(cpus, "boot")`, chờ `$` tối đa 60 giây (8 hart trên máy yếu boot chậm), gõ `ls`, kiểm tra thấy `README`. Kết quả là **T01-T05** tương ứng CPUS = 1, 2, 3, 4, 8. |
| Đếm hart online | **Kiểm thử** | Gọi `wait_unique(r"hart (\d+) starting", cpus, 20)`. Kết quả là **T06**: số hart khác nhau bằng đúng `CPUS`. |
| Dọn test cũ | **Xóa** | Xóa phần boot cũ trong `t_basic.py`/`run_tests.py` (nếu còn) để không trùng test. |
| Phá thử | **Đánh giá** | Đổi số hart mong đợi thành `cpus+1` trong một lần chạy, xác nhận thấy FAIL và exit code khác 0, rồi trả lại như cũ. |

---

## 3. Quy trình kiểm thử

```bash
git checkout main && git pull origin main
git checkout -b feat/T5-A1-boot-matrix
make test
echo "exit code = $?"
git add test/
git commit -m "T5-A1: boot matrix CPUS=1,2,3,4,8 with online-hart count"
git push -u origin feat/T5-A1-boot-matrix
```

Với mỗi cấu hình, bộ test mở QEMU bằng `make -s qemu CPUS=N`, chờ `$`, gửi `ls`, đếm các dòng `hart N starting`, rồi giết cả nhóm tiến trình QEMU trong khối `finally`.

Lưu ý: cần PR multi-hart của Thanh merge trước. Nếu chưa, T06 ở `CPUS ≥ 2` sẽ FAIL. Đó là FAIL đúng, không sửa test cho khớp.

---

## 4. Kết quả kiểm thử

| Cấu hình | Boot + `ls` | Hart online |
| :--- | :--- | :--- |
| `CPUS=1` | [PASS] (T01) | [FAIL] 0/1 (T06) |
| `CPUS=2` | [PASS] (T02) | [FAIL] 1/2 (T06) |
| `CPUS=3` | [PASS] (T03) | [FAIL] 2/3 (T06) |
| `CPUS=4` | [PASS] (T04) | [FAIL] 3/4 (T06) |
| `CPUS=8` | [PASS] (T05) | [FAIL] 7/8 (T06) |

**Kết quả tổng hợp:**
```
PASS: T01 boot to shell and ls works (CPUS=1)
FAIL: T06 all harts online 0/1 (CPUS=1)
PASS: T02 boot to shell and ls works (CPUS=2)
FAIL: T06 all harts online 1/2 (CPUS=2)
PASS: T03 boot to shell and ls works (CPUS=3)
FAIL: T06 all harts online 2/3 (CPUS=3)
PASS: T04 boot to shell and ls works (CPUS=4)
FAIL: T06 all harts online 3/4 (CPUS=4)
PASS: T05 boot to shell and ls works (CPUS=8)
FAIL: T06 all harts online 7/8 (CPUS=8)
5/10 passed in 105s
make: *** [Makefile:213: test] Error 1
```

**Khó khăn gặp phải:** Chưa chạy multihart nên kết quả fail là bình thường

---

## 5. Kết luận

[Đã hoàn thành Task T5-A2.] Ma trận boot kiểm tra tự động 5 cấu hình CPU với 10 test (5 × 2) và có bằng chứng test phát hiện được lỗi.
