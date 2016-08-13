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

#ifndef MULTITAP_DELAY_H_
#define MULTITAP_DELAY_H_

#include "parameters.hh"
#include "tap_allocator.hh"

#include "stmlib/dsp/filter.h"

using namespace stmlib;

class MultitapDelay
{
 public:
  MultitapDelay() { }
  ~MultitapDelay() { }

  void Init(short* buffer, int32_t buffer_size);
  bool Process(Parameters *params, ShortFrame* input, ShortFrame* output);

  void AddTap(Parameters *params, float repeat_time);
  void Clear(Parameters *params);
  void RemTap();

 private:
  template<bool quality>
  bool Process(Parameters *params, ShortFrame* input, ShortFrame* output);

  TapAllocator tap_allocator_;
  Tap taps_[kMaxTaps];
  AudioBuffer buffer_;
  float feedback_buffer[kBlockSize];   /* max block size */
  uint32_t last_repeat_time_;

  Parameters prev_params_;
  Svf dc_blocker_;

  Fader repeat_fader_;
  uint32_t counter_;

  // UI infos

  DISALLOW_COPY_AND_ASSIGN(MultitapDelay);
};

#endif
