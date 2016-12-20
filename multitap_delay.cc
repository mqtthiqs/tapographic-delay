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
#include "stmlib/dsp/rsqrt.h"
#include "multitap_delay.hh"

using namespace stmlib;

const int32_t kClockDefaultPeriod = 1 * SAMPLE_RATE;

void MultitapDelay::Init(short* buffer, int32_t buffer_size) {
  buffer_.Init(buffer, buffer_size);
  dc_blocker_.Init();
  dc_blocker_.set_f_q<FREQUENCY_FAST>(20.0f / SAMPLE_RATE, 0.6f);
  repeat_fader_.Init();
  clock_period_.Init(kClockDefaultPeriod);
  clock_period_smoothed_ = kClockDefaultPeriod;
  clocked_scale_ = kClockDefaultPeriod;

  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].Init();
  }
  tap_allocator_.Init(taps_);

  buffer_.Clear();
};

void MultitapDelay::set_repeat(bool state) {
  if (state) {
    repeat_fader_.fade_in(prev_params_.morph + 1.0f);
    // sample repeat time (for high quality setting) // TODO check
    float repeat_time = tap_allocator_.max_time() * prev_params_.scale;
    last_repeat_time_ = static_cast<uint32_t>(repeat_time);
  } else {
    repeat_fader_.fade_out(prev_params_.morph);
  }
}

void MultitapDelay::set_clocked(bool state) {
  clocked_ = state;
}


float MultitapDelay::ComputePanning(PanningMode panning_mode)
{
    float panning = 1.0f;
    if (panning_mode == PANNING_RANDOM) {
      panning = Random::GetFloat();
    } else if (panning_mode == PANNING_ALTERNATE) {
      panning = pan_state_ ? 1.0f : 0.0f;
      pan_state_ = !pan_state_;
    }

    return panning;
}

void MultitapDelay::ClockTick() {
  clock_period_.Process(clock_counter_);
  clock_counter_ = 0;
}

void MultitapDelay::AddTap(Parameters *params) {

  // inhibit tap add completely
  if (params->edit_mode == EDIT_OFF) {
    return;
  }

  // first tap does not count, it just starts the counter
  if (!counter_running_) {
    counter_running_ = true;
    tap_observable_.notify(TAP_DRY, 1.0f);
    return;
  }

  float time = static_cast<float>(counter_);
  float pan = ComputePanning(params->panning_mode);

  TapType type =
    params->edit_mode == EDIT_NORMAL ||
    // if it's the first tap, then notify it as "normal"
    tap_allocator_.max_time() == 0.0f ? TAP_ADDED :
    params->edit_mode == EDIT_OVERDUB ? TAP_ADDED_OVERDUB :
    TAP_FAIL;

  // add tap
  bool success = false;
  if (time < buffer_.size()) {
    success = tap_allocator_.Add(time / prev_params_.scale,
                                 params->velocity,
                                 params->velocity_type,
                                 pan);
  }

  // if we ran out of taps
  if (!success) type = TAP_ADDED_OVERWRITE;

  // UI feedback
  tap_observable_.notify(type, params->velocity);
  slot_modified_observable_.notify();
}

void MultitapDelay::RemoveLastTap() {
  if (tap_allocator_.RemoveLast()) {
    slot_modified_observable_.notify();
  }

  if(tap_allocator_.max_time() <= 0.0f) {
    counter_running_ = false;
    counter_ = 0;
  }
}

void MultitapDelay::Clear() {
  tap_allocator_.Clear();
  counter_running_ = false;
  counter_ = 0;
  slot_modified_observable_.notify();
}

void MultitapDelay::RepanTaps(PanningMode panning_mode) {
  for (int i=0; i<kMaxTaps; i++) {
    float pan = ComputePanning(panning_mode);
    taps_[i].set_panning(pan);
  }
}

// Dispatch
void MultitapDelay::Process(Parameters *params, ShortFrame* input, ShortFrame* output) {
  return params->quality ?
    (params->panning_mode == PANNING_LEFT ?
     Process<true, true>(params, input, output) :
     Process<true, false>(params, input, output)) :
    (params->panning_mode == PANNING_LEFT ?
     Process<false, true>(params, input, output) :
     Process<false, false>(params, input, output));
}

