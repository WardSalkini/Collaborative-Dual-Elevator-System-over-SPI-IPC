/**
 * Ipc.h — SPI IPC protocol for dual-elevator communication
 *
 * 8-byte full-duplex packet exchanged every 50 ms.
 */

#ifndef IPC_H
#define IPC_H

#include "Std_Types.h"

#define IPC_FRAME_LEN   8U
#define IPC_HEADER      0xA5U

/* ================================================================
 *  Command codes  (Master → Slave)
 * ================================================================ */
#define IPC_CMD_STATUS_REQ     0x00U   /* Just exchange status      */
#define IPC_CMD_ASSIGN_TARGET  0x01U   /* Assign floor + direction  */
#define IPC_CMD_EMERGENCY      0x02U   /* Force emergency stop      */
#define IPC_CMD_CLEAR_EMERG    0x03U   /* Clear emergency           */

/* ================================================================
 *  Direction constants (shared with elevator FSM)
 * ================================================================ */
#define DIR_IDLE   0U
#define DIR_UP     1U
#define DIR_DOWN   2U

/* ================================================================
 *  Packet structures
 * ================================================================ */

/* Master → Slave */
typedef struct {
    uint8 header;       /* 0xA5                                  */
    uint8 command;      /* IPC_CMD_*                             */
    uint8 targetFloor;  /* 0-3, or 0xFF = none                   */
    uint8 direction;    /* DIR_UP / DIR_DOWN / DIR_IDLE           */
    uint8 hallUpMask;   /* bit N = up call pending at floor N     */
    uint8 hallDnMask;   /* bit N = down call pending at floor N   */
    uint8 reserved;
    uint8 checksum;     /* XOR of bytes 0-6                      */
} IpcMasterPacket;

/* Slave → Master */
typedef struct {
    uint8 header;       /* 0xA5                                  */
    uint8 state;        /* Elevator FSM state                    */
    uint8 currentFloor; /* 0-3                                   */
    uint8 targetFloor;  /* 0-3 or 0xFF                           */
    uint8 direction;    /* DIR_UP / DIR_DOWN / DIR_IDLE           */
    uint8 requestMask;  /* cabin-button request bitmask           */
    uint8 flags;        /* bit0=emergency, bit1=busy             */
    uint8 checksum;     /* XOR of bytes 0-6                      */
} IpcSlavePacket;


/* ================================================================
 *  API
 * ================================================================ */

/**
 * @brief  Initialise IPC layer.
 * @param  role  SPI_ROLE_MASTER or SPI_ROLE_SLAVE  (from Spi.h)
 */
void Ipc_Init(uint8 role);

/* --- Master-side ------------------------------------------------ */

/**
 * @brief  Send a command to the slave and receive its status.
 *         Called from the 50 ms timer ISR flag handler in main loop.
 */
uint8 Ipc_MasterExchange(IpcMasterPacket *txPkt, IpcSlavePacket *rxPkt);


/** @brief  Returns 1 if no valid slave response within timeout. */
uint8 Ipc_IsCommFault(void);

/** @brief  Reset the comm-fault counter (called on valid packet). */
void Ipc_ClearCommFault(void);

/* --- Slave-side ------------------------------------------------- */

/**
 * @brief  Update the status packet that will be sent on next exchange.
 *         Must be called with interrupts disabled (critical section).
 */
void Ipc_SlaveUpdateStatus(IpcSlavePacket *pkt);

/**
 * @brief  Get the latest command received from master.
 *         Returns 1 if a new command is available.
 */
uint8 Ipc_SlaveGetCommand(IpcMasterPacket *pkt);

/* --- Shared ----------------------------------------------------- */

/** @brief  Compute XOR checksum of bytes 0..(len-2). */
uint8 Ipc_ComputeChecksum(uint8 *data, uint8 len);

/** @brief  Validate header and checksum of a raw frame. */
uint8 Ipc_ValidateFrame(uint8 *data, uint8 len);


#endif /* IPC_H */
