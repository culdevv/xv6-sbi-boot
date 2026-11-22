# T5-C2 - Kiểm tra Multi-hart với CPUS=1, 2, 3, 4 và 8

## 1. Mục tiêu

- Kiểm tra khả năng khởi động nhiều hart của xv6.
- Mỗi hart phải in thông báo `hart N starting`.
- Số hart online phải bằng số CPU được cấu hình trong QEMU.
- Kiểm tra shell và các lệnh `ls`, `ticks` ở những cấu hình CPUS=1, 2, 3, 4 và 8.
- Kiểm tra giả định boot hart luôn là hart 0.

## 2. Môi trường

- QEMU virt.
- OpenSBI.
- Kiến trúc RISC-V 64-bit.
- Số hart tối đa: NCPU=8.
- Nhánh Git: feat/T5-C2-hart-online.

## 3. Thay đổi mã nguồn

File thay đổi: `kernel/main.c`.

Sau khi hoàn thành khởi tạo chung và trước khi gọi `start_secondary_harts()`, bổ sung:

    printk("hart %d starting\n", cpuid());

Mục đích: boot hart cũng in thông báo online giống các hart phụ.

Giữ nguyên cơ chế atomic election hiện có:

- `boot_hart_done`: bảo đảm chỉ một hart thực hiện khởi tạo chung.
- `started`: đồng bộ để các hart còn lại chỉ tiếp tục sau khi khởi tạo chung hoàn tất.

Không giả định boot hart có ID bằng 0.

## 4. Khảo sát QEMU 20 giây

Thực hiện khảo sát với từng cấu hình CPUS=1, 2, 3, 4 và 8. Mỗi phiên chạy được giới hạn 20 giây bằng timeout.

| Cấu hình | Boot hart quan sát được | Số hart online | Kết quả |
|---|---:|---:|---|
| CPUS=1 | 0 | 1/1 | PASS |
| CPUS=2 | 0 | 2/2 | PASS |
| CPUS=3 | 0 | 3/3 | PASS |
| CPUS=4 | 1 | 4/4 | PASS |
| CPUS=8 | 3 | 8/8 | PASS |

Không phát hiện thông báo kernel panic trong các log khảo sát.

Các log khảo sát:

- `survey-cpus1-20s.log`
- `survey-cpus2-20s.log`
- `survey-cpus3-20s.log`
- `survey-cpus4-20s.log`
- `survey-cpus8-20s.log`

## 5. Kiểm thử tương tác

Sau khi bổ sung thông báo online cho boot hart, chạy QEMU và kiểm tra shell bằng `ls`, `ticks`.

| Cấu hình | Số hart online | ls | ticks |
|---|---:|---|---|
| CPUS=1 | 1/1 | PASS | PASS |
| CPUS=2 | 2/2 | PASS | PASS |
| CPUS=3 | 3/3 | PASS | PASS |
| CPUS=4 | 4/4 | PASS | PASS |
| CPUS=8 | 8/8 | PASS | PASS |

Kết quả lệnh `ticks` quan sát được:

- CPUS=1: `TICKS 103 123`
- CPUS=2: `TICKS 50 70`
- CPUS=3: `TICKS 47 67`
- CPUS=4: `TICKS 37 57`
- CPUS=8: `TICKS 45 65`

Các log kiểm thử tương tác:

- `cpus1-final.log`
- `cpus2-final.log`
- `cpus3-final.log`
- `cpus4-final.log`
- `cpus8-final.log`

## 6. Kiểm tra giả định boot hart

Tài liệu thực nghiệm HSM trước đây tại `docs/WEEK3/THANH/hsm-experiment.md` ghi nhận boot hart 0 trong các thử nghiệm SMP=1, 2 và 8.

Tuy nhiên, khảo sát T5-C2 ghi nhận:

- CPUS=4: boot hart 1.
- CPUS=8: boot hart 3.

Các lần kiểm thử tương tác cũng quan sát được boot hart 1 và 2 ở một số lần chạy CPUS=3 và CPUS=4.

Do đó, giả định boot hart luôn là hart 0 không đúng trong tất cả các lần chạy.

Giải pháp: giữ cơ chế atomic election bằng `__sync_lock_test_and_set()` để hart đầu tiên giành quyền thực hiện khởi tạo chung. Các hart còn lại chờ `started` được công bố rồi thực hiện khởi tạo riêng và vào scheduler.

## 7. Nhận xét

- Số hart online bằng đúng số CPU được cấu hình trong cả năm trường hợp.
- Thứ tự các hart in thông báo online không cố định.
- Các hart không tồn tại trong cấu hình QEMU có thể trả về `SBI_ERR_INVALID_PARAM (-3)` khi gọi `sbi_hart_get_status()`. Đây là kết quả dự kiến.
- Các lệnh `ls` và `ticks` hoạt động ở cả năm cấu hình.
- Không quan sát thấy kernel panic trong các lần kiểm thử.

## 8. Kết luận

Hoàn thành kiểm thử multi-hart với CPUS=1, 2, 3, 4 và 8.

Tất cả các hart được cấu hình đều in thông báo online. Shell hoạt động và các lệnh `ls`, `ticks` chạy thành công.

Cơ chế atomic election được giữ lại để không phụ thuộc vào ID của boot hart.
