# Quy ước làm việc nhóm (xv6-sbi-boot)

## 1. Quy định về Nhánh (Git Branching)
- Nhánh `main` luôn trong trạng thái build được. **Không bao giờ push trực tiếp lên `main`**.
- Mỗi task khởi tạo một nhánh riêng từ `main` mới nhất. Quy chuẩn tên nhánh:
  - `feat/SBI-xx-ten-tinh-nang`  (Phát triển tính năng kernel)
  - `exp/SBI-xx-ten-thuc-nghiem`   (Thử nghiệm mã nguồn)
  - `docs/SBI-xx-ten-tai-lieu`    (Báo cáo, tài liệu)
  - `fix/SBI-xx-ten-loi`         (Sửa lỗi)
- Hoàn thành và merge xong PR thì xóa nhánh phụ trên GitHub và local.

## 2. Quy định Commit & Pull Request (PR)
- **Commit:** Cấu trúc `SBI-xx: Mô tả ngắn gọn công việc`. Mỗi commit thực hiện một việc nhỏ (atomic commit).
- **Pull Request:** 
  - Gắn mã task Jira/Trello (`SBI-xx`) vào tiêu đề/mô tả PR.
  - Bắt buộc phải có **ít nhất 1 thành viên khác review và Approve** trước khi merge vào `main`.

## 3. Cấu trúc Thư mục Đóng góp Cá nhân (Bắt buộc)
Mọi thành viên khi push tài liệu, dữ liệu thực nghiệm hoặc ảnh minh chứng làm việc lên kho lưu trữ phải tuân thủ đúng cấu trúc phân chia theo **Tuần** và **Người thực hiện**:

### A. Thư mục Tài liệu & Dữ liệu (`docs/`)
Chứa các báo cáo, tài liệu phân tích, cơ sở dữ liệu hoặc kết quả output in ra:
```text
docs/
├── tuan-1/
│   ├── Luc/      (Ví dụ: baseline-Luc.md, log_cpus_1.txt)
│   ├── Nam/      (Ví dụ: baseline-Nam.md, database_log.json)
│   └── Thanh/    (Ví dụ: baseline-Thanh.md, analysis.pdf)
├── tuan-2/
│   ├── Luc/      (Ví dụ: dtb.md, virt.dts)
│   ├── Nam/      (Ví dụ: sbi_spec_notes.md)
│   └── Thanh/    (Ví dụ: test_results_output.log)
└── tuan-x/ ...
### B. Thư mục Thử nghiệm Mã nguồn (experiments/)
Chứa toàn bộ các mã nguồn thử nghiệm, đoạn code mẫu, bài test đơn lẻ:

Plaintext
experiments/
├── tuan-1/
│   ├── Luc/      (Các thử nghiệm cá nhân Tuần 1)
│   ├── Nam/
│   └── Thanh/
├── tuan-2/
│   ├── Luc/      (Ví dụ: exp_fwjump_reserved/)
│   ├── Nam/      (Ví dụ: exp_timer_ecall/)
│   └── Thanh/    (Ví dụ: exp_putchar_legacy/)
└── tuan-x/ ...
### C. Thư mực kết quả test sửa đổi kernel (tests/)
Plaintext
tests/
├── tuan-1/
│   ├── Luc/      (Các test cá nhân Tuần 1)
│   ├── Nam/
│   └── Thanh/
├── tuan-2/
│   ├── Luc/      (Ví dụ: feat_fwjump_reserved/)
│   ├── Nam/      (Ví dụ: feat_timer_ecall/)
│   └── Thanh/    (Ví dụ: feat_putchar_legacy/)
└── tuan-x/ ...

#### 4. Các tệp/thư mục Không Commit (.gitignore)
Tuyệt đối không push các tệp nhị phân tự sinh, tệp cấu hình trình soạn thảo hoặc bí mật cá nhân:

Tệp biên dịch nhị phân và đối tượng: *.o, *.d, kernel/kernel, fs.img, mkfs/mkfs, *.elf, *.asm, *.sym.

Tệp khóa/mật khẩu: Token personal, SSH keys, passwords.

Thư mục công cụ soạn thảo: .vscode/, .idea/, .DS_Store, *.swp.


---
