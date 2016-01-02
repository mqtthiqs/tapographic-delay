// Copyright 2013 Radoslaw Kwiecien.
// Copyright 2015 Dan Green.
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

#ifndef MULTITAP_DRIVERS_SDRAM_H_
#define MULTITAP_DRIVERS_SDRAM_H_

#include <stm32f4xx.h>

#define SDRAM_BASE 0xD0000000

//0x800000 = 8M = 8MBytes or 8M words = 16MBytes?

//#define SDRAM_SIZE 0x00800000
//#define SDRAM_SIZE 0x02000000

//SDRAM Bank 2 is 4 x 64MB, from 0xD000 0000 to 0xDFFF FFFF
//Thus, we can access 0x10000000 addresses, or 256M addresses
//#define SDRAM_SIZE 0x10000000
#define SDRAM_SIZE 0x02000000

//#define SDRAM_MEMORY_WIDTH            FMC_SDMemory_Width_8b
#define SDRAM_MEMORY_WIDTH            FMC_SDMemory_Width_16b
//#define SDRAM_MEMORY_WIDTH               FMC_SDMemory_Width_32b

//#define SDCLOCK_PERIOD	FMC_SDClock_Disable
#define SDCLOCK_PERIOD                   FMC_SDClock_Period_2
//#define SDCLOCK_PERIOD                FMC_SDClock_Period_3

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

namespace multitap {

class SDRAM {
 public:
  SDRAM() { }
  ~SDRAM() { }

  void Init();
  void FMC_Config(void);
  uint8_t Test(void);

  /* DISALLOW_COPY_AND_ASSIGN(SDRAM); //TODO */
};

}  // namespace multitap

#endif /* MULTITAP_DRIVERS_SDRAM_H_ */
