// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// CS4271 Codec support.

#include "drivers/codec.h"

#define CODEC_RESET_RCC RCC_AHB1Periph_GPIOB
#define CODEC_RESET_PIN GPIO_Pin_4
#define CODEC_RESET_GPIO GPIOB

#define CODEC_I2C                      I2C1
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_8
#define CODEC_I2C_SDA_PIN              GPIO_Pin_9
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource8
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource9
#define CODEC_TIMEOUT                  ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_TIMEOUT))
#define CODEC_I2C_SPEED                50000

#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              0x40003C0C
#define CODEC_I2S_EXT_ADDRESS          0x4000400C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2Sext_GPIO_AF           GPIO_AF_I2S3ext // DAN: ((uint8_t)0x05)
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_EXT_IRQ              SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC)
#define CODEC_I2S_WS_PIN               GPIO_Pin_4
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource4
#define CODEC_I2S_WS_GPIO              GPIOA
#define CODEC_I2S_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource10
#define CODEC_I2S_SCK_GPIO             GPIOC
#define CODEC_I2S_SDI_PIN              GPIO_Pin_11
#define CODEC_I2S_SDI_PINSRC           GPIO_PinSource11
#define CODEC_I2S_SDI_GPIO             GPIOC
#define CODEC_I2S_SDO_PIN              GPIO_Pin_12
#define CODEC_I2S_SDO_PINSRC           GPIO_PinSource12
#define CODEC_I2S_SDO_GPIO             GPIOC
#define CODEC_I2S_MCK_PIN              GPIO_Pin_7
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource7
#define CODEC_I2S_MCK_GPIO             GPIOC
#define AUDIO_I2S_IRQHandler           SPI3_IRQHandler

#define AUDIO_DMA_PERIPH_DATA_SIZE     DMA_PeripheralDataSize_HalfWord
#define AUDIO_DMA_MEM_DATA_SIZE        DMA_MemoryDataSize_HalfWord
#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF0
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF0
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF0
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF0
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF0
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_2
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream2_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2
#define AUDIO_I2S_EXT_DMA_REG          DMA1
#define AUDIO_I2S_EXT_DMA_ISR          LISR
#define AUDIO_I2S_EXT_DMA_IFCR         LIFCR

/* we only initialize the first 6 registers, the 7th is for
   pre/post-init and the 8th is read-only */
#define CS4271_NUM_REGS 6
#define CS4271_ADDR_0 0b0010000
#define CODEC_ADDRESS           (CS4271_ADDR_0<<1)

#define WAIT_LONG(x) { \
  uint32_t timeout = CODEC_LONG_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return false; } \
}

#define WAIT(x) { \
  uint32_t timeout = CODEC_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return false; } \
}

