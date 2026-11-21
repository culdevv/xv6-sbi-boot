# Phác thảo các hàm gọi OpenSBI cho kernel
# Từ yêu cầu tới dịch vụ 

Yêu cầu	Kernel cần làm gì	Phải nhờ OpenSBI?	Nhóm dịch vụ SBI cần
R3 Đồng hồ	Hẹn "ngắt tôi sau X nhịp nữa" để chia thời gian giữa các tiến trình	Có (không tự ghi được thanh ghi hẹn giờ quyền cao)	TIME
R4 Console	In chữ ra màn hình, nhận chữ gõ vào	In: có (qua SBI). Nhập: tùy quyết định ở bước 5	Console (legacy hoặc DBCN)
R5 Bật CPU phụ	Lúc đầu chỉ 1 CPU chạy, nó phải đánh thức các CPU còn lại	Có	HSM
R6 Tắt máy	Lệnh halt làm QEMU thoát	Có	SRST (hoặc legacy EID 8)
Kiểm tra môi trường	Biết OpenSBI có hỗ trợ dịch vụ nào, bản nào	Có (hỏi OpenSBI)	Base
R1, R2, R7	Đổi địa chỉ nạp, bỏ code quyền cao, nhiều CPU chạy đúng	Không gọi hàm SBI riêng	(không có)

Kết luận: cần 5 nhóm: Base, TIME, Console, HSM, SRST. Hai nhóm khác của SBI là IPI (gửi ngắt giữa các CPU) và RFENCE (xóa bộ nhớ đệm địa chỉ từ xa) không nằm trong đề tài; xv6 gốc không dùng chúng.


Bảng tổng hợp các hàm 

Hàm	EID	FID	Tham số	Trả về	Yêu cầu	Gọi từ file
sbi_ecall	tuỳ	tuỳ	eid, fid, a0..a2	struct sbiret	tất cả	sbi.c
sbi_probe_extension	0x10	3	eid	long (0/≠0)	môi trường	sbi.c
sbi_get_spec_version	0x10	0	(không)	long	môi trường	sbi.c
sbi_get_impl_id	0x10	1	(không)	long	môi trường	sbi.c
sbi_get_impl_version	0x10	2	(không)	long	môi trường	sbi.c
sbi_set_timer	0x54494D45	0	stime_value	int	R3	trap.c, main.c
sbi_putchar	1 (cũ)	0	c	int	R4	console.c
sbi_getchar	2 (cũ)	0	(không)	int (-1 = chưa có)	R4	console.c, trap.c
sbi_dbcn_write_byte	0x4442434E	2	c	int	R4	console.c
sbi_hart_start	0x48534D	0	hartid, start_addr, opaque	int	R5	main.c
sbi_hart_get_status	0x48534D	2	hartid	int (≥0 trạng thái)	R5, R7	main.c
sbi_shutdown	0x53525354	0	(type=0, reason=0)	không trả về	R6	sysproc.c
