/**
 * Exti.h — External Interrupt driver
 */

#ifndef EXTI_H
#define EXTI_H

#include "Std_Types.h"

/* Trigger edge selection */
#define EXTI_RISING     0U
#define EXTI_FALLING    1U
#define EXTI_BOTH       2U

/* Callback type */
typedef void (*ExtiCallback)(void);

/**
 * @brief  Configure an EXTI line for a given GPIO pin.
 *         Also sets SYSCFG_EXTICRx to route the port.
 * @param  PortName   GPIO_A .. GPIO_D
 * @param  PinNumber  0-15
 * @param  Trigger    EXTI_RISING, EXTI_FALLING, or EXTI_BOTH
 */
void Exti_Init(uint8 PortName, uint8 PinNumber, uint8 Trigger);

/**
 * @brief  Register a callback for an EXTI line.
 * @param  Line  0-15
 * @param  Cb    function pointer (called from ISR context)
 */
void Exti_SetCallback(uint8 Line, ExtiCallback Cb);

/**
 * @brief  Enable the EXTI line interrupt (unmask).
 */
void Exti_EnableLine(uint8 Line);

/**
 * @brief  Disable the EXTI line interrupt (mask).
 */
void Exti_DisableLine(uint8 Line);

#endif /* EXTI_H */
