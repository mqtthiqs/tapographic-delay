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
// Multitap delay

#ifndef MTD_MULTITAP_DELAY_H_
#define MTD_MULTITAP_DELAY_H_

#include "parameters.h"
#include "ring_buffer.h"
#include "clock.h"
#include "tap_allocator.h"

#include "stmlib/dsp/filter.h"

using namespace stmlib;

namespace mtd 
{
  class MultitapDelay
  {
  public:
    MultitapDelay() { }
    ~MultitapDelay() { }

    void Init(short* buffer, int32_t buffer_size, Clock* clock);
    void Process(Parameters *params, ShortFrame* input, ShortFrame* output);

    void AddTap(float velocity, EditMode edit_mode);
    void Clear();

  private:
    TapAllocator tap_allocator_;
    Tap taps_[kMaxTaps];
    RingBuffer<short> buffer_;
    int16_t feedback_buffer[kBlockSize];   /* max block size */
    float prev_repeat_time_;

    Parameters prev_params_;
    Svf dc_blocker_;

    Clock* clock_;

    bool counter_running_;
    uint32_t counter_;

    DISALLOW_COPY_AND_ASSIGN(MultitapDelay);
  };
}

#endif
