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
// Tap allocator

#ifndef MTD_TAP_ALLOCATOR_H_
#define MTD_TAP_ALLOCATOR_H_

#include "tap.h"

namespace mtd 
{
  class TapAllocator
  {
  public:
    TapAllocator() { }
    ~TapAllocator() { }

    void Init(Tap taps[kMaxTaps]);
    void Add(float time, float velocity, float panning);
    void Remove();
    void Clear();

    void set_fade_time(float fade_time) {
      fade_time_ = fade_time;
    }

    float max_time() { return max_time_; }
    uint8_t busy_voices() { return busy_voices_; }

  private:
    Tap* taps_;

    uint8_t busy_voices_;
    uint8_t next_voice_;
    uint8_t oldest_voice_;
    float fade_time_;

    float max_time_;

    DISALLOW_COPY_AND_ASSIGN(TapAllocator);
  };
}

#endif
