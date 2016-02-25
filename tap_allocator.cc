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

#include "tap_allocator.h"


namespace mtd 
{
  void TapAllocator::Init(Tap taps[kMaxTaps]) {
    taps_ = taps;
    fade_time_ = 1000.0f;

    for (size_t i=0; i<kMaxTaps; i++) {
      taps_[i].set_busy_voices_counter(&busy_voices_);
    }

    // Dummy IR generation
    for (size_t i=0; i<kMaxTaps; i++) {
      float t = static_cast<float>(i) + 1.0f;
      Add(t * t * SAMPLE_RATE * 0.8f / kMaxTaps + 1000.0f,
          t / kMaxTaps);
    }
  }

  void TapAllocator::Add(float time, float velocity) {
    taps_[next_voice_].fade_in(fade_time_);
    taps_[next_voice_].set_time(time);
    taps_[next_voice_].set_velocity(velocity);

    if (time > max_time_)
      max_time_ = time;

    if (busy_voices_ > kMaxTaps) {
      int16_t oldest_voice = next_voice_ == 0 ? kMaxTaps - 1 : next_voice_ - 1;
      taps_[oldest_voice].fade_out(fade_time_);
    }

    next_voice_ = next_voice_ == kMaxTaps-1 ? 0 : next_voice_ + 1;
  }

  void TapAllocator::Remove() {
    int16_t oldest_voice = next_voice_ - busy_voices_;
    if (oldest_voice < 0) oldest_voice += kMaxTaps;

    taps_[oldest_voice].fade_out(fade_time_);
  }

  void TapAllocator::Clear() {
    for (size_t i=0; i<kMaxTaps; i++) {
      taps_[i].fade_out(fade_time_);
    }
    max_time_ = 0.0f;
    next_voice_ = 0;
  }
}
