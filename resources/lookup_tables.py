#!/usr/bin/python2.5
#
# Copyright 2016 Matthias Puech.
#
# Author: Matthias Puech <matthias.puech@gmail.com>
# Based on code by: Olivier Gillet <ol.gillet@gmail.com>
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
lookup_tables.append(('xfade_in', np.sin(t)))
lookup_tables.append(('xfade_out', np.cos(t)))

"""----------------------------------------------------------------------------
Factory presets
----------------------------------------------------------------------------"""

def normalize(l):
    return l/max(l)

VEL_AMP=0
VEL_LP=1
VEL_BP=2

maxtaps = 32

slots = []

# # Variante 0
# base_tapos = [
#     [{'time': 1., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.}],
#     [{'time': 2., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33}],
#     [{'time': 3., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66}],
#     [{'time': 4., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.}],
# ]

# # Variante 1
# base_tapos = [
#     [{'time': 2., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 7., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 9., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 12., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#     ],
#     [{'time': 4., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 8., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#     ],
#     [{'time': 1., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 3., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 6., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 11., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#     ],
#     [{'time': 5., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 10., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#     ],
# ]

# # Variante 1_1
# base_tapos = [
#     [{'time': 2., 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 7., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 9., 'velo': 0.75, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 12., 'velo': 1.0, 'typ': VEL_BP, 'pan': 1.},
#     ],
#     [{'time': 4., 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 8., 'velo': 0.75, 'typ': VEL_BP, 'pan': 1.},
#     ],
#     [{'time': 1., 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 3., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 6., 'velo': 0.75, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 11., 'velo': 1., 'typ': VEL_BP, 'pan': 1.},
#     ],
#     [{'time': 5., 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 10., 'velo': 0.75, 'typ': VEL_BP, 'pan': 1.},
#     ],
# ]

# # Variante 2
# base_tapos = [
#     [{'time': 5., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 7., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 12., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#     ],
#     [{'time': 1., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 6., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 11., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#     ],
#     [{'time': 2., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 4., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 9., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#     ],
#     [{'time': 3., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 8., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 10., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#     ],
# ]

# # Variante 3
# base_tapos = [
#     [{'time': 4., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 6., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 11., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 15., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 18., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 23., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 25., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#      {'time': 30., 'velo': 0.3, 'typ': VEL_BP, 'pan': 0.},
#     ],
#     [{'time': 2., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 7., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 10., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 14., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 19., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 24., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#      {'time': 29., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.33},
#     ],
#     [{'time': 3., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 8., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 13., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 16., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 21., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 26., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 28., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#      {'time': 31., 'velo': 0.65, 'typ': VEL_BP, 'pan': 0.66},
#     ],
#     [{'time': 1., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 5., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 9., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 12., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 17., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 20., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 22., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#      {'time': 27., 'velo': 0.8, 'typ': VEL_BP, 'pan': 1.},
#     ],
# ]


# Variante 1_3
base_tapos = [
     [{'time': 2., 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.1},
      {'time': 7., 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.},
      {'time': 9., 'velo': 0.75, 'typ': VEL_BP, 'pan': 0.2},
      {'time': 12., 'velo': 1.0, 'typ': VEL_BP, 'pan': 0.1},
     ],
     [{'time': 4., 'velo': 0.20, 'typ': VEL_BP, 'pan': 0.43},
      {'time': 8., 'velo': 0.80, 'typ': VEL_BP, 'pan': 0.23},
     ],
     [{'time': 1., 'velo': 0.20, 'typ': VEL_BP, 'pan': 0.66},
      {'time': 3., 'velo': 0.46, 'typ': VEL_BP, 'pan': 0.76},
      {'time': 6., 'velo': 0.73, 'typ': VEL_BP, 'pan': 0.56},
      {'time': 11., 'velo': 0.95, 'typ': VEL_BP, 'pan': 0.66},
     ],
     [{'time': 5., 'velo': 0.27, 'typ': VEL_BP, 'pan': 0.9},
      {'time': 10., 'velo': 0.78, 'typ': VEL_BP, 'pan': 1.},
     ],
]


# # Variante 1_4
# base_tapos = [
#      [{'time': 1.95, 'velo': 0.25, 'typ': VEL_BP, 'pan': 0.1},
#       {'time': 6.81, 'velo': 0.5, 'typ': VEL_BP, 'pan': 0.},
#       {'time': 9.20, 'velo': 0.75, 'typ': VEL_BP, 'pan': 0.2},
#       {'time': 12.09, 'velo': 1.0, 'typ': VEL_BP, 'pan': 0.1},
#      ],
#      [{'time': 3.86, 'velo': 0.20, 'typ': VEL_BP, 'pan': 0.43},
#       {'time': 7.76, 'velo': 0.80, 'typ': VEL_BP, 'pan': 0.23},
#      ],
#      [{'time': 0.81, 'velo': 0.20, 'typ': VEL_BP, 'pan': 0.66},
#       {'time': 2.99, 'velo': 0.46, 'typ': VEL_BP, 'pan': 0.76},
#       {'time': 6.03, 'velo': 0.73, 'typ': VEL_BP, 'pan': 0.56},
#       {'time': 10.84, 'velo': 0.95, 'typ': VEL_BP, 'pan': 0.66},
#      ],
#      [{'time': 5.22, 'velo': 0.27, 'typ': VEL_BP, 'pan': 0.9},
#       {'time': 9.88, 'velo': 0.78, 'typ': VEL_BP, 'pan': 1.},
#      ],
# ]


for num in range(16):
    tapo = []
    for bit in range(4):
        if num & (1 << bit) != 0:
            tapo = tapo + base_tapos[bit]
    sorted(tapo, key=lambda x: x['time'])
    slots.append({'size': len(tapo),
           'times': map(lambda x: x['time'] * sample_rate, tapo),
           'velos': map(lambda x: x['velo'], tapo),
           'types': map(lambda x: x['typ'], tapo),
           'pans': map(lambda x: x['pan'], tapo)})

# complete slots with empty tapos

while(len(slots) < 24):
    slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})


# Formatting and writing to table

def pad(a):
    return np.append(a, np.repeat([0], maxtaps - len(a)))

times, velos, pans, types, sizes = [], [], [], [], []

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