template<bool quality, bool repeat_tap_on_output>
void MultitapDelay::Process(Parameters *params, ShortFrame* input, ShortFrame* output) {

  float buffer_headroom = quality ? 1.0f : 0.5f;

  // When no taps active, turn off clocked and repeat
  if (tap_allocator_.max_time() <= 0.0f) {
    set_clocked(false);
    set_repeat(false);
  }

  // compute IR scale to fit into clock period
  if (clocked_) {
    ONE_POLE(clock_period_smoothed_, clock_period_.value(), 0.002f);
      float clocked_scale = clock_period_smoothed_
      / tap_allocator_.max_time()
      * params->clock_ratio;
    ONE_POLE(clocked_scale_, clocked_scale, 0.1f);
    params->scale = clocked_scale_; // warning: overwrite params
  }

  // repeat time, in samples
  float repeat_time = tap_allocator_.max_time() * prev_params_.scale;

  // increment sample counter
  if (counter_running_) {
    counter_ += kBlockSize;
    // in the right edit modes, reset counter
    if ((params->edit_mode != EDIT_NORMAL || clocked_) &&
        counter_ > repeat_time) {
      counter_ -= repeat_time;
    }
  }

  // set fade time
  tap_allocator_.set_fade_time(params->morph);

  /* 1. Write (dry+repeat+feedback) to buffer */

  float gain = prev_params_.gain;
  float gain_end = params->gain;
  float gain_increment = (gain_end - gain) / kBlockSize;

  float feedback_compensation = static_cast<float>(tap_allocator_.total_volumes());
  if (feedback_compensation < 1.0f) feedback_compensation = 1.0f;
  feedback_compensation = fast_rsqrt_carmack(feedback_compensation);
  ONE_POLE(feedback_compensation_, feedback_compensation, 0.05f);
  params->feedback *= feedback_compensation_; // warning: overwrite params

  float feedback = prev_params_.feedback;
  float feedback_end = params->feedback;
  float feedback_increment = (feedback_end - feedback) / kBlockSize;

  float repeat_time_accum = 0.0f;
  float repeat_time_end = tap_allocator_.max_time() * params->scale;
  float repeat_time_increment = (repeat_time_end - repeat_time) / kBlockSize;

  for (size_t i=0; i<kBlockSize; i++) {
    float fb_sample = feedback_buffer_[i];
    int16_t sample;
    if (quality) {
      int16_t repeat_sample = buffer_.ReadShort(last_repeat_time_) / buffer_headroom;
      repeat_fader_.Process(repeat_sample);
      int16_t dry_sample = input[i].l;
      int32_t fb = static_cast<int32_t>(fb_sample * 32768.0f);
      int32_t s = gain * dry_sample + feedback * fb + repeat_sample;
      sample = Clip16(s);
    } else {
      // addition is done here to avoid rounding errors in the increment
      float repeat_sample = buffer_.ReadHermite(repeat_time + repeat_time_accum) / buffer_headroom;
      repeat_fader_.Process(repeat_sample);
      float dry_sample = static_cast<float>(input[i].l) / 32768.0f;
      float dither = (Random::GetFloat() - 0.5f) / 8192.0f;
      float s = gain * dry_sample + feedback * fb_sample + repeat_sample + dither;
      s = SoftLimit(s * buffer_headroom);
      sample = Clip16(static_cast<int32_t>(s * 32768.0f));
    }

    repeat_fader_.Prepare();
    buffer_.Write(sample);
    gain += gain_increment;
    feedback += feedback_increment;
    repeat_time_accum += repeat_time_increment;
  }

  /* 2. Read and sum taps from buffer */

  FloatFrame buf[kBlockSize];
  FloatFrame empty = {0.0f, 0.0f};
  std::fill(buf, buf+kBlockSize, empty);

  uint32_t rep_time = static_cast<uint32_t>(repeat_time);

  // TODO: rep time = 0? and what if quality?
  uint32_t counter_modulo = rep_time ? counter_ % rep_time : counter_;

  bool counter_modulo_reset = counter_running_ && counter_modulo < kBlockSize+1;
  float counter_on_tap = 0.0f;
  bool counter_modulo_on_tap = counter_modulo_reset;

  for (int i=0; i<kMaxTaps; i++) {
    taps_[i].Process(&prev_params_, params, &buffer_, buf);

    float time = taps_[i].time() * params->scale;
    if (counter_running_ && taps_[i].active()) {
      if (time <= counter_modulo && counter_modulo < time + kBlockSize+1) {
        counter_modulo_on_tap = taps_[i].velocity();
      }
      if (time <= counter_ && counter_ < time + kBlockSize+1) {
        counter_on_tap = taps_[i].velocity();
      }
    }
  }

  // notify UI of tap
  if (counter_modulo_reset) {
    reset_observable_.notify();
  }

  if (counter_on_tap > 0.0f) {
    tap_observable_.notify(TAP_CROSSED, counter_on_tap);
  }

  if (counter_modulo_on_tap) {
    tap_modulo_observable_.notify();
  }

  /* 3. Feed back, apply dry/wet, write to output */

  float drywet = prev_params_.drywet;
  float drywet_end = params->drywet;
  float drywet_increment = (drywet_end - drywet) / kBlockSize;

  repeat_time_accum = 0.0f;

  /* convert, output and feed back */
  for (size_t i=0; i<kBlockSize; i++) {
    FloatFrame sample = { buf[i].l / buffer_headroom,
                          buf[i].r / buffer_headroom };

    // write to feedback buffer
    float fb = sample.l + sample.r;
    feedback_buffer_[i] = dc_blocker_.Process<FILTER_MODE_HIGH_PASS>(fb);

    // add dry signal
    float dry = static_cast<float>(input[i].l) / 32768.0f;
    float fade_in = Interpolate(lut_xfade_in, drywet, 16.0f);
    float fade_out = Interpolate(lut_xfade_out, drywet, 16.0f);
    sample.l = dry * fade_out + sample.l * fade_in;

    // write to output buffer
    // // TODO what if repeat_time changes or = 0?
    if (repeat_tap_on_output) {
      sample.r = buffer_.ReadHermite(repeat_time + repeat_time_accum + kBlockSize - i) / buffer_headroom;
    } else {
      sample.r = dry * fade_out + sample.r * fade_in;
    }

    // offset for the input
    sample.l -= 3500.0f / 32768.0f;
    sample.r -= 3500.0f / 32768.0f;

    if (quality) {
      output[i].l = Clip16(static_cast<int32_t>(sample.l * 32768.0f));
      output[i].r = Clip16(static_cast<int32_t>(sample.r * 32768.0f));
    } else {
      output[i].l = SoftConvert(sample.l);
      output[i].r = SoftConvert(sample.r);
    }

    drywet += drywet_increment;
    repeat_time_accum += repeat_time_increment;
  }

  prev_params_ = *params;
  clock_counter_ += kBlockSize;
};
