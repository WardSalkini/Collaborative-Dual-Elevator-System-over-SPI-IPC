/**
 * State_Machine.c — Elevator FSM implementation
 *
 * Timer-based floor simulation: TIM5 ticks every 50 ms.
 * Floor transit = 5 ticks = 250 ms.
 * Door open = 3 ticks = 150 ms.
 */

#include "State_Machine.h"
#include "Board_Config.h"
#include "Pwm.h"
#include "Timer.h"

/* Timing constants (in ticks of the 50 ms floor-timer) */
#define FLOOR_TRANSIT_TICKS 5 /* 250 ms per floor  */
#define DOOR_OPEN_TICKS 3     /* 150 ms door open  */

/* ================================================================
 *  Helpers
 * ================================================================ */

static uint8 NextRequestedFloor(ElevatorContext *ctx) {
  /* If going UP, find the nearest requested floor above */
  if (ctx->direction == DIR_UP) {
    for (uint8 f = ctx->currentFloor; f < NUM_FLOORS; f++) {
      if (ctx->requestMask & (1U << f))
        return f;
    }
  }
  /* If going DOWN, find the nearest requested floor below */
  if (ctx->direction == DIR_DOWN) {
    for (sint8 f = (sint8)ctx->currentFloor; f >= 0; f--) {
      if (ctx->requestMask & (1U << (uint8)f))
        return (uint8)f;
    }
  }

  /* General: find nearest requested floor */
  uint8 bestFloor = 0xFF;
  uint8 bestDist = 0xFF;
  for (uint8 f = 0; f < NUM_FLOORS; f++) {
    if (ctx->requestMask & (1U << f)) {
      uint8 dist = (f > ctx->currentFloor) ? (f - ctx->currentFloor)
                                           : (ctx->currentFloor - f);
      if (dist < bestDist) {
        bestDist = dist;
        bestFloor = f;
      }
    }
  }
  return bestFloor;
}

static void SetMotorDuty(uint8 duty) {
  Pwm_SetDutyPercent(MOTOR_TIMER, MOTOR_CHANNEL, duty);
}

/* ================================================================
 *  Public API
 * ================================================================ */

void Elevator_Init(ElevatorContext *ctx) {
  ctx->state = ELEV_IDLE;
  ctx->currentFloor = 0;
  ctx->targetFloor = 0xFF;
  ctx->direction = DIR_IDLE;
  ctx->requestMask = 0;
  ctx->emergencyStop = 0;
  ctx->doorTimerActive = 0;
  ctx->floorTimerActive = 0;
  ctx->doorTickCount = 0;
  ctx->floorTickCount = 0;

  SetMotorDuty(MOTOR_DUTY_STOP);
}

void Elevator_AddRequest(ElevatorContext *ctx, uint8 floor) {
  if (floor < NUM_FLOORS) {
    ctx->requestMask |= (1U << floor);
  }
}

void Elevator_EmergencyStop(ElevatorContext *ctx) {
  ctx->emergencyStop = 1;
  ctx->state = ELEV_EMERGENCY;
  SetMotorDuty(MOTOR_DUTY_STOP);
}

void Elevator_EmergencyClear(ElevatorContext *ctx) {
  ctx->emergencyStop = 0;
  ctx->state = ELEV_IDLE;
}

void Elevator_FloorReached(ElevatorContext *ctx, uint8 floor) {
  ctx->currentFloor = floor;
}

