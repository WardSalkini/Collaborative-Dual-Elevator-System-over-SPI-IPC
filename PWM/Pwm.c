/**
 * Pwm.c
 *
 *  Created on: 4/12/2026
 *  Author    : AbdallahDarwish
 */

#include "Pwm.h"
#include "Timer_Private.h"   /* TimerType struct + base addresses */
#include "Bit_Math.h"
#include "Timer.h"

static uint32 Pwm_BaseAddresses[4] = {TIM2_BASE_ADDR, TIM3_BASE_ADDR, TIM4_BASE_ADDR,TIM5_BASE_ADDR};

/* Helper function to get CCR register address without risky pointer arithmetic */
static volatile uint32* Pwm_GetCcrRegister(TimerType* timer, uint8 Channel) {
    switch (Channel) {
        case 1: return &(timer->CCR1);
        case 2: return &(timer->CCR2);
        case 3: return &(timer->CCR3);
        case 4: return &(timer->CCR4);
        default: return NULL;
    }
}

void Pwm_Init(uint8 TimerId, uint8 Channel, uint16 Prescaler, uint16 AutoReload) {
    TimerType *timer = (TimerType *)(uintptr_t)Pwm_BaseAddresses[TimerId - TIMER2];


    /* Time-base configuration */
    timer->CR1 = 0;
    timer->PSC = Prescaler;
    timer->ARR = AutoReload;
    timer->CNT = 0;

    /* Channel configuration: PWM Mode 1 + output-compare preload */
    if (Channel <= 2) {
        uint8 shift = (Channel - 1) * 8;
        timer->CCMR1 &= ~((uint32) 0xFF << shift);
        timer->CCMR1 |= ((uint32) CCMR_OC_PWM1_PRELOAD << shift);
    } else {
        uint8 shift = (Channel - 3) * 8;
        timer->CCMR2 &= ~((uint32) 0xFF << shift);
        timer->CCMR2 |= ((uint32) CCMR_OC_PWM1_PRELOAD << shift);
    }

    /* Enable channel output in CCER (CCxE bit) */
    SET_BIT(timer->CCER, (Channel - 1) * 4);

    /* Initialize duty cycle to 0 */
    volatile uint32* ccr = Pwm_GetCcrRegister(timer, Channel);
    if (ccr) *ccr = 0;

    /* Enable Auto-reload preload and force update to load registers */
    SET_BIT(timer->CR1, CR1_ARPE);
    SET_BIT(timer->EGR, EGR_UG);
    timer->SR = 0;
}

/**
 *  Fixed-point duty-cycle conversion (no float!)
 *  CCR = (DutyPercent * ARR) / 100
 */
void Pwm_SetDutyPercent(uint8 TimerId, uint8 Channel, uint8 DutyPercent) {
    TimerType *timer = (TimerType *)(uintptr_t)Pwm_BaseAddresses[TimerId - TIMER2];
    if (DutyPercent > 100) DutyPercent = 100;

    uint32 arr = timer->ARR;
    uint32 ccr_val = ((uint32)DutyPercent * arr) / 100UL;

    volatile uint32* ccr_reg = Pwm_GetCcrRegister(timer, Channel);
    if (ccr_reg) *ccr_reg = ccr_val;
}

void Pwm_Start(uint8 TimerId, uint8 Channel) {
    TimerType *tim = (TimerType *)(uintptr_t)Pwm_BaseAddresses[TimerId - TIMER2];
    SET_BIT(tim->CCER, (Channel - 1) * 4);
    SET_BIT(tim->CR1, CR1_CEN);
}

void Pwm_Stop(uint8 TimerId, uint8 Channel) {
    TimerType *tim = (TimerType *)(uintptr_t)Pwm_BaseAddresses[TimerId - TIMER2];
    /* Disable only this channel's output */
    CLEAR_BIT(tim->CCER, (Channel - 1) * 4);
}
