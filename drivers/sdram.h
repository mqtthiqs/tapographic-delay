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

#ifndef MTD_DRIVERS_SDRAM_H_
#define MTD_DRIVERS_SDRAM_H_

#include <stm32f4xx.h>

#define SDRAM_BASE 0xD0000000
#define SDRAM_SIZE 0x02000000

namespace mtd {

class SDRAM {
 public:
  SDRAM() { }
  ~SDRAM() { }

  void Init();
  void Clear();
  bool Test();

  /* DISALLOW_COPY_AND_ASSIGN(SDRAM); //TODO */
};

}  // namespace mtd

#endif /* MTD_DRIVERS_SDRAM_H_ */
