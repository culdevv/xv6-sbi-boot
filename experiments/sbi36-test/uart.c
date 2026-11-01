#define UART0 0x10000000L

#define UART_THR 0
#define UART_LSR 5
#define UART_LSR_TX_IDLE (1 << 5)

static volatile unsigned char *uart =
    (volatile unsigned char *)UART0;

void uart_putc(char c)
{
    // Wait until UART transmitter is ready
    while ((uart[UART_LSR] & UART_LSR_TX_IDLE) == 0)
        ;

    // Send one character
    uart[UART_THR] = c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');

        uart_putc(*s++);
    }
}

void uart_puthex(unsigned long value)
{
    const char hex[] = "0123456789abcdef";

    uart_puts("0x");

    for (int i = 60; i >= 0; i -= 4)
        uart_putc(hex[(value >> i) & 0xf]);
}
