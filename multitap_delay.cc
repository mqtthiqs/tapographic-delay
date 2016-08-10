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
#include "stmlib/dsp/parameter_interpolator.h"

#include "multitap_delay.hh"

using namespace stmlib;

const float kBufferHeadroom = 0.5f;

void MultitapDelay::Init(short* buffer, int32_t buffer_size) {
  counter_ = 0;

  buffer_.Init(buffer, buffer_size);
  dc_blocker_.Init();
  dc_blocker_.set_f<FREQUENCY_FAST>(40.0f / SAMPLE_RATE);
  repeat_fader_.Init();

  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].Init();
  }
  tap_allocator_.Init(taps_);
};

void MultitapDelay::AddTap(Parameters *params, float repeat_time) {
  // first tap does not count, it just starts the counter
  if (!params->counter_running) {
    params->counter_running = true;
    params->last_tap_velocity = 1.0f;
    params->last_tap_type = TAP_DRY;
    return;
  }

  float time = static_cast<float>(counter_);

  // compute quantization
  if (repeat_time) {
    float q =
      params->quantize == QUANTIZE_8 ? 8.0f :
      params->quantize == QUANTIZE_16 ? 16.0f :
      repeat_time;
    time = round(time / repeat_time * q) * repeat_time / q;
  }

  // add tap
  if (time < buffer_.size()) {
    tap_allocator_.Add(time,
                       params->velocity,
                       params->velocity_type,
                       params->panning_mode);
  }

  // in overwrite mode, remove oldest tap
  if (params->edit_mode == EDIT_OVERWRITE) {
    tap_allocator_.Remove();
  }

  params->last_tap_velocity = params->velocity;
  params->last_tap_type =
    params->edit_mode == EDIT_NORMAL ? TAP_NORMAL :
    params->edit_mode == EDIT_OVERWRITE ? TAP_OVERWRITE :
    params->edit_mode == EDIT_OVERDUB ? TAP_OVERDUB :
    TAP_FAIL;
}

void MultitapDelay::RemTap() {
  tap_allocator_.Remove();
}

void MultitapDelay::Clear(Parameters *params) {
  params->counter_running = false;
  tap_allocator_.Clear();
  counter_ = 0;
}

bool MultitapDelay::Process(Parameters *params, ShortFrame* input, ShortFrame* output) {

  // repeat time, in samples
  float repeat_time = tap_allocator_.max_time() * prev_params_.scale;

  // add tap if needed
  if (params->tap) {
    AddTap(params, repeat_time);
  }

  if (repeat_time > buffer_.size() ||
      repeat_time < 100) {
    repeat_time = 0;
  }

  // increment sample counter
  if (params->counter_running) {
    counter_ += kBlockSize;
    // in the right edit modes, reset counter
    if (params->edit_mode != EDIT_NORMAL &&
        counter_ > repeat_time) {
      counter_ -= repeat_time;
    }
  }

  // set fade time
  tap_allocator_.set_fade_time(params->morph);
  tap_allocator_.Poll();

  // TODO bug: what if we turn it on while repeat_time_ < 100?
  // fade in/out the repeat buffer
  if (repeat_time // only if repeat time > 100
      && params->repeat
      && !prev_params_.repeat) {
    repeat_fader_.fade_in(params->morph);
  } else if (!params->repeat
             && prev_params_.repeat) {
    repeat_fader_.fade_out(params->morph);
  } else {
    repeat_fader_.Prepare();
  }

  /* 1. Write (dry+repeat+feedback) to buffer */

  float gain = prev_params_.gain;
  float gain_end = params->gain;
  float gain_increment = (gain_end - gain) / kBlockSize;

  float feedback = prev_params_.feedback;
  float feedback_end = params->feedback;
  float feedback_increment = (feedback_end - feedback) / kBlockSize;

  float repeat = 0.0f;
  float repeat_end = tap_allocator_.max_time() * params->scale;
  float repeat_increment = (repeat_end - repeat_time) / kBlockSize;

  for (size_t i=0; i<kBlockSize; i++) {
    float dry_sample = static_cast<float>(input[i].l) / 32768.0f;
    // addition is done here to avoid rounding errors in the increment
    float repeat_sample = buffer_.ReadHermite(repeat_time + repeat) / kBufferHeadroom;
    repeat_fader_.Process(repeat_sample);
    float fb_sample = feedback_buffer[i];
    float sample = gain * dry_sample + feedback * fb_sample + repeat_sample;
    sample = SoftLimit(sample * kBufferHeadroom);
    int16_t s = Clip16(static_cast<int32_t>(sample * 32768.0f));
    buffer_.Write(s);
    gain += gain_increment;
    feedback += feedback_increment;
    repeat += repeat_increment;
  }

  /* 2. Read and sum taps from buffer */

  FloatFrame buf[kBlockSize];
  FloatFrame empty = {0.0f, 0.0f};
  std::fill(buf, buf+kBlockSize, empty);

  // gate is high at the beginning of the loop
  bool gate = params->counter_running && counter_ < kBlockSize+1;

  for (int i=0; i<kMaxTaps; i++) {
    taps_[i].Process(&prev_params_, params, &buffer_, buf);

    if (params->counter_running
        && taps_[i].active()
        && taps_[i].time() * params->scale < counter_
        && counter_ < taps_[i].time() * params->scale + 2000) {
      gate = true;
    }
  }

  /* 3. Feed back, apply dry/wet, write to output */

  float drywet = prev_params_.drywet;
  float drywet_end = params->drywet;
  float drywet_increment = (drywet_end - drywet) / kBlockSize;

  /* convert, output and feed back */
  for (size_t i=0; i<kBlockSize; i++) {
    FloatFrame sample = { buf[i].l / kBufferHeadroom,
                          buf[i].r / kBufferHeadroom };

    // write to feedback buffer
    float fb = sample.l + sample.r;
    feedback_buffer[i] = dc_blocker_.Process<FILTER_MODE_HIGH_PASS>(fb);

    // add dry signal
    float dry = static_cast<float>(input[i].l) / 32768.0f;
    float fade_in = Interpolate(lut_xfade_in, drywet, 16.0f);
    float fade_out = Interpolate(lut_xfade_out, drywet, 16.0f);
    sample.l = dry * fade_out + sample.l * fade_in;
    sample.r = dry * fade_out + sample.r * fade_in;

    // write to output buffer
    output[i].l = SoftConvert(sample.l);
    output[i].r = SoftConvert(sample.r);
    drywet += drywet_increment;
  }

  prev_params_ = *params;

  return gate;
};
