/**
 * Ipc.c — SPI IPC protocol implementation
 */

#include "Ipc.h"
#include "Board_Config.h"
#include "Nvic.h"
#include "Spi.h"
#include "Timer.h"

/* ================================================================
 *  Internal state
 * ================================================================ */
static uint8 G_Role;

/* Comm-fault tracking */
static volatile uint8 G_CommFaultCounter = 0;
#define COMM_FAULT_THRESHOLD 4 /* 4 × 50 ms = 200 ms timeout */

static volatile uint8 G_SlaveCommandBuf[IPC_FRAME_LEN];
static volatile uint8 G_SlaveCommandReady = 0;

/* 50 ms exchange flag (set by TIM3 ISR) */
volatile uint8 G_IpcExchangeDue = 0;

/* ================================================================
 *  Checksum
 * ================================================================ */
uint8 Ipc_ComputeChecksum(uint8 *data, uint8 len) {
  uint8 cs = 0;
  for (uint8 i = 0; i < len - 1; i++) {
    cs ^= data[i];
  }
  return cs;
}

uint8 Ipc_ValidateFrame(uint8 *data, uint8 len) {
  if (data[0] != IPC_HEADER)
    return 0;
  return (data[len - 1] == Ipc_ComputeChecksum(data, len)) ? 1 : 0;
}

/* ================================================================
 *  Timer ISR callback — sets the exchange flag every 50 ms
 * ================================================================ */
static void Ipc_TimerCallback(void) { G_IpcExchangeDue = 1; }

/* ================================================================
 *  Slave SPI RX callback — called from SPI1_IRQHandler
 * ================================================================ */
static void Ipc_SlaveRxComplete(uint8 *rxData, uint8 len) {
  /* Copy received master command into buffer */
  for (uint8 i = 0; i < len && i < IPC_FRAME_LEN; i++) {
    G_SlaveCommandBuf[i] = rxData[i];
  }
  G_SlaveCommandReady = 1;
}

/* ================================================================
 *  Init
 * ================================================================ */
void Ipc_Init(uint8 role) {
  G_Role = role;

  if (role == SPI_ROLE_MASTER) {
    Spi1_InitMaster();
  } else {
    Spi1_InitSlave();
    Spi1_SetRxCallback(Ipc_SlaveRxComplete, IPC_FRAME_LEN);

    /* Pre-load default idle status */
    uint8 idle[IPC_FRAME_LEN] = {IPC_HEADER, 0, 0, 0xFF, DIR_IDLE, 0, 0, 0};
    idle[IPC_FRAME_LEN - 1] = Ipc_ComputeChecksum(idle, IPC_FRAME_LEN);
    Spi1_SlavePreload(idle, IPC_FRAME_LEN);
  }

  /* Setup TIM3 for 50 ms periodic interrupt */
  /* PSC = 15999 → 1 kHz tick, ARR = 49 → 50 ms period */
  Timer_Init(IPC_TIMER, 15999, 49);
  Timer_SetCallback(IPC_TIMER, Ipc_TimerCallback);
  Timer_EnableInterrupt(IPC_TIMER);
  Nvic_EnableIrq(IRQ_TIM3);
  Timer_Start(IPC_TIMER);
}

/* ================================================================
 *  Master exchange
 * ================================================================ */
uint8 Ipc_MasterExchange(IpcMasterPacket *txPkt, IpcSlavePacket *rxPkt) {
  uint8 txBuf[IPC_FRAME_LEN];
  uint8 rxBuf[IPC_FRAME_LEN];

  /* Build TX frame */
  txBuf[0] = IPC_HEADER;
  txBuf[1] = txPkt->command;
  txBuf[2] = txPkt->targetFloor;
  txBuf[3] = txPkt->direction;
  txBuf[4] = txPkt->hallUpMask;
  txBuf[5] = txPkt->hallDnMask;
  txBuf[6] = 0;
  txBuf[7] = Ipc_ComputeChecksum(txBuf, IPC_FRAME_LEN);

  /* Full-duplex transfer */
  if (Spi1_MasterTransferFrame(txBuf, rxBuf, IPC_FRAME_LEN) != SPI_OK) {
    G_CommFaultCounter++;
    return 0;
  }

  /* Validate slave response */
  if (Ipc_ValidateFrame(rxBuf, IPC_FRAME_LEN)) {
    rxPkt->header = rxBuf[0];
    rxPkt->state = rxBuf[1];
    rxPkt->currentFloor = rxBuf[2];
    rxPkt->targetFloor = rxBuf[3];
    rxPkt->direction = rxBuf[4];
    rxPkt->requestMask = rxBuf[5];
    rxPkt->flags = rxBuf[6];
    rxPkt->checksum = rxBuf[7];
    G_CommFaultCounter = 0;
    return 1;
  } else {
    G_CommFaultCounter++;
  }
  return 0;
}

uint8 Ipc_IsCommFault(void) {
  return (G_CommFaultCounter >= COMM_FAULT_THRESHOLD) ? 1 : 0;
}

void Ipc_ClearCommFault(void) { G_CommFaultCounter = 0; }

/* ================================================================
 *  Slave-side functions
 * ================================================================ */
void Ipc_SlaveUpdateStatus(IpcSlavePacket *pkt) {
  uint8 buf[IPC_FRAME_LEN];

  buf[0] = IPC_HEADER;
  buf[1] = pkt->state;
  buf[2] = pkt->currentFloor;
  buf[3] = pkt->targetFloor;
  buf[4] = pkt->direction;
  buf[5] = pkt->requestMask;
  buf[6] = pkt->flags;
  buf[7] = Ipc_ComputeChecksum(buf, IPC_FRAME_LEN);

  Enter_Critical();
  Spi1_SlavePreload(buf, IPC_FRAME_LEN);
  Exit_Critical();
}

uint8 Ipc_SlaveGetCommand(IpcMasterPacket *pkt) {
  if (!G_SlaveCommandReady)
    return 0;

  Enter_Critical();
  uint8 valid = Ipc_ValidateFrame((uint8 *)G_SlaveCommandBuf, IPC_FRAME_LEN);
  if (valid) {
    pkt->header = G_SlaveCommandBuf[0];
    pkt->command = G_SlaveCommandBuf[1];
    pkt->targetFloor = G_SlaveCommandBuf[2];
    pkt->direction = G_SlaveCommandBuf[3];
    pkt->hallUpMask = G_SlaveCommandBuf[4];
    pkt->hallDnMask = G_SlaveCommandBuf[5];
    pkt->reserved = G_SlaveCommandBuf[6];
    pkt->checksum = G_SlaveCommandBuf[7];
  }
  G_SlaveCommandReady = 0;
  Exit_Critical();

  return valid;
}
