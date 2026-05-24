/**
 * Spi_Private.h — SPI1 register map for STM32F401
 */

#ifndef SPI_PRIVATE_H
#define SPI_PRIVATE_H

#include "Std_Types.h"

/* SPI register map (STM32F401) */
typedef struct {
  volatile uint32 CR1;     /* 0x00 - Control register 1 */
  volatile uint32 CR2;     /* 0x04 - Control register 2 */
  volatile uint32 SR;      /* 0x08 - Status register    */
  volatile uint32 DR;      /* 0x0C - Data register      */
  volatile uint32 CRCPR;   /* 0x10 - CRC polynomial register */
  volatile uint32 RXCRCR;  /* 0x14 - RX CRC register    */
  volatile uint32 TXCRCR;  /* 0x18 - TX CRC register    */
  volatile uint32 I2SCFGR; /* 0x1C - I2S configuration register */
  volatile uint32 I2SPR;   /* 0x20 - I2S prescaler register */
} SpiType;

#define SPI1_BASE_ADDR 0x40013000UL
#define SPI1           ((SpiType *)SPI1_BASE_ADDR)

/* CR1 bit positions */
#define SPI_CR1_CPHA_Pos      0U
#define SPI_CR1_CPOL_Pos      1U
#define SPI_CR1_MSTR_Pos      2U
#define SPI_CR1_BR_Pos        3U
#define SPI_CR1_SPE_Pos       6U
#define SPI_CR1_LSBFIRST_Pos  7U
#define SPI_CR1_SSI_Pos       8U
#define SPI_CR1_SSM_Pos       9U
#define SPI_CR1_RXONLY_Pos    10U
#define SPI_CR1_DFF_Pos       11U
#define SPI_CR1_CRCNEXT_Pos   12U
#define SPI_CR1_CRCEN_Pos     13U
#define SPI_CR1_BIDIOE_Pos    14U
#define SPI_CR1_BIDIMODE_Pos  15U

/* CR2 bit positions */
#define SPI_CR2_RXDMAEN_Pos   0U
#define SPI_CR2_TXDMAEN_Pos   1U
#define SPI_CR2_SSOE_Pos      2U
#define SPI_CR2_FRF_Pos       4U
#define SPI_CR2_ERRIE_Pos     5U
#define SPI_CR2_RXNEIE_Pos    6U
#define SPI_CR2_TXEIE_Pos     7U

/* SR bit positions */
#define SPI_SR_RXNE_Pos       0U
#define SPI_SR_TXE_Pos        1U
#define SPI_SR_CHSIDE_Pos     2U
#define SPI_SR_UDR_Pos        3U
#define SPI_SR_CRCERR_Pos     4U
#define SPI_SR_MODF_Pos       5U
#define SPI_SR_OVR_Pos        6U
#define SPI_SR_BSY_Pos        7U
#define SPI_SR_FRE_Pos        8U

#endif /* SPI_PRIVATE_H */
