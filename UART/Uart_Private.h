/**
 * Uart_Private.h — USART register map for STM32F401
 */

#ifndef UART_PRIVATE_H
#define UART_PRIVATE_H

#include "Std_Types.h"

typedef struct {
    volatile uint32 SR;    /* 0x00 - Status register          */
    volatile uint32 DR;    /* 0x04 - Data register            */
    volatile uint32 BRR;   /* 0x08 - Baud rate register       */
    volatile uint32 CR1;   /* 0x0C - Control register 1       */
    volatile uint32 CR2;   /* 0x10 - Control register 2       */
    volatile uint32 CR3;   /* 0x14 - Control register 3       */
    volatile uint32 GTPR;  /* 0x18 - Guard time / prescaler   */
} UsartType;

#define USART1_BASE_ADDR  0x40011000UL
#define USART2_BASE_ADDR  0x40004400UL

#define USART1  ((UsartType *)USART1_BASE_ADDR)
#define USART2  ((UsartType *)USART2_BASE_ADDR)

/* SR bit positions */
#define USART_SR_PE     0U
#define USART_SR_FE     1U
#define USART_SR_NF     2U
#define USART_SR_ORE    3U
#define USART_SR_IDLE   4U
#define USART_SR_RXNE   5U
#define USART_SR_TC     6U
#define USART_SR_TXE    7U

/* CR1 bit positions */
#define USART_CR1_SBK    0U
#define USART_CR1_RE     2U
#define USART_CR1_TE     3U
#define USART_CR1_RXNEIE 5U
#define USART_CR1_TCIE   6U
#define USART_CR1_TXEIE  7U
#define USART_CR1_OVER8  15U
#define USART_CR1_UE     13U

#endif /* UART_PRIVATE_H */