namespace mtd {

Codec* Codec::instance_;

#define CS4271_REG_MODECTRL1	1
#define CS4271_REG_DACCTRL		2
#define CS4271_REG_DACMIX		3
#define CS4271_REG_DACAVOL		4
#define CS4271_REG_DACBVOL		5
#define CS4271_REG_ADCCTRL		6
#define CS4271_REG_MODECTRL2	7
#define CS4271_REG_CHIPID		8	/*Read-only*/

//Reg 1 (MODECTRL1):
#define SINGLE_SPEED		(0b00<<6)		/* 4-50kHz */
#define DOUBLE_SPEED		(0b10<<6)		/* 50-100kHz */
#define QUAD_SPEED			(0b11<<6)		/* 100-200kHz */
#define	RATIO0				(0b00<<4)		/* See table page 28 and 29 of datasheet */
#define	RATIO1				(0b01<<4)
#define	RATIO2				(0b10<<4)
#define	RATIO3				(0b11<<4)
#define	MASTER				(1<<3)
#define	SLAVE				(0<<3)
#define	DIF_LEFTJUST_24b	(0b000)
#define	DIF_I2S_24b			(0b001)
#define	DIF_RIGHTJUST_16b	(0b010)
#define	DIF_RIGHTJUST_24b	(0b011)
#define	DIF_RIGHTJUST_20b	(0b100)
#define	DIF_RIGHTJUST_18b	(0b101)

//Reg 2 (DACCTRL)
#define AUTOMUTE 		(1<<7)
#define SLOW_FILT_SEL	(1<<6)
#define FAST_FILT_SEL	(0<<6)
#define DEEMPH_OFF		(0<<4)
#define DEEMPH_44		(1<<4)
#define DEEMPH_48		(2<<4)
#define DEEMPH_32		(3<<4)
#define	SOFT_RAMPUP		(1<<3) /*An un-mute will be performed after executing a filter mode change, after a MCLK/LRCK ratio change or error, and after changing the Functional Mode.*/
#define	SOFT_RAMPDOWN	(1<<2) /*A mute will be performed prior to executing a filter mode change.*/
#define INVERT_SIGA_POL	(1<<1) /*When set, this bit activates an inversion of the signal polarity for the appropriate channel*/
#define INVERT_SIGB_POL	(1<<0)

//Reg 3 (DACMIX)
#define BEQA			(1<<6) /*If set, ignore AOUTB volume setting, and instead make channel B's volume equal channel A's volume as set by AOUTA */
#define SOFTRAMP		(1<<5) /*Allows level changes, both muting and attenuation, to be implemented by incrementally ramping, in 1/8 dB steps, from the current level to the new level at a rate of 1 dB per 8 left/right clock periods */
#define	ZEROCROSS		(1<<4) /*Dictates that signal level changes, either by attenuation changes or muting, will occur on a signal zero crossing to minimize audible artifacts*/
#define ATAPI_aLbR		(0b1001) /*channel A==>Left, channel B==>Right*/

//Reg 4: DACAVOL
//Reg 5: DACBVOL

//Reg 6 (ADCCTRL)
#define DITHER16		(1<<5) /*activates the Dither for 16-Bit Data feature*/
#define ADC_DIF_I2S		(1<<4) /*I2S, up to 24-bit data*/
#define ADC_DIF_LJUST	(0<<4) /*Left Justified, up to 24-bit data (default)*/
#define MUTEA			(1<<3)
#define MUTEB			(1<<2)
#define HPFDisableA		(1<<1)
#define HPFDisableB		(1<<0)


//Reg 7 (MODECTRL2)
#define PDN		(1<<0)		/* Power Down Enable */
#define CPEN	(1<<1)		/* Control Port Enable */
#define FREEZE	(1<<2)		/* Freezes effects of register changes */
#define MUTECAB	(1<<3)		/* Internal AND gate on AMUTEC and BMUTEC */
#define LOOP	(1<<4)		/* Digital loopback (ADC->DAC) */

//Reg 8 (CHIPID) (Read-only)
#define PART_mask	(0b11110000)
#define REV_mask	(0b00001111)

bool Codec::InitializeGPIO() {
  GPIO_InitTypeDef gpio_init;

  // Start GPIO peripheral clocks.
  RCC_AHB1PeriphClockCmd(CODEC_I2C_GPIO_CLOCK | CODEC_I2S_GPIO_CLOCK, ENABLE);

  // Initialize I2C pins
  gpio_init.GPIO_Pin = CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN;
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_OD;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2C_GPIO, &gpio_init);

  // Connect pins to I2C peripheral
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SCL_PINSRC, CODEC_I2C_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SDA_PINSRC, CODEC_I2C_GPIO_AF);

  // Initialize I2S pins
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

  gpio_init.GPIO_Pin = CODEC_I2S_SCK_PIN;
  GPIO_Init(CODEC_I2S_SCK_GPIO, &gpio_init);

  gpio_init.GPIO_Pin = CODEC_I2S_SDO_PIN;
  GPIO_Init(CODEC_I2S_SDO_GPIO, &gpio_init);

  gpio_init.GPIO_Pin = CODEC_I2S_SDI_PIN;
  GPIO_Init(CODEC_I2S_SDI_GPIO, &gpio_init);

  gpio_init.GPIO_Pin = CODEC_I2S_WS_PIN;
  GPIO_Init(CODEC_I2S_WS_GPIO, &gpio_init);

  gpio_init.GPIO_Pin = CODEC_I2S_MCK_PIN;
  GPIO_Init(CODEC_I2S_MCK_GPIO, &gpio_init);

  // Connect pins to I2S peripheral.
  GPIO_PinAFConfig(CODEC_I2S_WS_GPIO, CODEC_I2S_WS_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_SCK_GPIO, CODEC_I2S_SCK_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_SDO_GPIO, CODEC_I2S_SDO_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_SDI_GPIO, CODEC_I2S_SDI_PINSRC, CODEC_I2Sext_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF);

  // Initialize Reset Pin
  RCC_AHB1PeriphClockCmd(CODEC_RESET_RCC, ENABLE);

  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

  gpio_init.GPIO_Pin = CODEC_RESET_PIN;
  GPIO_Init(CODEC_RESET_GPIO, &gpio_init);
  // set reset pin low
  GPIO_ResetBits(CODEC_RESET_GPIO, CODEC_RESET_PIN);

  return true;
}

