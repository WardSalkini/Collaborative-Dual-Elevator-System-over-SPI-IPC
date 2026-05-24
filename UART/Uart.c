/**
 * Uart.c — USART1 driver (polling TX, no RX needed for telemetry)
 */

#include "Uart.h"
#include "Uart_Private.h"
#include "Gpio.h"

void Uart1_Init(uint32 BaudRate)
{
    /* PA9 = TX (AF7), PA10 = RX (AF7) */
    Gpio_Init(GPIO_A, 9,  GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(GPIO_A, 10, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(GPIO_A, 9,  GPIO_AF7);
    Gpio_SetAF(GPIO_A, 10, GPIO_AF7);

    /* Disable USART before configuration */
    USART1->CR1 = 0;

    /*
     * BRR calculation for 16 MHz PCLK2, oversampling by 16:
     *   USARTDIV = fCK / (16 * baud)
     *   BRR      = mantissa << 4 | fraction
     *
     * For 9600:   USARTDIV = 16000000/9600   = 1666.67 → BRR = 0x683B
     * For 115200: USARTDIV = 16000000/115200 = 138.89  → BRR = 0x008B
     *
     * General formula (integer math, rounding):
     *   BRR = (fCK + baud/2) / baud
     * This works because BRR is in 12.4 fixed-point and fCK/baud gives
     * the 16× oversampled value which equals mantissa*16 + frac.
     */
    uint32 fck = 16000000UL;
    USART1->BRR = (fck + BaudRate / 2) / BaudRate;

    /* 8-bit word (M=0), 1 stop bit (CR2 default), no parity */
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    /* Enable TX + USART */
    USART1->CR1 |= (1UL << USART_CR1_TE);
    USART1->CR1 |= (1UL << USART_CR1_UE);
}

void Uart1_SendChar(char c)
{
    while (!(USART1->SR & (1UL << USART_SR_TXE)));
    USART1->DR = (uint32)c;
}

void Uart1_SendString(const char *str)
{
    while (*str) {
        Uart1_SendChar(*str++);
    }
}

void Uart1_SendNumber(uint32 num)
{
    char buf[11]; /* max 10 digits + null */
    uint8 i = 0;

    if (num == 0) {
        Uart1_SendChar('0');
        return;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    /* Reverse print */
    while (i > 0) {
        Uart1_SendChar(buf[--i]);
    }
}
