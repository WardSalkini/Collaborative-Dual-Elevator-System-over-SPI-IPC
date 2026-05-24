/**
 * Uart.h — USART1 driver for telemetry output
 */

#ifndef UART_H
#define UART_H

#include "Std_Types.h"

/**
 * @brief  Initialise USART1 on PA9(TX)/PA10(RX).
 *         8N1, TX-only, polling mode (allowed by spec).
 * @param  BaudRate  e.g. 9600, 115200
 */
void Uart1_Init(uint32 BaudRate);

/**
 * @brief  Send a single character (blocking/polling).
 */
void Uart1_SendChar(char c);

/**
 * @brief  Send a null-terminated string (blocking/polling).
 */
void Uart1_SendString(const char *str);

/**
 * @brief  Send an unsigned integer as decimal text.
 */
void Uart1_SendNumber(uint32 num);

#endif /* UART_H */
