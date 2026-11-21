CROSS ?= riscv64-linux-gnu-
CFLAGS = -O1 -g -mcmodel=medany -mno-relax -ffreestanding -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -Wall
LDFLAGS = -nostdlib -static -Wl,--build-id=none -T sbi.ld

sbitest.elf: entry.S kmain.c sbi.ld
	$(CROSS)gcc $(CFLAGS) $(LDFLAGS) -o $@ entry.S kmain.c

clean:
	rm -f sbitest.elf
