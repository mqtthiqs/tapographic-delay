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

void MultitapDelay::Init(short* buffer, int32_t buffer_size) {
  buffer_.Init(buffer, buffer_size);
  dc_blocker_.Init();
  dc_blocker_.set_f_q<FREQUENCY_FAST>(10.0f / SAMPLE_RATE, 0.7f);
  repeat_fader_.Init();

  for (size_t i=0; i<kMaxTaps; i++) {
    taps_[i].Init();
  }
  tap_allocator_.Init(taps_);

  buffer_.Clear();
};

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

void MultitapDelay::AddTap(Parameters *params) {
  // first tap does not count, it just starts the counter
  if (!counter_running_) {
    counter_running_ = true;
    tap_observable_.notify(TAP_DRY, 1.0f);
    return;
  }

  float time = static_cast<float>(counter_);
  float pan = ComputePanning(params->panning_mode);

  // add tap
  bool success = false;
  if (time < buffer_.size()) {
    success = tap_allocator_.Add(time / params->scale,
                                 params->velocity,
                                 params->velocity_type,
                                 pan);

    // in overwrite mode, remove oldest tap
    if (success && params->edit_mode == EDIT_OVERWRITE) {
      tap_allocator_.RemoveFirst();
    }
  }

  // for UI feedback
  TapType type =
    !success ? TAP_ADDED_OVERWRITE :
    params->edit_mode == EDIT_NORMAL ? TAP_ADDED :
    params->edit_mode == EDIT_OVERWRITE ? TAP_ADDED_OVERWRITE :
    params->edit_mode == EDIT_OVERDUB ? TAP_ADDED_OVERDUB :
    TAP_FAIL;
  tap_observable_.notify(type, params->velocity);
  slot_modified_observable_.notify();
}

void MultitapDelay::RemoveLastTap() {
  if (tap_allocator_.RemoveLast()) {
    slot_modified_observable_.notify();
  }
}

void MultitapDelay::Clear() {
  counter_running_ = false;
  tap_allocator_.Clear();
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

  // repeat time, in samples
  float repeat_time = tap_allocator_.max_time() * prev_params_.scale;

  float buffer_headroom = quality ? 1.0f : 0.5f;

  // add tap if needed
  if (params->tap) {
    AddTap(params);
  }

  // reset repeat_time if needed
  if (repeat_time > buffer_.size() ||
      repeat_time < 100) {
    repeat_time = 0;
  }

  // increment sample counter
  if (counter_running_) {
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
    last_repeat_time_ = static_cast<uint32_t>(repeat_time); // sample repeat time
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

  float feedback_compensation = static_cast<float>(tap_allocator_.busy_voices());
  if (feedback_compensation < 1.0f) feedback_compensation = 1.0f;
  feedback_compensation = fast_rsqrt_carmack(feedback_compensation);
  params->feedback *= feedback_compensation;

  float feedback = prev_params_.feedback;
  float feedback_end = params->feedback;
  float feedback_increment = (feedback_end - feedback) / kBlockSize;

  float repeat = 0.0f;
  float repeat_end = tap_allocator_.max_time() * params->scale;
  float repeat_increment = (repeat_end - repeat_time) / kBlockSize;

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
      float repeat_sample = buffer_.ReadHermite(repeat_time + repeat) / buffer_headroom;
      repeat_fader_.Process(repeat_sample);
      float dry_sample = static_cast<float>(input[i].l) / 32768.0f;
      float dither = (Random::GetFloat() - 0.5f) / 8192.0f;
      float s = gain * dry_sample + feedback * fb_sample + repeat_sample + dither;
      s = SoftLimit(s * buffer_headroom);
      sample = Clip16(static_cast<int32_t>(s * 32768.0f));
    }

    buffer_.Write(sample);
    gain += gain_increment;
    feedback += feedback_increment;
    repeat += repeat_increment;
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
      if (time < counter_modulo && counter_modulo < time + kBlockSize+1) {
        counter_modulo_on_tap = taps_[i].velocity();
      }
      if (time < counter_ && counter_ < time + kBlockSize+1) {
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

  repeat = 0.0f;

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
      sample.r = buffer_.ReadHermite(repeat_time + repeat + kBlockSize - i) / buffer_headroom;
    } else {
      sample.r = dry * fade_out + sample.r * fade_in;
    }

    if (quality) {
      output[i].l = Clip16(static_cast<int32_t>(sample.l * 32768.0f));
      output[i].r = Clip16(static_cast<int32_t>(sample.r * 32768.0f));
    } else {
      output[i].l = SoftConvert(sample.l * 2.0f);
      output[i].r = SoftConvert(sample.r * 2.0f);
    }

    drywet += drywet_increment;
    repeat += repeat_increment;
  }

  prev_params_ = *params;
};
