# Quy ước làm việc nhóm

## Nhánh
- `main` luôn build được. Không push trực tiếp lên `main`.
- Mỗi task một nhánh, tạo từ `main` mới nhất. Tên nhánh theo dạng:
  - `feat/SBI-17-sbi-ecall`  (tính năng kernel)
  - `exp/SBI-9-hello-smode`  (thực nghiệm)
  - `docs/SBI-13-proposal`   (tài liệu)
  - `fix/SBI-30-ten-loi`     (sửa lỗi)
- Merge xong thì xóa nhánh.

## Commit
- Dạng `SBI-17: thêm sbi_ecall`. Mỗi commit làm một việc.

## Pull Request
- Mỗi PR gắn với một task (SBI-xx), mô tả đã làm gì và kiểm tra thế nào.
- Cần 1 người khác review và approve trước khi merge.

## Không commit
- File build (*.o, kernel/kernel, fs.img...), mật khẩu, token.