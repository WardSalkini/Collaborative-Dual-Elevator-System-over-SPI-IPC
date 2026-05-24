/**
 * Spi.c — SPI1 driver (Master + Slave) for dual-elevator IPC
 *
 * Pins: PA4=NSS  PA5=SCK  PA6=MISO  PA7=MOSI   (AF5)
 */

#include "Spi.h"
#include "Bit_Math.h"
#include "Gpio.h"
#include "Nvic.h"
#include "Spi_Private.h"

/* ----------------------------------------------------------------
 *  Slave ISR state
 * ---------------------------------------------------------------- */
/* Slave context */
static volatile uint8 G_SlaveTxBuf[16];
static volatile uint8 G_SlaveRxBuf[16];
static volatile uint8 G_SlaveLen = 0;
static volatile uint8 G_SlaveIdx = 0;

/* Double buffer for race-free preloading */
static volatile uint8 G_SlaveNextTxBuf[16];
static volatile uint8 G_SlaveNextLen = 0;
static volatile uint8 G_HasNextTxBuf = 0;

static volatile uint8 G_FrameReady = 0;

static SpiRxCallback G_RxCallback = (void *)0;
static uint8 G_RxFrameLen = 0;

/* ================================================================
 *  Common GPIO helpers
 * ================================================================ */
static void Spi1_InitPins_AF(void) {
  /* PA5 = SCK, PA6 = MISO, PA7 = MOSI  → AF5 push-pull */
  Gpio_Init(GPIO_A, 5, GPIO_AF, GPIO_PUSH_PULL);
  Gpio_Init(GPIO_A, 6, GPIO_AF, GPIO_PUSH_PULL);
  Gpio_Init(GPIO_A, 7, GPIO_AF, GPIO_PUSH_PULL);

  Gpio_SetAF(GPIO_A, 5, GPIO_AF5);
  Gpio_SetAF(GPIO_A, 6, GPIO_AF5);
  Gpio_SetAF(GPIO_A, 7, GPIO_AF5);
}

/* ================================================================
 *  Master
 * ================================================================ */
void Spi1_InitMaster(void) {
  Spi1_InitPins_AF();

  /* PA4 = CS as GPIO output, idle HIGH */
  Gpio_Init(GPIO_A, 4, GPIO_OUTPUT, GPIO_PUSH_PULL);
  Gpio_WritePin(GPIO_A, 4, HIGH);

  /* Reset CR1 */
  SPI1->CR1 = 0;

  /* Software slave management, SSI=1 (keeps NSS high internally) */
  SPI1->CR1 |= (1UL << SPI_CR1_SSM_Pos);
  SPI1->CR1 |= (1UL << SPI_CR1_SSI_Pos);

  /* Master mode */
  SPI1->CR1 |= (1UL << SPI_CR1_MSTR_Pos);

  /* Baud = fPCLK2 / 256  (BR[2:0] = 111) → 16MHz/256 = 62.5kHz */
  SPI1->CR1 &= ~(0x7UL << SPI_CR1_BR_Pos);
  SPI1->CR1 |= (0x7UL << SPI_CR1_BR_Pos);

  /* CPOL=0, CPHA=0, 8-bit, MSB first — all zero, already set */

  /* Enable SPI */
  SPI1->CR1 |= (1UL << SPI_CR1_SPE_Pos);
}

/* ================================================================
 *  Slave
 * ================================================================ */
void Spi1_InitSlave(void) {
  Spi1_InitPins_AF();

  /* PA4 = Hardware NSS input → AF5 */
  Gpio_Init(GPIO_A, 4, GPIO_AF, GPIO_PUSH_PULL);
  Gpio_SetAF(GPIO_A, 4, GPIO_AF5);

  /* Reset CR1 */
  SPI1->CR1 = 0;

  /* Hardware NSS: SSM=0 (use the external NSS pin) */
  /* MSTR=0 → slave mode (already zero) */

  /* CPOL=0, CPHA=0, 8-bit, MSB first — defaults */

  /* Enable RXNE interrupt in CR2 */
  SPI1->CR2 |= (1UL << SPI_CR2_RXNEIE_Pos);

  /* Enable SPI */
  SPI1->CR1 |= (1UL << SPI_CR1_SPE_Pos);
}

/* ================================================================
 *  Master CS control
 * ================================================================ */
void Spi1_CS_Low(void) { Gpio_WritePin(GPIO_A, 4, LOW); }

void Spi1_CS_High(void) { Gpio_WritePin(GPIO_A, 4, HIGH); }

/* ================================================================
 *  Master frame transfer with timeout protection
 * ================================================================ */
#define SPI_TIMEOUT 50000UL