bool Codec::InitializeControlInterface() {
  I2C_InitTypeDef i2c_init;

  // Initialize I2C
  RCC_APB1PeriphClockCmd(CODEC_I2C_CLK, ENABLE);

  I2C_DeInit(CODEC_I2C);
  i2c_init.I2C_Mode = I2C_Mode_I2C;
  i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
  i2c_init.I2C_OwnAddress1 = 0x33;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  i2c_init.I2C_ClockSpeed = CODEC_I2C_SPEED;

  I2C_Init(CODEC_I2C, &i2c_init);
  I2C_Cmd(CODEC_I2C, ENABLE);

  return true;
}

bool Codec::InitializeAudioInterface(int32_t sample_rate) {
  // Configure PLL and I2S master clock.
  RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);

  // The following values have been computed for a 8Mhz external crystal!
  RCC_PLLI2SCmd(DISABLE);
  if (sample_rate == 48000) {
    // 47.992kHz
    RCC_PLLI2SConfig(258, 3, 0);
  } else if (sample_rate == 44100) {
    // 44.11kHz
    RCC_PLLI2SConfig(271, 6, 0);
  } else if (sample_rate == 32000) {
    // 32.003kHz
    RCC_PLLI2SConfig(426, 4, 0);
  } else if (sample_rate == 96000) {
    // 95.95 kHz
    RCC_PLLI2SConfig(393, 4, 0);
  } else {
    // Unsupported sample rate!
    return false;
  }
  RCC_PLLI2SCmd(ENABLE);
  WAIT(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET);

  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  // Initialize I2S
  I2S_InitTypeDef i2s_init;

  SPI_I2S_DeInit(CODEC_I2S);
  i2s_init.I2S_AudioFreq = sample_rate;
  i2s_init.I2S_Standard = I2S_Standard_Phillips;
  i2s_init.I2S_DataFormat = I2S_DataFormat_24b;
  i2s_init.I2S_CPOL = I2S_CPOL_High;
  i2s_init.I2S_Mode = I2S_Mode_MasterTx;
  i2s_init.I2S_MCLKOutput = I2S_MCLKOutput_Enable;

  // Initialize the I2S main channel for TX
  I2S_Init(CODEC_I2S, &i2s_init);

  // Initialize the I2S extended channel for RX
  I2S_FullDuplexConfig(CODEC_I2S_EXT, &i2s_init);

  return true;
}

bool Codec::WriteControlRegister(uint8_t address, uint16_t data) {
  uint8_t byte_1 = ((address << 1) & 0xfe) | ((data >> 8) & 0x01);
  uint8_t byte_2 = data & 0xff;

  WAIT_LONG(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY));

  I2C_GenerateSTART(CODEC_I2C, ENABLE);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Transmitter);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(CODEC_I2C, byte_1);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

  I2C_SendData(CODEC_I2C, byte_2);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

  WAIT_LONG(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF));

  I2C_GenerateSTOP(CODEC_I2C, ENABLE);

  return true;
}

