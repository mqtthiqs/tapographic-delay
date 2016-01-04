// Copyright 2013 Radoslaw Kwiecien.
// Copyright 2015 Dan Green.
// Copyright 2016 Matthias Puech.
//
// Author: Radoslaw Kwiecien (radek@dxp.pl)
// Source: http://en.radzio.dxp.pl/stm32f429idiscovery/
// Modified by: Dan Green (matthias.puech@gmail.com)
// Modified by: Matthias Puech (matthias.puech@gmail.com)
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
// SDRAM driver

#include "sdram.h"

static void Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (100000 * nCount); index != 0; index--) {
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
  }
}

namespace multitap {

// GPIO configuration data
static  GPIO_TypeDef * const GPIOInitTable[] = {
  GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, //  0, 1, 2, 3, 4, 5, 12, 13, 14, 15,
  GPIOG, GPIOG, GPIOG, //  0, 1, 2,
  GPIOD, GPIOD, GPIOD, GPIOD, // 14, 15, 0, 1,
  GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, // 7, 8, 9, 10, 11, 12, 13, 14, 15,
  GPIOD, GPIOD, GPIOD, //  8, 9, 10,
  GPIOB, GPIOB, // 5, 6,
  GPIOC, // 0,
  GPIOE, GPIOE, // 0, 1,
  GPIOF, // 11,
  GPIOG, GPIOG, GPIOG, GPIOG, // 4, 5, 8, 15
  0
};

static uint8_t const PINInitTable[] = {
  0, 1, 2, 3, 4, 5, 12, 13, 14, 15,
  0, 1, 2,
  14, 15, 0, 1,
  7, 8, 9, 10, 11, 12, 13, 14, 15,
  8, 9, 10,
  5, 6,
  0,
  0, 1,
  11,
  4, 5, 8, 15,
  0
};

void SDRAM::Clear() {
  volatile uint32_t ptr = 0;
  for(ptr = SDRAM_BASE; ptr < (SDRAM_BASE + SDRAM_SIZE - 1); ptr += 4)
    *((uint32_t *)ptr) = 0xFFFFFFFF;
}

bool SDRAM::Test() {

	uint32_t addr;
  uint32_t t;
	uint32_t i;
	t=0;

  addr=SDRAM_BASE;
  for (i=0;i<SDRAM_SIZE;i++){
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
    *((uint16_t *)addr) = (uint16_t)i;
    addr += 2;
	}

  addr=SDRAM_BASE;
  for (i=0;i<5000;i++){
    while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
    t = *((uint16_t*)addr);
    if (t != i)
      return false;
    addr += 2;
	}

  return true;
}

/* Wait until the SDRAM controller is ready */
void Wait() {
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);
}


