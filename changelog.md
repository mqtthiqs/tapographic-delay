## Tapographic Delay ##
### Firmware version change log ###

#### v1.1 ####
Released Feb 28, 2018

  * Patching a clock into the Ext Clock jack will quantize all taps to the clock (when Sync mode is off). Delete/Sync button turns blue to indicate Quantize mode.
  * Feedback range increased, and polarity of taps is now randomly inverted, creating an overall more rich feedback effect.
  * Less padding on Dry signal, so turning Dry/Wet to fully Dry is closer to actual input signal level (at the expense of making it more possible to clip with Dry/Wet towards the center!)
  * Engaging Sync mode does not create a noise
  * Sync and Repeat modes are not disabled when all taps are cleared
  * Settings are saved when saving a slot
  * Repeat (on/off) and Sync mode (on/off) are saved across power-down when saving Settings
  * Velocity response of LPF taps made more dramatic
  * Minor bug fix: last tap was sometimes not recorded
  * Wider plateau set for Time knob so a ratio of 1.0 can more easily be set in Sync mode

  
  
### v1.0 ###
Released December 11, 2017

  * Initial release