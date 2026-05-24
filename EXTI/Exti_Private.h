/**
 * Exti_Private.h — EXTI + SYSCFG register definitions for STM32F401
 */

#ifndef EXTI_PRIVATE_H
#define EXTI_PRIVATE_H

#include "Std_Types.h"

/* EXTI register map */
typedef struct {
    volatile uint32 IMR;    /* 0x00 - Interrupt mask           */
    volatile uint32 EMR;    /* 0x04 - Event mask               */
    volatile uint32 RTSR;   /* 0x08 - Rising trigger selection */
    volatile uint32 FTSR;   /* 0x0C - Falling trigger selection*/
    volatile uint32 SWIER;  /* 0x10 - Software interrupt event */
    volatile uint32 PR;     /* 0x14 - Pending register         */
} ExtiType;

#define EXTI_BASE_ADDR  0x40013C00UL
#define EXTI            ((ExtiType *)EXTI_BASE_ADDR)

/* SYSCFG register map (only EXTICR needed) */
typedef struct {
    volatile uint32 MEMRMP;     /* 0x00 */
    volatile uint32 PMC;        /* 0x04 */
    volatile uint32 EXTICR[4];  /* 0x08 - 0x14 */
    volatile uint32 _reserved[2];
    volatile uint32 CMPCR;      /* 0x20 */
} SyscfgType;

#define SYSCFG_BASE_ADDR  0x40013800UL
#define SYSCFG            ((SyscfgType *)SYSCFG_BASE_ADDR)

/* Port indices for SYSCFG_EXTICR */
#define EXTI_PORT_A  0x00U
#define EXTI_PORT_B  0x01U
#define EXTI_PORT_C  0x02U
#define EXTI_PORT_D  0x03U

#endif /* EXTI_PRIVATE_H */
