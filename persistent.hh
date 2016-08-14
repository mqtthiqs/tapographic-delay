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
// Persistent data

#ifndef PERSISTENT_H_
#define PERSISTENT_H_

#include "parameters.hh"
#include "stmlib/system/storage.h"

class Persistent 
{
public:
  Persistent() { };
  ~Persistent() { };

  struct Data {
    uint8_t settings[4];
  };

  void Init() {
    if (!storage_.ParsimoniousLoad(&data_, &version_token_)) {
      // for (size_t i = 0; i < ADC_CHANNEL_NUM_OFFSETS; ++i) {
      //   data_.calibration_data.offset[i] = 0.505f;
      // }
      data_.settings[0] = 2;      // velocity parameter (0-4)
      data_.settings[1] = 0;      // bank (0-3)
      data_.settings[2] = 1;      // panning mode (0-2)
      data_.settings[3] = 0;      // quality (0-1)
      Save();
    }

    // sanitize settings
    CONSTRAIN(data_.settings[0], 0, 4);
    CONSTRAIN(data_.settings[1], 0, 3);
    CONSTRAIN(data_.settings[2], 0, 2);
    CONSTRAIN(data_.settings[3], 0, 1);
  }

  void Save() {
    storage_.ParsimoniousSave(data_, &version_token_);
  }

  Data* mutable_data() { return &data_; }

private:
  Data data_;
  uint16_t version_token_;
  stmlib::Storage<4> storage_;
};

#endif
