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

#ifndef CV_SCALER_H_
#define CV_SCALER_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/filter.h"
#include "average.hh"

#include "drivers/adc.hh"
#include "drivers/gate_input.hh"
#include "parameters.hh"
#include "persistent.hh"
#include "multitap_delay.hh"

using namespace stmlib;

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init(MultitapDelay* delay);
  void Read(Parameters* parameters);
  void Calibrate(Persistent* persistent);

 private:

  Adc adc_;
  GateInput gate_input_;
  MultitapDelay* delay_;

  Average<32> average_[ADC_CHANNEL_LAST];
  Average<256> average_scale_;
  Average<64> average_clock_ratio_;
  float scale_hy_, scale_lp_;
  OnePole fsr_filter_;
  bool taptrig_armed_;
  bool tapfsr_armed_;
  float freq_lp_;
  float amount_lp_;

  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

#endif
