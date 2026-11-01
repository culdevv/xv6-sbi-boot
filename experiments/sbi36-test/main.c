#include "uart.h"

typedef unsigned long uint64;

struct sbiret {
    long error;
    long value;
};

#define SBI_EXT_BASE              0x10
#define SBI_BASE_GET_SPEC_VERSION 0
#define SBI_BASE_GET_IMPL_ID      1
#define SBI_BASE_GET_IMPL_VERSION 2
#define SBI_BASE_PROBE_EXT        3

#define SBI_EXT_TIME              0x54494D45
#define SBI_EXT_UNKNOWN           0xDEADBEEF

// General SBI ecall function
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

void print_result(const char *name, struct sbiret ret)
{
    uart_puts(name);
    uart_puts("\n  error = ");
    uart_puthex((uint64)ret.error);
    uart_puts("\n  value = ");
    uart_puthex((uint64)ret.value);
    uart_puts("\n\n");
}

void main(void)
{
    struct sbiret ret;

    uart_puts("\n================================\n");
    uart_puts("       SBI-36 TEST PROGRAM\n");
    uart_puts("================================\n\n");

    // Test 1: Get SBI specification version
    ret = sbi_call(
        SBI_EXT_BASE,
        SBI_BASE_GET_SPEC_VERSION,
        0, 0, 0
    );
    print_result("TEST 1: SBI SPEC VERSION", ret);

    if (ret.error == 0) {
        uart_puts("SBI version raw value: ");
        uart_puthex((uint64)ret.value);
        uart_puts("\n");
    }

    // Test 2: Get OpenSBI implementation ID
    ret = sbi_call(
        SBI_EXT_BASE,
        SBI_BASE_GET_IMPL_ID,
        0, 0, 0
    );
    print_result("TEST 2: IMPLEMENTATION ID", ret);

    // Test 3: Get implementation version
    ret = sbi_call(
        SBI_EXT_BASE,
        SBI_BASE_GET_IMPL_VERSION,
        0, 0, 0
    );
    print_result("TEST 3: IMPLEMENTATION VERSION", ret);

    // Test 4: Probe SBI TIME extension
    ret = sbi_call(
        SBI_EXT_BASE,
        SBI_BASE_PROBE_EXT,
        SBI_EXT_TIME, 0, 0
    );
    print_result("TEST 4: PROBE TIME EXTENSION", ret);

    // Test 5: Call an unsupported extension
    ret = sbi_call(
        SBI_EXT_UNKNOWN,
        0, 0, 0, 0
    );
    print_result("TEST 5: UNSUPPORTED EXTENSION", ret);

    uart_puts("================================\n");
    uart_puts("          TEST FINISHED\n");
    uart_puts("================================\n");

    while (1) {
        asm volatile("wfi");
    }
}
