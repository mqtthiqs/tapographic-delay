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

#include "tap_allocator.hh"

void TapAllocator::Init(Tap taps[kMaxTaps]) {
  taps_ = taps;
  fade_time_ = 1000.0f;

  // Dummy IR generation
  for (size_t i=0; i<kMaxTaps; i++) {
    float t = static_cast<float>(i) + 1.0f;
    float pan = i&1;//Random::GetFloat();
    Add(t * t * t * SAMPLE_RATE * 0.005f / kMaxTaps + 500.0f,
        (t+1) / (kMaxTaps+1), pan);
  }
}

void TapAllocator::Add(float time, float velocity, float panning) {

  if ((next_voice_ + 1) % kMaxTaps != oldest_voice_) {
    taps_[next_voice_].fade_in(fade_time_);
    taps_[next_voice_].set_time(time);
    taps_[next_voice_].set_velocity(velocity);
    float gain_l = panning;
    float gain_r = 1.0f - panning;
    taps_[next_voice_].set_gains(gain_l, gain_r);

    if (time > max_time_)
      max_time_ = time;

    next_voice_ = (next_voice_ + 1) % kMaxTaps;
  } else {
    TapParameter p = {time, velocity, panning};
    queue_.Overwrite(p);
  }
}

void TapAllocator::Remove() {
  if (oldest_voice_ != next_voice_) {
    taps_[oldest_voice_].fade_out(fade_time_);
    oldest_voice_ = (oldest_voice_ + 1) % kMaxTaps;
  }
}

void TapAllocator::Poll() {
  if (queue_.readable()) {
    TapParameter p = queue_.Read();
    Add(p.time, p.velocity, p.panning);
  }
}

void TapAllocator::Clear() {
  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].fade_out(fade_time_);
  }
  max_time_ = 0.0f;
  next_voice_ = oldest_voice_ = 0;
}