uint8 Spi1_MasterTransferFrame(uint8 *TxBuf, uint8 *RxBuf, uint8 Len) {
  uint32 timeout;

  Spi1_CS_Low();

  for (uint8 i = 0; i < Len; i++) {
    /* Wait for TXE with timeout */
    timeout = SPI_TIMEOUT;
    while (!(SPI1->SR & (1UL << SPI_SR_TXE_Pos))) {
      if (--timeout == 0) {
        Spi1_CS_High();
        return SPI_NOK;
      }
    }
    SPI1->DR = TxBuf[i];

    /* Wait for RXNE with timeout */
    timeout = SPI_TIMEOUT;
    while (!(SPI1->SR & (1UL << SPI_SR_RXNE_Pos))) {
      if (--timeout == 0) {
        Spi1_CS_High();
        return SPI_NOK;
      }
    }
    RxBuf[i] = (uint8)SPI1->DR;
  }

  /* Wait until not busy with timeout */
  timeout = SPI_TIMEOUT;
  while (SPI1->SR & (1UL << SPI_SR_BSY_Pos)) {
    if (--timeout == 0)
      break;
  }

  Spi1_CS_High();
  return SPI_OK;
}

/* ================================================================
 *  Slave pre-load & callback registration
 * ================================================================ */
void Spi1_SlavePreload(uint8 *TxBuf, uint8 Len) {
  Enter_Critical();
  for (uint8 i = 0; i < Len && i < 16; i++) {
    G_SlaveNextTxBuf[i] = TxBuf[i];
  }
  G_SlaveNextLen = Len;
  G_HasNextTxBuf = 1;

  /* If not currently mid-transfer, apply immediately */
  if (G_SlaveIdx == 0) {
    for (uint8 i = 0; i < G_SlaveNextLen && i < 16; i++) {
      G_SlaveTxBuf[i] = G_SlaveNextTxBuf[i];
    }
    G_SlaveLen = G_SlaveNextLen;
    G_HasNextTxBuf = 0;
    /* Pre-load first byte into DR */
    SPI1->DR = G_SlaveTxBuf[0];
  }

  Exit_Critical();
}

void Spi1_SetRxCallback(SpiRxCallback Cb, uint8 FrameLen) {
  G_RxCallback = Cb;
  G_RxFrameLen = FrameLen;
}

/* ================================================================
 *  SPI1 IRQ Handler — Slave byte-by-byte reception
 * ================================================================ */
void SPI1_IRQHandler(void) {
  if (SPI1->SR & (1UL << SPI_SR_RXNE_Pos)) {
    uint8 received = (uint8)SPI1->DR;

    /* Auto-sync: If we receive the IPC header (0xA5) but we think we are in the
     * middle of a frame, we are out of sync. Reset the software index. */
    if (received == 0xA5 && G_SlaveIdx != 0) {
      G_SlaveIdx = 0;
    }

    if (G_SlaveIdx < G_SlaveLen) {
      G_SlaveRxBuf[G_SlaveIdx] = received;
      G_SlaveIdx++;

      /* Load next TX byte */
      if (G_SlaveIdx < G_SlaveLen) {
        SPI1->DR = G_SlaveTxBuf[G_SlaveIdx];
      }
    }

    /* Full frame received? */
    if (G_SlaveIdx >= G_RxFrameLen && G_RxFrameLen > 0) {
      if (G_RxCallback != (void *)0) {
        G_RxCallback((uint8 *)G_SlaveRxBuf, G_RxFrameLen);
      }

      /* Apply pending double-buffer to avoid race conditions */
      if (G_HasNextTxBuf) {
        for (uint8 i = 0; i < G_SlaveNextLen && i < 16; i++) {
          G_SlaveTxBuf[i] = G_SlaveNextTxBuf[i];
        }
        G_SlaveLen = G_SlaveNextLen;
        G_HasNextTxBuf = 0;
      }

      /* Reset for next frame and re-preload byte 0 */
      G_SlaveIdx = 0;
      SPI1->DR = G_SlaveTxBuf[0];
    }
  }

  /* Clear overrun if it occurred (read DR then SR) */
  if (SPI1->SR & (1UL << SPI_SR_OVR_Pos)) {
    volatile uint32 dummy = SPI1->DR;
    dummy = SPI1->SR;
    (void)dummy;
  }
}

/* ================================================================
 *  Legacy single-byte transfer (kept for backward compatibility)
 * ================================================================ */
uint8 Spi1_TransmitReceiveByte(uint8 TxData, uint8 *RxData) {
  if (SPI1->SR & (1UL << SPI_SR_TXE_Pos)) {
    SPI1->DR = TxData;
    while (SPI1->SR & (1UL << SPI_SR_BSY_Pos))
      ;
    *RxData = (uint8)SPI1->DR;
    return SPI_OK;
  }
  return SPI_NOK;
}
