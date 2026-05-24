/**
 * Timer.c — General-purpose timer driver (TIM2-TIM5) with callbacks
 */

#include "Timer.h"
#include "Timer_Private.h"
#include "Bit_Math.h"

static uint32 Timer_BaseAddresses[4] = {
    TIM2_BASE_ADDR, TIM3_BASE_ADDR, TIM4_BASE_ADDR, TIM5_BASE_ADDR
};

/* Callbacks for TIM2-TIM5 (index 0-3) */
static TimerCallback G_TimerCallbacks[4] = {0, 0, 0, 0};

static TimerType *Timer_Get(uint8 TimerId)
{
    return (TimerType *)(uintptr_t)Timer_BaseAddresses[TimerId - TIMER2];
}

void Timer_Init(uint8 TimerId, uint16 Prescaler, uint16 AutoReload)
{
    TimerType *timer = Timer_Get(TimerId);
    timer->CR1 = 0;
    timer->PSC = Prescaler;
    timer->ARR = AutoReload;
    timer->CNT = 0;
    SET_BIT(timer->EGR, EGR_UG);
    timer->SR = 0;
}

void Timer_Start(uint8 TimerId)
{
    TimerType *timer = Timer_Get(TimerId);
    SET_BIT(timer->CR1, CR1_CEN);
}

void Timer_Stop(uint8 TimerId)
{
    TimerType *timer = Timer_Get(TimerId);
    CLEAR_BIT(timer->CR1, CR1_CEN);
}

void Timer_EnableInterrupt(uint8 TimerId)
{
    TimerType *timer = Timer_Get(TimerId);
    SET_BIT(timer->DIER, DIER_UIE);
}

void Timer_ClearFlag(uint8 TimerId)
{
    TimerType *timer = Timer_Get(TimerId);
    timer->SR = 0;
}

void Timer_SetCallback(uint8 TimerId, TimerCallback Callback)
{
    if (TimerId >= TIMER2 && TimerId <= TIMER5) {
        G_TimerCallbacks[TimerId - TIMER2] = Callback;
    }
}

void Timer_DelayMs(uint8 TimerId, uint32 DelayMs)
{
    TimerType *timer = Timer_Get(TimerId);
    timer->CR1 = 0;
    timer->PSC = 15999;   /* 16 MHz / 16000 = 1 kHz → 1 ms ticks */
    timer->ARR = 0xFFFF;
    SET_BIT(timer->EGR, EGR_UG);
    timer->SR = 0;
    SET_BIT(timer->CR1, CR1_CEN);

    for (uint32 i = 0; i < DelayMs; i++) {
        while (!READ_BIT(timer->SR, SR_UIF));
        timer->SR = 0;
    }
    CLEAR_BIT(timer->CR1, CR1_CEN);
}

void Timer_OcToggleInit(uint8 TimerId, uint8 Channel,
                        uint16 Prescaler, uint16 Period)
{
    /* Not used in elevator project */
    (void)TimerId; (void)Channel; (void)Prescaler; (void)Period;
}

/* ================================================================
 *  IRQ Handlers for TIM2-TIM5
 * ================================================================ */

static void Timer_HandleIRQ(uint8 TimerId)
{
    TimerType *timer = Timer_Get(TimerId);
    if (READ_BIT(timer->SR, SR_UIF)) {
        timer->SR = 0;
        uint8 idx = TimerId - TIMER2;
        if (G_TimerCallbacks[idx] != (void *)0) {
            G_TimerCallbacks[idx]();
        }
    }
}

void TIM2_IRQHandler(void) { Timer_HandleIRQ(TIMER2); }
void TIM3_IRQHandler(void) { Timer_HandleIRQ(TIMER3); }
void TIM4_IRQHandler(void) { Timer_HandleIRQ(TIMER4); }
void TIM5_IRQHandler(void) { Timer_HandleIRQ(TIMER5); }
