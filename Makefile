# Copyright 2015 Matthias Puech.
#
# Author: Matthias Puech (matthias.puech@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Makefile driver.

# System specifications
F_CRYSTAL      = 8000000L
F_CPU          = 72000000L
SYSCLOCK       = SYSCLK_FREQ_72MHz
FAMILY         = f4xx
# USB            = enabled
SAMPLE_RATE    = 32768

APPLICATION_LARGE    = TRUE

# Preferred upload command
UPLOAD_COMMAND  = upload_combo_jtag

# Packages to build
TARGET         = multitap
BOOTLOADER     = multitap_bootloader
PACKAGES       = . drivers stmlib/utils stmlib/system
RESOURCES      = resources

include stmlib/makefile.inc

upload_dld_jtag:
	openocd \
	-f $(OPENOCD_SCRIPTS_PATH)interface_$(PGM_INTERFACE).cfg \
	-f $(OPENOCD_SCRIPTS_PATH)stm32$(FAMILY)_$(PGM_INTERFACE_TYPE).cfg \
	-f $(OPENOCD_SCRIPTS_PATH)prelude_$(FAMILY).cfg \
	-c "flash write_bank 0 resources/dld.bin 0x0" \
	-c "verify_image resources/dld.bin" \
	-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg
