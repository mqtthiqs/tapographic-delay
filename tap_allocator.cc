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

void TapAllocator::Init(Tap taps[kMaxTaps])
{
  taps_ = taps;
  fade_time_ = 1000.0f;

  // // Dummy IR generation
  // for (size_t i=0; i<kMaxTaps; i++) {
  //   float t = static_cast<float>(i) + 1.0f;
  //   float time = t * t * t * SAMPLE_RATE * 0.005f / kMaxTaps + 500.0f;
  //   float velocity = (t+1) / (kMaxTaps+1);
  //   Add(time, velocity, VELOCITY_BP, Random::GetFloat());
  // }

  // Add(5000.0f, 1.0f, VELOCITY_AMP, PANNING_ALTERNATE);
}

void TapAllocator::Load(uint8_t slot_nr)
{
  Clear();
  Slot slot = slots_[slot_nr];
  for (int i=0; i<slot.size; i++) {
    TapParameters p = slot.taps[i];
    Add(p.time, p.velocity, p.velocity_type, p.panning);
  }
}

void TapAllocator::Save(uint8_t slot_nr) 
{
  Slot* slot = &slots_[slot_nr];

  slot->size = busy_voices();

  for(int i=0; i<busy_voices(); i++) {
    int index = (oldest_voice_ + i) % kMaxTaps;
    slot->taps[i].time = taps_[index].time();
    slot->taps[i].velocity = taps_[index].velocity();
    slot->taps[i].velocity_type = taps_[index].velocity_type();
    slot->taps[i].panning = taps_[index].panning();
  }
}


bool TapAllocator::Add(float time, float velocity,
                       VelocityType velocity_type,
                       float panning)
{
  if (writeable()) {

    taps_[next_voice_].fade_in(fade_time_);
    taps_[next_voice_].set_time(time);
    taps_[next_voice_].set_velocity(velocity, velocity_type);
    taps_[next_voice_].set_panning(panning);

    if (time > max_time_)
      max_time_ = time;

    next_voice_ = (next_voice_ + 1) % kMaxTaps;

    return true;
  } else {
    // no taps left: queue and start fade out
    RemoveFirst();
    TapParameters p = {time, velocity, velocity_type, panning};
    queue_.Overwrite(p);
    return false;
  }
}

void TapAllocator::RecomputeMaxTime()
{
  float max = 0.0f;
  for(int i=0; i<busy_voices(); i++) {
    int index = (oldest_voice_ + i) % kMaxTaps;
    float time = taps_[index].time();
    if (time > max) max = time;
  }
  max_time_ = max;
}

// TODO use this bool info
bool TapAllocator::RemoveFirst()
{
  if (!empty()) {
    taps_[oldest_voice_].fade_out(fade_time_);
    oldest_voice_ = (oldest_voice_ + 1) % kMaxTaps;
    RecomputeMaxTime();
    return true;
  } else {
    return false;
  }
}

bool TapAllocator::RemoveLast() 
{
  if (!empty()) {
    next_voice_--;
    if (next_voice_ < 0) next_voice_ += kMaxTaps;
    taps_[next_voice_].fade_out(fade_time_);
    RecomputeMaxTime();
    return true;
  } else {
    return false;
  }
}

void TapAllocator::Poll()
{
  if (queue_.readable() && writeable()) {
    TapParameters p = queue_.Read();
    Add(p.time, p.velocity, p.velocity_type, p.panning);
  }
}

void TapAllocator::Clear()
{
  for(int i=0; i<busy_voices(); i++) {
    int index = (oldest_voice_ + i) % kMaxTaps;
    taps_[index].fade_out(fade_time_);
  }
  queue_.Flush();
  max_time_ = 0.0f;
  oldest_voice_ = next_voice_;
}
