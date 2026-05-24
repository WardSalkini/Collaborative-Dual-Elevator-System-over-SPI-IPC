/**
 * Dispatcher.c — Mandated task-allocation algorithm
 *
 * Scoring rules (lower is better):
 *   0   = Immediate (at floor, idle)
 *   1-9 = Perfect match (same direction, approaching)
 *  50+  = Idle fallback (distance-based)
 * 100+  = Passed match (same dir, already passed)
 * 200   = Opposite direction (do not assign now)
 * 255   = Unavailable (comm fault / emergency)
 */

#include "Dispatcher.h"
#include "Board_Config.h"
#include "Ipc.h"

static ElevatorContext *G_ElevA = (void *)0;
static ElevatorContext *G_ElevB = (void *)0;
static volatile uint8 G_CommFault = 0;
static Dispatcher_RemoteAssignCb G_RemoteAssignCb = (void *)0;
static uint8 G_LastAssigned = 0; /* 0 = A was last, 1 = B was last */

void Dispatcher_Init(ElevatorContext *elevA, ElevatorContext *elevB) {
  G_ElevA = elevA;
  G_ElevB = elevB;
}

void Dispatcher_SetCommFault(uint8 isFault) { G_CommFault = isFault; }

void Dispatcher_SetRemoteAssignCallback(Dispatcher_RemoteAssignCb cb) {
  G_RemoteAssignCb = cb;
}

/* ================================================================
 *  Score a single elevator against a hall call
 * ================================================================ */
static uint8 ScoreElevator(ElevatorContext *elev, uint8 callFloor,
                           uint8 callDir) {
  if (elev->state == ELEV_EMERGENCY)
    return 255;

  if (elev->state == ELEV_IDLE && elev->currentFloor == callFloor)
    return 0;

  if (elev->state == ELEV_IDLE) {
    uint8 dist = (callFloor > elev->currentFloor)
                     ? (callFloor - elev->currentFloor)
                     : (elev->currentFloor - callFloor);
    return 50 + dist;
  }

  /* Elevator is moving */
  if (elev->direction == callDir) {
    uint8 dist = (callFloor > elev->currentFloor)
                     ? (callFloor - elev->currentFloor)
                     : (elev->currentFloor - callFloor);

    uint8 approaching =
        (callDir == DIR_UP && elev->currentFloor <= callFloor) ||
        (callDir == DIR_DOWN && elev->currentFloor >= callFloor);

    return approaching ? (1 + dist) : (100 + dist);
  }

  return 200; /* moving in opposite direction */
}

/* ================================================================
 *  Dispatch
 * ================================================================ */
void Dispatcher_HandleHallCall(uint8 floor, uint8 direction) {
  if (floor >= NUM_FLOORS)
    return;

  /* Rule 1: Comm fault → Master takes all calls */
  if (G_CommFault) {
    Elevator_AddRequest(G_ElevA, floor);
    return;
  }

  uint8 scoreA = ScoreElevator(G_ElevA, floor, direction);
  uint8 scoreB = ScoreElevator(G_ElevB, floor, direction);

  /* ↓↓↓ REPLACE EVERYTHING FROM HERE ↓↓↓ */

  if (scoreA == 255 && scoreB == 255)
    return; /* both unavailable */

  if (scoreA < scoreB || (scoreA == scoreB && G_LastAssigned == 1)) {
    Elevator_AddRequest(G_ElevA, floor);
    G_LastAssigned = 0;
  } else {
    if (G_RemoteAssignCb != (void *)0)
      G_RemoteAssignCb(floor);
    else
      Elevator_AddRequest(G_ElevB, floor);
    G_LastAssigned = 1;
  }
}