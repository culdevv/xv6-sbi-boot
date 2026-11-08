/*
CREATE BY VU DINH LUC in 06/11/2026

Các lượt sửa đổi / đánh giá:
1. 06/11/2026: LỰA CHỌN DÙNG LEGACY CONSOLE HAY DBCN
Giai đoạn Core (Tuần 3 - Tuần 4): CHỌN LEGACY CONSOLE (a7 = 1) LÀM CHÍNH

Lý do: Giúp nhóm hoàn thiện mốc R1/R4 một cách nhanh nhất, chắc chắn chạy ổn định trên mọi máy ảo QEMU của 3 thành viên mà không phát sinh bug bộ nhớ/con trỏ.

Giúp hoàn thiện hàm sbi_putchar() trong file sbi.h/sbi.c cực kỳ gọn nhẹ.

Giai đoạn Hoàn thiện (Tuần 5 - Tuần 6): NÂNG CẤP DÙNG DBCN NẾU ĐƯỢC HỖ TRỢ (FALLBACK)

Trong hàm wrapper sbi_putchar() hoặc sbi_console_write(), nhóm sẽ dùng hàm sbi_probe_extension(SBI_EXT_DBCN) để kiểm tra OpenSBI hiện tại có hỗ trợ DBCN không:

Nếu hỗ trợ: Gọi DBCN Write Byte (FID = 2) hoặc DBCN Write (FID = 0) để tối ưu hiệu năng.

Nếu không hỗ trợ (trả về lỗi -2): Tự động lùi về dùng Legacy Console Putchar (a7 = 1).
*/

/*
 * kernel/sbi.h - RISC-V Supervisor Binary Interface
 *
 * Reference:
 * RISC-V Supervisor Binary Interface Specification v2.0
 *
 * Extensions:
 * BASE (0x10), TIME (0x54494D45), HSM (0x48534D),
 * SRST (0x53525354), DBCN (0x4442434E), Legacy.
 *
 * Calling convention:
 * a7 = EID, a6 = FID
 * a0-a5 = Arguments
 * a0 = Error code, a1 = Return value
 */

#ifndef __SBI_H__
#define __SBI_H__

/* 1. SBI Return Structure */

struct sbiret {
  long error;
  long value;
};

/* 2. SBI Error Codes */

#define SBI_SUCCESS                0 /* thanh cong*/
#define SBI_ERR_FAILED            -1 /* that bai */
#define SBI_ERR_NOT_SUPPORTED     -2 /* khong ho tro */
#define SBI_ERR_INVALID_PARAM     -3 /* tham so khong hop le */
#define SBI_ERR_DENIED            -4 /* bi tu choi */
#define SBI_ERR_INVALID_ADDRESS   -5 /* dia chi khong hop le */
#define SBI_ERR_ALREADY_AVAILABLE -6 /* da co san */
#define SBI_ERR_ALREADY_STARTED   -7 /* da bat dau */
#define SBI_ERR_ALREADY_STOPPED   -8 /* da dung */
#define SBI_ERR_NO_SHMEM          -9 /* khong co bo nho chia se */

/* 3. Extension IDs and Function IDs */

/* Base Extension */
#define SBI_EXT_BASE                  0x10
#define SBI_EXT_BASE_GET_SPEC_VERSION 0
#define SBI_EXT_BASE_GET_IMP_ID       1
#define SBI_EXT_BASE_GET_IMP_VERSION  2
#define SBI_EXT_BASE_PROBE_EXT        3
#define SBI_EXT_BASE_GET_MVENDORID    4
#define SBI_EXT_BASE_GET_MARCHID      5
#define SBI_EXT_BASE_GET_MIMPID       6

/* Legacy Extensions */
#define SBI_EXT_LEGACY_SET_TIMER              0x00
#define SBI_EXT_LEGACY_CONSOLE_PUTCHAR        0x01
#define SBI_EXT_LEGACY_CONSOLE_GETCHAR        0x02
#define SBI_EXT_LEGACY_CLEAR_IPI              0x03
#define SBI_EXT_LEGACY_SEND_IPI               0x04
#define SBI_EXT_LEGACY_REMOTE_FENCE_I         0x05
#define SBI_EXT_LEGACY_REMOTE_SFENCE_VMA      0x06
#define SBI_EXT_LEGACY_REMOTE_SFENCE_VMA_ASID 0x07
#define SBI_EXT_LEGACY_SHUTDOWN               0x08

/* TIME Extension */
#define SBI_EXT_TIME           0x54494D45
#define SBI_EXT_TIME_SET_TIMER 0

/* HSM Extension */
#define SBI_EXT_HSM                 0x48534D
#define SBI_EXT_HSM_HART_START      0
#define SBI_EXT_HSM_HART_STOP       1
#define SBI_EXT_HSM_HART_GET_STATUS 2
#define SBI_EXT_HSM_HART_SUSPEND    3

/* HSM States */
#define SBI_HSM_STATE_STARTED         0
#define SBI_HSM_STATE_STOPPED         1
#define SBI_HSM_STATE_START_PENDING   2
#define SBI_HSM_STATE_STOP_PENDING    3
#define SBI_HSM_STATE_SUSPENDED       4
#define SBI_HSM_STATE_SUSPEND_PENDING 5
#define SBI_HSM_STATE_RESUME_PENDING  6

/* SRST Extension */
#define SBI_EXT_SRST           0x53525354
#define SBI_EXT_SRST_SYS_RESET 0

#define SBI_SRST_TYPE_SHUTDOWN    0
#define SBI_SRST_TYPE_COLD_REBOOT 1
#define SBI_SRST_TYPE_WARM_REBOOT 2

#define SBI_SRST_REASON_NONE           0
#define SBI_SRST_REASON_SYSTEM_FAILURE 1

/* DBCN Extension */
#define SBI_EXT_DBCN                    0x4442434E
#define SBI_EXT_DBCN_CONSOLE_WRITE      0
#define SBI_EXT_DBCN_CONSOLE_READ       1
#define SBI_EXT_DBCN_CONSOLE_WRITE_BYTE 2

/* 4. SBI Function Prototypes */

/* Generic SBI call */
struct sbiret sbi_ecall(long eid, long fid, long a0, long a1, long a2, long a3,
                        long a4, long a5);

/* Console and Timer */
void sbi_putchar(int c);
int sbi_getchar(void);
void sbi_set_timer(unsigned long stime_value);

/* System Shutdown */
void sbi_shutdown(void);

/* Hart State Management */
struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr,
                             unsigned long opaque);

struct sbiret sbi_hart_get_status(unsigned long hartid);

/* Base Extension */
int sbi_probe_extension(long ext_id);

#endif /* __SBI_H__ */
