#!/usr/bin/python2.5
#
# Copyright 2014 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
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
# Lookup table definitions.

import numpy as np

import os
sample_rate = int(os.environ["SAMPLE_RATE"])

lookup_tables = []
int16_lookup_tables = []

"""----------------------------------------------------------------------------
XFade table
----------------------------------------------------------------------------"""

size = 17
t = np.arange(0, size) / float(size-1)
t = 1.04 * t - 0.02
t[t < 0] = 0
t[t >= 1] = 1
t *= np.pi / 2
lookup_tables.append(('xfade_in', np.sin(t) * (2 ** -0.5)))
lookup_tables.append(('xfade_out', np.cos(t) * (2 ** -0.5)))

"""----------------------------------------------------------------------------
Factory presets
----------------------------------------------------------------------------"""

VEL_AMP=0
VEL_LP=1
VEL_BP=2

maxtaps = 32

slots = []

# Bank 1: long bouncings
length = 7 # seconds
size = 15

t = (np.arange(float(size))+1)/float(size)

slots.append({
    'size': size,
    'times': t*t*t*sample_rate*length,
    'velos': t,
    'types': np.repeat([VEL_AMP], size),
    'pans': np.resize([0,1], size),
})

# Formatting and writing to table

def pad(a):
    return np.append(a, np.repeat([0], maxtaps - len(a)))

# TODO
times = []
velos = []
pans = []
types = []
sizes = []

for slot in slots:
    sizes.append(slot['size'])
    times.extend(pad(slot['times']))
    velos.extend(pad(slot['velos']))
    types.extend(pad(slot['types']))
    pans.extend(pad(slot['pans']))

lookup_tables.append(('preset_times', times))
lookup_tables.append(('preset_velos', velos))
lookup_tables.append(('preset_pans', pans))
int16_lookup_tables.append(('preset_types', types))
int16_lookup_tables.append(('preset_sizes', sizes))
