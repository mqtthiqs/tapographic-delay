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
  { false, 0.0001f },
  // ADC_MIX2_POT,
  { false, 0.0001f },
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
  fill(&lp_values_[0], &lp_values_[ADC_CHANNEL_LAST], 0.0f);
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

  for (int8_t i=0; i<2; i++) {
    float time =
      lp_values_[ADC_TIME1_POT + i] +
      lp_values_[ADC_TIME1_CV + i];
    CONSTRAIN(time, 0.0f, 1.0f);
    parameters->delay[i].time = time;

    float level =
      lp_values_[ADC_LEVEL1_POT + i] +
      lp_values_[ADC_LEVEL1_CV + i];
    CONSTRAIN(level, 0.0f, 1.0f);
    parameters->delay[i].level = level;

    float feedback =
      lp_values_[ADC_REGEN1_POT + i] +
      lp_values_[ADC_REGEN1_CV + i];
    CONSTRAIN(feedback, 0.0f, 1.0f);
    parameters->delay[i].feedback = feedback;

    float scale =
      lp_values_[ADC_MIX1_POT + i];
    CONSTRAIN(scale, 0.0f, 1.0f);
    parameters->delay[i].scale = scale;

    parameters->delay[i].repeat = gate_input_.value(GATE_INPUT_REPEAT1 + i);
    parameters->delay[i].reverse = gate_input_.value(GATE_INPUT_REVERSE1 + i);

    // TODO: time division, repeat, reverse, tap
  }

  gate_input_.Read();
  adc_.Convert();
}

}  // namespace mtd
