CROSS = riscv64-linux-gnu-

CC = $(CROSS)gcc

CFLAGS = -march=rv64imac -mabi=lp64 -mcmodel=medany \
         -ffreestanding -fno-builtin -fno-stack-protector \
         -fno-pie -O2 -Wall -Wextra

LDFLAGS = -nostdlib -nostartfiles -no-pie \
          -T linker.ld

OBJS = start.o main.o uart.o

all: sbi36.elf

sbi36.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

start.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o sbi36.elf
