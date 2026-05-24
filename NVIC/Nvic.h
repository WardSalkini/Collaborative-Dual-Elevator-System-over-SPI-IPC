/**
 * Nvic.h — NVIC driver with priority support
 */

#ifndef NVIC_H
#define NVIC_H
#include "Std_Types.h"

void Nvic_EnableIrq(uint8 IrqNumber);
void Nvic_DisableIrq(uint8 IrqNumber);

/**
 * @brief  Set interrupt priority (0 = highest, 15 = lowest).
 *         STM32F4 uses 4 priority bits in the upper nibble of each IPR byte.
 */
void Nvic_SetPriority(uint8 IrqNumber, uint8 Priority);

/* Critical section helpers */
static inline void Enter_Critical(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

static inline void Exit_Critical(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

static inline void EnableGlobalInterrupts(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

static inline void WaitForInterrupt(void)
{
    __asm volatile ("wfi");
}

#endif /* NVIC_H */
