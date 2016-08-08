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

import scipy.signal
import numpy
import pylab

lookup_tables = []
int16_lookup_tables = []

"""----------------------------------------------------------------------------
Sine table.
----------------------------------------------------------------------------"""

size = 1024
t = numpy.arange(0, size + size / 4 + 1) / float(size) * numpy.pi * 2
lookup_tables.append(('sin', numpy.sin(t)))

"""----------------------------------------------------------------------------
Raised cosine.
----------------------------------------------------------------------------"""

size = 1024
t = numpy.arange(0, size + 1) / float(size)
lookup_tables.append(('raised_cos', 1.0 - (numpy.cos(t * numpy.pi) + 1) / 2))

"""----------------------------------------------------------------------------
XFade table
----------------------------------------------------------------------------"""

size = 17
t = numpy.arange(0, size) / float(size-1)
t = 1.04 * t - 0.02
t[t < 0] = 0
t[t >= 1] = 1
t *= numpy.pi / 2
lookup_tables.append(('xfade_in', numpy.sin(t) * (2 ** -0.5)))
lookup_tables.append(('xfade_out', numpy.cos(t) * (2 ** -0.5)))
