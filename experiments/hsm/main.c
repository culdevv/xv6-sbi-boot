
#include "uart.h"
#include "../../kernel/sbi.h"

typedef unsigned long uint64;

#define INVALID_HART_ID 99
#define TEST_OPAQUE     0xCAFE

extern void _secondary(void);

/*
 * Triển khai sbi_ecall() theo prototype trong kernel/sbi.h.
 * Các thanh ghi a0-a5 chứa tham số, a6 chứa FID,
 * a7 chứa EID.
 */
struct sbiret sbi_ecall(long eid, long fid,
                        long arg0, long arg1, long arg2,
                        long arg3, long arg4, long arg5)
{
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    register long a3 asm("a3") = arg3;
    register long a4 asm("a4") = arg4;
    register long a5 asm("a5") = arg5;
    register long a6 asm("a6") = fid;
    register long a7 asm("a7") = eid;

    asm volatile(
        "ecall"
        : "+r"(a0), "+r"(a1)
        : "r"(a2), "r"(a3), "r"(a4), "r"(a5),
          "r"(a6), "r"(a7)
        : "memory"
    );

    struct sbiret ret = {
        .error = a0,
        .value = a1
    };

    return ret;
}

/* Triển khai API HSM đã khai báo trong kernel/sbi.h. */
struct sbiret sbi_hart_start(unsigned long hartid,
                             unsigned long start_addr,
                             unsigned long opaque)
{
    return sbi_ecall(
        SBI_EXT_HSM,
        SBI_EXT_HSM_HART_START,
        (long)hartid,
        (long)start_addr,
        (long)opaque,
        0, 0, 0
    );
}

struct sbiret sbi_hart_get_status(unsigned long hartid)
{
    return sbi_ecall(
        SBI_EXT_HSM,
        SBI_EXT_HSM_HART_GET_STATUS,
        (long)hartid,
        0, 0, 0, 0, 0
    );
}

/* Biến đánh dấu hart phụ đã chạy tới secondary_main(). */
static volatile uint64 secondary_online = 0;


/* Chuyển mã lỗi SBI thành tên dễ đọc. */
static const char *sbi_error_name(long error)
{
    switch (error) {
    case SBI_SUCCESS:
        return "SBI_SUCCESS";
    case SBI_ERR_FAILED:
        return "SBI_ERR_FAILED";
    case SBI_ERR_NOT_SUPPORTED:
        return "SBI_ERR_NOT_SUPPORTED";
    case SBI_ERR_INVALID_PARAM:
        return "SBI_ERR_INVALID_PARAM";
    case SBI_ERR_DENIED:
        return "SBI_ERR_DENIED";
    case SBI_ERR_INVALID_ADDRESS:
        return "SBI_ERR_INVALID_ADDRESS";
    case SBI_ERR_ALREADY_AVAILABLE:
        return "SBI_ERR_ALREADY_AVAILABLE";
    case SBI_ERR_ALREADY_STARTED:
        return "SBI_ERR_ALREADY_STARTED";
    case SBI_ERR_ALREADY_STOPPED:
        return "SBI_ERR_ALREADY_STOPPED";
    case SBI_ERR_NO_SHMEM:
        return "SBI_ERR_NO_SHMEM";
    default:
        return "UNKNOWN_SBI_ERROR";
    }
}

/* Chuyển trạng thái HSM thành tên dễ đọc. */
static const char *hsm_state_name(long state)
{
    switch (state) {
    case SBI_HSM_STATE_STARTED:
        return "STARTED";
    case SBI_HSM_STATE_STOPPED:
        return "STOPPED";
    case SBI_HSM_STATE_START_PENDING:
        return "START_PENDING";
    default:
        return "UNKNOWN_STATE";
    }
}

static void print_result(const char *name, struct sbiret ret)
{
    uart_puts(name);
    uart_puts("\n  error = ");
    uart_puthex((uint64)ret.error);
    uart_puts("\n  error name = ");
    uart_puts(sbi_error_name(ret.error));
    uart_puts("\n  value = ");
    uart_puthex((uint64)ret.value);
    uart_puts("\n\n");
}




/* Hàm được thực thi trên hart phụ. */
void secondary_main(uint64 hartid, uint64 opaque)
{
    uart_puts("\n[SECONDARY] hart ");
    uart_puthex(hartid);
    uart_puts(" up opaque=");
    uart_puthex(opaque);
    uart_puts("\n");

    secondary_online = 1;

    /* Giữ hart phụ hoạt động để quan sát trạng thái. */
    while (1) {
        asm volatile("wfi");
    }
}

void main(uint64 boot_hartid)
{
    struct sbiret ret;
    uint64 target_hartid = (boot_hartid == 0) ? 1 : 0;

    uart_puts("\n====================================\n");
    uart_puts("       SBI HSM EXPERIMENT\n");
    uart_puts("====================================\n");

    uart_puts("\n[TEST 1] Get status of target hart before start\n");
    ret = sbi_hart_get_status(target_hartid);
    print_result("HART_GET_STATUS(target)", ret);

    if (ret.error == SBI_SUCCESS) {
        uart_puts("Target hart state: ");
        uart_puts(hsm_state_name(ret.value));
        uart_puts("\n");
    }

    uart_puts("\n[TEST 2] Start target hart\n");
    ret = sbi_hart_start(target_hartid, (unsigned long)_secondary, TEST_OPAQUE);
    print_result("HART_START(target)", ret);

    /* Chờ hart phụ chạy, tối đa khoảng thời gian hữu hạn. */
    if (ret.error == SBI_SUCCESS) {
        volatile uint64 timeout;

        for (timeout = 0;
             timeout < 10000000 && secondary_online == 0;
             timeout++) {
            asm volatile("nop");
        }

        uart_puts("Secondary online marker: ");
        uart_puts(secondary_online ? "RECEIVED\n" : "NOT RECEIVED\n");
    }

    uart_puts("\n[TEST 3] Get status of target hart after start\n");
    ret = sbi_hart_get_status(target_hartid);
    print_result("HART_GET_STATUS(target)", ret);

    if (ret.error == SBI_SUCCESS) {
        uart_puts("Target hart state: ");
        uart_puts(hsm_state_name(ret.value));
        uart_puts("\n");
    }

    uart_puts("\n[TEST 4] Start target hart for the second time\n");
    ret = sbi_hart_start(target_hartid, (unsigned long)_secondary, TEST_OPAQUE);
    print_result("HART_START(target) AGAIN", ret);

    uart_puts("\n[TEST 5] Start the current boot hart\n");
    ret = sbi_hart_start(boot_hartid, (unsigned long)_secondary, TEST_OPAQUE);
    print_result("HART_START(boot hart)", ret);

    uart_puts("\n[TEST 6] Start invalid hart 99\n");
    ret = sbi_hart_start(INVALID_HART_ID, (unsigned long)_secondary, TEST_OPAQUE);
    print_result("HART_START(99)", ret);

    uart_puts("\n====================================\n");
    uart_puts("       HSM TEST FINISHED\n");
    uart_puts("====================================\n");

    while (1) {
        asm volatile("wfi");
    }
}
