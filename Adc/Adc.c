/**
 * Adc.c
 *
 *  Created on: 4/12/2026
 *  Author    : AbdallahDarwish
 */

#include "Adc.h"
#include "Adc_Private.h"
#include "Bit_Math.h"
#include "Nvic.h"

/* Global variables for Async mode */
static AdcSingleChannelCallback G_AdcSingleCallback = (void*)0;

void Adc_Init(uint8 Resolution) {
    /* Set ADC Prescaler to PCLK2/4 (to help Proteus simulator) */
    ADC_CCR &= ~(3UL << 16);
    ADC_CCR |= (1UL << 16);
    
    /* 1. Power on ADC (ADON) */
    ADC1->CR2 |= (1UL << 0);
    for(volatile int i=0; i<1000; i++); /* Increased stabilization delay */
    
    /* 2. Set Resolution */
    ADC1->CR1 &= ~(3UL << 24);
    ADC1->CR1 |= ((uint32)Resolution << 24);
    
    /* 3. Pre-enable NVIC and EOCIE for Async mode */
    Nvic_EnableIrq(18); 
    ADC1->CR1 |= (1UL << 5); 
}

void Adc_ConfigSingleChannel_Continuous(uint8 Channel) {
    /* 1. Set channel in SQR3 (1st conversion) */
    ADC1->SQR3 &= ~0x1F;
    ADC1->SQR3 |= (uint32)Channel;
    
    /* 2. Set Continuous mode (CONT) */
    ADC1->CR2 |= (1UL << 1);
}

void Adc_ConfigSingleChannel_OneShot(uint8 Channel) {
    /* 1. Set channel in SQR3 (1st conversion) */
    ADC1->SQR3 &= ~0x1F;
    ADC1->SQR3 |= (uint32)Channel;
    
    /* 2. Clear Continuous mode (CONT) */
    ADC1->CR2 &= ~(1UL << 1);
}

uint16 Adc_ReadSingleChannel(void) {
    uint32 timeout = 100000;
    
    /* Clear SR */
    ADC1->SR = 0;
    
    /* Start conversion */
    ADC1->CR2 |= (1UL << 30);
    
    /* Wait for EOC with timeout to prevent complete hang */
    while (!(ADC1->SR & (1UL << 1)) && timeout > 0) {
        timeout--;
    }
    
    return (uint16)ADC1->DR;
}


void Adc_ReadSingleChannelAsync(AdcSingleChannelCallback Callback) {
    G_AdcSingleCallback = Callback;
    
    /* Clear flags and Start conversion */
    ADC1->SR = 0;
    ADC1->CR2 |= (1UL << 30);
}

void Adc_StartConversion(void) {
    ADC1->SR = 0;
    ADC1->CR2 |= (1UL << 30);
}

void Adc_StopConversion(void) {
    ADC1->CR2 &= ~(1UL << 1); /* Disable CONT */
    ADC1->CR2 &= ~(1UL << 0); /* Disable ADON */
}

void ADC_IRQHandler(void) {
    if (ADC1->SR & (1UL << 1)) {
        uint16 result = (uint16)(ADC1->DR & 0xFFFF);
        ADC1->SR = 0; /* Clear all flags FIRST */
        
        if (G_AdcSingleCallback != (void*)0) {
            G_AdcSingleCallback(result);
        }
    }
}

