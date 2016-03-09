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

#include "cv_scaler.h"

#include <algorithm>
#include <cmath>

#include "stmlib/dsp/dsp.h"

namespace mtd {

using namespace std;

const CvTransformation transformations_[ADC_CHANNEL_LAST] = {
	// ADC_TIME1_POT,
  { false, 0.01f },
  // ADC_TIME2_POT,
  { false, 0.01f },
  // ADC_LEVEL1_POT,
  { false, 0.01f },
  // ADC_LEVEL2_POT,
  { false, 0.01f },
  // ADC_REGEN1_POT,
  { false, 0.01f },
  // ADC_REGEN2_POT,
  { false, 0.01f },
  // ADC_MIX1_POT,
  { false, 0.01f },
  // ADC_MIX2_POT,
  { false, 0.002f },
  // ADC_TIME1_CV,
  { false, 0.05f },
  // ADC_TIME2_CV,
  { false, 0.05f },
  // ADC_LEVEL1_CV,
  { false, 0.05f },
  // ADC_LEVEL2_CV,
  { false, 0.05f },
  // ADC_REGEN1_CV,
  { false, 0.05f },
  // ADC_REGEN2_CV,
  { false, 0.05f },
};

void CvScaler::Init() {
  fill(&lp_values_[0], &lp_values_[ADC_CHANNEL_LAST], 0.5f);
  scale_slope_ = 1.0f;
  adc_.Init();
  gate_input_.Init();
}

void CvScaler::Read(Parameters* parameters) {
  for (size_t i = 0; i < ADC_CHANNEL_LAST; ++i) {
    const CvTransformation& transformation = transformations_[i];
    
    float value = adc_.float_value(i);
    if (transformation.flip) {
      value = 1.0f - value;
    }

    lp_values_[i] += transformation.filter_coefficient * \
        (value - lp_values_[i]);
  }

  float time =
    lp_values_[ADC_TIME1_POT] +
    lp_values_[ADC_TIME1_CV];
  CONSTRAIN(time, 0.0f, 1.0f);
  parameters->time = time;

  float velocity =
    lp_values_[ADC_LEVEL1_POT] +
    lp_values_[ADC_LEVEL1_CV];
  CONSTRAIN(velocity, 0.0f, 1.0f);
  parameters->velocity = velocity;

  float feedback =
    lp_values_[ADC_REGEN1_POT] +
    lp_values_[ADC_REGEN1_CV];
  CONSTRAIN(feedback, 0.0f, 1.0f);
  parameters->feedback = feedback;

  float drywet =
    lp_values_[ADC_MIX1_POT];
  drywet = 1.0f - drywet;
  drywet = drywet * 1.1f - 0.05f;
  CONSTRAIN(drywet, 0.0f, 1.0f);
  parameters->drywet = drywet;

  const float kNotchSize = 0.06f;
  float scale =
    lp_values_[ADC_MIX2_POT];

  if (scale < 0.5f - kNotchSize) {
    scale += kNotchSize;
  } else if (scale > 0.5f + kNotchSize) {
    scale -= kNotchSize;
  } else {
    scale = 0.5f;
  }

  CONSTRAIN(scale, 0.0f, 1.0f);

  scale = scale*2;
  scale *= scale;
  SLOPE(scale_slope_, scale, 0.0003f, 0.0003f);
  parameters->scale = scale_slope_; // 0..1..4

  float jitter_amount =
    lp_values_[ADC_LEVEL2_POT];
  CONSTRAIN(jitter_amount, 0.0f, 1.0f);
  jitter_amount *= jitter_amount;
  parameters->jitter_amount = jitter_amount;

  float jitter_frequency =
    lp_values_[ADC_REGEN2_POT];
  CONSTRAIN(jitter_frequency, 0.0f, 1.0f);
  jitter_frequency *= jitter_frequency * jitter_frequency;
  parameters->jitter_frequency = jitter_frequency;

  float morph =
    lp_values_[ADC_TIME1_POT];
  CONSTRAIN(morph, 0.0f, 1.0f);
  morph *= morph;
  parameters->morph = morph * 4000.0f;

  parameters->ping = gate_input_.rising_edge(GATE_INPUT_PING);

  gate_input_.Read();
  adc_.Convert();
}

}  // namespace mtd
