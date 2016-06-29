// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
// Adapted from: Olivier Gillet (ol.gillet@gmail.com)
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

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "stmlib/stmlib.h"

#include "multitap_delay.h"
#include "parameters.h"

void write_wav_header(FILE* fp, int num_samples, int num_channels) {
  uint32_t l;
  uint16_t s;
  
  fwrite("RIFF", 4, 1, fp);
  l = 36 + num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  fwrite("WAVE", 4, 1, fp);
  
  fwrite("fmt ", 4, 1, fp);
  l = 16;
  fwrite(&l, 4, 1, fp);
  s = 1;
  fwrite(&s, 2, 1, fp);
  s = num_channels;
  fwrite(&s, 2, 1, fp);
  l = SAMPLE_RATE;
  fwrite(&l, 4, 1, fp);
  l = static_cast<uint32_t>(SAMPLE_RATE) * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
  s = 2 * num_channels;
  fwrite(&s, 2, 1, fp);
  s = 16;
  fwrite(&s, 2, 1, fp);
  
  fwrite("data", 4, 1, fp);
  l = num_samples * 2 * num_channels;
  fwrite(&l, 4, 1, fp);
}

const int kBufferSize = 1 << 20;
short buffer[kBufferSize];

MultitapDelay delay;
Clock clk;

Parameters params;

void TestDSP() {
  size_t duration = 20;

  params.velocity = 1.0f;
  params.feedback = 0.99f;
  params.drywet = 0.5f;
  params.morph = 0.0f;
  params.scale = 1.0f;
  params.jitter_amount = 0.1f;
  params.jitter_frequency = 0.001f;
  params.repeat = false;
  params.ping = false;
  params.edit_mode = EDIT_NORMAL;
  params.quantize = QUANTIZE_NONE;
  params.panning = PANNING_LEFT;
  params.velocity_type = VELOCITY_AMP;

  FILE* fp_in = fopen("audio/bleep.wav", "rb");
  FILE* fp_out = fopen("mtd.wav", "wb");

  size_t remaining_samples = SAMPLE_RATE * duration;
  write_wav_header(fp_out, remaining_samples, 2);
  fseek(fp_in, 48, SEEK_SET);

  while (remaining_samples) {
    ShortFrame input[kBlockSize];
    ShortFrame output[kBlockSize];
    
    if (fread(
          input,
          sizeof(ShortFrame),
          kBlockSize,
          fp_in) != kBlockSize) {
      ShortFrame z = {0, 0};
      std::fill(input, input+kBlockSize, z);
    }
    remaining_samples -= kBlockSize;

    // std::copy(input, input+kBlockSize, output);
    delay.Process(&params, input, output);

    fwrite(output, sizeof(ShortFrame), kBlockSize, fp_out);
  }

  fclose(fp_out);
  fclose(fp_in);
}

int main(void) {
  clk.Init();
  delay.Init(buffer, kBufferSize, &clk);
  TestDSP();
}
