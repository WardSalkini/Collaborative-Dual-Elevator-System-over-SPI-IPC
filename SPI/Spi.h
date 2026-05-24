/**
 * Spi.h — SPI1 driver for dual-elevator IPC
 *
 * Supports both Master and Slave roles on PA4-PA7.
 */

#ifndef SPI_H
#define SPI_H

#include "Std_Types.h"

/* Role constants */
#define SPI_ROLE_MASTER  1U
#define SPI_ROLE_SLAVE   0U

/* Return codes */
#define SPI_OK     0U
#define SPI_NOK    1U

/* Callback type for slave RX complete (full frame received) */
typedef void (*SpiRxCallback)(uint8 *RxData, uint8 Len);

/**
 * @brief  Initialise SPI1 as Master.
 *         PA5=SCK, PA6=MISO, PA7=MOSI (AF5), PA4=CS (GPIO output, idle HIGH).
 *         CPOL=0, CPHA=0, 8-bit, MSB-first, baud = fPCLK/4.
 */
void Spi1_InitMaster(void);

/**
 * @brief  Initialise SPI1 as Slave.
 *         PA5=SCK, PA6=MISO, PA7=MOSI, PA4=NSS (all AF5).
 *         Enables RXNE interrupt for non-blocking reception.
 */
void Spi1_InitSlave(void);

/**
 * @brief  (Master) Assert chip-select (PA4 LOW).
 */
void Spi1_CS_Low(void);

/**
 * @brief  (Master) De-assert chip-select (PA4 HIGH).
 */
void Spi1_CS_High(void);

/**
 * @brief  (Master) Full-duplex transfer of a frame.
 *         Asserts CS, exchanges Len bytes, de-asserts CS.
 * @return SPI_OK on success.
 */
uint8 Spi1_MasterTransferFrame(uint8 *TxBuf, uint8 *RxBuf, uint8 Len);

/**
 * @brief  (Slave) Pre-load the TX buffer that will be shifted out
 *         when the Master clocks in data.
 */
void Spi1_SlavePreload(uint8 *TxBuf, uint8 Len);

/**
 * @brief  (Slave) Register a callback invoked from SPI1_IRQHandler
 *         once a complete frame has been received.
 */
void Spi1_SetRxCallback(SpiRxCallback Cb, uint8 FrameLen);

/**
 * @brief  Legacy single-byte full-duplex transfer (kept for compatibility).
 */
uint8 Spi1_TransmitReceiveByte(uint8 TxData, uint8 *RxData);

#endif /* SPI_H */
