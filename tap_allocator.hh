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

#ifndef TAP_ALLOCATOR_H_
#define TAP_ALLOCATOR_H_

#include "tap.hh"
#include "stmlib/utils/ring_buffer.h"

class TapAllocator
{
 public:
  TapAllocator() { }
  ~TapAllocator() { }

  void Init(Tap taps[kMaxTaps]);
  bool Add(float time, float velocity, VelocityType velocity_type, float pan);
  bool RemoveFirst();
  bool RemoveLast();
  void Clear();
  void Poll();

  void set_fade_time(float fade_time) {
    fade_time_ = fade_time;
  }

  bool full() { return (next_voice_ + 1) % kMaxTaps == oldest_voice_; }
  bool empty() { return next_voice_ == oldest_voice_; }
  uint8_t busy_voices() {
    int busy = next_voice_ - oldest_voice_;
    if (busy < 0) busy += kMaxTaps;
    return busy;
  }
  float max_time() { return max_time_; }

 private:
  void RecomputeMaxTime();

  bool writeable() {
    return !full() && !taps_[next_voice_].active();
  };

  Tap* taps_;

  int8_t next_voice_;
  int8_t oldest_voice_;
  float fade_time_;
  float max_time_;

  struct TapParameter {
    float time;
    float velocity;
    VelocityType velocity_type;
    float pan;
  };

  stmlib::RingBuffer<TapParameter, kMaxTaps> queue_;

  DISALLOW_COPY_AND_ASSIGN(TapAllocator);
};

#endif
