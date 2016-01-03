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
// Multitap delay, main file

#include "stmlib/system/system_clock.h"
#include "drivers/system.h"
#include "drivers/leds.h"
#include "drivers/switches.h"
#include "drivers/gate_output.h"
#include "drivers/codec1.h"
#include "drivers/codec2.h"
#include "drivers/sdram.h"
#include "cv_scaler.h"

#include <stm32f4xx_conf.h>

using namespace multitap;
using namespace stmlib;

System sys;
Leds leds;
Switches switches;
GateOutput gate_output;
Codec1 codec1;
Codec2 codec2;
SDRAM sdram;
CvScaler cv_scaler;

Parameters parameters;

extern "C" {
  void NMI_Handler() { }
  void HardFault_Handler() { while (1); }
  void MemManage_Handler() { while (1); }
  void BusFault_Handler() { while (1); }
  void UsageFault_Handler() { while (1); }
  void SVC_Handler() { }
  void DebugMon_Handler() { }
  void PendSV_Handler() { }
  void assert_failed(uint8_t* file, uint32_t line) { while (1); }

  // slow timer for the UI
  void SysTick_Handler() {
    system_clock.Tick();  // increment global ms counter.
    switches.Debounce();
    leds.Write();
  }

  void FillBuffer1(ShortFrame* input, ShortFrame* output, size_t n) {
    cv_scaler.Read(&parameters);
    while (n--){
      output->l = input->l; //In A -> Out A
      output->r = input->r; //Aux A -> Dly A
      input++;
      output++;
    }
  }

  void FillBuffer2(ShortFrame* input, ShortFrame* output, size_t n) {
    cv_scaler.Read(&parameters);
    while (n--){
      output->l = input->l; //In B -> Out B
      output->r = input->r; //Aux B -> Dly B
      input++;
      output++;
    }
  }
}

void Init() {
  sys.Init(false);
  system_clock.Init();
  sdram.Init();
  leds.Init();
  switches.Init();
  gate_output.Init();
  cv_scaler.Init();
  if (!codec1.Init(true, 44100)) { while(1); }
  if (!codec1.Start(32, &FillBuffer1)) { while(1); }
  if (!codec2.Init(true, 44100)) { while(1); }
  if (!codec2.Start(32, &FillBuffer2)) { while(1); }

  sys.StartTimers();
}

int main(void) {
  Init();

  float phase = 0;

  while(1) {
    leds.set(LED_PING, phase < parameters.time[0]);
    leds.set(LED_REPEAT1, phase < parameters.level[0]);
    leds.set(LED_REPEAT2, phase < parameters.regen[0]);
    leds.set(LED_CH1, phase < parameters.mix[0]);
    leds.set(LED_CH2, true);

    phase += 1.0f / 100.0f;
    if (phase > 1.0f) phase--;

    leds.Write();
  }
}
