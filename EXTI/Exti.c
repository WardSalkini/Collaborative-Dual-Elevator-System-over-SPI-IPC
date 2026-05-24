/**
 * Exti.c — External Interrupt driver for STM32F401
 */

#include "Exti.h"
#include "Exti_Private.h"
#include "Gpio.h"
#include "Bit_Math.h"

/* Callback table — one per EXTI line 0-15 */
static ExtiCallback G_ExtiCallbacks[16] = {0};

/* Map PortName char to SYSCFG port index */
static uint8 PortCharToIndex(uint8 PortName)
{
    switch (PortName) {
        case 'A': return EXTI_PORT_A;
        case 'B': return EXTI_PORT_B;
        case 'C': return EXTI_PORT_C;
        case 'D': return EXTI_PORT_D;
        default:  return EXTI_PORT_A;
    }
}

void Exti_Init(uint8 PortName, uint8 PinNumber, uint8 Trigger)
{
    /* 1. Configure GPIO as input with pull-down */
    Gpio_Init(PortName, PinNumber, GPIO_INPUT, GPIO_PULL_DOWN);

    /* 2. Route pin to EXTI line via SYSCFG_EXTICRx */
    uint8 regIdx  = PinNumber / 4;       /* which EXTICR register (0-3) */
    uint8 bitPos  = (PinNumber % 4) * 4; /* bit offset within register  */
    uint8 portIdx = PortCharToIndex(PortName);

    SYSCFG->EXTICR[regIdx] &= ~(0x0FUL << bitPos);
    SYSCFG->EXTICR[regIdx] |=  ((uint32)portIdx << bitPos);

    /* 3. Set trigger edge */
    if (Trigger == EXTI_RISING || Trigger == EXTI_BOTH) {
        SET_BIT(EXTI->RTSR, PinNumber);
    } else {
        CLEAR_BIT(EXTI->RTSR, PinNumber);
    }

    if (Trigger == EXTI_FALLING || Trigger == EXTI_BOTH) {
        SET_BIT(EXTI->FTSR, PinNumber);
    } else {
        CLEAR_BIT(EXTI->FTSR, PinNumber);
    }

    /* 4. Unmask the line */
    SET_BIT(EXTI->IMR, PinNumber);
}

void Exti_SetCallback(uint8 Line, ExtiCallback Cb)
{
    if (Line < 16) {
        G_ExtiCallbacks[Line] = Cb;
    }
}

void Exti_EnableLine(uint8 Line)
{
    SET_BIT(EXTI->IMR, Line);
}

void Exti_DisableLine(uint8 Line)
{
    CLEAR_BIT(EXTI->IMR, Line);
}

/* ================================================================
 *  IRQ Handlers
 * ================================================================ */

static void Exti_HandleLine(uint8 Line)
{
    if (EXTI->PR & (1UL << Line)) {
        EXTI->PR = (1UL << Line);           /* Clear pending (write-1-to-clear) */
        if (G_ExtiCallbacks[Line] != (void *)0) {
            G_ExtiCallbacks[Line]();
        }
    }
}

void EXTI0_IRQHandler(void) { Exti_HandleLine(0); }
void EXTI1_IRQHandler(void) { Exti_HandleLine(1); }
void EXTI2_IRQHandler(void) { Exti_HandleLine(2); }
void EXTI3_IRQHandler(void) { Exti_HandleLine(3); }
void EXTI4_IRQHandler(void) { Exti_HandleLine(4); }

void EXTI9_5_IRQHandler(void)
{
    for (uint8 i = 5; i <= 9; i++) {
        Exti_HandleLine(i);
    }
}

void EXTI15_10_IRQHandler(void)
{
    for (uint8 i = 10; i <= 15; i++) {
        Exti_HandleLine(i);
    }
}