bool Codec::InitializeCodec(int32_t sample_rate) {
  bool s = true;  // success;

  // enable codec by putting reset pin high
  GPIO_SetBits(CODEC_RESET_GPIO, CODEC_RESET_PIN);
  for (register unsigned int i = 0; i < 100000; i++)
    __asm__ __volatile__ ("nop\n\t":::"memory");

  //Control Port Enable and Power Down Enable
  s = s && WriteControlRegister(CS4271_REG_MODECTRL2, CPEN | PDN);

  // s = s && WriteControlRegister(CS4271_REG_MODECTRL1,
  //                               SINGLE_SPEED | RATIO0 | MASTER |
  //                               DIF_I2S_24b); // TODO or SLAVE?

  // s = s && WriteControlRegister(CS4271_REG_DACCTRL,
  //                               FAST_FILT_SEL | DEEMPH_OFF | SOFT_RAMPUP | SOFT_RAMPDOWN);

  // s = s && WriteControlRegister(CS4271_REG_DACMIX, ATAPI_aLbR);

  // s = s && WriteControlRegister(CS4271_REG_DACAVOL, 0);
  // s = s && WriteControlRegister(CS4271_REG_DACBVOL, 0);

  // s = s && WriteControlRegister(CS4271_REG_ADCCTRL,
  //                               ADC_DIF_I2S | HPFDisableA | HPFDisableB);

  // s = s && WriteControlRegister(CS4271_REG_MODECTRL2, CPEN);

  return s;
}

bool Codec::InitializeDMA() {
  RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE);

  // DMA setup for TX.
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_DMA_STREAM);

  dma_init_tx_.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;
  dma_init_tx_.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
  dma_init_tx_.DMA_Memory0BaseAddr = (uint32_t)0;
  dma_init_tx_.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dma_init_tx_.DMA_BufferSize = (uint32_t)0xFFFE;
  dma_init_tx_.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init_tx_.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init_tx_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init_tx_.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init_tx_.DMA_Mode = DMA_Mode_Circular;
  dma_init_tx_.DMA_Priority = DMA_Priority_High;
  dma_init_tx_.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init_tx_.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dma_init_tx_.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init_tx_.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(AUDIO_I2S_DMA_STREAM, &dma_init_tx_);

  // DMA setup for RX.
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);

  dma_init_rx_.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;
  dma_init_rx_.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
  dma_init_rx_.DMA_Memory0BaseAddr = (uint32_t)0;
  dma_init_rx_.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dma_init_rx_.DMA_BufferSize = (uint32_t)0xFFFE;
  dma_init_rx_.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init_rx_.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init_rx_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init_rx_.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init_rx_.DMA_Mode = DMA_Mode_Circular;
  dma_init_rx_.DMA_Priority = DMA_Priority_High;
  dma_init_rx_.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init_rx_.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dma_init_rx_.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init_rx_.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &dma_init_rx_);

  // Enable the interrupts.
  DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

  // Enable the IRQ.
  NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

  // Start DMA from/to codec.
  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

  return true;
}

bool Codec::Init(int32_t sample_rate) {
  instance_ = this;

  sample_rate_ = sample_rate;

  return InitializeGPIO() && \
      InitializeControlInterface() && \
      InitializeAudioInterface(sample_rate) && \
      InitializeCodec(sample_rate) && \
      InitializeDMA();
}

bool Codec::Start(FillBufferCallback callback) {
  if (kBlockSize > kMaxCodecBlockSize) {
    return false;
  }

  callback_ = callback;

  // Enable the I2S TX and RX peripherals.
  if ((CODEC_I2S->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S, ENABLE);
  }
  if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S_EXT, ENABLE);
  }

  dma_init_tx_.DMA_Memory0BaseAddr = (uint32_t)(tx_dma_buffer_);
  dma_init_rx_.DMA_Memory0BaseAddr = (uint32_t)(rx_dma_buffer_);

  dma_init_tx_.DMA_BufferSize = 2 * kBlockSize * 2;
  dma_init_rx_.DMA_BufferSize = 2 * kBlockSize * 2;

  DMA_Init(AUDIO_I2S_DMA_STREAM, &dma_init_tx_);
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &dma_init_rx_);
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);

  return true;
}

void Codec::Stop() {
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

void Codec::Fill(size_t offset) {
  offset *= kBlockSize * 2;
  short* in = &rx_dma_buffer_[offset];
  short* out = &tx_dma_buffer_[offset];
  (*callback_)((ShortFrame*)(in), (ShortFrame*)(out));
}

}  // namespace mtd

extern "C" {
// Do not call into the firmware library to save on calls/jumps.
// if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET) {
//  DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);

void DMA1_Stream2_IRQHandler(void) {
  if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_TC) {
    AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_TC;
    mtd::Codec::GetInstance()->Fill(1);
  }
  if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_HT) {
    AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_HT;
    mtd::Codec::GetInstance()->Fill(0);
  }
}

}
