// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
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
// Calibration settings.

#include "cv_scaler.hh"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"

using namespace std;

const float kPotDeadZoneSize = 0.01f;
const float kScalePotNotchSize = 0.06f;

void CvScaler::Init() {
  adc_.Init();
  gate_input_.Init();
  for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
    average_[i].Init();
  }
}

inline float CropDeadZone(float x) {
  return x * (1.0f + 2.0f * kPotDeadZoneSize) - kPotDeadZoneSize;
}


void CvScaler::Read(Parameters* parameters) {

  float scaled_values[ADC_CHANNEL_LAST];

  /* 1. Apply pot laws */
  float val;

  // gain
  val = CropDeadZone(adc_.float_value(ADC_GAIN_POT));
  val *= val;          // quadratic
  scaled_values[ADC_GAIN_POT] = val;

  // scale
  val = CropDeadZone(adc_.float_value(ADC_SCALE_POT));
  scaled_values[ADC_SCALE_POT] = val;

  // feedback
  val = CropDeadZone(adc_.float_value(ADC_FEEDBACK_POT));
  scaled_values[ADC_FEEDBACK_POT] = val;

  // modulation
  val = CropDeadZone(adc_.float_value(ADC_MODULATION_POT));
  scaled_values[ADC_MODULATION_POT] = val;

  // drywet
  val = CropDeadZone(adc_.float_value(ADC_DRYWET_POT));
  scaled_values[ADC_DRYWET_POT] = val;

  // morph
  val = CropDeadZone(adc_.float_value(ADC_MORPH_POT));
  scaled_values[ADC_MORPH_POT] = val;

  /* 2. Offset and scale CVs */

  for (int i=ADC_SCALE_CV; i<ADC_CHANNEL_LAST; i++) {
    // TODO calibrate CV
    if (i < ADC_FSR_CV) {
      // bipolar CVs
      scaled_values[i] = 0.5f - adc_.float_value(i); // -0.5..0.5
    } else {
      // unipolar CVs
      scaled_values[i] = adc_.float_value(i); // -0.5..0.5
    }

  }

  /* 3. Filter pots and CVs */

  for (size_t i=0; i<ADC_CHANNEL_LAST; i++) {
    float value = scaled_values[i];
    average_[i].Process(value);
  }

  /* 4. Add CV and pot, constrain, and write to parameters */

  // gain
  val = average_[ADC_GAIN_POT].value();
  val *= 2.0f;
  parameters->gain = val;

  // scale
  val =
    average_[ADC_SCALE_POT].value() +
    average_[ADC_SCALE_CV].value();
  // flat zone at noon
  if (val < 0.5f - kScalePotNotchSize) {
    val += kScalePotNotchSize;
  } else if (val > 0.5f + kScalePotNotchSize) {
    val -= kScalePotNotchSize;
  } else {
    val = 0.5f;
  }
  // TODO: go all the way to 0..1
  CONSTRAIN(val, 0.0f, 1.0f);
  val *= val;
  val *= 4.0f;
  // // TODO refilter scale after adding CV
  // average_scale_.Process(TODO);
  // float scale_av = average_scale_.value();
  // ONE_POLE(scale_lp_, scale_av, 0.002f);
  parameters->scale = val;

  // feedback
  val =
    average_[ADC_FEEDBACK_POT].value() +
    average_[ADC_FEEDBACK_CV].value();
  CONSTRAIN(val, 0.0f, 1.0f);
  parameters->feedback = val;

  // modulation
  val =
    average_[ADC_MODULATION_POT].value() +
    average_[ADC_MODULATION_CV].value();
  CONSTRAIN(val, 0.0f, 1.0f);
  parameters->modulation = val;

  // drywet
  val =
    average_[ADC_DRYWET_POT].value() +
    average_[ADC_DRYWET_CV].value();
  CONSTRAIN(val, 0.0f, 1.0f);
  parameters->drywet = val;

  // morph
  val =
    average_[ADC_MORPH_POT].value() +
    average_[ADC_MORPH_CV].value();
  CONSTRAIN(val, 0.0f, 1.0f);
  val = (val + 0.1f) / 1.1f;
  val = val * val * val * val;
  val *= 500000.0f;
  parameters->morph = val;

  // velocity
  val = average_[ADC_FSR_CV].value();
  CONSTRAIN(val, 0.0f, 1.0f);
  parameters->velocity = val;

  // TODO: tap trig & velocity CV

  ////////////

  // repeat
  if (gate_input_.rising_edge(GATE_INPUT_REPEAT)) {
    parameters->repeat = true;
  } else if (gate_input_.falling_edge(GATE_INPUT_REPEAT)) {
    parameters->repeat = false;
  }

  // TODO
  parameters->test = adc_.float_value(ADC_FSR_CV);

  gate_input_.Read();
  adc_.Convert();
}
