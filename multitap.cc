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
#include "drivers/gate_input.h"
#include "drivers/gate_output.h"
#include "drivers/adc.h"
#include "drivers/codec.h"

#include <stm32f4xx_conf.h>

#include "stmlib/dsp/dsp.h"     // TODO temp
#include "resources.h"          // TODO temp

using namespace multitap;
using namespace stmlib;

System sys;
Leds leds;
Switches switches;
GateInput gate_input;
GateOutput gate_output;
Adc adc;
Codec codec;

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
    gate_input.Read();
    adc.Convert();
  }

  float phase = 0;
  float phase_increment = 1.0f / SAMPLE_RATE * 100.0f;

  void FillBuffer(Codec::Frame* input, Codec::Frame* output, size_t n) {
    while (n--){
      int16_t sin = Interpolate(lut_sin, phase, 1024.0f) * 30000.0f;
      output->l = input->l; //Out B
      output->r = static_cast<int16_t>(sin); //Dly B
      phase += phase_increment;
      if (phase > 1.0f) phase--;

      input++;
      output++;
    }
  }
}

void Init() {
  sys.Init(false);
  system_clock.Init();
  leds.Init();
  switches.Init();
  gate_input.Init();
  gate_output.Init();
  adc.Init();
  if (!codec.Init(true, 44100)) { while(1); }
  if (!codec.Start(32, &FillBuffer)) { while(1); }

  sys.StartTimers();
}

int main(void) {
  Init();

  int8_t count = 0;

  while(1) {
    leds.set(LED_PING, count < (adc.value(0) >> 8));
    leds.set(LED_REPEAT1, count < (adc.value(1) >> 8));
    leds.set(LED_REPEAT2, count < (adc.value(2) >> 8));
    leds.set(LED_CH1, count < (adc.value(3) >> 8));
    leds.set(LED_CH2, count < (adc.value(4) >> 8));

    count++;
    leds.Write();
  }
}
