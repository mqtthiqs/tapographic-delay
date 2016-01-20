// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Main clock

#include "drivers/gate_input.h"

#ifndef MTD_CLOCK_H_
#define MTD_CLOCK_H_

namespace mtd 
{
  const uint8_t kHistorySize = 3;

  class Clock
  {
  public:
    Clock() { }
    ~Clock() { }

    void Init();
    void Tick();
    void Tap();
    void RecordLastTap();
    void Start();
    void Stop();

    float phase() { return phase_; }
    float reset() {
      bool r = reset_;
      reset_ = false;
      return r;
    }
    bool running() { return running_; }
    float period() { return 1.0f / phase_increment_; }

  private:
    uint32_t counter_;
    uint32_t last_tap_;
    bool running_;
    bool reset_;
    float phase_;
    float phase_increment_;

    uint32_t history_[kHistorySize];
    uint8_t history_cursor_;

    DISALLOW_COPY_AND_ASSIGN(Clock);
  };
}

#endif  // MTD_CLOCK_H_
