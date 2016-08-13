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
    float time = t * t * t * SAMPLE_RATE * 0.005f / kMaxTaps + 500.0f;
    float velocity = (t+1) / (kMaxTaps+1);
    Add(time, velocity, VELOCITY_BP, PANNING_ALTERNATE);
  }

  // Add(5000.0f, 1.0f, VELOCITY_AMP, PANNING_ALTERNATE);
}

// TODO: differentiate manual tap (doesn't queue) and batch add when
// recalling IR (puts in queues)
bool TapAllocator::Add(float time, float velocity,
                       VelocityType velocity_type,
                       PanningMode panning_mode) {

  if ((next_voice_ + 1) % kMaxTaps != oldest_voice_) {

    // compute panning
    float panning = 0.0f;
    if (panning_mode == PANNING_RANDOM) {
      panning = Random::GetFloat();
    } else if (panning_mode == PANNING_ALTERNATE) {
      panning = pan_state_ ? 1.0f : 0.0f;
      pan_state_ = !pan_state_;
    }

    taps_[next_voice_].fade_in(fade_time_);
    taps_[next_voice_].set_time(time);
    taps_[next_voice_].set_velocity(velocity, velocity_type);
    taps_[next_voice_].set_panning(panning);

    if (time > max_time_)
      max_time_ = time;

    next_voice_ = (next_voice_ + 1) % kMaxTaps;
    busy_voices_++;

    return true;
  } else {
    // no taps left: queue and start fade out
    Remove();
    TapParameter p = {velocity_type, panning_mode, time, velocity};
    queue_.Overwrite(p);
    return false;
  }
}

// TODO use this info
bool TapAllocator::Remove() {
  if (oldest_voice_ != next_voice_) {
    taps_[oldest_voice_].fade_out(fade_time_);
    oldest_voice_ = (oldest_voice_ + 1) % kMaxTaps;
    busy_voices_--;
    return true;
  } else {
    return false;
  }
}

void TapAllocator::Poll() {
  if (queue_.readable()) {
    TapParameter p = queue_.Read();
    Add(p.time, p.velocity, p.velocity_type, p.panning_mode);
  }
}

void TapAllocator::Clear() {
  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].fade_out(fade_time_);
  }
  queue_.Flush();
  max_time_ = 0.0f;
  next_voice_ = oldest_voice_ = 0;
  busy_voices_ = 0;
}
