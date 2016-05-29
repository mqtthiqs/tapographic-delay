/*
 * codec.h
 *
 */

#ifndef __codec__
#define __codec__

#include <stm32f4xx.h>

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                       50000

// For CS4271
#define CODEC_RESET_RCC RCC_AHB1Periph_GPIOB
#define CODEC_RESET_pin GPIO_Pin_4
#define CODEC_RESET_GPIO GPIOB
#define CODEC_RESET_HIGH CODEC_RESET_GPIO->BSRRL = CODEC_RESET_pin
#define CODEC_RESET_LOW CODEC_RESET_GPIO->BSRRH = CODEC_RESET_pin

#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              0x40003C0C
#define CODEC_I2S_EXT_ADDRESS          0x4000400C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3

#define CODEC_I2Sext_GPIO_AF			((uint8_t)0x05) /* for PC11 */
//#define CODEC_I2Sext_GPIO_AF			GPIO_AF_I2S3ext /* for PB4 */

#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_EXT_IRQ              SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC)

#define CODEC_I2S_GPIO_WS              GPIOA
#define CODEC_I2S_WS_PIN               GPIO_Pin_4
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource4

#define CODEC_I2S_GPIO_SCK             GPIOC
#define CODEC_I2S_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource10

#define CODEC_I2S_GPIO_SDO             GPIOC
#define CODEC_I2S_SDO_PIN              GPIO_Pin_12
#define CODEC_I2S_SDO_PINSRC           GPIO_PinSource12

#define CODEC_I2S_GPIO_SDI             GPIOC
#define CODEC_I2S_SDI_PIN              GPIO_Pin_11
#define CODEC_I2S_SDI_PINSRC           GPIO_PinSource11

#define CODEC_I2S_MCK_GPIO             GPIOC
#define CODEC_I2S_MCK_PIN              GPIO_Pin_7
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource7

#define AUDIO_I2S_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S_EXT_IRQHandler       SPI3_IRQHandler

//SPI3_TX: S5 C0 or S7 C0
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

//I2S_EXT_RX: S0 C3 or S2 C2
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

/* I2S DMA Stream definitions */

#define CODEC_I2C                      I2C1
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_8
#define CODEC_I2C_SDA_PIN              GPIO_Pin_9
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource8
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource9

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define CODEC_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_TIMEOUT))

#define CODEC_BUFFER_SIZE 128
  typedef struct {
    short l;
    short r;
  } Frame;

typedef void (*FillBufferCallback)(Frame* rx, Frame* tx);

bool Init(int32_t sample_rate, FillBufferCallback cb);
void DeInit();

void Start();
void Stop();

#endif
