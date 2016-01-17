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

#include "clock.h"

#include "drivers/gate_input.h"

namespace mtd 
{
  void Clock::Init() {
    phase_ = 0.0f;
    phase_increment_ = 0.0f;
    counter_ = 0;
    running_ = false;
    for (int i=0; i<kHistorySize; i++)
      history_[i] = 0;
    history_cursor_ = 0;
  }

  void Clock::Tick() {
    if (running_) {
      phase_ += phase_increment_;
      if (phase_ > 1.0f) {
        phase_--;
        reset_ = true;
      }
      counter_++;
    }
  }

  void Clock::Start() {
    phase_ = 0.0f;
    running_ = true;
    reset_ = true;
  }

  void Clock::Stop() {
    running_ = false;
    counter_ = 0;
    phase_increment_ = 0.0f;
    for (int i=0; i<kHistorySize; i++)
      history_[i] = 0;
  }

  void Clock::Tap() {
    last_tap_ = counter_;
    counter_ = 0;
    Start();
  }

  void Clock::RecordLastTap() {
    history_[history_cursor_++] = last_tap_;

    if (history_cursor_ == kHistorySize)
      history_cursor_ = 0;

    float mean = 0;
    int8_t div = 0;
    for (int i=0; i<kHistorySize; i++) {
      uint32_t v = history_[i];
      if (v != 0) {
        mean += static_cast<float>(v);
        div++;
      }
    }
    mean /= div;

    if (mean > 1) {
      phase_increment_ = 1.0f / mean;
    }
  }
}
