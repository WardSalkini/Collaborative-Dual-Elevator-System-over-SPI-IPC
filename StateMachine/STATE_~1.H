/**
 * State_Machine.h — Elevator FSM for dual-elevator system
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "Std_Types.h"

/* ================================================================
 *  Elevator states
 * ================================================================ */
#define ELEV_IDLE          0U
#define ELEV_MOVING_UP     1U
#define ELEV_MOVING_DOWN   2U
#define ELEV_DOORS_OPEN    3U
#define ELEV_EMERGENCY     4U

/* ================================================================
 *  Direction constants  (shared with Ipc.h)
 * ================================================================ */
#ifndef DIR_IDLE
#define DIR_IDLE   0U
#define DIR_UP     1U
#define DIR_DOWN   2U
#endif

/* ================================================================
 *  Elevator context — all volatile for ISR/main sharing
 * ================================================================ */
typedef struct {
    volatile uint8 state;
    volatile uint8 currentFloor;    /* 0-3 */
    volatile uint8 targetFloor;     /* 0-3, 0xFF = none */
    volatile uint8 direction;
    volatile uint8 requestMask;     /* bit N = floor N requested */
    volatile uint8 emergencyStop;
    volatile uint8 doorTimerActive;
    volatile uint8 floorTimerActive;
    volatile uint32 doorTickCount;
    volatile uint32 floorTickCount;
} ElevatorContext;

/**
 * @brief  Initialise an elevator context (starts IDLE at floor 0).
 */
void Elevator_Init(ElevatorContext *ctx);

/**
 * @brief  Add a floor request (from cabin button or dispatcher).
 */
void Elevator_AddRequest(ElevatorContext *ctx, uint8 floor);

/**
 * @brief  Trigger emergency stop.
 */
void Elevator_EmergencyStop(ElevatorContext *ctx);

/**
 * @brief  Clear emergency and resume.
 */
void Elevator_EmergencyClear(ElevatorContext *ctx);

/**
 * @brief  Called on floor sensor trigger — update position.
 */
void Elevator_FloorReached(ElevatorContext *ctx, uint8 floor);

/**
 * @brief  Periodic tick (called from main loop or timer).
 *         Handles state transitions, motor speed, door timing.
 */
void Elevator_Tick(ElevatorContext *ctx);

/**
 * @brief  Get a string representation of the current state.
 */
const char *Elevator_StateStr(uint8 state);

#endif /* STATE_MACHINE_H */
