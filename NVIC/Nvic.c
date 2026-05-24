/**
 * Nvic.c — NVIC driver with priority support
 */

#include "Nvic.h"

typedef struct {
    volatile uint32 ISER[8];       /* 0x000 - Set-enable      */
    uint32 _r1[24];
    volatile uint32 ICER[8];       /* 0x080 - Clear-enable    */
    uint32 _r2[24];
    volatile uint32 ISPR[8];       /* 0x100 - Set-pending     */
    uint32 _r3[24];
    volatile uint32 ICPR[8];       /* 0x180 - Clear-pending   */
    uint32 _r4[24];
    volatile uint32 IABR[8];       /* 0x200 - Active bit      */
    uint32 _r5[56];
    volatile uint8  IPR[240];      /* 0x300 - Priority (byte-addressable) */
} NvicType;

#define NVIC  ((NvicType *)0xE000E100UL)

void Nvic_EnableIrq(uint8 IrqNumber)
{
    NVIC->ISER[IrqNumber / 32] = (1UL << (IrqNumber % 32));
}

void Nvic_DisableIrq(uint8 IrqNumber)
{
    NVIC->ICER[IrqNumber / 32] = (1UL << (IrqNumber % 32));
}

void Nvic_SetPriority(uint8 IrqNumber, uint8 Priority)
{
    /* STM32F4 implements 4 priority bits in the upper nibble */
    NVIC->IPR[IrqNumber] = (Priority & 0x0F) << 4;
}
