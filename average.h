// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
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
// Moving average filter

#ifndef MTD_AVERAGE_H_
#define MTD_AVERAGE_H_

#include <algorithm>

#include "stmlib/stmlib.h"

namespace mtd {

  /* SIZE must be a power of 2 */
  template<int SIZE>
    class Average {
  public:
    Average() { }
    ~Average() { }
  
    void Init() {
      last = 0.0f;
      cursor = 0;
      std::fill(history, history+SIZE, 0.0f);
    }
  
    void Process(float x) {
      last += x - history[(cursor+1) % SIZE];
      history[cursor++ % SIZE] = x;
    }

    float value() {
      return last / (SIZE-1);      
    }
    
    
  private:
    float last;
    float history[SIZE];
    size_t cursor;    

    DISALLOW_COPY_AND_ASSIGN(Average);
  };

}  // namespace mtd

#endif  // MTD_AVERAGE_H_
