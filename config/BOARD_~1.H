/**
 * Board_Config.h — Central pin-mapping for Dual-Elevator System
 *
 * Both MCUs are STM32F401xE.  Compile with -DBOARD_ROLE_MASTER=1
 * for Board A, or -DBOARD_ROLE_SLAVE=1 for Board B.
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* ================================================================
 *  Compile-time role selection
 * ================================================================ */
#if !defined(BOARD_ROLE_MASTER) && !defined(BOARD_ROLE_SLAVE)
#error "Define BOARD_ROLE_MASTER=1 or BOARD_ROLE_SLAVE=1"
#endif

/* ================================================================
 *  SPI1 — IPC Link  (same on both boards)
 *  PA4 = NSS, PA5 = SCK, PA6 = MISO, PA7 = MOSI
 * ================================================================ */
#define SPI_PORT GPIO_A
#define SPI_NSS_PIN 4
#define SPI_SCK_PIN 5
#define SPI_MISO_PIN 6
#define SPI_MOSI_PIN 7

/* ================================================================
 *  USART1 — Telemetry to PC
 *  PA9 = TX, PA10 = RX
 * ================================================================ */
#define UART_PORT GPIO_A
#define UART_TX_PIN 9
#define UART_RX_PIN 10

/* ================================================================
 *  Motor LED (PWM) — TIM4 Channel 1
 *  PB6 = TIM4_CH1 (AF2)
 * ================================================================ */
#define MOTOR_PORT GPIO_B
#define MOTOR_PIN 6
#define MOTOR_TIMER TIMER4
#define MOTOR_CHANNEL PWM_CHANNEL_1
#define MOTOR_AF GPIO_AF2

/* ================================================================
 *  Emergency Stop Button — PA0 (EXTI0, highest priority)
 *  Both boards
 * ================================================================ */
#define EMERG_PORT GPIO_A
#define EMERG_PIN 0

/* ================================================================
 *  Reset Button — PA11 (EXTI11)
 *  Both boards
 * ================================================================ */
#define RESET_PORT GPIO_A
#define RESET_PIN 11

/* ================================================================
 *  Cabin Buttons — Internal floor requests (both boards)
 *  PC1 = Floor 0,  PC2 = Floor 1,  PC3 = Floor 2,  PC4 = Floor 3
 * ================================================================ */
#define CABIN_PORT GPIO_C
#define CABIN_F0_PIN 1
#define CABIN_F1_PIN 2
#define CABIN_F2_PIN 3
#define CABIN_F3_PIN 4

/* ================================================================
 *  Floor Sensors — Position tracking (both boards)
 *  PB12 = Floor 0, PB13 = Floor 1, PB14 = Floor 2, PB15 = Floor 3
 * ================================================================ */
#define FSENSOR_PORT GPIO_B
#define FSENSOR_F0_PIN 12
#define FSENSOR_F1_PIN 13
#define FSENSOR_F2_PIN 14
#define FSENSOR_F3_PIN 15

/* ================================================================
 *  Hallway Call Buttons — Master board only  (6 buttons)
 *  U1(Up@F0)=PB5  D2(Dn@F1)=PC6  U2(Up@F1)=PB7
 *  D3(Dn@F2)=PB8  U3(Up@F2)=PB9  D4(Dn@F3)=PB10
 * ================================================================ */
#define HALL_U1_PORT GPIO_B
#define HALL_U1_PIN 5
#define HALL_D2_PORT GPIO_C
#define HALL_D2_PIN 6
#define HALL_U2_PORT GPIO_B
#define HALL_U2_PIN 7
#define HALL_D3_PORT GPIO_B
#define HALL_D3_PIN 8
#define HALL_U3_PORT GPIO_B
#define HALL_U3_PIN 9
#define HALL_D4_PORT GPIO_B
#define HALL_D4_PIN 10

/* ================================================================
 *  NVIC IRQ Numbers (STM32F401)
 * ================================================================ */
#define IRQ_EXTI0 6
#define IRQ_EXTI1 7
#define IRQ_EXTI2 8
#define IRQ_EXTI3 9
#define IRQ_EXTI4 10
#define IRQ_EXTI9_5 23
#define IRQ_EXTI15_10 40
#define IRQ_TIM2 28
#define IRQ_TIM3 29
#define IRQ_TIM4 30
#define IRQ_TIM5 50
#define IRQ_SPI1 35
#define IRQ_USART1 37
#define IRQ_USART2 38
#define IRQ_ADC 18

/* ================================================================
 *  Timer Assignments
 *  TIM2 = 500ms UART telemetry
 *  TIM3 = 50ms IPC exchange
 *  TIM4 = Motor PWM
 *  TIM5 = Floor transition (2s per floor)
 * ================================================================ */
#define TELEMETRY_TIMER TIMER2
#define IPC_TIMER TIMER3
#define FLOOR_TIMER TIMER5

/* ================================================================
 *  Telemetry Transport
 *  Set DMA_ENABLED to 1 to use DMA for UART transmission (efficient)
 *  Set DMA_ENABLED to 0 to use blocking UART (fallback)
 * ================================================================ */
#define DMA_ENABLED 0

/* Number of floors */
#define NUM_FLOORS 4

/* PWM duty-cycle constants */
#define MOTOR_DUTY_STOP 0
#define MOTOR_DUTY_SLOW 20
#define MOTOR_DUTY_FULL 100

#endif /* BOARD_CONFIG_H */
