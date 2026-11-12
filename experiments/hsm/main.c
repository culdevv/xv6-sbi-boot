#include "uart.h"

typedef unsigned long uint64;

#define SBI_EXT_HSM                 0x48534D
#define SBI_EXT_HSM_HART_START      0
#define SBI_EXT_HSM_HART_GET_STATUS 2

#define SBI_SUCCESS                 0
#define SBI_ERR_FAILED             -1
#define SBI_ERR_NOT_SUPPORTED      -2
#define SBI_ERR_INVALID_PARAM      -3
#define SBI_ERR_DENIED             -4
#define SBI_ERR_INVALID_ADDRESS    -5
#define SBI_ERR_ALREADY_AVAILABLE  -6
#define SBI_ERR_ALREADY_STARTED    -7
#define SBI_ERR_ALREADY_STOPPED    -8
#define SBI_ERR_NO_SHMEM           -9

#define HSM_STATE_STARTED           0
#define HSM_STATE_STOPPED           1
#define HSM_STATE_START_PENDING     2

#define INVALID_HART_ID             99
#define TEST_OPAQUE                 0xCAFE

extern void _secondary(void);

struct sbiret {
    long error;
    long value;
};

/* Biến đánh dấu hart phụ đã chạy tới secondary_main(). */
static volatile uint64 secondary_online = 0;

/* Thực hiện một SBI ecall tổng quát. */
static inline struct sbiret
sbi_call(uint64 eid, uint64 fid,
         uint64 arg0, uint64 arg1, uint64 arg2)
{
    register uint64 a0 asm("a0") = arg0;
    register uint64 a1 asm("a1") = arg1;
    register uint64 a2 asm("a2") = arg2;
    register uint64 a6 asm("a6") = fid;
    register uint64 a7 asm("a7") = eid;

    asm volatile(
        "ecall"
        : "+r"(a0), "+r"(a1)
        : "r"(a2), "r"(a6), "r"(a7)
        : "memory"
    );

    struct sbiret ret = {
        .error = (long)a0,
        .value = (long)a1
    };

    return ret;
}

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
    case HSM_STATE_STARTED:
        return "STARTED";
    case HSM_STATE_STOPPED:
        return "STOPPED";
    case HSM_STATE_START_PENDING:
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

/* Đọc trạng thái của một hart. */
static struct sbiret get_hart_status(uint64 hartid)
{
    return sbi_call(
        SBI_EXT_HSM,
        SBI_EXT_HSM_HART_GET_STATUS,
        hartid, 0, 0
    );
}

/* Yêu cầu OpenSBI khởi động một hart. */
static struct sbiret start_hart(
    uint64 hartid,
    void (*entry)(void),
    uint64 opaque)
{
    return sbi_call(
        SBI_EXT_HSM,
        SBI_EXT_HSM_HART_START,
        hartid,
        (uint64)entry,
        opaque
    );
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
    ret = get_hart_status(target_hartid);
    print_result("HART_GET_STATUS(target)", ret);

    if (ret.error == SBI_SUCCESS) {
        uart_puts("Target hart state: ");
        uart_puts(hsm_state_name(ret.value));
        uart_puts("\n");
    }

    uart_puts("\n[TEST 2] Start target hart\n");
    ret = start_hart(target_hartid, _secondary, TEST_OPAQUE);
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
    ret = get_hart_status(target_hartid);
    print_result("HART_GET_STATUS(target)", ret);

    if (ret.error == SBI_SUCCESS) {
        uart_puts("Target hart state: ");
        uart_puts(hsm_state_name(ret.value));
        uart_puts("\n");
    }

    uart_puts("\n[TEST 4] Start target hart for the second time\n");
    ret = start_hart(target_hartid, _secondary, TEST_OPAQUE);
    print_result("HART_START(target) AGAIN", ret);

    uart_puts("\n[TEST 5] Start the current boot hart\n");
    ret = start_hart(boot_hartid, _secondary, TEST_OPAQUE);
    print_result("HART_START(boot hart)", ret);

    uart_puts("\n[TEST 6] Start invalid hart 99\n");
    ret = start_hart(INVALID_HART_ID, _secondary, TEST_OPAQUE);
    print_result("HART_START(99)", ret);

    uart_puts("\n====================================\n");
    uart_puts("       HSM TEST FINISHED\n");
    uart_puts("====================================\n");

    while (1) {
        asm volatile("wfi");
    }
}
