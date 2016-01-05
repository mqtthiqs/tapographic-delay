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

#ifndef MTD_CV_SCALER_H_
#define MTD_CV_SCALER_H_

#include "stmlib/stmlib.h"

#include "drivers/adc.h"
#include "drivers/gate_input.h"
#include "parameters.h"

namespace mtd {

struct CvTransformation {
  bool flip;
  float filter_coefficient;
};

class CvScaler {
 public:
  CvScaler() { }
  ~CvScaler() { }
  
  void Init();
  void Read(Parameters* parameters);

 private:

  Adc adc_;
  GateInput gate_input_;

  float lp_values_[ADC_CHANNEL_LAST];

  DISALLOW_COPY_AND_ASSIGN(CvScaler);
};

}  // namespace mtd

#endif  // MTD_CV_SCALER_H_
