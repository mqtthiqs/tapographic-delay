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

#include "stmlib/dsp/dsp.h"

#include "multitap_delay.hh"

using namespace stmlib;

void MultitapDelay::Init(short* buffer, int32_t buffer_size, Clock* clock) {
  clock_ = clock;
    
  counter_ = 0;
  counter_running_ = true;    // TODO temp
  dry_fader_.fade_in(100); // TODO temp

  buffer_.Init(buffer, buffer_size);
  dc_blocker_.Init(1.0f - 20.0f / SAMPLE_RATE);
  repeat_fader_.Init();

  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].Init();
  }
  tap_allocator_.Init(taps_);
};

void MultitapDelay::AddTap(float velocity,
                           EditMode edit_mode,
                           Quantize quantize,
                           Panning panning) {
  // first tap does not count, it just starts the counter
  if (!counter_running_) {
    counter_running_ = true;
    return;
  }

  float time = static_cast<float>(counter_);

  // compute quantization
  if (repeat_time_) {
    float repeat = static_cast<float>(repeat_time_);
    float q =
      quantize == QUANTIZE_8 ? 8.0f :
      quantize == QUANTIZE_16 ? 16.0f :
      repeat;
    time = round(time / repeat * q)
      * repeat / q;
  }

  // compute panning
  float pan = 0.0f;
  if (panning == PANNING_RANDOM) {
    pan = Random::GetFloat();
  } else if (panning == PANNING_ALTERNATE) {
    static bool pan_state = true;
    pan = pan_state ? 1.0f : 0.0f;
    pan_state = !pan_state;
  }

  // add tap
  if (time < buffer_.size()) {
    tap_allocator_.Add(time, velocity, pan);
  }

  // in overwrite mode, remove oldest tap
  if (edit_mode == EDIT_OVERWRITE) {
    tap_allocator_.Remove();
  }
}

void MultitapDelay::RemTap() {
  tap_allocator_.Remove();
}

void MultitapDelay::Clear() {
  counter_running_ = false;
  tap_allocator_.Clear();
  counter_ = 0;
}

bool MultitapDelay::Process(Parameters *params, ShortFrame* input, ShortFrame* output) {

  // repeat time, in samples
  repeat_time_ = clock_->running() ?
    clock_->period() * kBlockSize :
    tap_allocator_.max_time() * params->scale;

  if (repeat_time_ > buffer_.size() ||
      repeat_time_ < 100) {
    repeat_time_ = 0;
  }

  // increment sample counter
  if (counter_running_) {
    counter_ += kBlockSize;
    // in the right edit modes, reset counter
    if (params->edit_mode != EDIT_NORMAL &&
        counter_ > repeat_time_) {
      counter_ -= repeat_time_;
    }
  }

  // set fade time
  tap_allocator_.set_fade_time(params->morph);
  tap_allocator_.Poll();

  { /* Write into the buffer */
    int16_t buffer[kBlockSize];

    buffer_.Read(buffer, repeat_time_, kBlockSize);

    // fade in/out the repeat buffer
    if (repeat_time_ // only if repeat time > 100
        && params->repeat
        && !previous_repeat_) {
      repeat_fader_.fade_in(params->morph);
    } else if (!params->repeat
               && previous_repeat_) {
      repeat_fader_.fade_out(params->morph);
    } else {
      repeat_fader_.Prepare();
    }

    previous_repeat_ = params->repeat;

    for (size_t i=0; i<kBlockSize; i++) {
      repeat_fader_.Process(buffer[i]);
      int32_t sample =
        static_cast<int32_t>(buffer[i])
        + static_cast<int32_t>(input[i].l)
        + params->feedback * feedback_buffer[i];
      // float s = static_cast<float>(sample) / 32768.0f;
      // buffer[i] = SoftConvert(s * 2);
      buffer[i] = Clip16(sample);
    }
    buffer_.Write(buffer, kBlockSize);
  }

  FloatFrame buf[kBlockSize];
  FloatFrame empty = {0.0f, 0.0f};
  std::fill(buf, buf+kBlockSize, empty);

  // gate is high at the beginning of the loop
  bool gate = counter_running_ && counter_ < kBlockSize+1;

  for (int i=0; i<kMaxTaps; i++) {
    taps_[i].Process(params->velocity_type,
                     prev_params_.scale, prev_params_.jitter_amount,
                     params->scale, params->jitter_amount, params->jitter_frequency,
                     &buffer_, buf);

    if (counter_running_
        && taps_[i].active()
        && taps_[i].time() * params->scale < counter_
        && counter_ < taps_[i].time() * params->scale + 2000) {
      gate = true;
    }
  }

  dry_fader_.Prepare();

  /* convert, output and feed back */
  for (size_t i=0; i<kBlockSize; i++) {
    float sample_l = buf[i].l;
    float sample_r = buf[i].r;
      
      
    float fb = sample_l + sample_r;
    dc_blocker_.Process(&fb, 1);

    // add dry signal
    float dry = static_cast<float>(input[i].l) / 32768.0f;
    dry_fader_.Process(dry);
    sample_l += (dry - sample_l) * params->drywet;
    sample_r += (dry - sample_r) * params->drywet;

    // write to feedback buffer in Q1.15 to leave headroom
    feedback_buffer[i] = Clip16(static_cast<int32_t>(16384.0f * fb));

    output[i].l = SoftConvert(sample_l);
    output[i].r = SoftConvert(sample_r);
  }

  prev_params_ = *params;

  return gate;
};
