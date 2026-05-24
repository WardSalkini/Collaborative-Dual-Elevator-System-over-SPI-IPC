/**
 * Dispatcher.h — Master-side task-allocation algorithm
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "Std_Types.h"
#include "State_Machine.h"

typedef void (*Dispatcher_RemoteAssignCb)(uint8 floor);

/**
 * @brief  Initialise the dispatcher with pointers to both elevators.
 * @param  elevA  Local elevator A context (Master board)
 * @param  elevB  Remote elevator B status (from IPC slave packet)
 */
void Dispatcher_Init(ElevatorContext *elevA, ElevatorContext *elevB);

/**
 * @brief  Set a callback for assigning a call to the remote elevator.
 *         If not set, the dispatcher will write to elevB directly.
 */
void Dispatcher_SetRemoteAssignCallback(Dispatcher_RemoteAssignCb cb);

/**
 * @brief  Handle a hallway call.  Runs the mandated scoring algorithm
 *         and dispatches to the best elevator.
 * @param  floor      0-3
 * @param  direction  DIR_UP or DIR_DOWN
 */
void Dispatcher_HandleHallCall(uint8 floor, uint8 direction);

/**
 * @brief  Must be called when comm-fault status changes so the
 *         dispatcher knows whether Elevator B is available.
 */
void Dispatcher_SetCommFault(uint8 isFault);

#endif /* DISPATCHER_H */