void Elevator_Tick(ElevatorContext *ctx) {
  /* Emergency overrides everything */
  if (ctx->emergencyStop) {
    SetMotorDuty(MOTOR_DUTY_STOP);
    ctx->state = ELEV_EMERGENCY;
    return;
  }

  switch (ctx->state) {

  /* ---- IDLE ------------------------------------------------ */
  case ELEV_IDLE:
    SetMotorDuty(MOTOR_DUTY_STOP);
    ctx->direction = DIR_IDLE;

    if (ctx->requestMask != 0) {
      ctx->targetFloor = NextRequestedFloor(ctx);

      if (ctx->targetFloor == ctx->currentFloor) {
        /* Already here — open doors */
        ctx->requestMask &= ~(1U << ctx->currentFloor);
        ctx->state = ELEV_DOORS_OPEN;
        ctx->doorTickCount = 0;
        ctx->doorTimerActive = 1;
      } else if (ctx->targetFloor < ctx->currentFloor) {
        ctx->direction = DIR_DOWN;
        ctx->state = ELEV_MOVING_DOWN;
        ctx->floorTickCount = 0;
        ctx->floorTimerActive = 1;
        SetMotorDuty(MOTOR_DUTY_FULL);
      } else if (ctx->targetFloor != 0xFF) {
        ctx->direction = DIR_UP;
        ctx->state = ELEV_MOVING_UP;
        ctx->floorTickCount = 0;
        ctx->floorTimerActive = 1;
        SetMotorDuty(MOTOR_DUTY_FULL);
      }
    }
    break;

  /* ---- MOVING UP ------------------------------------------- */
  case ELEV_MOVING_UP:
    if (ctx->floorTimerActive) {
      ctx->floorTickCount++;

      /* Approaching next floor — slow down near target */
      if (ctx->currentFloor + 1 == ctx->targetFloor &&
          ctx->floorTickCount >= FLOOR_TRANSIT_TICKS / 2) {
        SetMotorDuty(MOTOR_DUTY_SLOW);
      }

      if (ctx->floorTickCount >= FLOOR_TRANSIT_TICKS) {
        /* Arrived at next floor */
        ctx->currentFloor++;
        ctx->floorTickCount = 0;

        /* Should we stop here? */
        if (ctx->requestMask & (1U << ctx->currentFloor)) {
          ctx->requestMask &= ~(1U << ctx->currentFloor);
          ctx->state = ELEV_DOORS_OPEN;
          ctx->doorTickCount = 0;
          ctx->doorTimerActive = 1;
          ctx->floorTimerActive = 0;
          SetMotorDuty(MOTOR_DUTY_STOP);
        } else if (ctx->currentFloor >= ctx->targetFloor) {
          /* Reached target with no request — go idle */
          ctx->state = ELEV_IDLE;
          ctx->floorTimerActive = 0;
          ctx->targetFloor = 0xFF;
          SetMotorDuty(MOTOR_DUTY_STOP);
        } else {
          SetMotorDuty(MOTOR_DUTY_FULL);
        }
      }
    }
    break;

  /* ---- MOVING DOWN ----------------------------------------- */
  case ELEV_MOVING_DOWN:
    if (ctx->floorTimerActive) {
      ctx->floorTickCount++;

      /* Slow down when approaching target */
      if (ctx->currentFloor > 0 && ctx->currentFloor - 1 == ctx->targetFloor &&
          ctx->floorTickCount >= FLOOR_TRANSIT_TICKS / 2) {
        SetMotorDuty(MOTOR_DUTY_SLOW);
      }

      if (ctx->floorTickCount >= FLOOR_TRANSIT_TICKS) {
        ctx->currentFloor--;
        ctx->floorTickCount = 0;

        if (ctx->requestMask & (1U << ctx->currentFloor)) {
          ctx->requestMask &= ~(1U << ctx->currentFloor);
          ctx->state = ELEV_DOORS_OPEN;
          ctx->doorTickCount = 0;
          ctx->doorTimerActive = 1;
          ctx->floorTimerActive = 0;
          SetMotorDuty(MOTOR_DUTY_STOP);
        } else if (ctx->currentFloor <= ctx->targetFloor) {
          ctx->state = ELEV_IDLE;
          ctx->floorTimerActive = 0;
          ctx->targetFloor = 0xFF;
          SetMotorDuty(MOTOR_DUTY_STOP);
        } else {
          SetMotorDuty(MOTOR_DUTY_FULL);
        }
      }
    }
    break;

  /* ---- DOORS OPEN ------------------------------------------ */
  case ELEV_DOORS_OPEN:
    SetMotorDuty(MOTOR_DUTY_STOP);
    if (ctx->doorTimerActive) {
      ctx->doorTickCount++;
      if (ctx->doorTickCount >= DOOR_OPEN_TICKS) {
        ctx->doorTimerActive = 0;
        ctx->doorTickCount = 0;
        /* Check if more requests pending */
        if (ctx->requestMask != 0) {
          ctx->targetFloor = NextRequestedFloor(ctx);
          if (ctx->targetFloor > ctx->currentFloor) {
            ctx->direction = DIR_UP;
            ctx->state = ELEV_MOVING_UP;
            ctx->floorTimerActive = 1;
            ctx->floorTickCount = 0;
            SetMotorDuty(MOTOR_DUTY_FULL);
          } else if (ctx->targetFloor < ctx->currentFloor) {
            ctx->direction = DIR_DOWN;
            ctx->state = ELEV_MOVING_DOWN;
            ctx->floorTimerActive = 1;
            ctx->floorTickCount = 0;
            SetMotorDuty(MOTOR_DUTY_FULL);
          } else {
            ctx->requestMask &= ~(1U << ctx->currentFloor);
            ctx->doorTickCount = 0;
            ctx->doorTimerActive = 1;
          }
        } else {
          ctx->state = ELEV_IDLE;
          ctx->targetFloor = 0xFF;
          ctx->direction = DIR_IDLE;
        }
      }
    }
    break;

  /* ---- EMERGENCY ------------------------------------------- */
  case ELEV_EMERGENCY:
    SetMotorDuty(MOTOR_DUTY_STOP);
    ctx->state = ELEV_EMERGENCY;
    /* Stay until cleared externally */
    break;
  }
}

const char *Elevator_StateStr(uint8 state) {
  switch (state) {
  case ELEV_IDLE:
    return "IDLE";
  case ELEV_MOVING_UP:
    return "UP";
  case ELEV_MOVING_DOWN:
    return "DOWN";
  case ELEV_DOORS_OPEN:
    return "DOOR";
  case ELEV_EMERGENCY:
    return "EMRG";
  default:
    return "????";
  }
}
