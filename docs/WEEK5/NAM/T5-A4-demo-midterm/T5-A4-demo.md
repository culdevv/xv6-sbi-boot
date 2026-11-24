# T5-A6: Kịch bản demo và diễn tập midterm

* **Dự án:** `xv6-sbi-boot` (Đề tài 1 — Booting xv6 in S-mode on OpenSBI)
* **Giai đoạn:** Tuần 5 (Midterm Checkpoint)
* **Người thực hiện:** Nam
* **Nhánh/Tag thực hiện:** `main`, tag `midterm`
* **Trạng thái:** Chạy thử kịch bản trên `CPUS=8` đạt cả 5 bước; phần đo thời gian 3 lần diễn tập và gắn tag `midterm` thực hiện sau khi `make test` xanh trên `main`.

---

## 1. Mục tiêu công việc

Đảm bảo demo midterm 10 phút diễn ra trơn tru (boot 8 hart, xem tần số timer, `halt`), repo build sạch từ máy khác và có mốc `midterm` để quay lại.

Các mục tiêu cụ thể:

- Chạy thử toàn bộ kịch bản demo trên cấu hình `CPUS=8` và xác nhận từng bước.
- Điều hành diễn tập 3 lần, mỗi lần không quá 10 phút.
- Chuẩn bị phương án dự phòng (video, máy thứ hai).
- Kiểm tra clone sạch → build → `make test`, rồi gắn tag `midterm`.

---

## 2. Các công việc đã thực hiện (Activities)

| Hạng mục | Hành động | Mô tả chi tiết |
| :--- | :--- | :--- |
| Kịch bản demo | **Soạn và điều hành** | 0:00-1:00 mục tiêu và sơ đồ 3 lớp (C); 1:00-2:30 sơ đồ boot (A); 2:30-4:30 live boot 8 hart (A); 4:30-6:30 live `ls`, `ticks`, `tickrate` (B); 6:30-7:30 live `spin &` rồi `halt` (B); 7:30-8:30 kết quả test, CI, Jira (C); 8:30-10:00 Q&A (cả nhóm). |
| Chạy thử kịch bản | **Kiểm thử** | Chạy `make qemu CPUS=8`, lần lượt gõ `ls`, `ticks`, `tickrate`, `spin &`, `halt`. Kết quả ghi ở mục 4. |
| Diễn tập | **Thực hiện** | Chạy kịch bản 3 lần có bấm giờ, luân phiên người nói; hai người còn lại đóng vai giảng viên hỏi 3 câu bất kỳ; ghi chỗ ngắc ngứ để sửa kịch bản. |
| Dự phòng | **Chuẩn bị** | Video 2 phút chạy kịch bản; máy thứ hai đã clone sẵn tag `midterm` và build được. |
| Repo sạch | **Kiểm tra** | Clone vào `/tmp/mid-check`, build và chạy `make test` để tránh lỗi "chạy trên máy tôi". |
| Tag `midterm` | **Gắn mốc** | Chỉ gắn khi mọi PR đã merge, CI xanh và kiểm tra repo sạch đạt. Lưu log vào `test-results/midterm/`. |

---

## 3. Quy trình thực hiện

**Lệnh demo:**
```bash
git checkout midterm
make clean && make qemu CPUS=8
```
Sau khi boot, gõ lần lượt: `ls`, `ticks`, `tickrate`, `spin &`, `halt`.

**Kiểm tra repo sạch:**
```bash
git clone <url-repo> /tmp/mid-check
cd /tmp/mid-check
make kernel/kernel fs.img
make test
```

**Gắn tag:**
```bash
git checkout main && git pull origin main
make clean && make test
mkdir -p test-results/midterm && cp test-results/*.log test-results/midterm/
git add test-results/midterm && git commit -m "Midterm test logs" && git push origin main
git tag -a midterm -m "Midterm checkpoint: 1-8 harts, halt, tests in CI"
git push origin midterm
git rev-parse --short midterm
```

Sau khi gắn tag không sửa gì. Nếu phải sửa lỗi thì tạo tag mới `midterm-fix1`, giữ nguyên `midterm`.

---

## 4. Kết quả

**Kết quả chạy thử kịch bản trên `CPUS=8`:**

| Bước demo | Kết quả quan sát | Đánh giá |
| :--- | :--- | :--- |
| Boot 8 hart | Cả 8 hart (0 đến 7) in `hart N starting`; 7 hart phụ có `start request succeeded`; sau đó `init: starting sh` | **PASS** |
| `ls` | Liệt kê đủ `README` và các chương trình người dùng, có `ticks`, `halt`, `tickrate`, `printtest`, `spin` | **PASS** |
| `ticks` | `TICKS 125 145`, hiệu bằng 20 tick | **PASS** |
| `tickrate` | `TICKRATE_MILLIHZ 9948 ticks=30`, tức 9.948 Hz so với 10 Hz danh định (lệch dưới 0.6%) | **PASS** |
| `spin &` rồi `halt` | Shell vẫn nhận lệnh khi `spin` chạy nền; in `halting system` và QEMU thoát về terminal | **PASS** |

**Kết quả tổng hợp:**
```text
boot hart 6 initialized
hart 0..7 starting (8/8)
TICKS 125 145
TICKRATE_MILLIHZ 9948 ticks=30
halting system
```

**Nhận xét quan trọng:** hart khởi động trong lần chạy này là **hart 6** (`boot hart 6 initialized`), trong khi các lần chạy khác là hart 0. OpenSBI chọn hart boot khác nhau giữa các lần chạy, kernel xử lý đúng nên đây không phải lỗi. Khi trình bày không nói "hart 0 chạy trước", chỉ vào 8 dòng `hart N starting` và nói thứ tự thay đổi mỗi lần chạy.

**Các hạng mục hoàn tất sau khi `make test` xanh trên `main`:**

| Hạng mục | Kết quả | Ghi chú |
| :--- | :--- | :--- |
| Diễn tập lần 1 | 1 phút |
| Diễn tập lần 2 | 1 phút |
| Diễn tập lần 3 | 1 phút |
| Clone sạch, build, `make test` | PASS | 18/18 passed |
| Video và máy dự phòng | Có | [link video] |
| Tag `midterm` | Đã đẩy |

---

## 5. Khó khăn và xử lý

- **Hart boot thay đổi giữa các lần chạy** (hart 0 hoặc hart 6): kịch bản demo và câu trả lời Q&A được viết theo hướng không giả định hart boot là hart 0 ("OpenSBI chọn một hart làm hart boot, các hart còn lại được đánh thức bằng `hart_start`").
- **`make test` từng FAIL ở T04 và T11** do regex đọc dòng `TICKS a b` bị cắt cụt số khi serial chưa nhận đủ (ví dụ `82 1` thay vì `82 102`). Kernel đúng (T12 PASS, 9.95 Hz). Đã sửa bằng cách chờ đến hết dòng (`\r?\n`) trong PR riêng; tag `midterm` chỉ gắn sau khi PR này merge và `make test` xanh.

---

## 6. Kết luận

Kịch bản demo midterm chạy đúng cả 5 bước trên `CPUS=8`: boot đủ 8 hart, `ticks` và `tickrate` cho kết quả đúng (9.948 Hz), `spin &` rồi `halt` tắt máy sạch. Phần còn lại (diễn tập có bấm giờ, video dự phòng, gắn tag `midterm`) được điền vào bảng ở mục 4 khi hoàn tất, trước buổi demo.