void SDRAM::Init() {
    //Enable all gpios
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN |
                    RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN |
                    RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN;


    //For reasons unknown the write to GPIOB->AFR does not occur without a DMB
    //in the middle
    __DMB();


    //First, configure SDRAM GPIOs
    GPIOB->AFR[0]=0x0cc00000;
    GPIOC->AFR[0]=0x0000000c;
    GPIOD->AFR[0]=0x000000cc;
    GPIOD->AFR[1]=0xcc000ccc;
    GPIOE->AFR[0]=0xc00000cc;
    GPIOE->AFR[1]=0xcccccccc;
    GPIOF->AFR[0]=0x00cccccc;
    GPIOF->AFR[1]=0xccccc000;
    GPIOG->AFR[0]=0x00cc00cc;
    GPIOG->AFR[1]=0xc000000c;


    GPIOB->MODER=0x00002800;
    GPIOC->MODER=0x00000002;
    GPIOD->MODER=0xa02a000a;
    GPIOE->MODER=0xaaaa800a;
    GPIOF->MODER=0xaa800aaa;
    GPIOG->MODER=0x80020a0a;


    GPIOA->OSPEEDR=0xaaaaaaaa; //Default to 50MHz speed for all GPIOs...
    GPIOB->OSPEEDR=0xaaaaaaaa | 0x00003c00; //...but 100MHz for the SDRAM pins
    GPIOC->OSPEEDR=0xaaaaaaaa | 0x00000003;
    GPIOD->OSPEEDR=0xaaaaaaaa | 0xf03f000f;
    GPIOE->OSPEEDR=0xaaaaaaaa | 0xffffc00f;
    GPIOF->OSPEEDR=0xaaaaaaaa | 0xffc00fff;
    GPIOG->OSPEEDR=0xaaaaaaaa | 0xc0030f0f;
    GPIOH->OSPEEDR=0xaaaaaaaa;


    //Since we'we un-configured PB3/PB4 from the default at boot TDO,NTRST,
    //finish the job and remove the default pull-up
    GPIOB->PUPDR=0;


    //Second, actually start the SDRAM controller
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;


    //This doesn't seem to be needed, but better be safe
    __DMB();


    //SDRAM is a IS42S16400J -7 speed grade, connected to bank 2 (0xd0000000)
    //Some bits in SDCR[1] are don't care, and the have to be set in SDCR[0],
    //they aren't just don't care, the controller will fail if they aren't at 0
    FMC_Bank5_6->SDCR[0]=FMC_SDCR1_SDCLK_1// SDRAM runs @ half CPU frequency
                       | FMC_SDCR1_RBURST // Enable read burst
                       | 0;               //  0 delay between reads after CAS
    FMC_Bank5_6->SDCR[1]=1                //  9 bit column address
                       | FMC_SDCR1_NR_1   // 13 bit row address
                       | FMC_SDCR1_MWID_0 // 16 bit data bus
                       | FMC_SDCR1_NB     //  4 banks
                       | FMC_SDCR1_CAS_1; //  2 cycle CAS latency (F<133MHz)


    #ifdef SYSCLK_FREQ_180MHz
    //One SDRAM clock cycle is 11.1ns
    //Some bits in SDTR[1] are don't care, and the have to be set in SDTR[0],
    //they aren't just don't care, the controller will fail if they aren't at 0
    FMC_Bank5_6->SDTR[0]=(6-1)<<12        // 6 cycle TRC  (66.6ns>63ns)
                       | (2-1)<<20;       // 2 cycle TRP  (22.2ns>15ns)
    FMC_Bank5_6->SDTR[1]=(2-1)<<0         // 2 cycle TMRD
                       | (7-1)<<4         // 7 cycle TXSR (77.7ns>70ns)
                       | (4-1)<<8         // 4 cycle TRAS (44.4ns>42ns)
                       | (2-1)<<16        // 2 cycle TWR
                       | (2-1)<<24;       // 2 cycle TRCD (22.2ns>15ns)
    #elif defined(SYSCLK_FREQ_168MHz)
    //One SDRAM clock cycle is 11.9ns
    //Some bits in SDTR[1] are don't care, and the have to be set in SDTR[0],
    //they aren't just don't care, the controller will fail if they aren't at 0
    FMC_Bank5_6->SDTR[0]=(6-1)<<12        // 6 cycle TRC  (71.4ns>63ns)
                       | (2-1)<<20;       // 2 cycle TRP  (23.8ns>15ns)
    FMC_Bank5_6->SDTR[1]=(2-1)<<0         // 2 cycle TMRD
                       | (6-1)<<4         // 6 cycle TXSR (71.4ns>70ns)
                       | (4-1)<<8         // 4 cycle TRAS (47.6ns>42ns)
                       | (2-1)<<16        // 2 cycle TWR
                       | (2-1)<<24;       // 2 cycle TRCD (23.8ns>15ns)
    #else
    #error No SDRAM timings for this clock
    #endif


    FMC_Bank5_6->SDCMR=  FMC_SDCMR_CTB2   // Enable bank 2
                       | 1;               // MODE=001 clock enabled
    Wait();


    //ST and SDRAM datasheet agree a 100us delay is required here.
    Delay(10);


    FMC_Bank5_6->SDCMR=  FMC_SDCMR_CTB2   // Enable bank 2
                       | 2;               // MODE=010 precharge all command
    Wait();


    FMC_Bank5_6->SDCMR=  (8-1)<<5         // NRFS=8 SDRAM datasheet says
                                          // "at least two AUTO REFRESH cycles"
                       | FMC_SDCMR_CTB2   // Enable bank 2
                       | 3;               // MODE=011 auto refresh
    Wait();


    FMC_Bank5_6->SDCMR=0x220<<9           // MRD=0x220:CAS latency=2 burst len=1
                       | FMC_SDCMR_CTB2   // Enable bank 2
                       | 4;               // MODE=100 load mode register
    Wait();


    // 64ms/4096=15.625us
    #ifdef SYSCLK_FREQ_180MHz
    //15.625us*90MHz=1406-20=1386
    FMC_Bank5_6->SDRTR=1386<<1;
    #elif defined(SYSCLK_FREQ_168MHz)
    //15.625us*84MHz=1312-20=1292
    FMC_Bank5_6->SDRTR=1292<<1;
    #else
    #error No refresh timings for this clock
    #endif
}


}
